#include "UserConfig.h"
#include "GlobalConfig.h"
#include <ArduinoJson.h>
#include <SD.h>

const char* UserConfiguration::CONFIG_FILENAME = "/config.json";

bool UserConfiguration::loadFromSD() {
    if (xSemaphoreTake(xSDCardMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return false;
    }

    if (!SD.exists(CONFIG_FILENAME)) {
        xSemaphoreGive(xSDCardMutex);
        return false;
    }

    File configFile = SD.open(CONFIG_FILENAME, FILE_READ);
    if (!configFile) {
        xSemaphoreGive(xSDCardMutex);
        return false;
    }

    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();
    xSemaphoreGive(xSDCardMutex);

    if (error) {
        return false;
    }

    // Load boat config
    strlcpy(boat.name, doc["boat"]["name"] | "", sizeof(boat.name));
    strlcpy(boat.length, doc["boat"]["length"] | "", sizeof(boat.length));
    strlcpy(boat.weight, doc["boat"]["weight"] | "", sizeof(boat.weight));
    strlcpy(boat.type, doc["boat"]["type"] | "", sizeof(boat.type));
    strlcpy(boat.number, doc["boat"]["number"] | "", sizeof(boat.number));
    strlcpy(boat.notes, doc["boat"]["notes"] | "", sizeof(boat.notes));

    // Load other configurations similarly...
    // [Additional loading code for other sections]

    return true;
}

bool UserConfiguration::saveToSD() {
    if (xSemaphoreTake(xSDCardMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        return false;
    }

    StaticJsonDocument<2048> doc;

    // Save boat config
    JsonObject boatObj = doc.createNestedObject("boat");
    boatObj["name"] = boat.name;
    boatObj["length"] = boat.length;
    boatObj["weight"] = boat.weight;
    boatObj["type"] = boat.type;
    boatObj["number"] = boat.number;
    boatObj["notes"] = boat.notes;

    // Save other configurations similarly...
    // [Additional saving code for other sections]

    File configFile = SD.open(CONFIG_FILENAME, FILE_WRITE);
    if (!configFile) {
        xSemaphoreGive(xSDCardMutex);
        return false;
    }

    serializeJsonPretty(doc, configFile);
    configFile.close();
    xSemaphoreGive(xSDCardMutex);
    return true;
}

void UserConfiguration::logCurrentConfig() {
    if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer),
            "CONFIG UPDATE,%s,%s,"
            "BOAT,%s,%s,%s,%s,%s,%s,"
            "DRIVER,%s,%s,%s,%s,"
            "ENGINE,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
            CURRENT_TIMESTAMP, CURRENT_USER,
            boat.name, boat.length, boat.weight, boat.type, boat.number, boat.notes,
            driver.name, driver.height, driver.weight, driver.notes,
            engine.brand, engine.horsepower, engine.type, engine.timing,
            engine.ignitionType, engine.fuelPressure, engine.carbJetting,
            engine.rpmLimit, engine.fuelType, engine.fuelSystem,
            engine.strokeNumber, engine.cylinderNumber, engine.coolingSystem,
            engine.notes
        );
        writeDataToSD(buffer);
        xSemaphoreGive(xDataMutex);
    }
}

float UserConfiguration::calculatePropSlip(float gpsSpeed, float rpm, float propPitch) {
    // Convert GPS speed to MPH if it's not already
    float speedMPH = gpsSpeed * 0.621371; // Convert km/h to mph
    
    // Calculate theoretical speed
    float theoreticalMPH = (rpm * propPitch) / 1056.0; // Standard prop speed formula
    
    // Calculate slip percentage
    if (theoreticalMPH > 0) {
        return ((theoreticalMPH - speedMPH) / theoreticalMPH) * 100.0;
    }
    return 0.0;
}

UserConfiguration userConfig;