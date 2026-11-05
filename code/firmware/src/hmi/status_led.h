#pragma once
#include <Arduino.h>
#include "pinout.h"
#include "data_types.h"

// ========================================================
// ONBOARD WS2812 RGB LED STATUS PATTERNS
// ========================================================

/// @brief Initialize the onboard addressable LED
void statusLedInit();

/// @brief Set a static color on the onboard WS2812 LED
void statusLedSet(uint8_t r, uint8_t g, uint8_t b);

/// @brief Update LED color based on current system state
void statusLedFromState(SystemState state);

