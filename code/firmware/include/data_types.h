#pragma once
#include <Arduino.h>

// ========================================================
// SYSTEM STATE MACHINE
// ========================================================
enum SystemState : uint8_t {
    STATE_IDLE              = 0,
    STATE_INFUSING          = 1,
    STATE_RETRACTING        = 2,
    STATE_ALARM_OCCLUSION   = 3,
    STATE_ALARM_LIMIT_END   = 4,
    STATE_ALARM_LIMIT_HOME  = 5
};

// ========================================================
// INTER-CORE TELEMETRY PACKET (Core 1 -> Core 0 via Queue)
// ========================================================
struct TelemetryPacket {
    SystemState state;
    unsigned long steps;
    float   weightGrams;
    int     distanceMm;
    float   flowRateMlH;
    bool    homeHit;
    bool    endHit;
};

// ========================================================
// MQTT COMMAND ENVELOPE (Core 0 -> Core 1 via Queue)
// ========================================================
struct CommandPacket {
    char    command[16];   // "INFUSE", "PAUSE", "RETRACT", "TARE_SCALE", "SET_RATE"
    float   param;         // Optional numeric parameter (e.g., new rate)
};

// ========================================================
// HELPER: State enum to human-readable string
// ========================================================
inline const char* stateToString(SystemState s) {
    switch (s) {
        case STATE_IDLE:            return "IDLE";
        case STATE_INFUSING:        return "INFUSING";
        case STATE_RETRACTING:      return "RETRACTING";
        case STATE_ALARM_OCCLUSION: return "ALARM_OCCLUSION";
        case STATE_ALARM_LIMIT_END: return "ALARM_LIMIT_END";
        case STATE_ALARM_LIMIT_HOME:return "ALARM_LIMIT_HOME";
        default:                    return "UNKNOWN";
    }
}

