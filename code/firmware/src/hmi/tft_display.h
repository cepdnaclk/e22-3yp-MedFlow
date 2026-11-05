#pragma once
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "pinout.h"
#include "data_types.h"

// ========================================================
// ILI9341 SPI COLOR TFT DISPLAY (320x240)
// Selective region redraw to minimize SPI bus time
// ========================================================

/// @brief Initialize the FSPI bus and ILI9341 display controller
void tftInit();

/// @brief Draw the static dashboard frame (borders, labels)
void tftDrawFrame();

/// @brief Update the dynamic telemetry values on screen (selective redraw)
/// @param state    Current system state enum
/// @param steps    Accumulated step count
/// @param force    Current load cell force (grams)
/// @param distance Current ToF plunger distance (mm), -1 if OOR
/// @param homeHit  Home limit switch state
/// @param endHit   End limit switch state
void tftUpdateTelemetry(SystemState state, unsigned long steps, float force, int distance, bool homeHit, bool endHit);

