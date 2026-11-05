#pragma once

// ========================================================
// ESP32-S3 N16R8 HARDWARE PINOUT MAPPING
// Reference: Comprehensive Engineering Specification
// ========================================================

// Stepper Motor (A4988 / HR4988 / TMC2209)
#define STEP_PIN              16    // Hardware Timer Output Pulse
#define DIR_PIN               17    // Logic HIGH = Infuse/Forward, LOW = Retract
#define ENABLE_PIN            18    // Active-LOW driver enable output

// Safety Travel Limit Switches (Mechanical Endstops)
#define LIMIT_SW_HOME_PIN     6     // Carriage retracted (Home position)
#define LIMIT_SW_END_PIN      7     // Syringe fully dispensed (End position)

// HX711 24-Bit ADC (Force / Occlusion Strain Gauge)
#define HX711_DOUT_PIN        4     // 24-bit bit-bang serial input
#define HX711_SCK_PIN         5     // Clock pulse generation

// I2C Interface for VL53L0X Laser Time-of-Flight Sensor
#define I2C_SDA_PIN           1     // Hardware I2C Data
#define I2C_SCL_PIN           2     // Hardware I2C Clock

// ILI9341 SPI Color TFT Display (FSPI Bus)
#define TFT_MOSI              11    // Hardware FSPI Data Out
#define TFT_SCK               12    // Hardware FSPI Clock
#define TFT_MISO              13    // Hardware FSPI Data In
#define TFT_CS                10    // Chip Select
#define TFT_DC                9     // Data / Command Selection
#define TFT_RST               -1    // Hardware Reset tied to 3V3 rail

// Onboard WS2812 Addressable RGB LED
#define RGB_LED_PIN           48

