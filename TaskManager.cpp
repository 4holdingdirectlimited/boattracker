#include "TaskManager.h"
#include "GlobalConfig.h"
#include "TaskConfig.h"
#include "WebInterface.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include "esp_system.h"
#include "esp_task_wdt.h"

// Static member initialization
SensorManager* TaskManager::sensorManager = nullptr;
DisplayManager* TaskManager::displayManager = nullptr;
TaskHandle_t TaskManager::wifiTaskHandle = nullptr;
TaskHandle_t TaskManager::sensorTaskHandle = nullptr;

void TaskManager::begin(SensorManager* sMgr, DisplayManager* dMgr) {
    sensorManager = sMgr;
    displayManager = dMgr;
}

void TaskManager::createTasks() {
    // Create WiFi Task on core 0
    xTaskCreatePinnedToCore(
        wifiTask,
        "WiFi Task",
        WIFI_STACK_SIZE,
        NULL,
        WIFI_TASK_PRIORITY,
        &wifiTaskHandle,
        WIFI_TASK_CORE
    );

    // Create Sensor Task on core 1
    xTaskCreatePinnedToCore(
        sensorTask,
        "Sensor Task",
        SENSOR_STACK_SIZE,
        NULL,
        SENSOR_TASK_PRIORITY,
        &sensorTaskHandle,
        SENSOR_TASK_CORE
    );

    // Subscribe tasks to watchdog
    esp_err_t err;
    err = esp_task_wdt_add(wifiTaskHandle);
    if (err != ESP_OK) {
        Serial.println("Error adding WiFi task to watchdog");
    }

    err = esp_task_wdt_add(sensorTaskHandle);
    if (err != ESP_OK) {
        Serial.println("Error adding Sensor task to watchdog");
    }
}

void TaskManager::wifiTask(void* parameter) {
    static const uint32_t WIFI_CHECK_INTERVAL = 1000; // Check every second
    static const uint32_t POWER_SAVE_DELAY = 5000;    // Wait 5 seconds before power save
    
    unsigned long lastClientCheck = 0;
    bool wifiActive = false;
    
    while (true) {
        esp_task_wdt_reset();
        
        unsigned long currentMillis = millis();
        
        // Only process WiFi if we're not running or have a recent client
        if (!isRunning || webInterface.isClientConnected()) {
            if (!wifiActive) {
                webInterface.begin();
                wifiActive = true;
            }
            webInterface.handleClient();
            lastClientCheck = currentMillis;
        } else {
            // If no client for POWER_SAVE_DELAY, disable WiFi
            if (wifiActive && (currentMillis - lastClientCheck > POWER_SAVE_DELAY)) {
                webInterface.stop();
                wifiActive = false;
                vTaskDelay(pdMS_TO_TICKS(WIFI_CHECK_INTERVAL));
                continue;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(WIFI_TASK_DELAY));
    }
}

void TaskManager::sensorTask(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    float propSlip = 0.0f;

    while (true) {
        esp_task_wdt_reset();
        
        if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            // Update GPS data
            while (gpsSerial.available()) {
                sensorManager->processGPSData(gpsSerial.read());
            }

            // Update sensor data
            sensorManager->update(sensorData);

            // Calculate prop slip if we have valid GPS data and RPM
            if (gps.location.isValid() && sensorData.rpm1 > 0) {
                float propPitch = atof(userConfig.driveline.propPitch);
                propSlip = userConfig.calculatePropSlip(
                    gps.speed.kmph(),
                    sensorData.rpm1,
                    propPitch
                );
            }

            // Update display
            displayManager->updateDisplay(
                sensorData,
                sensorManager->getLatitude(),
                sensorManager->getLongitude(),
                sensorManager->getSpeed(),
                sensorManager->isGPSValid()
            );

            // Log data including prop slip
            if (isRunning) {
                char dataBuffer[DATA_BUFFER_SIZE];
                snprintf(dataBuffer, DATA_BUFFER_SIZE,
                    "%s,%s,%.6f,%.6f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,"
                    "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,"
                    "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
                    CURRENT_TIMESTAMP,
                    CURRENT_USER,
                    sensorManager->getLatitude(),
                    sensorManager->getLongitude(),
                    sensorManager->getSpeed(),
                    sensorData.temperature,
                    sensorData.humidity,
                    sensorData.pressure,
                    sensorData.altitude,
                    sensorData.accelX,
                    sensorData.accelY,
                    sensorData.accelZ,
                    sensorData.gyroX,
                    sensorData.gyroY,
                    sensorData.gyroZ,
                    sensorData.magX,
                    sensorData.magY,
                    sensorData.magZ,
                    sensorData.rpm1,
                    sensorData.rpm2,
                    sensorData.gForceX,
                    sensorData.gForceY,
                    sensorData.gForceZ,
                    sensorData.gForceTotal,
                    sensorData.batteryVoltage,
                    propSlip
                );
                writeDataToSD(dataBuffer);
            }

            xSemaphoreGive(xDataMutex);
        }
        
        vTaskDelayUntil(&xLastWakeTime, SENSOR_TASK_DELAY);
    }
}