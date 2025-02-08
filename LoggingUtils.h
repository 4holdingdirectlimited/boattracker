#ifndef LOGGING_UTILS_H
#define LOGGING_UTILS_H

#include <Arduino.h>
#include <SD.h>
#include "GlobalConfig.h"

// File System Constants
constexpr size_t MAX_LOG_FILES = 10;
constexpr size_t MAX_FILE_SIZE = (1024 * 1024); // 1MB
constexpr size_t TIMESTAMP_BUFFER_SIZE = 32;
constexpr size_t DATA_BUFFER_SIZE = 512;
constexpr unsigned long FILE_ROTATION_INTERVAL = (10 * 60 * 1000);  // 10 minutes

// Function declarations
void getUTCTimestamp(char* buffer, size_t size);
void createNewFile(bool isRunningLog = false);
void manageLogFiles();
void logData();
void logError(const String& error);
void writeDataToSD(const char* data);

#endif