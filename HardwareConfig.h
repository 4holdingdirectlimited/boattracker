#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// Pin Definitions
constexpr uint8_t GPS_RX_PIN = 16;
constexpr uint8_t GPS_TX_PIN = 17;
constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint8_t SPI_MISO = 19;
constexpr uint8_t SPI_MOSI = 23;
constexpr uint8_t SPI_SCK = 18;
constexpr uint8_t chipSelect = 5;

// GPS Configuration
constexpr uint32_t GPS_BAUD_RATE = 9600;

// Display Configuration
constexpr uint16_t SCREEN_WIDTH = 128;
constexpr uint16_t SCREEN_HEIGHT = 64;

// RPM Configuration
constexpr uint8_t RPM_PIN_1 = 25;
constexpr uint8_t RPM_PIN_2 = 26;

// Battery Monitoring
constexpr uint8_t BATTERY_PIN = 34;
constexpr float BATTERY_DIVIDER_RATIO = 2.0;

#endif