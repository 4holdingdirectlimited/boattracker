#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <Arduino.h>
#include "SensorManager.h"
#include "DisplayManager.h"

class TaskManager {
public:
    static void begin(SensorManager* sMgr, DisplayManager* dMgr);
    static void createTasks();

private:
    static void wifiTask(void* parameter);
    static void sensorTask(void* parameter);
    
    static SensorManager* sensorManager;
    static DisplayManager* displayManager;
    static TaskHandle_t wifiTaskHandle;
    static TaskHandle_t sensorTaskHandle;
};

#endif