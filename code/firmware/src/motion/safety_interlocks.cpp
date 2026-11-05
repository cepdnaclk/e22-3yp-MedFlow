#include "safety_interlocks.h"
#include "data_types.h"

// Simple software debounce: require N consecutive identical reads
static const uint8_t DEBOUNCE_COUNT = 3;
static uint8_t homeDebounce = 0;
static uint8_t endDebounce  = 0;
static bool homeState = false;
static bool endState  = false;

void safetyInit() {
    pinMode(LIMIT_SW_HOME_PIN, INPUT_PULLUP);
    pinMode(LIMIT_SW_END_PIN, INPUT_PULLUP);
}

bool isHomeLimitHit() {
    bool raw = (digitalRead(LIMIT_SW_HOME_PIN) == LOW);  // Active-LOW wiring
    if (raw == homeState) {
        homeDebounce = 0;
    } else {
        homeDebounce++;
        if (homeDebounce >= DEBOUNCE_COUNT) {
            homeState = raw;
            homeDebounce = 0;
        }
    }
    return homeState;
}

bool isEndLimitHit() {
    bool raw = (digitalRead(LIMIT_SW_END_PIN) == LOW);  // Active-LOW wiring
    if (raw == endState) {
        endDebounce = 0;
    } else {
        endDebounce++;
        if (endDebounce >= DEBOUNCE_COUNT) {
            endState = raw;
            endDebounce = 0;
        }
    }
    return endState;
}

uint8_t safetyCheckLimits(bool currentDirection) {
    bool homeHit = isHomeLimitHit();
    bool endHit  = isEndLimitHit();

    // Forward (infusing) and hit the end-of-travel switch
    if (endHit && currentDirection) {
        stepperStop();
        return STATE_ALARM_LIMIT_END;
    }

    // Retracting and hit the home switch
    if (homeHit && !currentDirection) {
        stepperStop();
        return STATE_ALARM_LIMIT_HOME;
    }

    return STATE_IDLE;  // No alarm
}

