#include "MinimalWiFi.h"
#include <esp_wifi.h>
#include <esp_event.h>

MinimalWiFi::MinimalWiFi() : hasClient(false) {}

bool MinimalWiFi::begin(const char* ssid, const char* password) {
    // Initialize WiFi with minimal power configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.ampdu_tx_enable = 0;
    cfg.ampdu_rx_enable = 0;
    esp_wifi_init(&cfg);
    
    // Set to minimal power mode for ~10m range
    setMinimalPower();
    
    // Configure AP
    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
    strncpy((char*)wifi_config.ap.password, password, sizeof(wifi_config.ap.password));
    wifi_config.ap.ssid_len = strlen(ssid);
    wifi_config.ap.channel = 6;  // Channel 6 from original code
    wifi_config.ap.max_connection = 4;  // Max 4 connections from original code
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    
    // Set AP configuration
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    
    // Register event handler
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                             &MinimalWiFi::wifiEventHandler, this);
    
    // Start WiFi
    esp_wifi_start();
    
    return true;
}

void MinimalWiFi::setMinimalPower() {
    // Set minimum WiFi power for approximately 10m range
    esp_wifi_set_max_tx_power(8);  // Reduced power
    
    // Configure for minimal power consumption
    esp_wifi_set_ps(WIFI_PS_NONE);
    esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B);
    esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20);
    
    // Disable features we don't need
    wifi_country_t country = {
        .cc = "XX",
        .schan = 1,
        .nchan = 1,
        .max_tx_power = 8,
        .policy = WIFI_COUNTRY_POLICY_MANUAL
    };
    esp_wifi_set_country(&country);
}

void MinimalWiFi::stop() {
    esp_wifi_stop();
    hasClient = false;
}

bool MinimalWiFi::isClientConnected() {
    return hasClient;
}

void MinimalWiFi::wifiEventHandler(void* arg, esp_event_base_t event_base, 
                                 int32_t event_id, void* event_data) {
    MinimalWiFi* wifi = static_cast<MinimalWiFi*>(arg);
    
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_AP_STACONNECTED: {
                wifi_event_ap_staconnected_t* event = 
                    (wifi_event_ap_staconnected_t*) event_data;
                wifi->hasClient = true;
                break;
            }
            case WIFI_EVENT_AP_STADISCONNECTED: {
                wifi_event_ap_stadisconnected_t* event = 
                    (wifi_event_ap_stadisconnected_t*) event_data;
                // Only set hasClient to false if all clients disconnected
                wifi_sta_list_t sta_list;
                esp_wifi_ap_get_sta_list(&sta_list);
                if (sta_list.num == 0) {
                    wifi->hasClient = false;
                }
                break;
            }
        }
    }
}