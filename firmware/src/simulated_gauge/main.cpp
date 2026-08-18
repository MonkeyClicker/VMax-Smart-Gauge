#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>

#include "LvglPort.h"
#include "SimulatedEngineData.h"

using namespace esp_panel::board;
using namespace esp_panel::drivers;

namespace {
constexpr uint32_t UPDATE_INTERVAL_MS = 100;
Board board;
EngineSimulator simulator;
uint32_t lastUpdateMs = 0;

lv_obj_t *rpmArc;
lv_obj_t *rpmValue;
lv_obj_t *speedValue;
lv_obj_t *trimValue;
lv_obj_t *tempValue;
lv_obj_t *oilValue;
lv_obj_t *voltageValue;
lv_obj_t *fuelFlowValue;
lv_obj_t *hoursValue;
lv_obj_t *statusPill;
lv_obj_t *statusLabel;
lv_style_t cardStyle;
lv_style_t labelStyle;

lv_obj_t *makeValueCard(lv_obj_t *parent, const char *title, int x, int y,
                        int width, int height, lv_obj_t **valueLabel) {
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_add_style(card, &cardStyle, 0);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, width, height);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *caption = lv_label_create(card);
    lv_obj_add_style(caption, &labelStyle, 0);
    lv_label_set_text(caption, title);
    lv_obj_align(caption, LV_ALIGN_TOP_LEFT, 0, 0);

    *valueLabel = lv_label_create(card);
    lv_obj_set_style_text_color(*valueLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(*valueLabel, &lv_font_montserrat_24, 0);
    lv_obj_align(*valueLabel, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    return card;
}

void createGaugeScreen() {
    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x071019), 0);

    lv_style_init(&cardStyle);
    lv_style_set_bg_color(&cardStyle, lv_color_hex(0x101E2A));
    lv_style_set_bg_opa(&cardStyle, LV_OPA_COVER);
    lv_style_set_border_color(&cardStyle, lv_color_hex(0x294153));
    lv_style_set_border_width(&cardStyle, 1);
    lv_style_set_radius(&cardStyle, 12);
    lv_style_set_pad_all(&cardStyle, 12);

    lv_style_init(&labelStyle);
    lv_style_set_text_color(&labelStyle, lv_color_hex(0x8FAABD));
    lv_style_set_text_font(&labelStyle, &lv_font_montserrat_14);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "V MAX SMART GAUGE");
    lv_obj_set_style_text_color(title, lv_color_hex(0xE7F5FF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(title, 24, 16);

    lv_obj_t *mode = lv_label_create(screen);
    lv_label_set_text(mode, "SIMULATED DATA");
    lv_obj_set_style_text_color(mode, lv_color_hex(0x40D9FF), 0);
    lv_obj_set_style_text_font(mode, &lv_font_montserrat_14, 0);
    lv_obj_align(mode, LV_ALIGN_TOP_RIGHT, -24, 20);

    rpmArc = lv_arc_create(screen);
    lv_obj_set_size(rpmArc, 300, 300);
    lv_obj_set_pos(rpmArc, 28, 82);
    lv_arc_set_range(rpmArc, 0, 6000);
    lv_arc_set_bg_angles(rpmArc, 135, 45);
    lv_obj_remove_style(rpmArc, nullptr, LV_PART_KNOB);
    lv_obj_clear_flag(rpmArc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(rpmArc, 24, LV_PART_MAIN);
    lv_obj_set_style_arc_color(rpmArc, lv_color_hex(0x243747), LV_PART_MAIN);
    lv_obj_set_style_arc_width(rpmArc, 24, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(rpmArc, lv_color_hex(0x00B7E8), LV_PART_INDICATOR);

    rpmValue = lv_label_create(screen);
    lv_obj_set_style_text_color(rpmValue, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(rpmValue, &lv_font_montserrat_48, 0);
    lv_obj_set_width(rpmValue, 240);
    lv_obj_set_style_text_align(rpmValue, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(rpmValue, 58, 176);

    lv_obj_t *rpmCaption = lv_label_create(screen);
    lv_label_set_text(rpmCaption, "RPM");
    lv_obj_add_style(rpmCaption, &labelStyle, 0);
    lv_obj_set_pos(rpmCaption, 164, 242);

    lv_obj_t *speedCaption = lv_label_create(screen);
    lv_label_set_text(speedCaption, "SPEED");
    lv_obj_add_style(speedCaption, &labelStyle, 0);
    lv_obj_set_pos(speedCaption, 144, 300);

    speedValue = lv_label_create(screen);
    lv_obj_set_style_text_color(speedValue, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(speedValue, &lv_font_montserrat_28, 0);
    lv_obj_set_width(speedValue, 220);
    lv_obj_set_style_text_align(speedValue, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(speedValue, 68, 325);

    makeValueCard(screen, "TRIM", 354, 76, 200, 92, &trimValue);
    makeValueCard(screen, "ENGINE TEMP", 570, 76, 200, 92, &tempValue);
    makeValueCard(screen, "OIL PRESSURE", 354, 184, 200, 92, &oilValue);
    makeValueCard(screen, "BATTERY", 570, 184, 200, 92, &voltageValue);
    makeValueCard(screen, "FUEL FLOW", 354, 292, 200, 92, &fuelFlowValue);
    makeValueCard(screen, "ENGINE HOURS", 570, 292, 200, 92, &hoursValue);

    statusPill = lv_obj_create(screen);
    lv_obj_set_size(statusPill, 416, 48);
    lv_obj_set_pos(statusPill, 354, 404);
    lv_obj_set_style_radius(statusPill, 24, 0);
    lv_obj_set_style_border_width(statusPill, 0, 0);
    lv_obj_clear_flag(statusPill, LV_OBJ_FLAG_SCROLLABLE);
    statusLabel = lv_label_create(statusPill);
    lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_16, 0);
    lv_obj_center(statusLabel);
}

void updateGauge(const SimulatedEngineData &data) {
    lv_arc_set_value(rpmArc, static_cast<int>(data.rpm));
    lv_label_set_text_fmt(rpmValue, "%d", static_cast<int>(data.rpm));
    lv_label_set_text_fmt(speedValue, "%.1f mph", data.speedMph);
    lv_label_set_text_fmt(trimValue, "%.0f %%", data.trimPercent);
    lv_label_set_text_fmt(tempValue, "%.0f F", data.coolantTempF);
    lv_label_set_text_fmt(oilValue, "%.0f psi", data.oilPressurePsi);
    lv_label_set_text_fmt(voltageValue, "%.1f V", data.batteryVoltage);
    lv_label_set_text_fmt(fuelFlowValue, "%.1f gph", data.fuelFlowGph);
    lv_label_set_text_fmt(hoursValue, "%.1f h", data.engineHours);
    lv_obj_set_style_bg_color(statusPill,
        lv_color_hex(data.warning ? 0xE23E3E : 0x159B62), 0);
    lv_label_set_text(statusLabel, data.warning ? "WARNING" : "ENGINE STATUS OK");
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xFFFFFF), 0);
}

[[noreturn]] void stopWithError(const char *message) {
    Serial.println(message);
    while (true) delay(1000);
}
} // namespace

void setup() {
    Serial.begin(115200);
    delay(1200);
    Serial.println("V MAX Smart Gauge - simulated display");

    if (!board.init()) stopWithError("Display board init failed");
    LCD *lcd = board.getLCD();
    if (lcd == nullptr) stopWithError("LCD not available");
    auto *bus = lcd->getBus();
    if (bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
        static_cast<BusRGB *>(bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
    }
    if (!board.begin()) stopWithError("Display board start failed");
    if (!startLvgl(board)) stopWithError("LVGL start failed");

    if (!lockLvgl()) stopWithError("LVGL lock failed");
    createGaugeScreen();
    updateGauge(simulator.update(millis()));
    unlockLvgl();
    Serial.println("Gauge ready: 800x480 simulated single-engine data");
}

void loop() {
    const uint32_t nowMs = millis();
    if (static_cast<uint32_t>(nowMs - lastUpdateMs) >= UPDATE_INTERVAL_MS) {
        lastUpdateMs = nowMs;
        if (lockLvgl()) {
            updateGauge(simulator.update(nowMs));
            unlockLvgl();
        }
    }
    delay(10);
}

