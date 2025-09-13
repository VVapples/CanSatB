#ifndef pos_est_H
#define pos_est_H

#include <Arduino.h>
#include "gps.h"
#include "9axis.h"
#include <SD.h>

struct PositionData {
    float latitude;
    float longitude;
    float heading;
};

PositionData getPositionData();
PositionData getTargetPosition();
float estimateTargetDirection(const PositionData& currentPos, const PositionData& targetPos);
float calculateDistanceToTarget(const PositionData& currentPos, const PositionData& targetPos);

#endif // pos_est_H