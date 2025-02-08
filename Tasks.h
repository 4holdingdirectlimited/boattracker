#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>
#include <WiFi.h>
#include "SensorData.h"
#include "ErrorHandler.h"
#include "TaskConfig.h"
#include "GlobalConfig.h"

// External declarations
extern SensorData sensorData;
extern SemaphoreHandle_t xDataMutex;

void sensorTask(void *pvParameters);
void wifiTask(void *pvParameters);
void errorHandlingTask(void *pvParameters);
void handleWiFiClient(WiFiClient& client);

#endif