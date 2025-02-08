#include "DisplayManager.h"

DisplayManager::DisplayManager() 
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
}

bool DisplayManager::begin() {
    if(!display.begin(0x3C, true)) { // Address 0x3C for 128x64
        Serial.println(F("SH110X allocation failed"));
        return false;
    }
    
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.cp437(true);
    
    showStartupScreen();
    return true;
}

void DisplayManager::showStartupScreen() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(F("RaceBoat"));
    display.println(F("DataLogger"));
    display.setTextSize(1);
    display.println(F("\nBy: 4holdingdirectlimited"));
    display.println(F("Initializing..."));
    display.display();
    delay(2000);
}

void DisplayManager::showErrorScreen(const char* error) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("ERROR:"));
    display.println(error);
    display.display();
}

void DisplayManager::updateDisplay(const SensorData& data, 
                                float latitude, float longitude, 
                                float speed, bool gpsValid) {
    display.clearDisplay();
    
    // If RPM1 is active, show large RPM display
    if (data.rpm1 > 0) {
        display.setTextSize(4);
        display.setCursor(0, 16);
        display.printf("%4d", (int)data.rpm1);
    } else {
        // Otherwise show detailed information
        display.setTextSize(1);
        display.setCursor(0, 0);
        
        // RPM Information
        display.printf("RPM1: %d\n", (int)data.rpm1);
        display.printf("RPM2: %d\n", (int)data.rpm2);
        
        // Environmental Data
        display.printf("T:%.1fC ", data.temperature);
        display.printf("H:%.0f%%\n", data.humidity);
        display.printf("P:%.0fhPa\n", data.pressure);
        
        // Battery Voltage
        display.printf("Bat:%.1fV\n", data.batteryVoltage);
        
        // GPS Data if valid
        if (gpsValid) {
            display.printf("%.1fkm/h\n", speed);
            display.printf("%.5f\n", latitude);
            display.printf("%.5f\n", longitude);
        } else {
            display.println("GPS: No Fix");
        }
        
        // G-Force
        display.printf("G:%.1f\n", data.gForceTotal);
    }
    
    display.display();
}

void DisplayManager::showConfigScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("Configuration Mode"));
    display.println(F("\nConnect to WiFi:"));
    display.println(F("SSID: DataLogger"));
    display.println(F("\nhttp://192.168.4.1"));
    display.println(F("\nPassword:"));
    display.println(F("DataLogger"));
    display.display();
}

void DisplayManager::clearDisplay() {
    display.clearDisplay();
    display.setCursor(0, 0);
}