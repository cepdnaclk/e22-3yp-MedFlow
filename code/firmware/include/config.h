#pragma once

// ========================================================
// OPERATIONAL CALIBRATION & NETWORK CONFIGURATION
// ========================================================

#define DEVICE_ID                     "PUMP-S3-001"
#define FIRMWARE_VERSION              "1.0.0"

// Medical Safety Thresholds
#define LOADCELL_CALIBRATION_FACTOR   420.0f
#define OCCLUSION_LIMIT_GRAMS         1800.0f   // Safety trip point (~1.8 kg force)

// Motor Kinematics (400us toggle = 800us period = 1250 steps/sec)
#define STEP_TIMER_INTERVAL_US        400

// Network Credentials (Replace with actual credentials or BLE-provisioned values)
#define WIFI_SSID                     "Hospital_IoT_Network"
#define WIFI_PASSWORD                 "SecureHospitalKey2026"

// MQTT Broker Configuration (Docker Container Host IP)
#define MQTT_BROKER_HOST              "192.168.1.100"
#define MQTT_BROKER_PORT              1883

// Telemetry Transmit Intervals
#define TELEMETRY_INTERVAL_MS         200       // 5 Hz refresh rate

