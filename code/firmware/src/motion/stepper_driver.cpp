#include "stepper_driver.h"

hw_timer_t *motorTimer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool motorRunFlag = false;
volatile bool motorDirection = true;   // true = Forward (Infuse)
volatile unsigned long totalSteps = 0;
static volatile bool stepPinState = false;

// ========================================================
// ISR: Microsecond-Accurate Step Pulse Toggle
// Runs on Core 1 with highest priority via hardware timer
// ========================================================
void IRAM_ATTR onMotorTimerISR() {
    portENTER_CRITICAL_ISR(&timerMux);
    if (motorRunFlag) {
        stepPinState = !stepPinState;
        digitalWrite(STEP_PIN, stepPinState ? HIGH : LOW);
        if (!stepPinState) {
            totalSteps++;
        }
    } else {
        digitalWrite(STEP_PIN, LOW);
        stepPinState = false;
    }
    portEXIT_CRITICAL_ISR(&timerMux);
}

void stepperInit() {
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(ENABLE_PIN, OUTPUT);

    digitalWrite(STEP_PIN, LOW);
    digitalWrite(DIR_PIN, motorDirection ? HIGH : LOW);
    digitalWrite(ENABLE_PIN, LOW);  // Active-LOW: enable driver

    // Configure hardware timer (ESP32 Core 3.x API)
    motorTimer = timerBegin(1000000);  // 1 MHz tick (1 µs resolution)
    timerAttachInterrupt(motorTimer, &onMotorTimerISR);
    timerAlarm(motorTimer, STEP_TIMER_INTERVAL_US, true, 0);
}

void stepperStart() {
    portENTER_CRITICAL(&timerMux);
    motorRunFlag = true;
    portEXIT_CRITICAL(&timerMux);
}

void stepperStop() {
    portENTER_CRITICAL(&timerMux);
    motorRunFlag = false;
    portEXIT_CRITICAL(&timerMux);
}

void stepperSetDirection(bool forward) {
    motorDirection = forward;
    digitalWrite(DIR_PIN, forward ? HIGH : LOW);
}

void stepperSetEnabled(bool enabled) {
    digitalWrite(ENABLE_PIN, enabled ? LOW : HIGH);  // Active-LOW
}

unsigned long stepperGetSteps() {
    portENTER_CRITICAL(&timerMux);
    unsigned long s = totalSteps;
    portEXIT_CRITICAL(&timerMux);
    return s;
}

void stepperResetSteps() {
    portENTER_CRITICAL(&timerMux);
    totalSteps = 0;
    portEXIT_CRITICAL(&timerMux);
}

