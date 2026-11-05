#include "mqtt_client.h"

static WiFiClient espClient;
static PubSubClient client(espClient);

static unsigned long lastMqttReconnect = 0;
static const unsigned long MQTT_RECONNECT_INTERVAL_MS = 3000;

void mqttInit(MQTT_CALLBACK_SIGNATURE) {
    client.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    client.setCallback(callback);
    client.setBufferSize(512);
    Serial.printf("[MQTT] Broker target: %s:%d\n", MQTT_BROKER_HOST, MQTT_BROKER_PORT);
}

bool mqttMaintain() {
    if (client.connected()) return true;

    unsigned long now = millis();
    if (now - lastMqttReconnect < MQTT_RECONNECT_INTERVAL_MS) return false;
    lastMqttReconnect = now;

    String clientId = String("ESP32-S3-") + WiFi.macAddress();
    Serial.printf("[MQTT] Attempting connection as %s...\n", clientId.c_str());

    if (client.connect(clientId.c_str())) {
        String cmdTopic = String("pumps/") + DEVICE_ID + "/cmd";
        client.subscribe(cmdTopic.c_str(), 1);
        Serial.printf("[MQTT] Connected. Subscribed to %s\n", cmdTopic.c_str());
        return true;
    } else {
        Serial.printf("[MQTT] Failed, rc=%d. Will retry...\n", client.state());
        return false;
    }
}

void mqttPublishTelemetry(const TelemetryPacket &pkt) {
    if (!client.connected()) return;

    StaticJsonDocument<384> doc;
    doc["device_id"]     = DEVICE_ID;
    doc["state"]         = stateToString(pkt.state);
    doc["steps"]         = pkt.steps;
    doc["force_grams"]   = pkt.weightGrams;
    doc["distance_mm"]   = pkt.distanceMm;
    doc["flow_rate_ml_h"] = pkt.flowRateMlH;

    JsonObject limits = doc.createNestedObject("limit_switches");
    limits["home"] = pkt.homeHit;
    limits["end"]  = pkt.endHit;

    doc["wifi_rssi"] = WiFi.RSSI();

    char buffer[384];
    serializeJson(doc, buffer);
    String topic = String("pumps/") + DEVICE_ID + "/telemetry";
    client.publish(topic.c_str(), buffer);
}

void mqttPublishAlarm(const TelemetryPacket &pkt, const char* alarmType, const char* message) {
    if (!client.connected()) return;

    StaticJsonDocument<256> doc;
    doc["device_id"]     = DEVICE_ID;
    doc["alarm_type"]    = alarmType;
    doc["severity"]      = "CRITICAL";
    doc["force_grams"]   = pkt.weightGrams;
    doc["motor_stopped"] = true;
    doc["message"]       = message;

    char buffer[256];
    serializeJson(doc, buffer);
    String topic = String("pumps/") + DEVICE_ID + "/alarms";
    client.publish(topic.c_str(), buffer, true);  // Retained alarm
}

void mqttLoop() {
    client.loop();
}

