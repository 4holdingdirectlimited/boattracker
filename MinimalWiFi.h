#ifndef MINIMAL_WIFI_H
#define MINIMAL_WIFI_H

#include <esp_wifi.h>
#include <WiFi.h>

class MinimalWiFi {
public:
    MinimalWiFi();
    bool begin(const char* ssid, const char* password);
    void stop();
    bool isClientConnected();
    void setMinimalPower();
    void handleClient();

private:
    static void wifiEventHandler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data);
    bool hasClient;
    static constexpr uint8_t MAX_CLIENTS = 1;
    static constexpr int8_t TARGET_POWER = 8;  // Approximately 10m range
    static constexpr uint8_t WIFI_CHANNEL = 1;
};

#endif