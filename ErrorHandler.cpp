#include "ErrorHandler.h"
#include "GlobalConfig.h"
#include "HardwareConfig.h"  // For chipSelect
#include <WiFi.h>

// External declarations for WiFi credentials
extern const char* ssid;
extern const char* password;

ErrorHandler::ErrorHandler() : queueIndex(0) {
    errorMutex = xSemaphoreCreateMutex();
}

void ErrorHandler::handleError(ErrorComponent component, ErrorSeverity severity, const String& message, bool canRetry) {
    if (xSemaphoreTake(errorMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        if (queueIndex < ERROR_QUEUE_SIZE) {
            errorQueue[queueIndex].component = component;
            errorQueue[queueIndex].severity = severity;
            errorQueue[queueIndex].message = message;
            errorQueue[queueIndex].timestamp = millis();
            errorQueue[queueIndex].needsRetry = canRetry;
            errorQueue[queueIndex].retryCount = 0;
            queueIndex++;

            // Log to Serial immediately
            Serial.printf("[%s] %s: %s\n",
                severity == ErrorSeverity::CRITICAL ? "CRITICAL" :
                severity == ErrorSeverity::ERROR ? "ERROR" :
                severity == ErrorSeverity::WARNING ? "WARNING" : "INFO",
                component == ErrorComponent::SENSORS ? "SENSORS" :
                component == ErrorComponent::SD_CARD ? "SD_CARD" :
                component == ErrorComponent::WIFI ? "WIFI" :
                component == ErrorComponent::GPS ? "GPS" :
                component == ErrorComponent::MEMORY ? "MEMORY" : "SYSTEM",
                message.c_str());

            // Display critical errors
            if (severity == ErrorSeverity::CRITICAL) {
                displayError(errorQueue[queueIndex - 1]);
            }

            // Log to SD card if available
            writeErrorToSD(errorQueue[queueIndex - 1]);
        }
        xSemaphoreGive(errorMutex);
    }
}

void ErrorHandler::writeErrorToSD(const ErrorEvent& error) {
    if (xSemaphoreTake(xSDCardMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        File errorLog = SD.open("/error_log.txt", FILE_APPEND);
        if (errorLog) {
            char timestamp[32];
            getUTCTimestamp(timestamp, sizeof(timestamp));
            
            errorLog.printf("%s [%s] %s: %s\n",
                timestamp,
                CURRENT_USER,
                error.severity == ErrorSeverity::CRITICAL ? "CRITICAL" :
                error.severity == ErrorSeverity::ERROR ? "ERROR" :
                error.severity == ErrorSeverity::WARNING ? "WARNING" : "INFO",
                error.message.c_str());
            errorLog.close();
        }
        xSemaphoreGive(xSDCardMutex);
    }
}

void ErrorHandler::displayError(const ErrorEvent& error) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("ERROR:");
    display.println(error.message);
    display.display();
}

void ErrorHandler::processErrorQueue() {
    if (xSemaphoreTake(errorMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        for (size_t i = 0; i < queueIndex; i++) {
            if (errorQueue[i].needsRetry && errorQueue[i].retryCount < MAX_RETRY_COUNT) {
                bool resolved = false;
                switch (errorQueue[i].component) {
                    case ErrorComponent::SENSORS:
                        resolved = retrySensorInitialization();
                        break;
                    case ErrorComponent::SD_CARD:
                        resolved = retrySDCard();
                        break;
                    case ErrorComponent::WIFI:
                        resolved = retryWiFi();
                        break;
                    default:
                        break;
                }
                
                if (!resolved) {
                    errorQueue[i].retryCount++;
                    if (errorQueue[i].retryCount >= MAX_RETRY_COUNT) {
                        errorQueue[i].severity = ErrorSeverity::CRITICAL;
                        writeErrorToSD(errorQueue[i]);
                    }
                } else {
                    removeErrorFromQueue(i);
                    i--;
                }
            }
        }
        xSemaphoreGive(errorMutex);
    }
}

void ErrorHandler::removeErrorFromQueue(size_t index) {
    if (index < queueIndex) {
        for (size_t i = index; i < queueIndex - 1; i++) {
            errorQueue[i] = errorQueue[i + 1];
        }
        queueIndex--;
    }
}

bool ErrorHandler::retrySensorInitialization() {
    // Implementation for sensor retry
    return false;
}

bool ErrorHandler::retrySDCard() {
    // Use chipSelect from HardwareConfig.h
    if (!SD.begin(chipSelect)) {
        return false;
    }
    return true;
}

bool ErrorHandler::retryWiFi() {
    // Use WiFi credentials from external declarations
    return WiFi.softAP(ssid, password, 6, 0, 4);
}

bool ErrorHandler::hasErrors() const {
    return queueIndex > 0;
}

void ErrorHandler::clearErrors() {
    if (xSemaphoreTake(errorMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        queueIndex = 0;
        xSemaphoreGive(errorMutex);
    }
}