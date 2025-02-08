#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include <Arduino.h>
#include "OptimizedGPS.h"
#include "SensorData.h"

// Current timestamp and user
extern const char* CURRENT_USER;
extern const char* CURRENT_TIMESTAMP;

// Global variables
extern bool isRunning;
extern String fileName;

// Mutexes
extern SemaphoreHandle_t xDataMutex;
extern SemaphoreHandle_t xSDCardMutex;

// WiFi settings
extern const char* ssid;
extern const char* password;

// Global objects
extern MinimalGPS gps;
extern HardwareSerial gpsSerial;
extern SensorData sensorData;

#endif