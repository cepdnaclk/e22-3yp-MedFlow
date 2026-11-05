#include "hx711_loadcell.h"

static HX711 scale;
static float lastKnownForce = 0.0f;

void loadCellInit() {
    scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
    scale.set_scale(LOADCELL_CALIBRATION_FACTOR);
    scale.set_offset(0);

    // Allow ADC to stabilize
    delay(200);

    if (scale.is_ready()) {
        scale.tare(10);  // Average 10 readings for baseline zero
        Serial.println("[HX711] Load cell initialized and tared.");
    } else {
        Serial.println("[HX711] WARNING: Load cell not responding on startup.");
    }
}

float loadCellReadGrams() {
    if (scale.is_ready()) {
        lastKnownForce = scale.get_units(1);  // Single-sample read
        if (lastKnownForce < 0.0f) {
            lastKnownForce = 0.0f;  // Clamp negative drift
        }
    }
    return lastKnownForce;
}

void loadCellTare() {
    if (scale.is_ready()) {
        scale.tare(10);
        lastKnownForce = 0.0f;
        Serial.println("[HX711] Tare complete.");
    }
}

bool loadCellIsOccluded(float forceGrams) {
    return forceGrams >= OCCLUSION_LIMIT_GRAMS;
}

