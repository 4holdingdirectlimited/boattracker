#include "LoggingUtils.h"
#include "GlobalConfig.h"
#include <SD.h>

void writeDataToSD(const char* data) {
    if (xSemaphoreTake(xSDCardMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        File dataFile = SD.open(fileName, FILE_APPEND);
        if (dataFile) {
            dataFile.println(data);
            dataFile.close();
            
            // Check file size and rotate if necessary
            if (SD.exists(fileName)) {
                File file = SD.open(fileName);
                if (file) {
                    if (file.size() >= MAX_FILE_SIZE) {
                        file.close();
                        createNewFile();
                    } else {
                        file.close();
                    }
                }
            }
        } else {
            Serial.println("Error opening data file!");
            logError("Failed to open data file for writing");
        }
        xSemaphoreGive(xSDCardMutex);
    }
}

void getUTCTimestamp(char* buffer, size_t size) {
    snprintf(buffer, size, CURRENT_TIMESTAMP);
}

void createNewFile(bool isRunningLog) {
    if (xSemaphoreTake(xSDCardMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        // First check if SD is still working
        if (!SD.begin(chipSelect)) {
            Serial.println("SD card not responding during file creation");
            xSemaphoreGive(xSDCardMutex);
            return;
        }

        char timestamp[TIMESTAMP_BUFFER_SIZE];
        getUTCTimestamp(timestamp, sizeof(timestamp));
        
        String newFileName = String("/DATA_") + timestamp + ".csv";
        newFileName.replace(" ", "_");
        newFileName.replace(":", "-");
        
        // Try to remove if exists
        if (SD.exists(newFileName)) {
            SD.remove(newFileName);
        }
        
        // Create new file with headers
        File dataFile = SD.open(newFileName, FILE_WRITE);
        if (dataFile) {
            dataFile.println("Timestamp,User,Latitude,Longitude,Speed,Temperature,Humidity,AccelX,AccelY,AccelZ,"
                           "GyroX,GyroY,GyroZ,Pressure,Altitude,MagX,MagY,MagZ,RPM1,RPM2,"
                           "GForceX,GForceY,GForceZ,GForceTotal,BatteryVoltage");
            dataFile.close();
            fileName = newFileName;
            Serial.println("Created new log file: " + fileName);
        } else {
            Serial.println("Failed to create new log file: " + newFileName);
            logError("Failed to create new log file");
        }
        
        // Manage old files
        manageLogFiles();
        xSemaphoreGive(xSDCardMutex);
    }
}

void manageLogFiles() {
    File root = SD.open("/");
    if (!root) {
        Serial.println("Failed to open root directory");
        return;
    }
    
    struct LogFile {
        String name;
        time_t timestamp;
    };
    
    std::vector<LogFile> logFiles;
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory() && String(file.name()).startsWith("/DATA_")) {
            LogFile log;
            log.name = String(file.name());
            log.timestamp = 0; // You would parse the date string here
            logFiles.push_back(log);
        }
        file = root.openNextFile();
    }
    
    if (logFiles.size() > MAX_LOG_FILES) {
        std::sort(logFiles.begin(), logFiles.end(), 
            [](const LogFile& a, const LogFile& b) {
                return a.timestamp < b.timestamp;
            });
            
        while (logFiles.size() > MAX_LOG_FILES) {
            SD.remove(logFiles[0].name);
            logFiles.erase(logFiles.begin());
        }
    }
    
    root.close();
}

void logData() {
    if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        char timestampBuffer[TIMESTAMP_BUFFER_SIZE];
        getUTCTimestamp(timestampBuffer, sizeof(timestampBuffer));

        char dataBuffer[DATA_BUFFER_SIZE];
        snprintf(dataBuffer, DATA_BUFFER_SIZE,
            "%s,%s,%.6f,%.6f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
            timestampBuffer,
            CURRENT_USER,
            gps.location.lat(),
            gps.location.lng(),
            gps.speed.kmph(),
            sensorData.temperature,
            sensorData.humidity,
            sensorData.accelX,
            sensorData.accelY,
            sensorData.accelZ,
            sensorData.gyroX,
            sensorData.gyroY,
            sensorData.gyroZ,
            sensorData.pressure,
            sensorData.altitude,
            sensorData.magX,
            sensorData.magY,
            sensorData.magZ,
            sensorData.rpm1,
            sensorData.rpm2,
            sensorData.gForceX,
            sensorData.gForceY,
            sensorData.gForceZ,
            sensorData.gForceTotal,
            sensorData.batteryVoltage);

        writeDataToSD(dataBuffer);
        xSemaphoreGive(xDataMutex);
    }
}

void logError(const String& error) {
    if (xSemaphoreTake(xSDCardMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        char timestamp[TIMESTAMP_BUFFER_SIZE];
        getUTCTimestamp(timestamp, sizeof(timestamp));
        
        String errorFileName = "/ERROR_LOG.txt";
        File errorFile = SD.open(errorFileName, FILE_APPEND);
        if (errorFile) {
            errorFile.printf("[%s] %s\n", timestamp, error.c_str());
            errorFile.close();
        }
        xSemaphoreGive(xSDCardMutex);
    }
}