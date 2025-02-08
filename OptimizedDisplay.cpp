#include "OptimizedDisplay.h"
#include <stdarg.h>
#include <stdio.h>

// 5x7 font for numbers only (optimized)
const uint8_t PROGMEM numberFont[] = {
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E  // 9
};

bool OptimizedOLED::begin() {
    Wire.beginTransmission(DISPLAY_ADDR);
    Wire.write(0x00);  // Command stream
    Wire.write(0xAE);  // Display off
    Wire.write(0xD5);  // Set display clock
    Wire.write(0x80);  // Recommended value
    Wire.write(0xA8);  // Set multiplex
    Wire.write(0x3F);  // 64 rows
    Wire.write(0xD3);  // Set display offset
    Wire.write(0x00);  // No offset
    Wire.write(0x40);  // Start line
    Wire.write(0x8D);  // Charge pump
    Wire.write(0x14);  // Enable
    Wire.write(0x20);  // Memory mode
    Wire.write(0x00);  // Horizontal
    Wire.write(0xA1);  // Seg remap
    Wire.write(0xC8);  // Com scan dec
    Wire.write(0xDA);  // Com pins
    Wire.write(0x12);  // Alternative
    Wire.write(0x81);  // Contrast
    Wire.write(0xCF);  // Maximum
    Wire.write(0xD9);  // Precharge
    Wire.write(0xF1);  // 
    Wire.write(0xDB);  // VCOMH Deselect
    Wire.write(0x40);  
    Wire.write(0xA4);  // Resume display
    Wire.write(0xA6);  // Normal display
    Wire.write(0xAF);  // Display on
    return Wire.endTransmission() == 0;
}

void OptimizedOLED::clearDisplay() {
    memset(buffer, 0, sizeof(buffer));
}

void OptimizedOLED::display() {
    Wire.beginTransmission(DISPLAY_ADDR);
    Wire.write(0x00);  // Command
    Wire.write(0x21);  // Column address
    Wire.write(0);     // Start
    Wire.write(127);   // End
    Wire.write(0x22);  // Page address
    Wire.write(0);     // Start
    Wire.write(7);     // End
    Wire.endTransmission();

    // Send buffer in 16-byte chunks
    for (int i = 0; i < sizeof(buffer); i += 16) {
        Wire.beginTransmission(DISPLAY_ADDR);
        Wire.write(0x40);  // Data
        for (int j = 0; j < 16 && (i + j) < sizeof(buffer); j++) {
            Wire.write(buffer[i + j]);
        }
        Wire.endTransmission();
    }
}

void OptimizedOLED::drawFastNumber(uint16_t number, uint8_t x, uint8_t y, uint8_t size) {
    char numStr[6];
    sprintf(numStr, "%4d", number);
    
    for (char* p = numStr; *p; p++) {
        if (*p >= '0' && *p <= '9') {
            const uint8_t* charPtr = &numberFont[(*p - '0') * 5];
            for (int i = 0; i < 5; i++) {
                uint8_t line = pgm_read_byte(charPtr + i);
                for (int j = 0; j < size; j++) {
                    buffer[x + (i * size) + (y * DISPLAY_WIDTH)] = line;
                }
            }
            x += 6 * size;
        } else {
            x += 3 * size;
        }
    }
}