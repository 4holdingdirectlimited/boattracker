#include "SensorManager.h"
#include <esp_adc_cal.h>

SensorManager::SensorManager() : mag(12345) {} // Initialize HMC5883L with ID

bool SensorManager::begin() {
    bool success = true;
    
    // Initialize I2C (using pins from original code)
    Wire.begin(19, 18);
    
    // Initialize AHT10
    if (!aht.begin()) {
        Serial.println("Failed to initialize AHT10!");
        success = false;
    }
    
    // Initialize BMP085/180
    if (!bmp.begin()) {
        Serial.println("Failed to initialize BMP085/180!");
        success = false;
    }
    
    // Initialize MPU6050
    mpu.initialize();
    if (!mpu.testConnection()) {
        Serial.println("Failed to initialize MPU6050!");
        success = false;
    }
    
    // Initialize HMC5883L
    if (!mag.begin()) {
        Serial.println("Failed to initialize HMC5883L!");
        success = false;
    }

    // Initialize RPM pins with interrupt handlers
    pinMode(hallSensorPin1, INPUT_PULLUP);
    pinMode(hallSensorPin2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(hallSensorPin1), rpm1Interrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(hallSensorPin2), rpm2Interrupt, FALLING);
    
    // Initialize ADC for battery monitoring
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    
    return success;
}

void IRAM_ATTR SensorManager::rpm1Interrupt() {
    unsigned long currentTime = millis();
    unsigned long interval = currentTime - lastPulseTime1;
    
    if (interval > DEBOUNCE_TIME) {
        pulseIntervals1[intervalIndex1] = interval;
        intervalIndex1 = (intervalIndex1 + 1) % 10;
        lastPulseTime1 = currentTime;
    }
}

void IRAM_ATTR SensorManager::rpm2Interrupt() {
    unsigned long currentTime = millis();
    unsigned long interval = currentTime - lastPulseTime2;
    
    if (interval > DEBOUNCE_TIME) {
        pulseIntervals2[intervalIndex2] = interval;
        intervalIndex2 = (intervalIndex2 + 1) % 10;
        lastPulseTime2 = currentTime;
    }
}

void SensorManager::updateEnvironmentalData(SensorData& data) {
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    
    // Update temperature and humidity with EMA filtering
    data.temperature = calculateEMA(temp.temperature, prevTemp);
    data.humidity = calculateEMA(humidity.relative_humidity, prevHumidity);
    
    // Update pressure and altitude with EMA filtering
    data.pressure = calculateEMA(bmp.readPressure() / 100.0, prevPressure);
    data.altitude = calculateEMA(bmp.readAltitude(1013.25), prevAltitude);
    
    // Store current values for next EMA calculation
    prevTemp = data.temperature;
    prevHumidity = data.humidity;
    prevPressure = data.pressure;
    prevAltitude = data.altitude;
}

void SensorManager::updateMotionData(SensorData& data) {
    // Read accelerometer and gyro from MPU6050
    Vector accel = readNormalizeAccel();
    Vector gyro = readNormalizeGyro();
    
    data.accelX = accel.XAxis;
    data.accelY = accel.YAxis;
    data.accelZ = accel.ZAxis;
    
    data.gyroX = gyro.XAxis;
    data.gyroY = gyro.YAxis;
    data.gyroZ = gyro.ZAxis;
    
    // Read magnetometer
    sensors_event_t magEvent;
    mag.getEvent(&magEvent);
    data.magX = magEvent.magnetic.x;
    data.magY = magEvent.magnetic.y;
    data.magZ = magEvent.magnetic.z;
    
    // Calculate G-forces
    const float gravity = 9.81;
    data.gForceX = abs(data.accelX);
    data.gForceY = abs(data.accelY);
    data.gForceZ = abs(data.accelZ - 1.0); // Subtract 1g for vertical axis
    data.gForceTotal = sqrt(pow(data.gForceX, 2) + 
                           pow(data.gForceY, 2) + 
                           pow(data.accelZ - 1.0, 2));
}

Vector SensorManager::readNormalizeAccel() {
    Vector normAccel;
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    normAccel.XAxis = ax / 16384.0;
    normAccel.YAxis = ay / 16384.0;
    normAccel.ZAxis = az / 16384.0;
    return normAccel;
}

Vector SensorManager::readNormalizeGyro() {
    Vector normGyro;
    int16_t gx, gy, gz;
    mpu.getRotation(&gx, &gy, &gz);
    normGyro.XAxis = gx / 131.0;
    normGyro.YAxis = gy / 131.0;
    normGyro.ZAxis = gz / 131.0;
    return normGyro;
}

float SensorManager::calculateRPM(volatile unsigned long* intervals, volatile uint8_t index, unsigned long lastPulseTime) {
    unsigned long currentTime = millis();
    
    if (currentTime - lastPulseTime > RPM_TIMEOUT) {
        return 0;
    }
    
    unsigned long totalInterval = 0;
    uint8_t validIntervals = 0;
    
    for (int i = 0; i < 10; i++) {
        if (intervals[i] > 0) {
            totalInterval += intervals[i];
            validIntervals++;
        }
    }
    
    if (validIntervals == 0) return 0;
    
    float averageInterval = (float)totalInterval / validIntervals;
    float rpmRaw = 60000.0 / averageInterval;
    
    static float filteredRPM = 0;
    filteredRPM = calculateEMA(rpmRaw, filteredRPM, 0.3);
    
    return filteredRPM;
}

void SensorManager::updateRPMData(SensorData& data) {
    noInterrupts();
    data.rpm1 = calculateRPM(pulseIntervals1, intervalIndex1, lastPulseTime1);
    data.rpm2 = calculateRPM(pulseIntervals2, intervalIndex2, lastPulseTime2);
    interrupts();
}

void SensorManager::updateBatteryVoltage(SensorData& data) {
    uint32_t adc_reading = adc1_get_raw(ADC1_CHANNEL_6);
    
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);
    
    const float R1 = 100000.0; // 100k resistor
    const float R2 = 10000.0;  // 10k resistor
    data.batteryVoltage = voltage * (R1 + R2) / R2 / 1000.0;
}

void SensorManager::update(SensorData& data) {
    updateEnvironmentalData(data);
    updateMotionData(data);
    updateRPMData(data);
    updateBatteryVoltage(data);
}

float SensorManager::calculateEMA(float current, float prev, float alpha) {
    return (alpha * current) + ((1.0 - alpha) * prev);
}

void SensorManager::processGPSData(char c) {
    gps.encode(c);
}

float SensorManager::getLatitude() const {
    return gps.location.isValid() ? gps.location.lat() : 0.0;
}

float SensorManager::getLongitude() const {
    return gps.location.isValid() ? gps.location.lng() : 0.0;
}

float SensorManager::getSpeed() const {
    return gps.speed.isValid() ? gps.speed.kmph() : 0.0;
}

bool SensorManager::isGPSValid() const {
    return gps.location.isValid() && gps.speed.isValid();
}