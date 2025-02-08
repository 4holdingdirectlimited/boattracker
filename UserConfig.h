#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include <Arduino.h>

struct BoatConfig {
    char name[21];
    char length[21];
    char weight[21];
    char type[21];
    char number[21];
    char notes[101];
};

struct DriverConfig {
    char name[21];
    char height[21];
    char weight[21];
    char notes[101];
};

struct EngineConfig {
    char brand[21];
    char horsepower[21];
    char type[10];        // inboard/outboard
    char timing[21];
    char ignitionType[21];
    char fuelPressure[21];
    char carbJetting[21];
    char rpmLimit[21];
    char fuelType[21];
    char fuelSystem[21];  // carb/injection
    char strokeNumber[21];
    char cylinderNumber[21];
    char coolingSystem[21];
    char notes[101];
};

struct DrivelineConfig {
    char gearRatio[21];
    char gearboxStyle[21];
    char propDiameter[21];
    char propPitch[21];
    char propRake[21];
    char propStyle[21];
    char propNumber[21];
    char notes[101];
};

struct WaterConfig {
    char type[10];       // fresh/salt
    char temperature[21];
    char notes[101];
};

struct LoggerConfig {
    char rpm1MagnetsPerRev[21];
    char rpm2MagnetsPerRev[21];
    char rpm1CableLength[21];
    char rpm2CableLength[21];
    char notes[101];
};

class UserConfiguration {
public:
    BoatConfig boat;
    DriverConfig driver;
    EngineConfig engine;
    DrivelineConfig driveline;
    WaterConfig water;
    LoggerConfig logger;
    
    bool loadFromSD();
    bool saveToSD();
    void logCurrentConfig();
    float calculatePropSlip(float gpsSpeed, float rpm, float propPitch);
    
private:
    static const char* CONFIG_FILENAME;
};

extern UserConfiguration userConfig;

#endif