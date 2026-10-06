#include "GaugeUi.h"
#include "LvglPort.h"
#include <Arduino.h>
#include <lvgl.h>
#include "../../test/host/simulated_gauge_checks.h"

namespace {
enum class Page : uint8_t { Engine, Performance, Trip, Diagnostics, Settings };
enum class Units : uint8_t { US, Marine, Metric };
enum Field : uint8_t { Speed, Trim, Flow, Temp, Oil, Course, Fuel, Voltage, Hours,
                       Economy, Rpm, Best, Distance, Used, Average, MaxSpeed, MaxRpm, Elapsed, Count };
constexpr const char *PAGE_NAMES[] = {"ENGINE", "PERFORMANCE", "TRIP", "DIAGNOSTICS", "SETTINGS"};
constexpr const char *SCENARIO_NAMES[] = {"NORMAL", "WARNING", "OFFLINE", "UNAVAILABLE"};
EngineSimulator *engine = nullptr;
Page page = Page::Engine;
Page requestedPage = Page::Engine;
Units units = Units::US;
bool night = false;
bool rebuild = false;
bool acknowledged = false;
bool resetArmed = false;
uint32_t resetArmedMs = 0;
uint32_t touchCount = 0;
uint32_t lastTableMs = 0;
DemoScenario requestedScenario = DemoScenario::Normal;
bool scenarioPending = false;
bool tripResetPending = false;
lv_obj_t *body = nullptr;
lv_obj_t *banner = nullptr;
lv_obj_t *bannerText = nullptr;
lv_obj_t *headerStatus = nullptr;
lv_obj_t *nav[5]{};
lv_obj_t *values[Count]{};
lv_obj_t *tach = nullptr;
lv_obj_t *table = nullptr;
lv_obj_t *bestDetail = nullptr;
lv_obj_t *diagnosticInput = nullptr;
lv_obj_t *diagnosticSystem = nullptr;
lv_obj_t *ackButton = nullptr;
lv_obj_t *resetButton = nullptr;

uint32_t bg() { return night ? 0x03080E : 0x071019; }
uint32_t cardBg() { return night ? 0x0A1420 : 0x101E2A; }
uint32_t ink() { return night ? 0xA6BBCB : 0xE7F5FF; }
uint32_t muted() { return night ? 0x607889 : 0x8FAABD; }
uint32_t accent() { return night ? 0x2C86A0 : 0x40D9FF; }
const char *speedUnit() { return units == Units::US ? "mph" : units == Units::Marine ? "kn" : "km/h"; }
const char *distanceUnit() { return units == Units::US ? "mi" : units == Units::Marine ? "nm" : "km"; }
const char *fuelUnit() { return units == Units::Metric ? "L" : "gal"; }
const char *flowUnit() { return units == Units::Metric ? "L/h" : "gph"; }
const char *economyUnit() { return units == Units::US ? "mpg" : units == Units::Marine ? "nm/gal" : "km/L"; }
float distanceFactor() { return units == Units::US ? 1 : units == Units::Marine ? 0.868976F : 1.609344F; }
float fuelFactor() { return units == Units::Metric ? 3.7854118F : 1; }
float economyFactor() { return distanceFactor() / fuelFactor(); }

lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y, int width,
                const lv_font_t *font = &lv_font_montserrat_14, uint32_t color = 0) {
    auto *obj = lv_label_create(parent);
    lv_label_set_text(obj, text);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_width(obj, width);
    lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(color ? color : ink()), 0);
    return obj;
}

lv_obj_t *panel(lv_obj_t *parent, int x, int y, int w, int h) {
    auto *obj = lv_obj_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_bg_color(obj, lv_color_hex(cardBg()), 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(0x294153), 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_radius(obj, 10, 0);
    lv_obj_set_style_pad_all(obj, 10, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

lv_obj_t *button(lv_obj_t *parent, const char *text, int x, int y, int w, int h,
                 lv_event_cb_t callback, uintptr_t id = 0, bool selected = false) {
    auto *obj = lv_btn_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_radius(obj, 8, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(selected ? 0x175675 : cardBg()), 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x236986), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(selected ? accent() : 0x294153), 0);
    auto *caption = label(obj, text, 0, 0, w - 16, &lv_font_montserrat_16);
    lv_obj_set_style_text_align(caption, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(caption);
    lv_obj_add_event_cb(obj, callback, LV_EVENT_CLICKED, reinterpret_cast<void *>(id));
    return obj;
}

void card(Field field, const char *caption, int x, int y, int w, int h = 92) {
    auto *obj = panel(body, x, y, w, h);
    label(obj, caption, 0, 0, w - 22, &lv_font_montserrat_14, muted());
    values[field] = label(obj, "--", 0, 33, w - 22, &lv_font_montserrat_24);
}

void pageEvent(lv_event_t *event) {
    ++touchCount;
    requestedPage = static_cast<Page>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));
}
void scenarioEvent(lv_event_t *event) {
    ++touchCount;
    requestedScenario = static_cast<DemoScenario>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));
    scenarioPending = true;
    acknowledged = false;
    rebuild = true;
}
void unitsEvent(lv_event_t *event) {
    ++touchCount;
    units = static_cast<Units>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));
    rebuild = true;
}
void themeEvent(lv_event_t *) { ++touchCount; night = !night; rebuild = true; }
void ackEvent(lv_event_t *) { ++touchCount; acknowledged = true; }
void resetEvent(lv_event_t *) {
    ++touchCount;
    if (resetArmed && millis() - resetArmedMs < 5000) {
        tripResetPending = true;
        resetArmed = false;
    } else {
        resetArmed = true;
        resetArmedMs = millis();
    }
}

void createEngine() {
    tach = lv_arc_create(body);
    lv_obj_set_pos(tach, 5, 0);
    lv_obj_set_size(tach, 262, 262);
    lv_arc_set_range(tach, 0, 6000);
    lv_arc_set_bg_angles(tach, 135, 45);
    lv_obj_remove_style(tach, nullptr, LV_PART_KNOB);
    lv_obj_clear_flag(tach, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(tach, 20, LV_PART_MAIN);
    lv_obj_set_style_arc_color(tach, lv_color_hex(0x243747), LV_PART_MAIN);
    lv_obj_set_style_arc_width(tach, 20, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(tach, lv_color_hex(accent()), LV_PART_INDICATOR);
    label(body, "0", 14, 217, 40, &lv_font_montserrat_14, muted());
    label(body, "6000", 214, 217, 58, &lv_font_montserrat_14, muted());
    values[Rpm] = label(body, "700", 37, 80, 200, &lv_font_montserrat_48);
    lv_obj_set_style_text_align(values[Rpm], LV_TEXT_ALIGN_CENTER, 0);
    auto *caption = label(body, "ENGINE RPM", 42, 145, 190, &lv_font_montserrat_16, muted());
    lv_obj_set_style_text_align(caption, LV_TEXT_ALIGN_CENTER, 0);
    label(body, "YAMAHA V MAX SHO 250", 32, 181, 225, &lv_font_montserrat_14, accent());
    card(Speed, "GPS SPEED", 284, 0, 156);
    card(Trim, "TRIM", 448, 0, 156);
    card(Flow, "FUEL FLOW", 612, 0, 164);
    card(Temp, "ENGINE TEMP", 284, 100, 156);
    card(Oil, "OIL PRESSURE", 448, 100, 156);
    card(Course, "COURSE", 612, 100, 164);
    card(Fuel, "FUEL LEVEL", 0, 230, 188, 82);
    card(Voltage, "BATTERY", 196, 230, 188, 82);
    card(Hours, "ENGINE HOURS", 392, 230, 188, 82);
    card(Economy, "ECONOMY", 588, 230, 188, 82);
}

void createPerformance() {
    card(Rpm, "ENGINE RPM", 0, 0, 188);
    card(Speed, "GPS SPEED", 196, 0, 188);
    card(Flow, "FUEL FLOW", 392, 0, 188);
    card(Economy, "CURRENT ECONOMY", 588, 0, 188);
    bestDetail = label(body, "Learning best cruise economy...", 10, 108, 756, &lv_font_montserrat_16, accent());
    table = lv_table_create(body);
    lv_obj_set_pos(table, 0, 142);
    lv_obj_set_size(table, 776, 172);
    lv_obj_clear_flag(table, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(table, lv_color_hex(cardBg()), 0);
    lv_obj_set_style_border_width(table, 0, 0);
    lv_obj_set_style_pad_all(table, 0, 0);
    lv_obj_set_style_text_font(table, &lv_font_montserrat_14, LV_PART_ITEMS);
    lv_obj_set_style_text_color(table, lv_color_hex(ink()), LV_PART_ITEMS);
    lv_obj_set_style_bg_color(table, lv_color_hex(cardBg()), LV_PART_ITEMS);
    lv_obj_set_style_border_color(table, lv_color_hex(0x294153), LV_PART_ITEMS);
    lv_obj_set_style_pad_top(table, 4, LV_PART_ITEMS);
    lv_obj_set_style_pad_bottom(table, 4, LV_PART_ITEMS);
    lv_table_set_col_cnt(table, 5);
    lv_table_set_row_cnt(table, 7);
    const char *headers[] = {"RPM BAND", speedUnit(), flowUnit(), economyUnit(), "TRIM"};
    for (int c = 0; c < 5; ++c) {
        lv_table_set_col_width(table, c, c == 0 ? 168 : 152);
        lv_table_set_cell_value(table, 0, c, headers[c]);
    }
    for (int r = 0; r < 6; ++r) {
        lv_table_set_cell_value_fmt(table, r + 1, 0, "%d-%d", 2000 + 500 * r, 2499 + 500 * r);
        for (int c = 1; c < 5; ++c) lv_table_set_cell_value(table, r + 1, c, "--");
    }
    lastTableMs = 0;
}

void createTrip() {
    card(Elapsed, "ELAPSED TIME", 0, 0, 252);
    card(Distance, "TRIP DISTANCE", 262, 0, 252);
    card(Used, "FUEL USED", 524, 0, 252);
    card(Average, "AVERAGE ECONOMY", 0, 102, 252);
    card(MaxSpeed, "MAXIMUM SPEED", 262, 102, 252);
    card(MaxRpm, "MAXIMUM RPM", 524, 102, 252);
    bestDetail = label(body, "Best cruise: learning...", 10, 213, 756, &lv_font_montserrat_16, accent());
    label(body, "Trip totals are kept for this session only.", 10, 259, 468, &lv_font_montserrat_14, muted());
    resetButton = button(body, "RESET TRIP", 524, 252, 252, 56, resetEvent);
}

void createDiagnostics() {
    auto *input = panel(body, 0, 0, 384, 246);
    label(input, "DATA & CONNECTIONS", 0, 0, 362, &lv_font_montserrat_16, accent());
    diagnosticInput = label(input, "", 0, 34, 362);
    lv_obj_set_style_text_line_space(diagnosticInput, 8, 0);
    auto *system = panel(body, 394, 0, 382, 246);
    label(system, "DISPLAY & DEMO STATUS", 0, 0, 360, &lv_font_montserrat_16, accent());
    diagnosticSystem = label(system, "", 0, 34, 360);
    lv_obj_set_style_text_line_space(diagnosticSystem, 8, 0);
    ackButton = button(body, "ACKNOWLEDGE DEMO WARNING", 0, 258, 384, 52, ackEvent);
    button(body, "CHOOSE DEMO SCENARIO", 394, 258, 382, 52, pageEvent, static_cast<uintptr_t>(Page::Settings));
}

void createSettings() {
    label(body, "DISPLAY UNITS", 4, 0, 340, &lv_font_montserrat_14, muted());
    const char *unitNames[] = {"US / MPH", "MARINE / KNOTS", "METRIC"};
    for (int i = 0; i < 3; ++i)
        button(body, unitNames[i], i * 262, 25, 252, 48, unitsEvent, i, static_cast<int>(units) == i);
    button(body, night ? "NIGHT THEME - TAP FOR DAY" : "DAY THEME - TAP FOR NIGHT",
           0, 87, 776, 48, themeEvent);
    label(body, "DEMO SCENARIO", 4, 154, 340, &lv_font_montserrat_14, muted());
    for (int i = 0; i < 4; ++i)
        button(body, SCENARIO_NAMES[i], i * 196, 178, 188, 48, scenarioEvent, i,
               static_cast<int>(requestedScenario) == i);
    label(body, "Unavailable hides tank data. Offline marks engine/navigation values stale.",
          4, 241, 768, &lv_font_montserrat_14, muted());
    label(body, "Display preferences are session-only. Theme does not change backlight.",
          4, 266, 768, &lv_font_montserrat_14, muted());
    label(body, "CAN, SD, Wi-Fi, OTA and buzzer hardware remain inactive in this demo.",
          4, 291, 768, &lv_font_montserrat_14, muted());
}

void buildUi() {
    // One page at a time keeps the LVGL object heap bounded during repeated navigation.
    auto *screen = lv_scr_act();
    lv_obj_clean(screen);
    for (auto &value : values) value = nullptr;
    tach = table = bestDetail = diagnosticInput = diagnosticSystem = nullptr;
    ackButton = resetButton = nullptr;
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_hex(bg()), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    label(screen, "V MAX", 16, 13, 106, &lv_font_montserrat_20);
    label(screen, PAGE_NAMES[static_cast<int>(page)], 136, 16, 262, &lv_font_montserrat_16, muted());
    label(screen, "SIMULATED", 504, 16, 126, &lv_font_montserrat_14, accent());
    headerStatus = label(screen, "DEMO / ONLINE", 636, 16, 152, &lv_font_montserrat_14);
    banner = button(screen, "", 12, 50, 776, 32, pageEvent, static_cast<uintptr_t>(Page::Diagnostics));
    bannerText = lv_obj_get_child(banner, 0);
    lv_obj_set_style_text_font(bannerText, &lv_font_montserrat_14, 0);
    lv_obj_set_style_border_width(banner, 0, 0);
    body = lv_obj_create(screen);
    lv_obj_set_pos(body, 12, 94);
    lv_obj_set_size(body, 776, 316);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(body, 0, 0);
    lv_obj_set_style_pad_all(body, 0, 0);
    lv_obj_clear_flag(body, LV_OBJ_FLAG_SCROLLABLE);
    switch (page) {
        case Page::Engine: createEngine(); break;
        case Page::Performance: createPerformance(); break;
        case Page::Trip: createTrip(); break;
        case Page::Diagnostics: createDiagnostics(); break;
        case Page::Settings: createSettings(); break;
    }
    for (int i = 0; i < 5; ++i)
        nav[i] = button(screen, PAGE_NAMES[i], 12 + i * 156, 422, 148, 48,
                        pageEvent, i, static_cast<int>(page) == i);
    Serial.printf("UI page: %s | touch events=%lu\n", PAGE_NAMES[static_cast<int>(page)],
                  static_cast<unsigned long>(touchCount));
}

void number(Field field, float value, const char *unit, int decimals, bool stale = false, bool missing = false) {
    if (!values[field]) return;
    if (stale || missing) {
        lv_label_set_text(values[field], stale ? "STALE" : "--");
        lv_obj_set_style_text_color(values[field], lv_color_hex(stale ? 0xE5A34D : muted()), 0);
    } else {
        lv_label_set_text_fmt(values[field], decimals == 0 ? "%.0f %s" : decimals == 1 ? "%.1f %s" : decimals == 3 ? "%.3f %s" : "%.2f %s", value, unit);
        lv_obj_set_style_text_color(values[field], lv_color_hex(ink()), 0);
    }
}

unsigned checkLayout(lv_obj_t *obj) {
    unsigned failures = 0;
    if (lv_obj_check_type(obj, &lv_label_class)) {
        lv_point_t extent;
        lv_txt_get_size(&extent, lv_label_get_text(obj), lv_obj_get_style_text_font(obj, 0),
                       lv_obj_get_style_text_letter_space(obj, 0), lv_obj_get_style_text_line_space(obj, 0),
                       LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if (extent.x > lv_obj_get_content_width(obj)) {
            Serial.printf("UI clipped text: %s\n", lv_label_get_text(obj));
            ++failures;
        }
        lv_area_t bounds, parentBounds;
        lv_obj_get_coords(obj, &bounds);
        lv_obj_get_coords(lv_obj_get_parent(obj), &parentBounds);
        if (bounds.x1 < parentBounds.x1 || bounds.y1 < parentBounds.y1 ||
            bounds.x2 > parentBounds.x2 || bounds.y2 > parentBounds.y2) {
            Serial.printf("UI label outside parent: %s\n", lv_label_get_text(obj));
            ++failures;
        }
    }
    for (uint32_t i = 0; i < lv_obj_get_child_cnt(obj); ++i)
        failures += checkLayout(lv_obj_get_child(obj, i));
    return failures;
}
} // namespace

void createGaugeUi(EngineSimulator &simulator) {
    engine = &simulator;
    buildUi();
}

void handleGaugeUiCommand(char command) {
    // Bench automation uses the same deferred page/scenario paths as touch.
    if (command >= '1' && command <= '5') requestedPage = static_cast<Page>(command - '1');
    else if (command == 'N' || command == 'W' || command == 'O' || command == 'U') {
        requestedScenario = command == 'N' ? DemoScenario::Normal : command == 'W' ? DemoScenario::Warning :
                            command == 'O' ? DemoScenario::Offline : DemoScenario::Unavailable;
        scenarioPending = true;
        acknowledged = false;
        rebuild = true;
    } else if (command == 'u' || command == 'k' || command == 'm') {
        units = command == 'u' ? Units::US : command == 'k' ? Units::Marine : Units::Metric;
        rebuild = true;
    } else if (command == 'd' || command == 'n') {
        night = command == 'n';
        rebuild = true;
    } else if (command == 'a') acknowledged = true;
    else if (command == 'r') resetEvent(nullptr);
    else if (command == 't') Serial.println(simulatedGaugeChecks() ? "Simulator checks: PASS" : "Simulator checks: FAIL");
    else if (command == 'v') {
        lv_obj_update_layout(lv_scr_act());
        const unsigned failures = checkLayout(lv_scr_act());
        lv_mem_monitor_t memory;
        lv_mem_monitor(&memory);
        Serial.printf("UI layout: %s | failures=%u | LVGL free=%u | largest=%u\n",
                      failures ? "FAIL" : "PASS", failures, static_cast<unsigned>(memory.free_size),
                      static_cast<unsigned>(memory.free_biggest_size));
    }
}

void updateGaugeUi(uint32_t nowMs) {
    if (scenarioPending) {
        engine->setScenario(requestedScenario);
        scenarioPending = false;
    }
    if (tripResetPending) {
        engine->resetTrip();
        tripResetPending = false;
    }
    if (requestedPage != page || rebuild) {
        page = requestedPage;
        resetArmed = false;
        buildUi();
        rebuild = false;
    }
    // Refresh after queued commands so no old values flash on a newly selected page.
    const auto &data = engine->update(nowMs);
    const bool stale = data.scenario == DemoScenario::Offline;
    const bool missing = data.scenario == DemoScenario::Unavailable;
    const uint32_t statusColor = data.warning ? 0x8D2626 : stale ? 0x6B461E : 0x124834;
    lv_obj_set_style_bg_color(banner, lv_color_hex(statusColor), 0);
    lv_label_set_text(bannerText, data.warning ?
        (acknowledged ? "DEMO WARNING: HIGH ENGINE TEMPERATURE - ACKNOWLEDGED" : "DEMO WARNING: HIGH ENGINE TEMPERATURE - TAP FOR DETAILS") :
        stale ? "DEMO OFFLINE - LIVE VALUES ARE STALE; TRIP ACCUMULATION PAUSED" :
        missing ? "DEMO ONLINE - TANK DATA UNAVAILABLE" :
        "ENGINE STATUS OK - SIMULATED DATA ONLY");
    lv_label_set_text(headerStatus, stale ? "DEMO / OFFLINE" : "DEMO / ONLINE");
    lv_obj_set_style_text_color(headerStatus, lv_color_hex(stale ? 0xE5A34D : 0x67D5A4), 0);
    number(Rpm, data.rpm, "", 0, stale);
    number(Speed, data.speedMph * distanceFactor(), speedUnit(), 1, stale);
    number(Trim, data.trimPercent, "%", 0, stale);
    number(Flow, data.fuelFlowGph * fuelFactor(), flowUnit(), 1, stale);
    number(Temp, units == Units::Metric ? (data.coolantTempF - 32) / 1.8F : data.coolantTempF,
           units == Units::Metric ? "C" : "F", 0, stale);
    number(Oil, data.oilPressurePsi * (units == Units::Metric ? 6.894757F : 1),
           units == Units::Metric ? "kPa" : "psi", 0, stale);
    number(Course, data.courseDegrees, "deg", 0, stale);
    number(Fuel, data.fuelLevelPercent, "%", 0, stale, missing);
    number(Voltage, data.batteryVoltage, "V", 1, stale);
    number(Hours, data.engineHours, "h", 1, stale);
    number(Economy, data.economyMpg * economyFactor(), economyUnit(), 2, stale);
    number(Distance, data.tripDistanceMiles * distanceFactor(), distanceUnit(), 2);
    number(Used, data.tripFuelGallons * fuelFactor(), fuelUnit(), 3);
    number(Average, data.tripFuelGallons > 0.000001 ?
           data.tripDistanceMiles / data.tripFuelGallons * economyFactor() : 0,
           economyUnit(), 2, false, data.tripFuelGallons <= 0.000001);
    number(MaxSpeed, data.maxSpeedMph * distanceFactor(), speedUnit(), 1);
    number(MaxRpm, data.maxRpm, "rpm", 0);
    if (tach) {
        lv_arc_set_value(tach, stale ? 0 : static_cast<int>(data.rpm));
        lv_obj_set_style_arc_color(tach, lv_color_hex(stale ? muted() : accent()), LV_PART_INDICATOR);
    }
    if (values[Temp] && data.warning)
        lv_obj_set_style_text_color(values[Temp], lv_color_hex(0xFF7878), 0);
    if (values[Elapsed]) {
        const uint64_t seconds = data.tripElapsedMs / 1000;
        lv_label_set_text_fmt(values[Elapsed], "%02lu:%02lu:%02lu",
            static_cast<unsigned long>(seconds / 3600), static_cast<unsigned long>(seconds / 60 % 60),
            static_cast<unsigned long>(seconds % 60));
    }
    if (bestDetail) {
        if (data.bestEconomyMpg > 0)
            lv_label_set_text_fmt(bestDetail, "BEST CRUISE  %.2f %s  |  %.0f RPM  |  %.0f%% TRIM",
                data.bestEconomyMpg * economyFactor(), economyUnit(), data.bestRpm, data.bestTrimPercent);
        else lv_label_set_text(bestDetail, "BEST CRUISE  --  |  Learning above 2000 RPM");
    }
    if (table && (lastTableMs == 0 || nowMs - lastTableMs >= 1000)) {
        lastTableMs = nowMs;
        for (int r = 0; r < 6; ++r) {
            const auto &sample = data.cruise[r];
            if (!sample.observed) continue;
            lv_table_set_cell_value_fmt(table, r + 1, 1, "%.1f", sample.speedMph * distanceFactor());
            lv_table_set_cell_value_fmt(table, r + 1, 2, "%.1f", sample.fuelFlowGph * fuelFactor());
            lv_table_set_cell_value_fmt(table, r + 1, 3, "%.2f", sample.economyMpg * economyFactor());
            lv_table_set_cell_value_fmt(table, r + 1, 4, "%.0f%%", sample.trimPercent);
        }
    }
    if (diagnosticInput) {
        lv_label_set_text_fmt(diagnosticInput,
            "Input: GENERATED DEMO DATA\nScenario: %s\nData age: %.1f s\nEngine/GPS: %s\nTank: %s\nCAN / NMEA: NOT CONNECTED\nSD / Wi-Fi / OTA: INACTIVE",
            SCENARIO_NAMES[static_cast<int>(data.scenario)], data.dataAgeMs / 1000.0,
            stale ? "STALE" : "FRESH (SIMULATED)", missing ? "UNAVAILABLE" : stale ? "STALE" : "SIMULATED");
        lv_label_set_text_fmt(diagnosticSystem,
            "Build: multipage-demo v1\nUptime: %lu s\nFree heap: %u bytes\nFree PSRAM: %u bytes\nTouch events: %lu\nAlarm: %s\nBuzzer output: INACTIVE",
            static_cast<unsigned long>(nowMs / 1000), ESP.getFreeHeap(), ESP.getFreePsram(),
            static_cast<unsigned long>(touchCount), data.warning ? (acknowledged ? "ACKNOWLEDGED" : "DEMO WARNING") : "NONE");
        lv_obj_t *caption = lv_obj_get_child(ackButton, 0);
        lv_label_set_text(caption, acknowledged ? "DEMO WARNING ACKNOWLEDGED" : "ACKNOWLEDGE DEMO WARNING");
        if (data.warning && !acknowledged) lv_obj_clear_state(ackButton, LV_STATE_DISABLED);
        else lv_obj_add_state(ackButton, LV_STATE_DISABLED);
    }
    if (resetButton) {
        if (resetArmed && nowMs - resetArmedMs >= 5000) resetArmed = false;
        lv_label_set_text(lv_obj_get_child(resetButton, 0), resetArmed ? "TAP AGAIN TO RESET" : "RESET TRIP");
    }
}
