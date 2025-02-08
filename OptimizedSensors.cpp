#include "OptimizedSensors.h"

bool OptimizedMPU6050::begin() {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);  // PWR_MGMT_1 register
    Wire.write(0);     // Wake up MPU-6050
    return Wire.endTransmission() == 0;
}

void OptimizedMPU6050::readAccelGyro(float* ax, float* ay, float* az, float* gx, float* gy, float* gz) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);  // Starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 14, true);

    int16_t raw_ax = (Wire.read() << 8) | Wire.read();
    int16_t raw_ay = (Wire.read() << 8) | Wire.read();
    int16_t raw_az = (Wire.read() << 8) | Wire.read();
    Wire.read(); Wire.read(); // Skip temperature
    int16_t raw_gx = (Wire.read() << 8) | Wire.read();
    int16_t raw_gy = (Wire.read() << 8) | Wire.read();
    int16_t raw_gz = (Wire.read() << 8) | Wire.read();

    *ax = raw_ax / ACCEL_SCALE;
    *ay = raw_ay / ACCEL_SCALE;
    *az = raw_az / ACCEL_SCALE;
    *gx = raw_gx / GYRO_SCALE;
    *gy = raw_gy / GYRO_SCALE;
    *gz = raw_gz / GYRO_SCALE;
}

bool OptimizedAHT10::begin() {
    Wire.beginTransmission(AHT10_ADDR);
    Wire.write(AHT10_CMD_CALIBRATE);
    Wire.write(0x08);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) return false;
    delay(500);
    return true;
}

bool OptimizedAHT10::readTempHum(float* temp, float* hum) {
    Wire.beginTransmission(AHT10_ADDR);
    Wire.write(AHT10_CMD_MEASURE);
    Wire.write(0x33);
    Wire.write(0x00);
    if (Wire.endTransmission() != 0) return false;
    
    delay(75);
    Wire.requestFrom(AHT10_ADDR, 6);
    
    uint32_t data = 0;
    for (int i = 0; i < 6; i++) {
        if (Wire.available()) {
            data = (data << 8) | Wire.read();
        }
    }
    
    *hum = ((data >> 12) & 0xFFFFF) / 1048576.0 * 100;
    *temp = ((data & 0xFFF) / 4096.0) * 200 - 50;
    
    return true;
}

bool OptimizedBMP180::begin() {
    Wire.beginTransmission(BMP180_ADDR);
    Wire.write(0xAA);
    Wire.endTransmission();
    
    Wire.requestFrom(BMP180_ADDR, 22);
    ac1 = (Wire.read() << 8) | Wire.read();
    ac2 = (Wire.read() << 8) | Wire.read();
    ac3 = (Wire.read() << 8) | Wire.read();
    ac4 = (Wire.read() << 8) | Wire.read();
    ac5 = (Wire.read() << 8) | Wire.read();
    ac6 = (Wire.read() << 8) | Wire.read();
    b1 = (Wire.read() << 8) | Wire.read();
    b2 = (Wire.read() << 8) | Wire.read();
    mb = (Wire.read() << 8) | Wire.read();
    mc = (Wire.read() << 8) | Wire.read();
    md = (Wire.read() << 8) | Wire.read();
    
    return true;
}

float OptimizedBMP180::readPressure() {
    // Start temperature measurement
    Wire.beginTransmission(BMP180_ADDR);
    Wire.write(0xF4);
    Wire.write(0x2E);
    Wire.endTransmission();
    delay(5);

    // Read temperature
    Wire.beginTransmission(BMP180_ADDR);
    Wire.write(0xF6);
    Wire.endTransmission();
    Wire.requestFrom(BMP180_ADDR, 2);
    int32_t ut = (Wire.read() << 8) | Wire.read();

    // Start pressure measurement
    Wire.beginTransmission(BMP180_ADDR);
    Wire.write(0xF4);
    Wire.write(0x34 + (3 << 6));
    Wire.endTransmission();
    delay(26);

    // Read pressure
    Wire.beginTransmission(BMP180_ADDR);
    Wire.write(0xF6);
    Wire.endTransmission();
    Wire.requestFrom(BMP180_ADDR, 3);
    int32_t up = ((Wire.read() << 16) | (Wire.read() << 8) | Wire.read()) >> (8 - 3);

    // Calculate true temperature
    int32_t x1 = ((ut - ac6) * ac5) >> 15;
    int32_t x2 = (mc << 11) / (x1 + md);
    int32_t b5 = x1 + x2;

    // Calculate true pressure
    int32_t b6 = b5 - 4000;
    x1 = (b2 * ((b6 * b6) >> 12)) >> 11;
    x2 = (ac2 * b6) >> 11;
    int32_t x3 = x1 + x2;
    int32_t b3 = (((ac1 * 4 + x3) << 3) + 2) >> 2;
    x1 = (ac3 * b6) >> 13;
    x2 = (b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    uint32_t b4 = (ac4 * (uint32_t)(x3 + 32768)) >> 15;
    uint32_t b7 = ((uint32_t)up - b3) * (50000 >> 3);
    int32_t p;
    if (b7 < 0x80000000) {
        p = (b7 * 2) / b4;
    } else {
        p = (b7 / b4) * 2;
    }
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p = p + ((x1 + x2 + 3791) >> 4);
    
    return p / 100.0;
}

float OptimizedBMP180::readAltitude(float seaLevelPressure) {
    float pressure = readPressure();
    return 44330 * (1.0 - pow(pressure / seaLevelPressure, 0.1903));
}