#include "HallSensors.h"
#include "HardwareConfig.h"

// Initialize variables
volatile unsigned long pulseCount1 = 0;
volatile unsigned long pulseCount2 = 0;
volatile unsigned long lastPulseTime1 = 0;
volatile unsigned long lastPulseTime2 = 0;
volatile unsigned long pulseIntervals1[10] = {0};
volatile unsigned long pulseIntervals2[10] = {0};
volatile uint8_t intervalIndex1 = 0;
volatile uint8_t intervalIndex2 = 0;

void IRAM_ATTR detectPulse1() {
    unsigned long currentTime = millis();
    unsigned long interval = currentTime - lastPulseTime1;
    
    if (interval > DEBOUNCE_TIME) {
        pulseCount1++;
        pulseIntervals1[intervalIndex1] = interval;
        intervalIndex1 = (intervalIndex1 + 1) % 10;
        lastPulseTime1 = currentTime;
    }
}

void IRAM_ATTR detectPulse2() {
    unsigned long currentTime = millis();
    unsigned long interval = currentTime - lastPulseTime2;
    
    if (interval > DEBOUNCE_TIME) {
        pulseCount2++;
        pulseIntervals2[intervalIndex2] = interval;
        intervalIndex2 = (intervalIndex2 + 1) % 10;
        lastPulseTime2 = currentTime;
    }
}

void initializeHallSensors() {
    pinMode(hallSensorPin1, INPUT_PULLUP);
    pinMode(hallSensorPin2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(hallSensorPin1), detectPulse1, FALLING);
    attachInterrupt(digitalPinToInterrupt(hallSensorPin2), detectPulse2, FALLING);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);
    Serial.println("Hall sensors initialized");
}