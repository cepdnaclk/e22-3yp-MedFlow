#pragma once
#include <HX711.h>
#include "pinout.h"
#include "config.h"

// ========================================================
// HX711 24-BIT LOAD CELL ADC
// Force calibration, tare, and continuous reading for
// occlusion back-pressure monitoring (threshold: 1800 g)
// ========================================================

/// @brief Initialize HX711 with configured DOUT/SCK pins and calibration factor
void loadCellInit();

/// @brief Read the current force in grams (single-sample, non-blocking if ready)
/// @return Force in grams, or last known value if ADC not ready
float loadCellReadGrams();

/// @brief Zero (tare) the load cell — call with no load applied
void loadCellTare();

/// @brief Check if force exceeds the occlusion safety limit
/// @param forceGrams Current force reading
/// @return true if force >= OCCLUSION_LIMIT_GRAMS
bool loadCellIsOccluded(float forceGrams);

