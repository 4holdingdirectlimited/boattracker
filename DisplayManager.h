#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "SensorData.h"

class DisplayManager {
public:
    DisplayManager();
    bool begin();
    void updateDisplay(const SensorData& data, float latitude, float longitude, float speed, bool gpsValid);
    void showStartupScreen();
    void showErrorScreen(const char* error);
    void showConfigScreen();

private:
    Adafruit_SH1106G display;
    static const uint8_t SCREEN_WIDTH = 128;
    static const uint8_t SCREEN_HEIGHT = 64;
    static const int8_t OLED_RESET = -1;
    
    void drawHeader();
    void drawGPSData(float latitude, float longitude, float speed, bool gpsValid);
    void drawSensorData(const SensorData& data);
    void clearDisplay();
};

#endif