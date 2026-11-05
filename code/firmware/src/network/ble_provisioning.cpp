#include "ble_provisioning.h"

static BLEServer* pServer = NULL;
static BLECharacteristic* pCharSSID = NULL;
static BLECharacteristic* pCharPass = NULL;
static BLECharacteristic* pCharStatus = NULL;
static BLECharacteristic* pCharTelemetry = NULL;

static bool deviceConnected = false;
static bool newCredentialsAvailable = false;
static char pendingSSID[33] = {0};
static char pendingPass[65] = {0};

// ========================================================
// BLE SERVER CALLBACKS
// ========================================================
class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* server) override {
        deviceConnected = true;
        Serial.println("[BLE] Client connected.");
    }
    void onDisconnect(BLEServer* server) override {
        deviceConnected = false;
        Serial.println("[BLE] Client disconnected. Restarting advertising...");
        server->startAdvertising();
    }
};

// ========================================================
// WRITE CALLBACKS FOR WI-FI CREDENTIAL CHARACTERISTICS
// ========================================================
class SSIDWriteCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override {
        String value = pChar->getValue();
        if (value.length() > 0 && value.length() <= 32) {
            strncpy(pendingSSID, value.c_str(), 32);
            pendingSSID[32] = '\0';
            Serial.printf("[BLE] Received SSID: %s\n", pendingSSID);
        }
    }
};

class PassWriteCallback : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override {
        String value = pChar->getValue();
        if (value.length() > 0 && value.length() <= 64) {
            strncpy(pendingPass, value.c_str(), 64);
            pendingPass[64] = '\0';
            newCredentialsAvailable = true;
            Serial.println("[BLE] Received Wi-Fi password. Credentials ready.");
        }
    }
};

void bleProvisioningInit() {
    BLEDevice::init("InfusionPump-S3");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    BLEService* pService = pServer->createService(BLE_SERVICE_UUID);

    // Wi-Fi SSID (writable by phone app)
    pCharSSID = pService->createCharacteristic(
        BLE_CHAR_WIFI_SSID_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pCharSSID->setCallbacks(new SSIDWriteCallback());

    // Wi-Fi Password (writable by phone app)
    pCharPass = pService->createCharacteristic(
        BLE_CHAR_WIFI_PASS_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pCharPass->setCallbacks(new PassWriteCallback());

    // Status (readable by phone app)
    pCharStatus = pService->createCharacteristic(
        BLE_CHAR_STATUS_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharStatus->addDescriptor(new BLE2902());
    pCharStatus->setValue("BOOTING");

    // Telemetry (notify to phone app)
    pCharTelemetry = pService->createCharacteristic(
        BLE_CHAR_TELEMETRY_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharTelemetry->addDescriptor(new BLE2902());

    pService->start();
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();

    Serial.println("[BLE] GATT server started. Advertising as 'InfusionPump-S3'.");
}

bool bleHasNewCredentials(char* ssidOut, char* passOut) {
    if (!newCredentialsAvailable) return false;

    strncpy(ssidOut, pendingSSID, 33);
    strncpy(passOut, pendingPass, 65);
    newCredentialsAvailable = false;
    return true;
}

void bleUpdateStatus(const char* statusText) {
    if (pCharStatus) {
        pCharStatus->setValue(statusText);
        if (deviceConnected) {
            pCharStatus->notify();
        }
    }
}

void bleNotifyTelemetry(const char* jsonPayload) {
    if (pCharTelemetry && deviceConnected) {
        pCharTelemetry->setValue(jsonPayload);
        pCharTelemetry->notify();
    }
}

bool bleIsClientConnected() {
    return deviceConnected;
}

