#pragma once
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "data_types.h"

// ========================================================
// MQTT CLIENT
// Telemetry publishing & command topic subscription
// ========================================================

/// @brief Initialize the MQTT client with broker address and command callback
/// @param cmdCallback Function to handle incoming command messages
void mqttInit(MQTT_CALLBACK_SIGNATURE);

/// @brief Maintain MQTT connection. Reconnects and resubscribes if dropped.
///        Call periodically from the networking task loop.
/// @return true if currently connected to the broker
bool mqttMaintain();

/// @brief Publish a telemetry JSON packet to pumps/{DEVICE_ID}/telemetry
void mqttPublishTelemetry(const TelemetryPacket &pkt);

/// @brief Publish an alarm JSON packet to pumps/{DEVICE_ID}/alarms (QoS 1)
void mqttPublishAlarm(const TelemetryPacket &pkt, const char* alarmType, const char* message);

/// @brief Process incoming MQTT messages — call in the task loop
void mqttLoop();

