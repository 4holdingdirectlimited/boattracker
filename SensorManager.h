#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Wire.h>
#include <Adafruit_AHT10.h>
#include <Adafruit_BMP085.h>
#include <MPU6050.h>
#include <Adafruit_HMC5883_U.h>
#include "SensorData.h"
#include <TinyGPS++.h>

class SensorManager {
public:
    SensorManager();
    bool begin();
    void update(SensorData& data);
    void processGPSData(char c);
    
    float getLatitude() const;
    float getLongitude() const;
    float getSpeed() const;
    bool isGPSValid() const;

private:
    // Sensors
    Adafruit_AHT10 aht;
    Adafruit_BMP085 bmp;
    MPU6050 mpu;
    Adafruit_HMC5883_Unified mag;
    TinyGPSPlus gps;

    // Pin definitions from original code
    static const int hallSensorPin1 = 13;
    static const int hallSensorPin2 = 14;
    static const int ledPin = 2;
    
    // Previous readings for EMA filtering
    float prevTemp = 0.0f;
    float prevHumidity = 0.0f;
    float prevPressure = 0.0f;
    float prevAltitude = 0.0f;
    
    // RPM calculation variables
    static const unsigned long DEBOUNCE_TIME = 1;    // 1ms debounce
    static const unsigned long RPM_TIMEOUT = 2000;   // 2 second timeout
    volatile unsigned long lastPulseTime1 = 0;
    volatile unsigned long lastPulseTime2 = 0;
    volatile unsigned long pulseIntervals1[10] = {0};
    volatile unsigned long pulseIntervals2[10] = {0};
    volatile uint8_t intervalIndex1 = 0;
    volatile uint8_t intervalIndex2 = 0;

    // Methods
    void initializeRPMPins();
    static void IRAM_ATTR rpm1Interrupt();
    static void IRAM_ATTR rpm2Interrupt();
    float calculateEMA(float current, float prev, float alpha = 0.2f);
    void updateEnvironmentalData(SensorData& data);
    void updateMotionData(SensorData& data);
    void updateRPMData(SensorData& data);
    void updateBatteryVoltage(SensorData& data);
    float calculateRPM(volatile unsigned long* intervals, volatile uint8_t index, unsigned long lastPulseTime);
    Vector readNormalizeAccel();
    Vector readNormalizeGyro();
};

#endif