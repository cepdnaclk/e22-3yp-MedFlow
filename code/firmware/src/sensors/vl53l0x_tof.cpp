#include "vl53l0x_tof.h"

static Adafruit_VL53L0X lox = Adafruit_VL53L0X();
static bool sensorReady = false;

bool tofInit() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    if (lox.begin()) {
        sensorReady = true;
        Serial.println("[VL53L0X] ToF sensor initialized on I2C.");
        return true;
    } else {
        sensorReady = false;
        Serial.println("[VL53L0X] ERROR: Failed to detect sensor on I2C bus.");
        return false;
    }
}

int tofReadDistanceMm() {
    if (!sensorReady) return -1;

    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);

    if (measure.RangeStatus != 4) {  // 4 = phase failure (out of range)
        return measure.RangeMilliMeter;
    }
    return -1;  // Out of range
}

