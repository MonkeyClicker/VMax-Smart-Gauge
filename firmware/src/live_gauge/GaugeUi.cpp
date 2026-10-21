#include "GaugeUi.h"
#include "FuelInventory.h"
#include <Arduino.h>
#include <Preferences.h>
#include <lvgl.h>
#include <cstdio>
#include <cstring>

namespace liveui {
namespace {
enum Field { Rpm, Speed, Trim, Flow, Coolant, Oil, Course, Fuel, Volts, Hours, Economy,
             Water, Distance, Used, Average, MaxSpeed, MaxRpm, Elapsed, Best, Count };
constexpr const char *PAGES[] = {"ENGINE", "PERFORMANCE", "TRIP", "DIAGNOSTICS", "SETTINGS"};
Preferences preferences;
live::FuelInventory fuel;
uint32_t fuelPersistAt = 0;
bool fuelSaveFailed = false;
lv_obj_t *fuelTitle = nullptr;
bool persistFuel(const live::FuelInventory &candidate) {
    const auto record = candidate.record();
    return preferences.putBytes("fuelState", &record, sizeof(record)) == sizeof(record);
}
bool tankSavePending = false;
lv_obj_t *tankInput = nullptr, *tankMessage = nullptr;
uint8_t page = 0, requestedPage = 0, units = 0;
bool night = false, rebuild = false, saveSettings = false, tripReset = false, sourceReset = false;
bool splash = true, resetArmed = false, sourceArmed = false;
uint32_t splashStart = 0, armedAt = 0;
lv_obj_t *values[Count]{}, *banner = nullptr, *bannerText = nullptr, *status = nullptr;
lv_obj_t *body = nullptr, *dial = nullptr, *detail = nullptr, *diagnostic = nullptr;
lv_obj_t *splashProgress = nullptr, *resetCaption = nullptr, *sourceCaption = nullptr;
uint32_t background() { return night ? 0x03080E : 0x071019; }
uint32_t foreground() { return night ? 0x9BB4C7 : 0xE7F5FF; }
uint32_t cyan() { return night ? 0x287C96 : 0x40D9FF; }
const char *speedUnit() { return units == 0 ? "mph" : units == 1 ? "kn" : "km/h"; }
const char *distanceUnit() { return units == 0 ? "mi" : units == 1 ? "nm" : "km"; }
const char *economyUnit() { return units == 0 ? "mpg" : units == 1 ? "nm/gal" : "km/L"; }
double distanceScale() { return units == 0 ? 1.0 / 1609.344 : units == 1 ? 1.0 / 1852 : 0.001; }
double speedScale() { return distanceScale() * 3600; }
double fuelScale() { return units == 2 ? 1 : 1.0 / 3.785411784; }

lv_obj_t *label(lv_obj_t *parent, const char *text, int x, int y, int width,
                const lv_font_t *font = &lv_font_montserrat_14, uint32_t color = 0) {
    auto *o = lv_label_create(parent); lv_label_set_text(o, text);
    lv_obj_set_pos(o, x, y); lv_obj_set_width(o, width);
    lv_label_set_long_mode(o, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_font(o, font, 0);
    lv_obj_set_style_text_color(o, lv_color_hex(color ? color : foreground()), 0);
    return o;
}
lv_obj_t *panel(lv_obj_t *parent, int x, int y, int w, int h) {
    auto *o = lv_obj_create(parent); lv_obj_set_pos(o, x, y); lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, lv_color_hex(night ? 0x09131D : 0x101E2A), 0);
    lv_obj_set_style_border_color(o, lv_color_hex(0x294153), 0);
    lv_obj_set_style_border_width(o, 1, 0); lv_obj_set_style_radius(o, 10, 0);
    lv_obj_set_style_pad_all(o, 10, 0); lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE);
    return o;
}
lv_obj_t *button(lv_obj_t *parent, const char *text, int x, int y, int w, int h,
                 lv_event_cb_t event, uintptr_t id = 0, bool selected = false) {
    auto *o = lv_btn_create(parent); lv_obj_set_pos(o, x, y); lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, lv_color_hex(selected ? 0x175675 : 0x101E2A), 0);
    lv_obj_set_style_shadow_width(o, 0, 0); lv_obj_set_style_radius(o, 8, 0);
    auto *caption = label(o, text, 0, 0, w - 16, &lv_font_montserrat_16);
    lv_obj_set_style_text_align(caption, LV_TEXT_ALIGN_CENTER, 0); lv_obj_center(caption);
    lv_obj_add_event_cb(o, event, LV_EVENT_CLICKED, reinterpret_cast<void *>(id)); return caption;
}
void pageEvent(lv_event_t *e) { requestedPage = reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)); }
void unitsEvent(lv_event_t *e) {
    units = reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)); rebuild = saveSettings = true;
}
void themeEvent(lv_event_t *) { night = !night; rebuild = saveSettings = true; }
void resetEvent(lv_event_t *) {
    const uint32_t now = millis();
    if (resetArmed && uint32_t(now - armedAt) < 5000) { tripReset = true; resetArmed = false; }
    else { resetArmed = true; armedAt = now; }
}
void sourcesEvent(lv_event_t *) {
    const uint32_t now = millis();
    if (sourceArmed && uint32_t(now - armedAt) < 5000) { sourceReset = true; sourceArmed = false; }
    else { sourceArmed = true; armedAt = now; }
}
void tankHint() {
    if (page == 5) lv_label_set_text(tankMessage, "Enter 1-9999 whole US gallons.");
    else lv_label_set_text_fmt(tankMessage, "Enter 0-%lu whole US gallons.", (unsigned long)fuel.capacity);
}
void tankKeyEvent(lv_event_t *e) {
    const uintptr_t key = reinterpret_cast<uintptr_t>(lv_event_get_user_data(e));
    if (key == 10) lv_textarea_set_text(tankInput, "");
    else if (key == 11) lv_textarea_del_char(tankInput);
    else {
        const char digit[] = {char('0' + key), 0};
        lv_textarea_add_text(tankInput, digit);
    }
    tankHint();
}
void fullEvent(lv_event_t *) {
    char text[16]; snprintf(text, sizeof(text), "%lu", (unsigned long)fuel.capacity);
    lv_textarea_set_text(tankInput, text);
}
void tankSaveEvent(lv_event_t *) { tankSavePending = true; }
void card(Field field, const char *title, int x, int y, int w, int h = 92) {
    auto *o = panel(body, x, y, w, h);
    label(o, title, 0, 0, w - 22, &lv_font_montserrat_14, 0x8FAABD);
    values[field] = label(o, "--", 0, 33, w - 22, &lv_font_montserrat_24);
}
void build() {
    std::memset(values, 0, sizeof(values)); dial = detail = diagnostic = nullptr;
    resetCaption = sourceCaption = tankInput = tankMessage = fuelTitle = nullptr;
    auto *screen = lv_scr_act(); lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(background()), 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    label(screen, "V MAX", 16, 13, 110, &lv_font_montserrat_20, cyan());
    label(screen, page == 5 ? "TANK CAPACITY" : page == 6 ? "SET FUEL AMOUNT" : PAGES[page], 140, 16, 285, &lv_font_montserrat_16);
    char version[32]; snprintf(version, sizeof(version), "LIVE v%s", live::VERSION);
    label(screen, version, 500, 17, 130, &lv_font_montserrat_14, cyan());
    status = label(screen, "WAITING", 640, 17, 148, &lv_font_montserrat_14);
    banner = panel(screen, 12, 50, 776, 32); lv_obj_set_style_pad_all(banner, 6, 0);
    bannerText = label(banner, "Waiting for NMEA 2000 data", 0, 0, 758);
    lv_obj_add_flag(banner, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(banner, pageEvent, LV_EVENT_CLICKED, reinterpret_cast<void *>(3));
    body = lv_obj_create(screen); lv_obj_set_pos(body, 12, 94); lv_obj_set_size(body, 776, 316);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, 0); lv_obj_set_style_border_width(body, 0, 0);
    lv_obj_set_style_pad_all(body, 0, 0); lv_obj_clear_flag(body, LV_OBJ_FLAG_SCROLLABLE);
    for (uintptr_t i = 0; i < 5; ++i) button(screen, PAGES[i], 12 + i * 156, 422, 148, 48, pageEvent, i, i == (page >= 4 ? 4 : page));
    if (page == 0) {
        dial = lv_arc_create(body); lv_obj_set_pos(dial, 5, 0); lv_obj_set_size(dial, 262, 220);
        lv_arc_set_range(dial, 0, 6000); lv_arc_set_rotation(dial, 135); lv_arc_set_bg_angles(dial, 0, 270);
        lv_obj_remove_style(dial, nullptr, LV_PART_KNOB); lv_obj_clear_flag(dial, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_arc_color(dial, lv_color_hex(cyan()), LV_PART_INDICATOR);
        values[Rpm] = label(body, "--", 20, 70, 232, &lv_font_montserrat_48);
        lv_obj_set_style_text_align(values[Rpm], LV_TEXT_ALIGN_CENTER, 0);
        auto *caption = label(body, "ENGINE RPM", 20, 132, 232, &lv_font_montserrat_16, cyan());
        lv_obj_set_style_text_align(caption, LV_TEXT_ALIGN_CENTER, 0);
        label(body, "YAMAHA V MAX SHO 250", 30, 174, 230);
        card(Speed, speedUnit(), 284, 0, 156); card(Trim, "Trim %", 448, 0, 156);
        card(Flow, units == 2 ? "Fuel L/h" : "Fuel gph", 612, 0, 164);
        card(Coolant, units == 2 ? "Coolant C" : "Coolant F", 284, 100, 156);
        card(Oil, units == 2 ? "Oil kPa" : "Oil psi", 448, 100, 156);
        card(Course, "Course true", 612, 100, 164);
        card(Fuel, "Fuel US gal", 0, 230, 188, 82);
        fuelTitle = lv_obj_get_child(lv_obj_get_parent(values[Fuel]), 0); card(Volts, "Charging V", 196, 230, 188, 82);
        card(Hours, "Engine hours", 392, 230, 188, 82); card(Economy, economyUnit(), 588, 230, 188, 82);
    } else if (page == 1) {
        card(Speed, speedUnit(), 0, 0, 188); card(Rpm, "Engine RPM", 196, 0, 188);
        card(Flow, units == 2 ? "Fuel L/h" : "Fuel gph", 392, 0, 188); card(Economy, economyUnit(), 588, 0, 188);
        char bestTitle[48]; snprintf(bestTitle, sizeof(bestTitle), "Session best %s", economyUnit());
        card(Best, bestTitle, 0, 106, 384, 110);
        card(Water, units == 2 ? "Water C" : "Water F", 394, 106, 382, 110);
        detail = label(body, "", 8, 234, 760, &lv_font_montserrat_16);
        label(body, "Best requires 5 s steady RPM/speed and healthy CAN. GPS speed over ground.", 8, 286, 760);
    } else if (page == 2) {
        card(Elapsed, "Valid running time", 0, 0, 252); card(Distance, distanceUnit(), 262, 0, 252);
        card(Used, units == 2 ? "Fuel used L" : "Fuel used gal", 524, 0, 252);
        char maxTitle[32]; snprintf(maxTitle, sizeof(maxTitle), "Max speed %s", speedUnit());
        card(Average, economyUnit(), 0, 102, 252); card(MaxSpeed, maxTitle, 262, 102, 252);
        card(MaxRpm, "Max RPM", 524, 102, 252);
        detail = label(body, "", 8, 216, 760, &lv_font_montserrat_16);
        label(body, "Session only; resets on power loss.\nTotals pause if inputs/CAN are unreliable.", 8, 256, 510);
        resetCaption = button(body, "RESET TRIP", 524, 252, 252, 56, resetEvent);
    } else if (page == 3) {
        auto *o = panel(body, 0, 0, 776, 310);
        diagnostic = label(o, "", 0, 0, 752, &lv_font_montserrat_16);
        lv_obj_set_style_text_line_space(diagnostic, 7, 0);
    } else if (page == 4) {
        label(body, "DISPLAY UNITS", 4, 0, 300, &lv_font_montserrat_16, cyan());
        const char *names[] = {"US: mph / gal", "MARINE: kn / gal", "METRIC: km/h / L"};
        for (uintptr_t i = 0; i < 3; ++i) button(body, names[i], i * 262, 28, 252, 48, unitsEvent, i, units == i);
        button(body, night ? "NIGHT COLORS - TAP FOR DAY" : "DAY COLORS - TAP FOR NIGHT", 0, 86, 776, 44, themeEvent);
        char capacity[80];
        if (fuel.capacity) snprintf(capacity, sizeof(capacity), "TANK CAPACITY: %lu US gal - TAP TO EDIT", (unsigned long)fuel.capacity);
        else snprintf(capacity, sizeof(capacity), "SET TANK CAPACITY - NOT CONFIGURED");
        button(body, capacity, 0, 140, 776, 44, pageEvent, 5);
        auto *amount = button(body, fuel.capacity ? "SET / RESET FUEL AMOUNT" : "SET TANK CAPACITY FIRST", 0, 194, 776, 44, pageEvent, 6);
        if (!fuel.capacity) lv_obj_add_state(lv_obj_get_parent(amount), LV_STATE_DISABLED);
        sourceCaption = button(body, "REDETECT SOURCES + RESET TRIP", 0, 248, 776, 44, sourcesEvent);
        label(body, "Fuel amount is saved. Fresh network levels override the estimate.", 8, 298, 760);
    } else {
        label(body, page == 5 ? "TANK CAPACITY" : "SET FUEL AMOUNT", 4, 4, 306, &lv_font_montserrat_20, cyan());
        label(body, "Whole US gallons", 4, 38, 300, &lv_font_montserrat_16);
        tankInput = lv_textarea_create(body); lv_obj_set_pos(tankInput, 0, 70);
        lv_obj_set_size(tankInput, 302, 64); lv_textarea_set_one_line(tankInput, true);
        lv_textarea_set_accepted_chars(tankInput, "0123456789"); lv_textarea_set_max_length(tankInput, 4);
        lv_obj_set_style_text_font(tankInput, &lv_font_montserrat_28, 0);
        lv_obj_set_style_text_color(tankInput, lv_color_hex(foreground()), 0);
        lv_obj_set_style_bg_color(tankInput, lv_color_hex(0x101E2A), 0);
        char text[16] = "";
        if (page == 5 && fuel.capacity) snprintf(text, sizeof(text), "%lu", (unsigned long)fuel.capacity);
        else if (page == 6 && fuel.known) snprintf(text, sizeof(text), "%.0f", fuel.gallons);
        lv_textarea_set_text(tankInput, text); lv_textarea_set_placeholder_text(tankInput, "Not set");
        tankMessage = label(body, "", 4, 151, 306); tankHint();
        if (page == 5) label(body, "Fresh network levels take priority.\nCapacity does not set fuel contents.", 4, 180, 306);
        else {
            button(body, "FULL", 0, 185, 302, 44, fullEvent);
        }
        const char *keys[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "CLEAR", "0", "BACK"};
        for (uintptr_t i = 0; i < 12; ++i) {
            const uintptr_t key = i < 9 ? i + 1 : i == 9 ? 10 : i == 10 ? 0 : 11;
            button(body, keys[i], 330 + (i % 3) * 146, (i / 3) * 64, 138, 56, tankKeyEvent, key);
        }
        button(body, "CANCEL", 0, 240, 146, 56, pageEvent, 4);
        button(body, "SAVE", 156, 240, 146, 56, tankSaveEvent, 0, true);
        label(body, "Saved fuel amount. Fresh network levels take priority.", 330, 274, 440);

    }
}
void numeric(Field f, const live::Value &v, uint32_t now, double scale = 1, double offset = 0, unsigned decimals = 1) {
    if (!values[f]) return;
    const char *message = !v.seen ? "--" : !v.available ? "N/A" : !v.fresh(now) ? "STALE" : nullptr;
    if (message) lv_label_set_text(values[f], message);
    else lv_label_set_text_fmt(values[f], "%.*f", int(decimals), v.value * scale + offset);
    lv_obj_set_style_text_color(values[f], lv_color_hex(message ? 0x8FAABD : foreground()), 0);
}
void number(Field f, double v, unsigned decimals = 1) {
    live::Value value; value.set(v, 0); numeric(f, value, 0, 1, 0, decimals);
}
} // namespace

void start() {
    if (preferences.begin("live-gauge", false)) {
        units = preferences.getUChar("units", 0); if (units > 2) units = 0;
        night = preferences.getBool("night", false);
        fuel.capacity = preferences.getUInt("tankGal", 0);
        if (fuel.capacity > live::MAX_TANK_GALLONS) fuel.capacity = 0;
        live::FuelRecord record;
        if (preferences.getBytesLength("fuelState") == sizeof(record) &&
            preferences.getBytes("fuelState", &record, sizeof(record)) == sizeof(record)) fuel.restore(record);
    }
    auto *screen = lv_scr_act(); lv_obj_set_style_bg_color(screen, lv_color_hex(0x071019), 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    auto *ring = lv_spinner_create(screen, 1400, 70); lv_obj_set_pos(ring, 330, 38); lv_obj_set_size(ring, 140, 140);
    lv_obj_set_style_arc_color(ring, lv_color_hex(0x153248), LV_PART_MAIN);
    lv_obj_set_style_arc_color(ring, lv_color_hex(0x40D9FF), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(ring, 8, LV_PART_MAIN); lv_obj_set_style_arc_width(ring, 8, LV_PART_INDICATOR);
    auto *title = label(screen, "V MAX", 100, 195, 600, &lv_font_montserrat_48, 0xE7F5FF);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    auto *subtitle = label(screen, "S M A R T   G A U G E", 100, 260, 600, &lv_font_montserrat_24, 0x40D9FF);
    lv_obj_set_style_text_align(subtitle, LV_TEXT_ALIGN_CENTER, 0);
    char version[64]; snprintf(version, sizeof(version), "Firmware v%s  |  LIVE DATA", live::VERSION);
    auto *v = label(screen, version, 100, 322, 600, &lv_font_montserrat_16);
    lv_obj_set_style_text_align(v, LV_TEXT_ALIGN_CENTER, 0);
    splashProgress = label(screen, "Starting receiver - listening for NMEA 2000", 90, 396, 620, &lv_font_montserrat_16, 0x8FAABD);
    lv_obj_set_style_text_align(splashProgress, LV_TEXT_ALIGN_CENTER, 0);
    // Flush synchronously: begin the five seconds after the first frame is visible.
    lv_refr_now(nullptr); splashStart = millis();
}
void fault(const char *message) {
    splash = false; auto *screen = lv_scr_act(); lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x071019), 0);
    label(screen, "STARTUP FAULT", 60, 140, 680, &lv_font_montserrat_28, 0xFF756B);
    label(screen, message, 60, 210, 680, &lv_font_montserrat_20);
    label(screen, "Receiver stopped. Restart after checking the connection/configuration.", 60, 290, 680);
}
void update(const live::Snapshot &s, uint32_t now, uint32_t rawDrops, uint32_t logFailures, bool rawActive) {
    const auto &d = s.data; const auto &c = s.diagnostics; const auto &t = s.trip;
    fuel.update(d, c, now);
    if (splash) {
        if (uint32_t(now - splashStart) < 5000 && !d.critical(now) && (!c.statusValid || c.canReady)) {
            lv_label_set_text(splashProgress, c.frameSeen ? "Receiver active - preparing live gauges" : "Listening - waiting for NMEA 2000 data"); return;
        }
        splash = false; rebuild = true;
    }
    if (tankSavePending) {
        tankSavePending = false;
        uint32_t requested = 0;
        if ((page != 5 && page != 6) || !tankInput || !tankMessage) return;
        const char *text = lv_textarea_get_text(tankInput);
        live::FuelInventory candidate = fuel;
        bool valid = page == 5 ? live::parseTankGallons(text, requested) : live::parseInitialGallons(text, fuel.capacity, requested);
        if (valid) valid = page == 5 ? candidate.setCapacity(requested, d, now) : candidate.setInitial(requested);
        if (!valid) {
            if (page == 5) lv_label_set_text(tankMessage, "Invalid size / below fuel amount.");
            else tankHint();
        }
        else {
            candidate.update(d, c, now); // A fresh network reading overrides even a manual reset.
            if (!persistFuel(candidate)) lv_label_set_text(tankMessage, "Could not save. Please try again.");
            else { fuel = candidate; fuel.dirty = false; fuelSaveFailed = false; fuelPersistAt = now; requestedPage = 4; rebuild = true; }
        }
    }
    if (fuel.dirty && uint32_t(now - fuelPersistAt) >= 30000) {
        fuelSaveFailed = !persistFuel(fuel);
        if (!fuelSaveFailed) fuel.dirty = false;
        fuelPersistAt = now;
    }
    if (saveSettings) { preferences.putUChar("units", units); preferences.putBool("night", night); saveSettings = false; }
    if (requestedPage != page) { page = requestedPage; resetArmed = sourceArmed = false; rebuild = true; }
    if (rebuild) { rebuild = false; build(); }
    if ((resetArmed || sourceArmed) && uint32_t(now - armedAt) >= 5000) resetArmed = sourceArmed = false;
    if (resetCaption) lv_label_set_text(resetCaption, resetArmed ? "TAP TO CONFIRM" : "RESET TRIP");
    if (sourceCaption) lv_label_set_text(sourceCaption, sourceArmed ? "TAP TO CONFIRM - CLEARS TRIP" : "REDETECT SOURCES + RESET TRIP");
    const bool received = c.frameSeen && uint32_t(now - c.lastFrame) <= live::FRESH_MS;
    const bool engine = d.rpm.fresh(now), gps = d.speedMs.fresh(now);
    const bool fuelUncertain = fuel.known && !fuel.networkActive && fuel.incomplete;
    const char *message = !c.canReady ? "CAN receiver unavailable - check diagnostics" :
        d.critical(now) ? "ENGINE WARNING - check Yamaha instruments immediately" :
        !c.healthy(now) ? "CAN errors / receive loss - trip totals paused; tap for diagnostics" :
        d.warning(now) ? "Engine reports status flags - check Yamaha instruments" :
        !received ? "No recent NMEA 2000 traffic - check power and wiring" :
        !engine ? "Waiting for fresh engine RPM - unavailable fields remain blank" :
        !gps ? "Engine data received - waiting for GPS speed" :
        !d.flowLph.fresh(now) ? "Engine + GPS received - fuel flow unavailable; trip totals paused" :
        fuelUncertain ? "Fuel estimate has gaps - confirm/reset amount in Settings" :
        "Live engine + GPS data - listen only";
    lv_label_set_text(bannerText, message);
    lv_obj_set_style_border_color(banner, lv_color_hex(d.warning(now) || !c.healthy(now) || fuelUncertain ? 0xFFB454 : cyan()), 0);
    lv_label_set_text(status, !received ? "WAITING" : !c.healthy(now) ? "CAN WARNING" : engine && gps ? "ENGINE + GPS" : "DATA RECEIVED");
    numeric(Rpm, d.rpm, now, 1, 0, 0); numeric(Speed, d.speedMs, now, speedScale());
    numeric(Trim, d.trim, now, 1, 0, 0); numeric(Flow, d.flowLph, now, fuelScale());
    numeric(Coolant, d.coolantK, now, units == 2 ? 1 : 1.8, units == 2 ? -273.15 : -459.67);
    numeric(Water, d.waterK, now, units == 2 ? 1 : 1.8, units == 2 ? -273.15 : -459.67);
    numeric(Oil, d.oilPa, now, units == 2 ? 0.001 : 1.0 / 6894.757293);
    numeric(Course, d.courseRad, now, 180 / 3.141592653589793, 0, 0);
    numeric(Fuel, fuel.reading(now), now);
    if (fuelTitle) lv_label_set_text_fmt(fuelTitle, "Fuel %s US gal", fuel.source()); numeric(Volts, d.voltage, now, 1, 0, 2);
    numeric(Hours, d.hoursSec, now, 1.0 / 3600);
    live::Value economy;
    if (gps && d.flowLph.fresh(now) && engine && d.rpm.value > 0 && d.flowLph.value > 0.3 && d.speedMs.value > 1)
        economy.set(d.speedMs.value * 3600 / d.flowLph.value, now);
    numeric(Economy, economy, now, distanceScale() / fuelScale());
    if (dial) {
        lv_arc_set_value(dial, engine ? int(fmin(d.rpm.value, 6000)) : 0);
        lv_obj_set_style_arc_opa(dial, engine ? LV_OPA_COVER : LV_OPA_30, LV_PART_INDICATOR);
    }
    if (page == 1 && detail) {
        live::Value best; if (t.bestMetersPerLiter > 0) best.set(t.bestMetersPerLiter, now);
        numeric(Best, best, now, distanceScale() / fuelScale(), 0, 2);
        if (t.bestMetersPerLiter > 0) lv_label_set_text_fmt(detail, "Session best at %.0f RPM\n%s", t.bestRpm, t.partial ? "Coverage incomplete: best is from valid intervals only." : "Best from valid running intervals this session.");
        else lv_label_set_text(detail, "Waiting for steady running data.\nNo best-economy sample recorded yet.");
    }
    if (page == 2) {
        number(Elapsed, t.seconds / 60); number(Distance, t.meters * distanceScale(), 2);
        number(Used, t.liters * fuelScale(), 2);
        live::Value maxSpeed, maxRpm;
        if (t.speedSeen) maxSpeed.set(t.maxSpeedMs, now);
        if (t.rpmSeen) maxRpm.set(t.maxRpm, now);
        numeric(MaxSpeed, maxSpeed, now, speedScale()); numeric(MaxRpm, maxRpm, now, 1, 0, 0);
        live::Value average; if (t.liters > 0) average.set(t.meters / t.liters, now);
        numeric(Average, average, now, distanceScale() / fuelScale(), 0, 2);
        lv_label_set_text(detail, t.partial ? "INCOMPLETE COVERAGE - totals include valid intervals only" : "Valid running time in minutes - aligned engine + GPS inputs");
    }
    if (page == 3 && diagnostic) {
        char flags1[16] = "--", flags2[16] = "--", lat[24] = "--", lon[24] = "--";
        if (d.status1.fresh(now)) snprintf(flags1, sizeof(flags1), "0x%04X", unsigned(d.status1.value));
        if (d.status2.fresh(now)) snprintf(flags2, sizeof(flags2), "0x%04X", unsigned(d.status2.value));
        if (d.latitude.fresh(now)) snprintf(lat, sizeof(lat), "%.6f", d.latitude.value);
        if (d.longitude.fresh(now)) snprintf(lon, sizeof(lon), "%.6f", d.longitude.value);
        char capacity[24] = "not set", level[24] = "--";
        if (fuel.capacity) snprintf(capacity, sizeof(capacity), "%lu US gal", (unsigned long)fuel.capacity);
        if (d.levelPct.fresh(now)) snprintf(level, sizeof(level), "%.1f%%", d.levelPct.value);
        else if (d.levelPct.seen) snprintf(level, sizeof(level), "%s", d.levelPct.available ? "STALE" : "N/A");
        lv_label_set_text_fmt(diagnostic,
            "Firmware v%s | RX only 250 kbit/s | TX15 RX16\n"
            "Engine SA %u / instance %u | GPS SA %u | tank SA %u\n"
            "Tank %s | level %s | fuel %s%s\n"
            "Frames %lu | engine dynamic %lu | packet drops %lu\n"
            "CAN %s / state %d | errors %lu | missed %lu | overruns %lu\n"
            "REC %lu / TEC %lu | raw log drops %lu | write failures %lu\n"
            "Engine status words %s / %s (OEM interpretation unverified)\n"
            "GPS %s, %s | uptime %lu s\n"
            "Raw USB %s: r=start, s=stop, i=info. No SD logging.\n"
            "Proprietary engine fields are not used as measurements.",
            live::VERSION, d.engine.source, d.engine.instance, d.gps.source, d.tank.source, capacity, level, fuel.source(), fuelSaveFailed ? " SAVE FAILED" : "",
            (unsigned long)c.frames, (unsigned long)c.dynamicMessages, (unsigned long)c.packetDrops,
            c.statusValid ? "sampled" : "unknown", c.canState, (unsigned long)c.busErrors,
            (unsigned long)c.missed, (unsigned long)c.overruns, (unsigned long)c.rxErrors,
            (unsigned long)c.txErrors, (unsigned long)rawDrops, (unsigned long)logFailures,
            flags1, flags2, lat, lon, (unsigned long)(now / 1000), rawActive ? "ON" : "OFF");
    }
}
bool takeTripReset() { bool value = tripReset; tripReset = false; return value; }
bool takeSourceReset() { bool value = sourceReset; sourceReset = false; return value; }
} // namespace liveui
