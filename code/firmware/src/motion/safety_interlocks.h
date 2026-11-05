#pragma once
#include <Arduino.h>
#include "pinout.h"
#include "stepper_driver.h"

// ========================================================
// SAFETY INTERLOCKS
// Limit switch debounce & immediate motor cutoff
// ========================================================

/// @brief Initialize limit switch pins with internal pull-ups
void safetyInit();

/// @brief Read the HOME limit switch (debounced)
/// @return true if the switch is actively triggered (carriage at home)
bool isHomeLimitHit();

/// @brief Read the END limit switch (debounced)
/// @return true if the switch is actively triggered (syringe fully dispensed)
bool isEndLimitHit();

/// @brief Run continuous safety check — call every loop iteration.
///        Immediately halts the motor if a limit switch is hit in the
///        direction of current travel. Returns the alarm state triggered,
///        or STATE_IDLE if no alarm.
/// @param currentDirection true = forward (infusing), false = retracting
uint8_t safetyCheckLimits(bool currentDirection);

