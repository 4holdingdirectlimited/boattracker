#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

struct SensorData {
    // Environmental sensors
    float temperature = 0.0f;
    float humidity = 0.0f;
    float pressure = 0.0f;
    float altitude = 0.0f;
    
    // Motion sensors
    float accelX = 0.0f;
    float accelY = 0.0f;
    float accelZ = 0.0f;
    float gyroX = 0.0f;
    float gyroY = 0.0f;
    float gyroZ = 0.0f;
    float magX = 0.0f;
    float magY = 0.0f;
    float magZ = 0.0f;
    
    // RPM sensors
    float rpm1 = 0.0f;
    float rpm2 = 0.0f;
    
    // G-Force calculations
    float gForceX = 0.0f;
    float gForceY = 0.0f;
    float gForceZ = 0.0f;
    float gForceTotal = 0.0f;
    
    // Power monitoring
    float batteryVoltage = 0.0f;
};

#endif