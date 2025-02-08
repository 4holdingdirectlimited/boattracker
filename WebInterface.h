#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include "MinimalWiFi.h"
#include <WebServer.h>
#include "UserConfig.h"

class WebInterface {
public:
    WebInterface();
    void begin();
    void handleClient();
    bool isClientConnected();
    void setConfigUpdateCallback(void (*callback)());

private:
    MinimalWiFi wifi;
    WebServer server;
    void (*onConfigUpdate)();
    
    void handleRoot();
    void handleUpdate();
    void handleGetConfig();
    String generateHTML();
    String generateSuccessHTML();
    void updateConfigFromPost();
    
    // Security settings
    static constexpr uint16_t SESSION_TIMEOUT = 300000; // 5 minutes
    unsigned long lastActivity;
    bool authenticated;
    void checkTimeout();
};

#endif