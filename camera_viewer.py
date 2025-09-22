#!/usr/bin/env python3

# Step 1: Arduino Setup

# In main.cpp, set #define STREAM_MODE true
# Upload code to ESP32
# Connect Unit-Cam (G1→GPIO16, G3→GPIO17)


# Step 2: PC Setup

# Install Python dependencies:
# pip install pyserial pillow tkinter

# Run the viewer:
# python camera_viewer.py


# Step 3: Connect & Stream

# In the Python GUI, set your COM port (e.g., COM3)
# Click "Connect"
# You should see "Stream started" in the log
# Images will appear at ~1 FPS
# 🎯 Expected Performance:
# Frame Rate: ~1 FPS (adjustable via STREAM_INTERVAL)
# Resolution: Depends on Unit-Cam output (auto-detected)
# Latency: ~1-2 seconds total (capture + transmission)

# 🔧 Customization Options:

# For Higher FPS:
# #define STREAM_INTERVAL 500  // 2 FPS

# For Lower Data Rate:
# #define MAX_IMAGE_SIZE 4096  // Smaller buffer

# For Different COM Port:

# Change the default in Python GUI or set manually
# This solution provides a working camera stream from your Unit-Cam to your PC screen! The Python viewer will show you exactly what the camera sees, and you can adjust the frame rate and quality as needed.

"""
M5Stack Unit-Cam Stream Viewer
Receives camera stream from ESP32 via Serial and displays it on PC screen
"""

import serial
import time
import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk
import io
import threading
import queue

class CameraViewer:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("M5Stack Unit-Cam Stream Viewer")
        self.root.geometry("800x600")
        
        # Serial connection
        self.serial_port = None
        self.is_connected = False
        self.is_streaming = False
        
        # Image queue for thread communication
        self.image_queue = queue.Queue()
        
        # Setup GUI
        self.setup_gui()
        
        # Start image update thread
        self.update_thread = threading.Thread(target=self.update_images, daemon=True)
        self.update_thread.start()
    
    def setup_gui(self):
        # Control frame
        control_frame = ttk.Frame(self.root)
        control_frame.pack(pady=10)
        
        # Serial port selection
        ttk.Label(control_frame, text="Serial Port:").grid(row=0, column=0, padx=5)
        self.port_var = tk.StringVar(value="COM3")  # Default, change as needed
        port_entry = ttk.Entry(control_frame, textvariable=self.port_var, width=10)
        port_entry.grid(row=0, column=1, padx=5)
        
        # Baud rate
        ttk.Label(control_frame, text="Baud Rate:").grid(row=0, column=2, padx=5)
        self.baud_var = tk.StringVar(value="115200")
        baud_entry = ttk.Entry(control_frame, textvariable=self.baud_var, width=10)
        baud_entry.grid(row=0, column=3, padx=5)
        
        # Connect button
        self.connect_btn = ttk.Button(control_frame, text="Connect", command=self.toggle_connection)
        self.connect_btn.grid(row=0, column=4, padx=10)
        
        # Status label
        self.status_label = ttk.Label(control_frame, text="Disconnected", foreground="red")
        self.status_label.grid(row=0, column=5, padx=10)
        
        # FPS counter
        self.fps_label = ttk.Label(control_frame, text="FPS: 0.0")
        self.fps_label.grid(row=0, column=6, padx=10)
        
        # Image display frame
        image_frame = ttk.Frame(self.root)
        image_frame.pack(expand=True, fill='both', padx=10, pady=10)
        
        # Canvas for image display
        self.canvas = tk.Canvas(image_frame, bg='black')
        self.canvas.pack(expand=True, fill='both')
        
        # Log frame
        log_frame = ttk.Frame(self.root)
        log_frame.pack(fill='x', padx=10, pady=(0, 10))
        
        ttk.Label(log_frame, text="Log:").pack(anchor='w')
        
        # Log text with scrollbar
        log_container = ttk.Frame(log_frame)
        log_container.pack(fill='x')
        
        self.log_text = tk.Text(log_container, height=8, wrap='word')
        scrollbar = ttk.Scrollbar(log_container, orient='vertical', command=self.log_text.yview)
        self.log_text.configure(yscrollcommand=scrollbar.set)
        
        self.log_text.pack(side='left', fill='both', expand=True)
        scrollbar.pack(side='right', fill='y')
    
    def log_message(self, message):
        """Add message to log with timestamp"""
        timestamp = time.strftime("%H:%M:%S")
        self.log_text.insert('end', f"[{timestamp}] {message}\n")
        self.log_text.see('end')
        print(f"[{timestamp}] {message}")  # Also print to console
    
    def toggle_connection(self):
        """Toggle serial connection"""
        if not self.is_connected:
            self.connect_serial()
        else:
            self.disconnect_serial()
    
    def connect_serial(self):
        """Connect to ESP32 via serial"""
        try:
            port = self.port_var.get()
            baud = int(self.baud_var.get())
            
            self.serial_port = serial.Serial(port, baud, timeout=1)
            self.is_connected = True
            
            self.status_label.config(text="Connected", foreground="green")
            self.connect_btn.config(text="Disconnect")
            self.log_message(f"Connected to {port} at {baud} baud")
            
            # Start streaming thread
            self.streaming_thread = threading.Thread(target=self.stream_receiver, daemon=True)
            self.streaming_thread.start()
            
        except Exception as e:
            self.log_message(f"Connection failed: {str(e)}")
    
    def disconnect_serial(self):
        """Disconnect from ESP32"""
        self.is_connected = False
        self.is_streaming = False
        
        if self.serial_port:
            self.serial_port.close()
            self.serial_port = None
        
        self.status_label.config(text="Disconnected", foreground="red")
        self.connect_btn.config(text="Connect")
        self.log_message("Disconnected")
    
    def stream_receiver(self):
        """Receive and process camera stream from ESP32"""
        frame_buffer = bytearray()
        frame_state = "waiting_header"
        frame_size = 0
        bytes_received = 0
        cmd_byte = 0
        frame_count = 0
        fps_start_time = time.time()
        crc_xor = 0x00
        
        # Frame protocol constants
        PACK_FIRST_BYTE = 0xAA
        PACK_SECOND_BYTE = 0x55
        
        while self.is_connected:
            try:
                if self.serial_port.in_waiting > 0:
                    data = self.serial_port.read(self.serial_port.in_waiting)
                    
                    for byte_val in data:
                        if frame_state == "waiting_header":
                            if byte_val == PACK_FIRST_BYTE:
                                frame_state = "waiting_second_byte"
                                frame_buffer = bytearray()
                                frame_buffer.append(byte_val)
                                
                        elif frame_state == "waiting_second_byte":
                            frame_buffer.append(byte_val)
                            if byte_val == PACK_SECOND_BYTE:
                                frame_state = "waiting_length"
                                bytes_received = 0
                                frame_size = 0
                            else:
                                frame_state = "waiting_header"
                                
                        elif frame_state == "waiting_length":
                            frame_buffer.append(byte_val)
                            if bytes_received == 0:
                                frame_size = (byte_val << 24)
                            elif bytes_received == 1:
                                frame_size |= (byte_val << 16)
                            elif bytes_received == 2:
                                frame_size |= (byte_val << 8)
                            elif bytes_received == 3:
                                frame_size |= byte_val
                            
                            bytes_received += 1
                            if bytes_received >= 4:
                                frame_state = "waiting_length_crc"
                                
                        elif frame_state == "waiting_length_crc":
                            frame_buffer.append(byte_val)
                            # Calculate CRC for length bytes
                            crc_xor = 0x00 ^ frame_buffer[2] ^ frame_buffer[3] ^ frame_buffer[4] ^ frame_buffer[5]
                            if crc_xor == byte_val:
                                if frame_size > 50000 or frame_size < 2:  # Sanity check
                                    frame_state = "waiting_header"
                                    self.log_message(f"Invalid frame size: {frame_size}")
                                else:
                                    frame_state = "waiting_cmd"
                            else:
                                frame_state = "waiting_header"
                                self.log_message(f"Length CRC mismatch: expected {crc_xor}, got {byte_val}")
                                
                        elif frame_state == "waiting_cmd":
                            frame_buffer.append(byte_val)
                            cmd_byte = byte_val
                            frame_state = "waiting_data"
                            bytes_received = 0
                            
                        elif frame_state == "waiting_data":
                            frame_buffer.append(byte_val)
                            bytes_received += 1
                            if bytes_received >= (frame_size - 2):  # -2 for cmd and final CRC
                                frame_state = "waiting_final_crc"
                                
                        elif frame_state == "waiting_final_crc":
                            frame_buffer.append(byte_val)
                            # Calculate final CRC for entire frame
                            crc_xor = 0x00
                            for i in range(len(frame_buffer) - 1):
                                crc_xor ^= frame_buffer[i]
                            
                            if crc_xor == byte_val:
                                # Complete frame received - extract JPEG data
                                jpeg_data = frame_buffer[7:-1]  # Skip header, length, length_crc, cmd, and final_crc
                                self.process_jpeg_frame(bytes(jpeg_data))
                                frame_count += 1
                                
                                # Calculate FPS every 5 frames
                                if frame_count % 5 == 0:
                                    elapsed = time.time() - fps_start_time
                                    fps = 5 / elapsed if elapsed > 0 else 0
                                    self.root.after(0, lambda: self.fps_label.config(text=f"FPS: {fps:.1f}"))
                                    fps_start_time = time.time()
                            else:
                                self.log_message(f"Final CRC mismatch: expected {crc_xor}, got {byte_val}")
                            
                            frame_state = "waiting_header"
                
                time.sleep(0.001)  # Small delay to prevent 100% CPU usage
                
            except Exception as e:
                self.log_message(f"Stream error: {str(e)}")
                break
    
    def process_jpeg_frame(self, frame_data):
        """Process received JPEG frame data and queue for display"""
        try:
            if len(frame_data) > 100:  # Only process if we have substantial data
                # Try to decode as JPEG
                try:
                    img = Image.open(io.BytesIO(frame_data))
                    # Successfully decoded JPEG
                    self.image_queue.put(img)
                    self.log_message(f"Received JPEG frame: {len(frame_data)} bytes, {img.size}")
                except Exception as e:
                    # If JPEG decode fails, log the error but continue
                    self.log_message(f"JPEG decode failed ({len(frame_data)} bytes): {str(e)}")
                    
                    # Try to display as raw data pattern for debugging
                    self.image_queue.put(frame_data)
            
        except Exception as e:
            self.log_message(f"Frame processing error: {str(e)}")
    
    def process_frame(self, frame_data):
        """Legacy method - redirects to JPEG processing"""
        self.process_jpeg_frame(frame_data)
    
    def update_images(self):
        """Update image display from queue (runs in main thread)"""
        while True:
            try:
                # Check for new frames
                frame_data = self.image_queue.get(timeout=0.1)
                
                if isinstance(frame_data, Image.Image):
                    # It's already a PIL Image (JPEG decoded successfully)
                    display_img = frame_data
                    
                    # Resize for display if needed
                    if display_img.size[0] > 640 or display_img.size[1] > 480:
                        display_img.thumbnail((640, 480), Image.Resampling.LANCZOS)
                    
                    # Convert to PhotoImage and display
                    self.root.after(0, self.display_image, display_img)
                    
                else:
                    # It's raw data - create a visualization pattern
                    width = min(320, int(len(frame_data) ** 0.5))
                    height = len(frame_data) // width if width > 0 else 1
                    
                    if width > 0 and height > 0:
                        # Create grayscale image from raw data
                        img_data = list(frame_data[:width * height])
                        if len(img_data) < width * height:
                            img_data.extend([0] * (width * height - len(img_data)))
                        
                        img = Image.new('L', (width, height))
                        img.putdata(img_data)
                        
                        # Resize for display
                        display_img = img.resize((320, 240), Image.Resampling.NEAREST)
                        
                        # Convert to PhotoImage and display
                        self.root.after(0, self.display_image, display_img)
                
            except queue.Empty:
                continue
            except Exception as e:
                self.log_message(f"Image update error: {str(e)}")
    
    def display_image(self, img):
        """Display image on canvas"""
        try:
            photo = ImageTk.PhotoImage(img)
            
            # Clear canvas and display image
            self.canvas.delete("all")
            canvas_width = self.canvas.winfo_width()
            canvas_height = self.canvas.winfo_height()
            
            if canvas_width > 1 and canvas_height > 1:
                x = (canvas_width - photo.width()) // 2
                y = (canvas_height - photo.height()) // 2
                self.canvas.create_image(x, y, anchor='nw', image=photo)
                
                # Keep a reference to prevent garbage collection
                self.canvas.image = photo
        
        except Exception as e:
            self.log_message(f"Display error: {str(e)}")
    
    def run(self):
        """Start the GUI application"""
        self.log_message("Camera Viewer started")
        self.log_message("1. Set your COM port (e.g., COM3, COM4)")
        self.log_message("2. Click Connect")
        self.log_message("3. Upload streaming code to ESP32")
        self.root.mainloop()

if __name__ == "__main__":
    app = CameraViewer()
    app.run()
