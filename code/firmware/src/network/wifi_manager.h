#pragma once
#include <WiFi.h>
#include "config.h"

// ========================================================
// WI-FI STATION / AP MANAGER
// Automatic reconnection with watchdog
// ========================================================

/// @brief Initialize Wi-Fi in Station mode and begin connection
void wifiInit();

/// @brief Check and maintain connection, reconnect if dropped.
///        Call periodically from the networking task loop.
/// @return true if currently connected
bool wifiMaintain();

/// @brief Get the current RSSI signal strength
/// @return RSSI in dBm, or 0 if not connected
int wifiGetRSSI();

/// @brief Check if Wi-Fi is currently connected
bool wifiIsConnected();

