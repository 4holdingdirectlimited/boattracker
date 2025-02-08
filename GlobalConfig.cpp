#include "GlobalConfig.h"

// Current user and timestamp
const char* CURRENT_USER = "4holdingdirectlimited";
const char* CURRENT_TIMESTAMP = "2025-02-08 03:49:45";  // Updated timestamp

// WiFi settings
const char* ssid = "DataLogger";
const char* password = "DataLogger";

// Global variables initialization
bool isRunning = false;
String fileName;

// Mutex initialization
SemaphoreHandle_t xDataMutex = NULL;
SemaphoreHandle_t xSDCardMutex = NULL;

// Global objects initialization
MinimalGPS gps;
HardwareSerial gpsSerial(1);
SensorData sensorData;