#ifndef SENSOR_UTILS_H
#define SENSOR_UTILS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHT10.h>
#include <Adafruit_BMP085.h>
#include <MPU6050.h>
#include <Adafruit_HMC5883_U.h>
#include "SensorData.h"
#include "ErrorHandler.h"

// External declarations for sensor objects
extern Adafruit_AHT10 aht;
extern Adafruit_BMP085 bmp;
extern MPU6050 mpu;
extern Adafruit_HMC5883_Unified mag;
extern SensorData sensorData;
extern ErrorHandler errorHandler;

// Function declarations
void initializeSensors();
void updateSensorData();
float calculateGForce(float ax, float ay, float az);
void calculateRPM();

#endif