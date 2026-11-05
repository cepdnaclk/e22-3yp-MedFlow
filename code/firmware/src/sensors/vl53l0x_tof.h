#pragma once
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include "pinout.h"

// ========================================================
// VL53L0X LASER TIME-OF-FLIGHT SENSOR (I2C)
// Measures plunger displacement / syringe travel distance
// ========================================================

/// @brief Initialize I2C bus and VL53L0X sensor
/// @return true if sensor acknowledged on the I2C bus
bool tofInit();

/// @brief Read the current distance measurement
/// @return Distance in millimeters, or -1 if out of range / error
int tofReadDistanceMm();

