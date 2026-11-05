#include "status_led.h"

void statusLedInit() {
    // WS2812 on GPIO 48 is driven by neopixelWrite() — no extra init needed
    statusLedSet(30, 30, 0);  // Boot: yellow
}

void statusLedSet(uint8_t r, uint8_t g, uint8_t b) {
    neopixelWrite(RGB_LED_PIN, r, g, b);
}

void statusLedFromState(SystemState state) {
    switch (state) {
        case STATE_INFUSING:
            statusLedSet(0, 30, 0);    // Green — running
            break;
        case STATE_RETRACTING:
            statusLedSet(30, 15, 0);   // Orange — retract
            break;
        case STATE_IDLE:
            statusLedSet(0, 0, 20);    // Blue — idle
            break;
        case STATE_ALARM_OCCLUSION:
            statusLedSet(50, 0, 30);   // Magenta — occlusion critical
            break;
        case STATE_ALARM_LIMIT_END:
        case STATE_ALARM_LIMIT_HOME:
            statusLedSet(40, 0, 0);    // Red — limit hit
            break;
        default:
            statusLedSet(10, 10, 10);  // Dim white — unknown
            break;
    }
}

