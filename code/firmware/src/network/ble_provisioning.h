#pragma once
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ========================================================
// BLE GATT SERVICE FOR SMARTPHONE CONFIGURATION
// Allows bedside provisioning of Wi-Fi credentials and
// direct local monitoring without Wi-Fi infrastructure
// ========================================================

// Service & Characteristic UUIDs
#define BLE_SERVICE_UUID            "12345678-1234-1234-1234-123456789abc"
#define BLE_CHAR_WIFI_SSID_UUID     "12345678-1234-1234-1234-123456789ab1"
#define BLE_CHAR_WIFI_PASS_UUID     "12345678-1234-1234-1234-123456789ab2"
#define BLE_CHAR_STATUS_UUID        "12345678-1234-1234-1234-123456789ab3"
#define BLE_CHAR_TELEMETRY_UUID     "12345678-1234-1234-1234-123456789ab4"

/// @brief Initialize the BLE GATT server with provisioning characteristics
void bleProvisioningInit();

/// @brief Check if a BLE client has written new Wi-Fi credentials
/// @param ssidOut Buffer to receive SSID (max 32 chars)
/// @param passOut Buffer to receive password (max 64 chars)
/// @return true if new credentials were received since last check
bool bleHasNewCredentials(char* ssidOut, char* passOut);

/// @brief Update the BLE status characteristic (readable by phone app)
void bleUpdateStatus(const char* statusText);

/// @brief Send a telemetry notification to connected BLE clients
void bleNotifyTelemetry(const char* jsonPayload);

/// @brief Check if a BLE client is currently connected
bool bleIsClientConnected();

