#pragma once

#include "BoatData.h"
#include <stddef.h>

namespace scanner {
constexpr size_t MAX_TRACKED_PGNS = 64;
constexpr uint32_t RATE_WINDOW_MS = 5000;

inline uint16_t read16(const uint8_t *p) {
    return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
}

inline uint32_t extractPgn(uint32_t id) {
    const uint32_t pf = (id >> 16) & 0xFF;
    return ((id >> 8) & 0x1FFFF) & (pf < 240 ? 0x1FF00 : 0x1FFFF);
}

// Each snapshot belongs to one source/instance. Clear all measurements when
// identity changes so unavailable fields cannot inherit another device's data.
inline void decode(BoatData &boat, uint32_t pgn, uint8_t source,
                   const uint8_t *data, size_t length, uint32_t nowMs) {
    if (pgn == 127488 && length >= 6) {
        auto &engine = boat.engine;
        if (!engine.detected || engine.sourceAddress != source || engine.instance != data[0]) {
            engine = EngineData{};
        }
        engine.detected = true;
        engine.sourceAddress = source;
        engine.instance = data[0];
        const uint16_t rpm = read16(data + 1);
        if (rpm == 0xFFFF) engine.rpm.invalidate(nowMs);
        else engine.rpm.set(rpm * 0.25, nowMs);
        const int trim = data[5] < 128 ? data[5] : int(data[5]) - 256;
        if (trim == 127) engine.trimPercent.invalidate(nowMs);
        else engine.trimPercent.set(trim, nowMs);
    } else if (pgn == 129026 && length >= 6) {
        auto &nav = boat.navigation;
        if (!nav.detected || nav.sourceAddress != source) nav = NavigationData{};
        nav.detected = true;
        nav.sourceAddress = source;
        const uint16_t cog = read16(data + 2);
        const uint16_t sog = read16(data + 4);
        if (cog == 0xFFFF) nav.courseOverGroundRad.invalidate(nowMs);
        else nav.courseOverGroundRad.set(cog * 0.0001, nowMs);
        if (sog == 0xFFFF) nav.speedOverGroundMs.invalidate(nowMs);
        else nav.speedOverGroundMs.set(sog * 0.01, nowMs);
    } else if (pgn == 127505 && length >= 7 && (data[0] >> 4) == 0) {
        auto &tank = boat.fuelTank;
        const uint8_t instance = data[0] & 0x0F;
        if (!tank.detected || tank.sourceAddress != source || tank.instance != instance) {
            tank = FuelTankData{};
        }
        tank.detected = true;
        tank.sourceAddress = source;
        tank.instance = instance;
        const uint16_t rawLevel = read16(data + 1);
        const int32_t level = rawLevel < 0x8000 ? rawLevel : int32_t(rawLevel) - 65536;
        const uint32_t capacity = uint32_t(data[3]) | (uint32_t(data[4]) << 8) |
                                  (uint32_t(data[5]) << 16) | (uint32_t(data[6]) << 24);
        if (rawLevel == 0x7FFF) tank.levelPercent.invalidate(nowMs);
        else tank.levelPercent.set(level * 0.004, nowMs);
        if (capacity == 0xFFFFFFFF) tank.capacityL.invalidate(nowMs);
        else tank.capacityL.set(capacity * 0.1, nowMs);
    }
}

struct PgnStat {
    uint32_t pgn = 0;
    uint8_t source = 0xFF;
    bool used = false;
    uint32_t count = 0;
    uint32_t lastMs = 0;
    uint32_t windowCount = 0;
};

struct Stats {
    PgnStat entries[MAX_TRACKED_PGNS]{};
    uint32_t evictions = 0;
    uint32_t windowStartMs = 0;

    void record(uint32_t pgn, uint8_t source, uint32_t nowMs) {
        PgnStat *slot = nullptr;
        for (auto &entry : entries) {
            if (entry.used && entry.pgn == pgn && entry.source == source) {
                slot = &entry;
                break;
            }
        }
        if (!slot) {
            for (auto &entry : entries) {
                if (!entry.used) { slot = &entry; break; }
            }
            if (!slot) {
                slot = &entries[0];
                for (auto &entry : entries) {
                    if (uint32_t(nowMs - entry.lastMs) > uint32_t(nowMs - slot->lastMs)) slot = &entry;
                }
                ++evictions;
            }
            *slot = PgnStat{};
            slot->used = true;
            slot->pgn = pgn;
            slot->source = source;
        }
        ++slot->count;
        ++slot->windowCount;
        slot->lastMs = nowMs;
    }

    double rate(const PgnStat &entry, uint32_t nowMs) const {
        const uint32_t elapsed = nowMs - windowStartMs;
        if (!elapsed || uint32_t(nowMs - entry.lastMs) >= RATE_WINDOW_MS) return 0.0;
        return entry.windowCount * 1000.0 / elapsed;
    }

    void resetWindow(uint32_t nowMs) {
        for (auto &entry : entries) entry.windowCount = 0;
        windowStartMs = nowMs;
    }
};
} // namespace scanner
