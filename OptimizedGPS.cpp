#include "OptimizedGPS.h"

MinimalGPS::MinimalGPS() : 
    TinyGPSPlus(),
    lastSyncTime(0),
    hasValidFix(false),
    satelliteCount(0),
    hdopValue(0) {
}

bool MinimalGPS::encode(char c) {
    bool result = TinyGPSPlus::encode(c);
    
    if (result) {
        // Update status flags only when we have new data
        if (location.isUpdated()) {
            hasValidFix = location.isValid();
            satelliteCount = satellites.value();
            hdopValue = hdop.value();
        }
    }
    
    return result;
}

bool MinimalGPS::isFullyLocked() const {
    return hasValidFix && 
           satelliteCount >= 4 && 
           hdopValue < 200; // HDOP < 2.0
}

uint32_t MinimalGPS::getSatellites() const {
    return satelliteCount;
}

uint32_t MinimalGPS::getHDOP() const {
    return hdopValue;
}

void MinimalGPS::updateLastSync() {
    lastSyncTime = millis();
}

bool MinimalGPS::needsSync() const {
    return (millis() - lastSyncTime) >= SYNC_INTERVAL;
}