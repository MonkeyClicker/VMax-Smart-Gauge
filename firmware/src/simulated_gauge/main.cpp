#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>

#include "LvglPort.h"
#include "SimulatedEngineData.h"
#include "GaugeUi.h"

using namespace esp_panel::board;
using namespace esp_panel::drivers;

namespace {
constexpr uint32_t UPDATE_INTERVAL_MS = 100;
Board board;
EngineSimulator simulator;
uint32_t lastUpdateMs = 0;
uint32_t lastHeartbeatMs = 0;

[[noreturn]] void stopWithError(const char *message) {
    while (true) {
        Serial.println(message);
        delay(1000);
    }
}
} // namespace

void setup() {
    Serial.begin(115200);
    const uint32_t serialStartMs = millis();
    while (!Serial && millis() - serialStartMs < 5000) delay(10);
    Serial.println("V MAX Smart Gauge - simulated display");
    Serial.printf("Flash=%u PSRAM=%u free PSRAM=%u\n",
                  ESP.getFlashChipSize(), ESP.getPsramSize(), ESP.getFreePsram());
    if (ESP.getPsramSize() == 0) stopWithError("PSRAM unavailable: check OPI PSRAM configuration");

    Serial.println("Initializing display board");
    if (!board.init()) stopWithError("Display board init failed");
    LCD *lcd = board.getLCD();
    if (lcd == nullptr) stopWithError("LCD not available");
    auto *bus = lcd->getBus();
    if (bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
        static_cast<BusRGB *>(bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
    }
    Serial.println("Starting LCD, touch, and backlight");
    if (!board.begin()) stopWithError("Display board start failed");
    Serial.println("Starting LVGL");
    if (!startLvgl(board)) stopWithError("LVGL start failed");

    if (!lockLvgl()) stopWithError("LVGL lock failed");
    createGaugeUi(simulator);
    updateGaugeUi(millis());
    unlockLvgl();
    Serial.println("Gauge ready: 800x480 five-page simulated gauge");
}

void loop() {
    const uint32_t nowMs = millis();
    if (static_cast<uint32_t>(nowMs - lastHeartbeatMs) >= 5000) {
        lastHeartbeatMs = nowMs;
        Serial.printf("Gauge alive: uptime=%lu ms free heap=%u free PSRAM=%u\n",
                      static_cast<unsigned long>(nowMs), ESP.getFreeHeap(), ESP.getFreePsram());
    }
    if (static_cast<uint32_t>(nowMs - lastUpdateMs) >= UPDATE_INTERVAL_MS) {
        lastUpdateMs = nowMs;
        if (lockLvgl()) {
            for (int i = 0; i < 16 && Serial.available(); ++i)
                handleGaugeUiCommand(static_cast<char>(Serial.read()));
            updateGaugeUi(nowMs);
            unlockLvgl();
        }
    }
    delay(10);
}

