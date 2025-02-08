#ifndef TASK_CONFIG_H
#define TASK_CONFIG_H

// Task Priorities (0-24, higher number = higher priority)
constexpr uint8_t WIFI_TASK_PRIORITY = 2;
constexpr uint8_t SENSOR_TASK_PRIORITY = 3;

// Task Stack Sizes
constexpr uint32_t WIFI_STACK_SIZE = 4096;
constexpr uint32_t SENSOR_STACK_SIZE = 4096;

// Task Core Assignments
constexpr BaseType_t WIFI_TASK_CORE = 0;
constexpr BaseType_t SENSOR_TASK_CORE = 1;

// Task Delays
constexpr TickType_t WIFI_TASK_DELAY = pdMS_TO_TICKS(100);
constexpr TickType_t SENSOR_TASK_DELAY = pdMS_TO_TICKS(50);

// Watchdog Timeouts
constexpr uint32_t WDT_TIMEOUT = 30000;  // 30 seconds

#endif