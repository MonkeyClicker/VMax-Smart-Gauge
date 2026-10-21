#pragma once
#include "LiveModel.h"

namespace live {
constexpr uint32_t MAX_TANK_GALLONS = 9999;
inline bool parseTankGallons(const char *text, uint32_t &gallons) {
    if (!text || !*text) return false;
    uint32_t parsed = 0;
    for (const char *p = text; *p; ++p) {
        if (*p < '0' || *p > '9') return false;
        const uint32_t digit = uint32_t(*p - '0');
        if (parsed > (MAX_TANK_GALLONS - digit) / 10) return false;
        parsed = parsed * 10 + digit;
    }
    if (!parsed) return false;
    gallons = parsed;
    return true;
}
// Preserve the sender's availability and timestamp; configuration is never a
// baseline fuel quantity and never makes an old network reading fresh.
inline Value remainingTankGallons(const Value &percentage, uint32_t capacity) {
    if (!capacity || capacity > MAX_TANK_GALLONS) return Value{};
    Value result = percentage;
    if (result.available) {
        if (!std::isfinite(result.value) || result.value < 0 || result.value > 100) result.available = false;
        else result.value = result.value * capacity / 100;
    }
    return result;
}
}
