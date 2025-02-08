#ifndef JSON_CONFIG_H
#define JSON_CONFIG_H

#include <ArduinoJson.h>

// Calculate the required size for JSON document
constexpr size_t calculateJsonSize() {
    // Base structure size
    size_t size = JSON_OBJECT_SIZE(8);  // timestamp, user, running, gps, sensors, motion, rpm, battery, filename
    
    // Nested objects sizes
    size += JSON_OBJECT_SIZE(4);  // GPS object
    size += JSON_OBJECT_SIZE(4);  // sensors object
    size += JSON_OBJECT_SIZE(4);  // motion object with nested objects
    size += JSON_OBJECT_SIZE(3);  // accel object
    size += JSON_OBJECT_SIZE(4);  // gforce object
    size += JSON_OBJECT_SIZE(3);  // gyro object
    size += JSON_OBJECT_SIZE(3);  // mag object
    size += JSON_OBJECT_SIZE(2);  // rpm object
    
    // Add buffer for string values and additional fields
    size += 512;  // Buffer for strings and future expansion
    
    return size;
}

// Configuration for JSON document size
constexpr size_t JSON_DOC_SIZE = calculateJsonSize();

// Configuration for JSON string buffer size
constexpr size_t JSON_STRING_BUFFER_SIZE = JSON_DOC_SIZE + 256;

#endif // JSON_CONFIG_H