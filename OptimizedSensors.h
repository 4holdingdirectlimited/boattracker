#ifndef OPTIMIZED_SENSORS_H
#define OPTIMIZED_SENSORS_H

#include <Wire.h>

class OptimizedMPU6050 {
private:
    static constexpr uint8_t MPU_ADDR = 0x68;
    static constexpr float ACCEL_SCALE = 16384.0;
    static constexpr float GYRO_SCALE = 131.0;

public:
    bool begin();
    void readAccelGyro(float* ax, float* ay, float* az, float* gx, float* gy, float* gz);
};

class OptimizedAHT10 {
private:
    static constexpr uint8_t AHT10_ADDR = 0x38;
    static constexpr uint8_t AHT10_CMD_CALIBRATE = 0xE1;
    static constexpr uint8_t AHT10_CMD_MEASURE = 0xAC;

public:
    bool begin();
    bool readTempHum(float* temp, float* hum);
};

class OptimizedBMP180 {
private:
    static constexpr uint8_t BMP180_ADDR = 0x77;
    int16_t ac1, ac2, ac3;
    uint16_t ac4, ac5, ac6;
    int16_t b1, b2;
    int16_t mb, mc, md;

public:
    bool begin();
    float readPressure();
    float readAltitude(float seaLevelPressure = 1013.25);
};

#endif