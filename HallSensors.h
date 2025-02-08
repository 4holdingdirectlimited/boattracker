#ifndef HALL_SENSORS_H
#define HALL_SENSORS_H

#include <Arduino.h>
#include "HardwareConfig.h"  // Include this for DEBOUNCE_TIME

// External declarations for hall sensor variables
extern volatile unsigned long pulseCount1;
extern volatile unsigned long pulseCount2;
extern volatile unsigned long lastPulseTime1;
extern volatile unsigned long lastPulseTime2;
extern volatile unsigned long pulseIntervals1[10];
extern volatile unsigned long pulseIntervals2[10];
extern volatile uint8_t intervalIndex1;
extern volatile uint8_t intervalIndex2;

// Interrupt Service Routines declarations
void IRAM_ATTR detectPulse1();
void IRAM_ATTR detectPulse2();

// Initialize hall sensors
void initializeHallSensors();

#endif