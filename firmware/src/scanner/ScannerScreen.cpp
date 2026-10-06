#include "ScannerScreen.h"
#include "../simulated_gauge/LvglPort.h"
#include <Arduino.h>
#include <freertos/queue.h>
#include <atomic>

namespace {
esp_panel::board::Board board;
QueueHandle_t mailbox = nullptr;
ScannerScreenStatus status;
std::atomic<uint32_t> refreshCount{0};
lv_obj_t *segments[20]{};
lv_obj_t *running = nullptr;
lv_obj_t *network = nullptr;
lv_obj_t *values[4]{};
lv_obj_t *detail = nullptr;
constexpr uint32_t CYAN = 0x24DCFF;
constexpr uint32_t INK = 0xEDF8FF;

lv_obj_t *text(const char *value, int x, int y, int width,
               const lv_font_t *font, uint32_t color) {
    auto *obj = lv_label_create(lv_scr_act());
    lv_label_set_text(obj, value);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_width(obj, width);
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(color), 0);
    return obj;
}

lv_obj_t *arc(int diameter, int start, int end, int thickness, uint32_t color) {
    auto *obj = lv_arc_create(lv_scr_act());
    lv_obj_set_size(obj, diameter, diameter);
    lv_obj_set_pos(obj, 400 - diameter / 2, 183 - diameter / 2);
    lv_obj_remove_style(obj, nullptr, LV_PART_KNOB);
    lv_obj_set_style_arc_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(obj, thickness, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(obj, false, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(obj, lv_color_hex(color), LV_PART_INDICATOR);
    lv_arc_set_rotation(obj, 270);
    lv_arc_set_bg_angles(obj, 0, 360);
    lv_arc_set_angles(obj, start, end);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    return obj;
}

void refresh(lv_timer_t *) {
    ++refreshCount;
    ScannerScreenStatus latest;
    if (xQueueReceive(mailbox, &latest, 0) == pdTRUE) status = latest;
    const uint32_t now = millis();
    const bool alive = now - status.heartbeatMs < 1500;
    const bool healthy = alive && status.canStatusValid && status.canRunning;
    // No independent animation: rotation advances only with receive-loop heartbeats.
    const int phase = (status.heartbeatMs / 80) % 20;
    for (int i = 0; i < 20; ++i) {
        const int age = (phase - i + 20) % 20;
        const uint32_t color = !healthy ? 0x23465B :
            age == 0 ? 0xE0FCFF : age < 3 ? CYAN : age < 6 ? 0x168BBB : 0x124266;
        lv_obj_set_style_arc_color(segments[i], lv_color_hex(color), LV_PART_INDICATOR);
    }
    lv_label_set_text(running, !alive ? "STALLED" : !status.canStatusValid ? "CHECK CAN" :
                      !status.canRunning ? "CAN STOPPED" : "RUNNING");
    lv_obj_set_style_text_color(running, lv_color_hex(healthy ? INK : 0xFF987E), 0);
    const char *traffic = !healthy ? "SCANNER NEEDS ATTENTION" :
        status.frames == 0 ? "WAITING FOR NETWORK" :
        now - status.lastFrameMs >= 5000 ? "NETWORK TRAFFIC STALE" : "RECEIVING NMEA DATA";
    lv_label_set_text(network, traffic);
    lv_label_set_text_fmt(values[0], "%llu", static_cast<unsigned long long>(status.frames));
    lv_label_set_text_fmt(values[1], "%lu", static_cast<unsigned long>(status.trackedPgns));
    if (status.canStatusValid)
        lv_label_set_text_fmt(values[2], "%lu", static_cast<unsigned long>(status.busErrors));
    else lv_label_set_text(values[2], "--");
    const uint32_t seconds = now / 1000;
    lv_label_set_text_fmt(values[3], "%02lu:%02lu:%02lu",
        static_cast<unsigned long>(seconds / 3600),
        static_cast<unsigned long>((seconds / 60) % 60), static_cast<unsigned long>(seconds % 60));
    lv_label_set_text_fmt(detail, "LISTEN ONLY  /  250 kbit/s  /  Lost frames: %lu",
                         static_cast<unsigned long>(status.losses));
    for (auto *value : values) {
        lv_point_t size;
        lv_txt_get_size(&size, lv_label_get_text(value), &lv_font_montserrat_24, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        lv_obj_set_style_text_font(value, size.x > 175 ? &lv_font_montserrat_14 : &lv_font_montserrat_24, 0);
    }
}
}

uint32_t scannerScreenRefreshCount() { return refreshCount.load(); }

void publishScannerScreen(const ScannerScreenStatus &value) {
    if (mailbox) xQueueOverwrite(mailbox, &value);
}

bool startScannerScreen() {
    mailbox = xQueueCreate(1, sizeof(ScannerScreenStatus));
    if (!mailbox || ESP.getPsramSize() == 0 || !board.init()) return false;
    auto *lcd = board.getLCD();
    if (!lcd) return false;
    auto *bus = lcd->getBus();
    if (bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB)
        static_cast<esp_panel::drivers::BusRGB *>(bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
    if (!board.begin() || !startLvgl(board) || !lockLvgl()) return false;
    auto *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x031226), 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    text("V MAX", 20, 12, 140, &lv_font_montserrat_28, CYAN);
    text("NMEA SCANNER", 180, 17, 250, &lv_font_montserrat_20, INK);
    text("LIVE MONITOR", 605, 20, 170, &lv_font_montserrat_14, 0x82B6D9);
    // Layered navy rings and curved turbine blades, all native LVGL shapes.
    arc(282, 0, 359, 2, 0x1476AD);
    arc(274, 0, 359, 3, CYAN);
    arc(226, 0, 359, 2, 0x28678A);
    for (int i = 0; i < 10; ++i) {
        auto *blade = arc(194, i * 36, i * 36 + 24, 26, 0x092B48);
        lv_obj_set_style_arc_opa(blade, LV_OPA_60, LV_PART_INDICATOR);
    }
    for (int i = 0; i < 20; ++i) segments[i] = arc(258, i * 18, i * 18 + 14, 23, 0x124266);
    running = text("STARTING", 260, 161, 280, &lv_font_montserrat_28, INK);
    text("RECEIVE LOOP", 290, 197, 220, &lv_font_montserrat_14, 0x82B6D9);
    network = text("INITIALIZING CAN", 80, 327, 640, &lv_font_montserrat_24, CYAN);
    const char *names[] = {"Frames", "Tracked PGNs", "CAN bus errors", "Uptime"};
    for (int i = 0; i < 4; ++i) {
        text(names[i], 20 + i * 195, 371, 175, &lv_font_montserrat_16, 0x82B6D9);
        values[i] = text(i == 3 ? "00:00:00" : "0", 20 + i * 195, 394, 175,
                         &lv_font_montserrat_24, INK);
    }
    detail = text("LISTEN ONLY  /  250 kbit/s", 20, 433, 760, &lv_font_montserrat_16, INK);
    text("Animation = scanner running   /   Traffic = received frames", 20, 458, 760,
         &lv_font_montserrat_14, 0x82B6D9);
    lv_timer_create(refresh, 80, nullptr);
    lv_obj_update_layout(screen);
    unsigned layoutErrors = 0;
    for (uint32_t i = 0; i < lv_obj_get_child_cnt(screen); ++i) {
        auto *child = lv_obj_get_child(screen, i);
        if (!lv_obj_check_type(child, &lv_label_class)) continue;
        const auto *font = lv_obj_get_style_text_font(child, LV_PART_MAIN);
        lv_point_t size;
        lv_txt_get_size(&size, lv_label_get_text(child), font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        lv_area_t area;
        lv_obj_get_coords(child, &area);
        if (size.x > lv_obj_get_width(child) || area.x1 < 0 || area.y1 < 0 || area.x2 >= 800 || area.y2 >= 480)
            ++layoutErrors;
    }
    unlockLvgl();
    Serial.printf("Scanner screen label bounds/text fit: %u errors\n", layoutErrors);
    Serial.println("Scanner screen ready: blue turbine 800x480");
    return true;
}
