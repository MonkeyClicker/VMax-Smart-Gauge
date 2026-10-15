#pragma once
#include "TankCapacity.h"

namespace live {
inline bool parseInitialGallons(const char *text, uint32_t capacity, uint32_t &gallons) {
    if (!capacity || capacity > MAX_TANK_GALLONS || !text || !*text) return false;
    bool zero = true;
    for (const char *p = text; *p; ++p) if (*p != '0') zero = false;
    if (zero) { gallons = 0; return true; }
    uint32_t value;
    if (!parseTankGallons(text, value) || value > capacity) return false;
    gallons = value; return true;
}
struct FuelRecord {
    double gallons = 0;
    uint32_t magic = 0x56464c31, schema = 1, capacity = 0, flags = 0;
    bool valid() const {
        return magic == 0x56464c31 && schema == 1 && capacity <= MAX_TANK_GALLONS &&
               (flags & ~3u) == 0 && std::isfinite(gallons) && gallons >= 0 &&
               ((flags & 1) ? capacity > 0 && gallons <= capacity : gallons == 0);
    }
};
class FuelInventory {
    bool tickSeen = false, previousValid = false, networkAnchored = false;
    uint32_t lastTick = 0, networkAt = 0;
    uint8_t networkSource = 255, networkInstance = 255;
    double previousFlow = 0;
    Value networkValue;
public:
    uint32_t capacity = 0;
    double gallons = 0;
    bool known = false, incomplete = false, networkActive = false, dirty = false;
    FuelRecord record() const {
        FuelRecord r; r.capacity = capacity; r.gallons = known ? gallons : 0;
        r.flags = (known ? 1u : 0u) | (incomplete ? 2u : 0u); return r;
    }
    bool restore(const FuelRecord &r) {
        if (!r.valid()) return false;
        *this = FuelInventory{}; capacity = r.capacity; gallons = r.gallons;
        known = (r.flags & 1) != 0;
        // Consumption while power was absent cannot be reconstructed.
        incomplete = known || (r.flags & 2) != 0; return true;
    }
    bool setCapacity(uint32_t value, const Data &data, uint32_t now) {
        if (!value || value > MAX_TANK_GALLONS) return false;
        if (known && gallons > value && !remainingTankGallons(data.levelPct, value).fresh(now)) return false;
        capacity = value; networkAnchored = previousValid = false;
        // A fresh network level is authoritative when changing tank size.
        const auto network = remainingTankGallons(data.levelPct, value);
        if (network.fresh(now)) { gallons = network.value; known = true; incomplete = false; }
        dirty = true; return true;
    }
    bool setInitial(uint32_t value) {
        if (!capacity || value > capacity) return false;
        gallons = value; known = true; incomplete = false; dirty = true;
        networkAnchored = previousValid = false; return true;
    }
    void update(const Data &data, const Diagnostics &can, uint32_t now) {
        networkValue = remainingTankGallons(data.levelPct, capacity);
        networkActive = networkValue.fresh(now);
        const bool anchor = networkActive && (!networkAnchored || networkAt != networkValue.updated ||
            networkSource != data.tank.source || networkInstance != data.tank.instance);
        if (anchor) {
            if (!known || gallons != networkValue.value || incomplete) dirty = true;
            gallons = networkValue.value; known = true; incomplete = false;
            networkAnchored = true; networkAt = networkValue.updated;
            networkSource = data.tank.source; networkInstance = data.tank.instance;
            previousValid = false;
        }
        const bool engineStopped = data.rpm.fresh(now) && data.rpm.value == 0;
        const bool flowKnown = data.flowLph.fresh(now) && data.flowLph.value >= 0;
        const bool valid = can.healthy(now) && (flowKnown || engineStopped);
        const double flow = flowKnown ? data.flowLph.value : 0;
        const uint32_t elapsed = uint32_t(now - lastTick);
        if (known && tickSeen && elapsed > 0 && !anchor) {
            if (!valid || elapsed > 1000) {
                if (!incomplete) dirty = true;
                incomplete = true;
            } else if (previousValid) {
                double used = (previousFlow + flow) * 0.5 * (elapsed / 1000.0) / 3600 / 3.785411784;
                if (used > 0) {
                    const double next = std::fmax(0, gallons - used);
                    if (next != gallons) { gallons = next; dirty = true; }
                }
            }
        }
        previousFlow = flow; previousValid = valid; lastTick = now; tickSeen = true;
    }
    Value reading(uint32_t now) const {
        if (networkActive) return networkValue;
        Value result; if (known) result.set(gallons, now); return result;
    }
    const char *source() const { return networkActive ? "NET" : incomplete ? "EST?" : "EST"; }
};
}
