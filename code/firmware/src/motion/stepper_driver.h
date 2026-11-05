#pragma once
#include <Arduino.h>
#include "pinout.h"
#include "config.h"

// ========================================================
// STEPPER MOTOR DRIVER (A4988 / TMC2209)
// Hardware Timer ISR-driven step pulse generation
// ========================================================

extern hw_timer_t *motorTimer;
extern portMUX_TYPE timerMux;
extern volatile bool motorRunFlag;
extern volatile bool motorDirection;
extern volatile unsigned long totalSteps;

/// @brief Initialize stepper GPIO pins and hardware timer ISR
void stepperInit();

/// @brief Start the hardware timer (begins step pulse generation)
void stepperStart();

/// @brief Stop the hardware timer (halts motion immediately)
void stepperStop();

/// @brief Set motor direction
/// @param forward true = Infuse (push plunger), false = Retract
void stepperSetDirection(bool forward);

/// @brief Enable or disable the A4988 driver output stage
/// @param enabled true = driver active (ENABLE pin LOW)
void stepperSetEnabled(bool enabled);

/// @brief Read the current accumulated step count (thread-safe)
unsigned long stepperGetSteps();

/// @brief Reset the step counter to zero
void stepperResetSteps();

