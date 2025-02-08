#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <time.h>

#include "GlobalConfig.h"
#include "HardwareConfig.h"
#include "TaskConfig.h"
#include "SensorManager.h"
#include "DisplayManager.h"
#include "TaskManager.h"
#include "LoggingUtils.h"

// Global Objects
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SensorManager sensorManager;
DisplayManager displayManager(display);

void setup() {
    Serial.begin(115200);
    Serial.printf("System starting up. Current user: %s\n", CURRENT_USER);
    Serial.printf("Current timestamp: %s\n", CURRENT_TIMESTAMP);

    // Initialize mutexes
    xDataMutex = xSemaphoreCreateMutex();
    xSDCardMutex = xSemaphoreCreateMutex();

    if (!xDataMutex || !xSDCardMutex) {
        Serial.println("Failed to create mutexes!");
        while(1) { delay(1000); }
    }

    // Initialize Watchdog
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = WDT_TIMEOUT,
        .idle_core_mask = (1 << 0),
        .trigger_panic = true
    };
    
    esp_task_wdt_deinit();
    if (esp_task_wdt_init(&wdt_config) != ESP_OK) {
        Serial.println("Failed to initialize watchdog");
    }

    // Initialize GPS
    gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    Serial.println("GPS initialized");

    // Initialize I2C
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(400000); // Set to 400kHz
    Serial.println("I2C initialized");

    // Initialize Display
    if (!displayManager.begin()) {
        Serial.println("Failed to initialize display");
    }
    displayManager.showInitMessage(CURRENT_USER, CURRENT_TIMESTAMP);
    Serial.println("Display initialized");

    // Initialize Sensors
    if (!sensorManager.begin()) {
        Serial.println("Failed to initialize one or more sensors");
        displayManager.showError("Sensor Init Failed");
    } else {
        Serial.println("Sensors initialized");
    }

    // Initialize SD Card with robust error handling
    if (xSemaphoreTake(xSDCardMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        Serial.println("Initializing SD card...");
        
        // End any existing SPI and reinitialize
        SPI.end();
        SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, chipSelect);
        
        // Try SD initialization multiple times
        int retryCount = 0;
        bool sdInitSuccess = false;
        
        while (!sdInitSuccess && retryCount < 3) {
            if (SD.begin(chipSelect)) {
                sdInitSuccess = true;
            } else {
                Serial.println("SD Card initialization failed! Retrying...");
                delay(1000);
                retryCount++;
            }
        }
        
        if (sdInitSuccess) {
            Serial.println("SD card initialized successfully!");
            uint64_t cardSize = SD.cardSize() / (1024 * 1024);
            Serial.printf("SD Card Size: %lluMB\n", cardSize);
            
            // Test write access
            File testFile = SD.open("/test.txt", FILE_WRITE);
            if (testFile) {
                testFile.println("Test write");
                testFile.close();
                SD.remove("/test.txt");
                Serial.println("SD card write test successful");
            } else {
                Serial.println("Failed to open test file!");
                displayManager.showError("SD Write Failed");
            }
        } else {
            Serial.println("SD Card initialization failed after 3 attempts!");
            displayManager.showError("SD Card Failed");
        }
        xSemaphoreGive(xSDCardMutex);
    }

    // Initialize WiFi
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password, 6, 0, 4);  // Channel 6, Hidden=false, Max_connections=4
    Serial.println("WiFi initialized");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    displayManager.showWiFiInfo(ssid, password, WiFi.softAPIP().toString().c_str());

    // Create initial log file
    createNewFile();

    // Initialize Task Manager
    TaskManager::begin(&sensorManager, &displayManager);
    TaskManager::createTasks();

    // Final setup message
    Serial.println("Setup complete!");
    
    // Display setup completion
    displayManager.showSetupComplete();
}

void loop() {
    // The main loop is empty because all work is done in the tasks
    // Just delay to prevent watchdog issues
    vTaskDelay(pdMS_TO_TICKS(1000));
}