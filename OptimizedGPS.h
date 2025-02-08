#ifndef OPTIMIZED_GPS_H
#define OPTIMIZED_GPS_H

#include <TinyGPS++.h>

class MinimalGPS : public TinyGPSPlus {
public:
    MinimalGPS();
    
    // Optimized encode method
    bool encode(char c);
    
    // Status check methods
    bool isFullyLocked() const;
    uint32_t getSatellites() const;
    uint32_t getHDOP() const;
    
    // Time management
    void updateLastSync();
    bool needsSync() const;
    
private:
    unsigned long lastSyncTime;
    static const unsigned long SYNC_INTERVAL = 3600000; // 1 hour in milliseconds
    bool hasValidFix;
    uint32_t satelliteCount;
    uint32_t hdopValue;
};

#endif