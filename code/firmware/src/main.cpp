#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_VL53L0X.h>
#include <HX711.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "pinout.h"
#include "config.h"

// ========================================================
// OPERATIONAL STATES & DATA STRUCTURES
// ========================================================
enum SystemState {
    STATE_IDLE,
    STATE_INFUSING,
    STATE_RETRACTING,
    STATE_ALARM_OCCLUSION,
    STATE_ALARM_LIMIT_END,
    STATE_ALARM_LIMIT_HOME
};

struct TelemetryPacket {
    SystemState state;
    unsigned long steps;
    float weightGrams;
    int distanceMm;
    bool homeHit;
    bool endHit;
};

// FreeRTOS Inter-Core Queue (Core 1 -> Core 0)
QueueHandle_t telemetryQueue;

// ========================================================
// HARDWARE INSTANCES & SHARED REGISTERS
// ========================================================
SPIClass tftSPI(FSPI);
Adafruit_ILI9341 tft = Adafruit_ILI9341(&tftSPI, TFT_DC, TFT_CS, TFT_RST);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
HX711 scale;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

hw_timer_t *motorTimer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile SystemState currentState = STATE_INFUSING;
volatile bool motorRunFlag = true;
volatile bool motorDirection = true; // true = Forward (Infuse), false = Reverse (Retract)
volatile unsigned long totalSteps = 0;
volatile bool stepPinState = false;

// ========================================================
// HARDWARE ISR: Microsecond-Accurate Step Generator
// ========================================================
void IRAM_ATTR onMotorTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    if (motorRunFlag) {
        stepPinState = !stepPinState;
        digitalWrite(STEP_PIN, stepPinState ? HIGH : LOW);
        if (!stepPinState) {
            totalSteps++;
        }
    } else {
        digitalWrite(STEP_PIN, LOW);
    }
    portEXIT_CRITICAL_ISR(&timerMux);
}

void setStatusLED(uint8_t r, uint8_t g, uint8_t b) {
    neopixelWrite(RGB_LED_PIN, r, g, b);
}

void drawDashboardFrame() {
    tft.fillScreen(ILI9341_BLACK);
    tft.drawRect(5, 5, 310, 230, ILI9341_WHITE);
    tft.drawFastHLine(5, 42, 310, ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_GREEN);
    tft.setCursor(18, 15);
    tft.print("SMART INFUSION (IoT)");
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(15, 55);  tft.print("State   :");
    tft.setCursor(15, 90);  tft.print("Steps   :");
    tft.setCursor(15, 125); tft.print("Force   :");
    tft.setCursor(15, 160); tft.print("Travel  :");
    tft.setCursor(15, 195); tft.print("Switches:");
}

// ========================================================
// CORE 0 TASK: Networking, Wi-Fi & MQTT Client
// ========================================================
void onMqttCommandReceived(char* topic, byte* payload, unsigned int length) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) return;

    const char* command = doc["command"];
    if (strcmp(command, "PAUSE") == 0) {
        portENTER_CRITICAL(&timerMux);
        motorRunFlag = false;
        portEXIT_CRITICAL(&timerMux);
        currentState = STATE_IDLE;
        setStatusLED(30, 30, 0);
    } else if (strcmp(command, "INFUSE") == 0) {
        motorDirection = true;
        digitalWrite(DIR_PIN, HIGH);
        portENTER_CRITICAL(&timerMux);
        motorRunFlag = true;
        portEXIT_CRITICAL(&timerMux);
        currentState = STATE_INFUSING;
        setStatusLED(0, 30, 0);
    } else if (strcmp(command, "RETRACT") == 0) {
        motorDirection = false;
        digitalWrite(DIR_PIN, LOW);
        portENTER_CRITICAL(&timerMux);
        motorRunFlag = true;
        portEXIT_CRITICAL(&timerMux);
        currentState = STATE_RETRACTING;
        setStatusLED(30, 15, 0);
    } else if (strcmp(command, "TARE_SCALE") == 0) {
        scale.tare();
    }
}

void TaskNetworkLoop(void *pvParameters) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    mqttClient.setCallback(onMqttCommandReceived);

    TelemetryPacket packet;
    char buffer[384];

    for (;;) {
        // Maintain Wi-Fi Connection
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.reconnect();
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        // Maintain MQTT Connection
        if (!mqttClient.connected()) {
            String clientName = String("ESP32-S3-") + WiFi.macAddress();
            if (mqttClient.connect(clientName.c_str())) {
                String cmdTopic = String("pumps/") + DEVICE_ID + "/cmd";
                mqttClient.subscribe(cmdTopic.c_str());
            } else {
                vTaskDelay(pdMS_TO_TICKS(2000));
                continue;
            }
        }
        mqttClient.loop();

        // Check if Core 1 sent fresh telemetry
        if (xQueueReceive(telemetryQueue, &packet, pdMS_TO_TICKS(50)) == pdTRUE) {
            StaticJsonDocument<384> doc;
            doc["device_id"] = DEVICE_ID;
            
            const char* stateStr = "IDLE";
            if (packet.state == STATE_INFUSING) stateStr = "INFUSING";
            else if (packet.state == STATE_RETRACTING) stateStr = "RETRACTING";
            else if (packet.state == STATE_ALARM_OCCLUSION) stateStr = "ALARM_OCCLUSION";
            else if (packet.state == STATE_ALARM_LIMIT_END) stateStr = "ALARM_LIMIT_END";
            else if (packet.state == STATE_ALARM_LIMIT_HOME) stateStr = "ALARM_LIMIT_HOME";

            doc["state"] = stateStr;
            doc["steps"] = packet.steps;
            doc["force_grams"] = packet.weightGrams;
            doc["distance_mm"] = packet.distanceMm;
            
            JsonObject limits = doc.createNestedObject("limit_switches");
            limits["home"] = packet.homeHit;
            limits["end"] = packet.endHit;
            doc["wifi_rssi"] = WiFi.RSSI();

            serializeJson(doc, buffer);
            String teleTopic = String("pumps/") + DEVICE_ID + "/telemetry";
            mqttClient.publish(teleTopic.c_str(), buffer);

            // Publish separate alarm packet if safety tripped
            if (packet.state == STATE_ALARM_OCCLUSION || packet.state == STATE_ALARM_LIMIT_END || packet.state == STATE_ALARM_LIMIT_HOME) {
                StaticJsonDocument<256> alarmDoc;
                alarmDoc["device_id"] = DEVICE_ID;
                alarmDoc["alarm_type"] = (packet.state == STATE_ALARM_OCCLUSION) ? "OCCLUSION_DETECTED" : "LIMIT_TRIP";
                alarmDoc["severity"] = "CRITICAL";
                alarmDoc["force_grams"] = packet.weightGrams;
                alarmDoc["motor_stopped"] = true;
                alarmDoc["message"] = (packet.state == STATE_ALARM_OCCLUSION) ? "Occlusion back-pressure exceeded 1800g" : "Endstop limit switch hit";

                char alarmBuf[256];
                serializeJson(alarmDoc, alarmBuf);
                String alarmTopic = String("pumps/") + DEVICE_ID + "/alarms";
                mqttClient.publish(alarmTopic.c_str(), alarmBuf);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ========================================================
// CORE 1: Setup & Main Real-Time Control Loop
// ========================================================
void setup() {
    Serial.begin(115200);
    setStatusLED(30, 30, 0); // Booting yellow

    // 1. Inter-Core Queue for Telemetry Packets
    telemetryQueue = xQueueCreate(10, sizeof(TelemetryPacket));

    // 2. Motion GPIOs
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW);
    digitalWrite(DIR_PIN, motorDirection ? HIGH : LOW);
    digitalWrite(STEP_PIN, LOW);

    // 3. Limit Switches
    pinMode(LIMIT_SW_HOME_PIN, INPUT_PULLUP);
    pinMode(LIMIT_SW_END_PIN, INPUT_PULLUP);

    // 4. SPI Display
    tftSPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
    tft.begin(8000000);
    tft.setRotation(1);
    drawDashboardFrame();

    // 5. I2C ToF Sensor
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    lox.begin();

    // 6. HX711 Load Cell
    scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
    scale.set_scale(LOADCELL_CALIBRATION_FACTOR);
    scale.set_offset(0);

    // 7. Hardware Step Timer (ESP32 Core 3.x API)
    motorTimer = timerBegin(1000000);
    timerAttachInterrupt(motorTimer, &onMotorTimer);
    timerAlarm(motorTimer, STEP_TIMER_INTERVAL_US, true, 0);

    // 8. Launch Network Task on Core 0 (Protects Core 1 deterministic motion)
    xTaskCreatePinnedToCore(
        TaskNetworkLoop,
        "TaskNetwork",
        8192,
        NULL,
        1,
        NULL,
        0 // Pin to Core 0
    );

    setStatusLED(0, 30, 0); // Running green
}

unsigned long lastTelemetryMillis = 0;

void loop() {
    unsigned long currentMillis = millis();

    // 1. Continuous Safety Interlock Checks
    bool homeHit = (digitalRead(LIMIT_SW_HOME_PIN) == LOW);
    bool endHit = (digitalRead(LIMIT_SW_END_PIN) == LOW);

    if (endHit && motorDirection) {
        portENTER_CRITICAL(&timerMux);
        motorRunFlag = false;
        portEXIT_CRITICAL(&timerMux);
        currentState = STATE_ALARM_LIMIT_END;
        setStatusLED(40, 0, 0);
    } else if (homeHit && !motorDirection) {
        portENTER_CRITICAL(&timerMux);
        motorRunFlag = false;
        portEXIT_CRITICAL(&timerMux);
        currentState = STATE_ALARM_LIMIT_HOME;
        setStatusLED(40, 0, 0);
    }

    // 2. Non-blocking Telemetry, Sensors & Display Update Loop
    if (currentMillis - lastTelemetryMillis >= TELEMETRY_INTERVAL_MS) {
        lastTelemetryMillis = currentMillis;

        // Sample Load Cell
        float currentWeight = 0.0;
        if (scale.is_ready()) {
            currentWeight = scale.get_units(1);
        }

        // Occlusion Check
        if (currentWeight >= OCCLUSION_LIMIT_GRAMS && motorRunFlag && motorDirection) {
            portENTER_CRITICAL(&timerMux);
            motorRunFlag = false;
            portEXIT_CRITICAL(&timerMux);
            currentState = STATE_ALARM_OCCLUSION;
            setStatusLED(50, 0, 30);
        }

        // Sample ToF Distance
        int currentDistanceMm = -1;
        VL53L0X_RangingMeasurementData_t measure;
        lox.rangingTest(&measure, false);
        if (measure.RangeStatus != 4) {
            currentDistanceMm = measure.RangeMilliMeter;
        }

        portENTER_CRITICAL(&timerMux);
        unsigned long currentSteps = totalSteps;
        portEXIT_CRITICAL(&timerMux);

        // Send telemetry packet across to Core 0 Network Task
        TelemetryPacket packet;
        packet.state = currentState;
        packet.steps = currentSteps;
        packet.weightGrams = currentWeight;
        packet.distanceMm = currentDistanceMm;
        packet.homeHit = homeHit;
        packet.endHit = endHit;
        xQueueOverwrite(telemetryQueue, &packet);

        // Update local TFT Screen
        tft.setTextSize(2);
        tft.fillRect(130, 55, 175, 22, ILI9341_BLACK);
        tft.setCursor(130, 55);
        if (currentState == STATE_INFUSING) { tft.setTextColor(ILI9341_GREEN); tft.print("INFUSING"); }
        else if (currentState == STATE_RETRACTING) { tft.setTextColor(ILI9341_YELLOW); tft.print("RETRACTING"); }
        else if (currentState == STATE_ALARM_OCCLUSION) { tft.setTextColor(ILI9341_RED); tft.print("OCCLUSION!"); }
        else if (currentState == STATE_ALARM_LIMIT_END) { tft.setTextColor(ILI9341_RED); tft.print("END REACHED"); }
        else if (currentState == STATE_ALARM_LIMIT_HOME) { tft.setTextColor(ILI9341_MAGENTA); tft.print("HOME REACHED"); }
        else { tft.setTextColor(ILI9341_WHITE); tft.print("IDLE"); }

        tft.fillRect(130, 90, 175, 22, ILI9341_BLACK);
        tft.setCursor(130, 90);
        tft.setTextColor(ILI9341_CYAN);
        tft.print(currentSteps);

        tft.fillRect(130, 125, 175, 22, ILI9341_BLACK);
        tft.setCursor(130, 125);
        tft.setTextColor(currentWeight > (OCCLUSION_LIMIT_GRAMS * 0.8) ? ILI9341_RED : ILI9341_MAGENTA);
        tft.print(currentWeight, 0); tft.print(" g");

        tft.fillRect(130, 160, 175, 22, ILI9341_BLACK);
        tft.setCursor(130, 160);
        tft.setTextColor(ILI9341_WHITE);
        if (currentDistanceMm >= 0) { tft.print(currentDistanceMm); tft.print(" mm"); }
        else { tft.print("OOR"); }

        tft.fillRect(130, 195, 175, 22, ILI9341_BLACK);
        tft.setCursor(130, 195);
        tft.setTextColor(homeHit ? ILI9341_YELLOW : ILI9341_DARKGREY);
        tft.print("H:"); tft.print(homeHit ? "1 " : "0 ");
        tft.setTextColor(endHit ? ILI9341_RED : ILI9341_DARKGREY);
        tft.print("E:"); tft.print(endHit ? "1" : "0");
    }
}

