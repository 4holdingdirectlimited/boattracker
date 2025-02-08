#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <Arduino.h>
#include <SD.h>
#include <Adafruit_SH110X.h>
#include "LoggingUtils.h"

// Forward declarations for external variables
extern SemaphoreHandle_t xSDCardMutex;
extern const char* CURRENT_USER;
extern Adafruit_SH1106G display;
extern void getUTCTimestamp(char* buffer, size_t size);

enum class ErrorSeverity {
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

enum class ErrorComponent {
    SENSORS,
    SD_CARD,
    WIFI,
    GPS,
    MEMORY,
    SYSTEM
};

class ErrorHandler {
public:
    ErrorHandler();
    void handleError(ErrorComponent component, ErrorSeverity severity, const String& message, bool canRetry = false);
    void processErrorQueue();
    bool hasErrors() const;
    void clearErrors();

private:
    static const uint8_t MAX_RETRY_COUNT = 3;
    static const size_t ERROR_QUEUE_SIZE = 10;
    
    struct ErrorEvent {
        ErrorSeverity severity;
        ErrorComponent component;
        String message;
        unsigned long timestamp;
        bool needsRetry;
        uint8_t retryCount;
    };

    ErrorEvent errorQueue[ERROR_QUEUE_SIZE];
    size_t queueIndex;
    SemaphoreHandle_t errorMutex;
    
    void writeErrorToSD(const ErrorEvent& error);
    void displayError(const ErrorEvent& error);
    bool shouldRetry(const ErrorEvent& error);
    void removeErrorFromQueue(size_t index);
    
    // Add retry function declarations
    bool retrySensorInitialization();
    bool retrySDCard();
    bool retryWiFi();
};

#endif