#ifndef OPTIMIZED_DISPLAY_H
#define OPTIMIZED_DISPLAY_H

#include <Wire.h>

class OptimizedOLED {
private:
    static constexpr uint8_t DISPLAY_ADDR = 0x3C;
    static constexpr uint8_t DISPLAY_WIDTH = 128;
    static constexpr uint8_t DISPLAY_HEIGHT = 64;
    static constexpr uint8_t DISPLAY_PAGES = 8;
    uint8_t buffer[1024];  // 128 * 64 / 8 = 1024 bytes

public:
    bool begin();
    void clearDisplay();
    void display();
    void setTextSize(uint8_t size);
    void setCursor(uint8_t x, uint8_t y);
    void print(const char* text);
    void printf(const char* format, ...);
    void drawFastNumber(uint16_t number, uint8_t x, uint8_t y, uint8_t size = 4);
};

#endif