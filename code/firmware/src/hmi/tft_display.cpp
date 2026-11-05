#include "tft_display.h"
#include "config.h"

static SPIClass tftSPI(FSPI);
static Adafruit_ILI9341 tft(&tftSPI, TFT_DC, TFT_CS, TFT_RST);

void tftInit() {
    tftSPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
    tft.begin(8000000);  // 8 MHz SPI clock
    tft.setRotation(1);  // Landscape orientation
    Serial.println("[TFT] ILI9341 display initialized (320x240 landscape).");
}

void tftDrawFrame() {
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

void tftUpdateTelemetry(SystemState state, unsigned long steps, float force, int distance, bool homeHit, bool endHit) {
    tft.setTextSize(2);

    // State
    tft.fillRect(130, 55, 175, 22, ILI9341_BLACK);
    tft.setCursor(130, 55);
    switch (state) {
        case STATE_INFUSING:
            tft.setTextColor(ILI9341_GREEN);   tft.print("INFUSING");    break;
        case STATE_RETRACTING:
            tft.setTextColor(ILI9341_YELLOW);  tft.print("RETRACTING");  break;
        case STATE_ALARM_OCCLUSION:
            tft.setTextColor(ILI9341_RED);     tft.print("OCCLUSION!");  break;
        case STATE_ALARM_LIMIT_END:
            tft.setTextColor(ILI9341_RED);     tft.print("END REACHED"); break;
        case STATE_ALARM_LIMIT_HOME:
            tft.setTextColor(ILI9341_MAGENTA); tft.print("HOME REACHED");break;
        default:
            tft.setTextColor(ILI9341_WHITE);   tft.print("IDLE");        break;
    }

    // Steps
    tft.fillRect(130, 90, 175, 22, ILI9341_BLACK);
    tft.setCursor(130, 90);
    tft.setTextColor(ILI9341_CYAN);
    tft.print(steps);

    // Force
    tft.fillRect(130, 125, 175, 22, ILI9341_BLACK);
    tft.setCursor(130, 125);
    tft.setTextColor(force > (OCCLUSION_LIMIT_GRAMS * 0.8f) ? ILI9341_RED : ILI9341_MAGENTA);
    tft.print(force, 0);
    tft.print(" g");

    // Distance
    tft.fillRect(130, 160, 175, 22, ILI9341_BLACK);
    tft.setCursor(130, 160);
    tft.setTextColor(ILI9341_WHITE);
    if (distance >= 0) {
        tft.print(distance);
        tft.print(" mm");
    } else {
        tft.print("OOR");
    }

    // Limit switches
    tft.fillRect(130, 195, 175, 22, ILI9341_BLACK);
    tft.setCursor(130, 195);
    tft.setTextColor(homeHit ? ILI9341_YELLOW : ILI9341_DARKGREY);
    tft.print("H:"); tft.print(homeHit ? "1 " : "0 ");
    tft.setTextColor(endHit ? ILI9341_RED : ILI9341_DARKGREY);
    tft.print("E:"); tft.print(endHit ? "1" : "0");
}

