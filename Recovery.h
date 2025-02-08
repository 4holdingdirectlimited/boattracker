#ifndef RECOVERY_H
#define RECOVERY_H

#include <Arduino.h>
#include <SD.h>
#include <WiFi.h>

extern const char* ssid;
extern const char* password;
extern const int chipSelect;

class Recovery {
public:
    static bool retrySensorInitialization();
    static bool retrySDCard();
    static bool retryWiFi();
    
private:
    static const int MAX_RETRY_ATTEMPTS = 3;
};

#endif