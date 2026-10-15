#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>

// Portable live-data model. No display, USB or Arduino calls on the receive path.
namespace live {
constexpr const char *VERSION = "0.1.2";
constexpr uint32_t FRESH_MS = 5000;
struct Value {
    double value = 0;
    uint32_t updated = 0;
    bool seen = false, available = false;
    void set(double v, uint32_t now, bool valid = true) {
        seen = true; available = valid && std::isfinite(v); updated = now;
        if (available) value = v;
    }
    bool fresh(uint32_t now) const { return seen && available && uint32_t(now - updated) <= FRESH_MS; }
};
struct Identity {
    bool selected = false;
    uint8_t source = 255, instance = 255;
    bool accepts(uint8_t s, uint8_t i = 0) {
        if (!selected) { selected = true; source = s; instance = i; }
        return source == s && instance == i;
    }
};
struct Data {
    Identity engine, gps, tank, water;
    Value rpm, trim, oilPa, oilK, coolantK, voltage, flowLph, hoursSec;
    Value load, torque, status1, status2, speedMs, courseRad, latitude, longitude;
    Value levelPct, capacityL, waterK;
    bool warning(uint32_t now) const {
        return (status1.fresh(now) && uint16_t(status1.value) != 0) ||
               (status2.fresh(now) && (uint16_t(status2.value) & 0x001f) != 0);
    }
    bool critical(uint32_t now) const {
        return status1.fresh(now) && (uint16_t(status1.value) & 0x001e) != 0;
    }
};
inline uint16_t u16(const uint8_t *p) { return uint16_t(p[0]) | uint16_t(p[1]) << 8; }
inline uint32_t u32(const uint8_t *p) {
    return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
}
inline int32_t signed32(uint32_t n) { return n < 0x80000000u ? int32_t(n) : int32_t(int64_t(n) - 0x100000000LL); }
inline int signed16(uint16_t n) { return n < 0x8000 ? n : int(n) - 65536; }
inline int signed8(uint8_t n) { return n < 128 ? n : int(n) - 256; }
inline uint32_t pgnOf(uint32_t id) {
    return ((id >> 8) & 0x1ffff) & (((id >> 16) & 255) < 240 ? 0x1ff00 : 0x1ffff);
}
inline void unsignedValue(Value &v, uint16_t raw, double scale, uint32_t now) {
    v.set(raw * scale, now, raw < 0xfffd);
}
inline void signedValue(Value &v, uint16_t raw, double scale, uint32_t now) {
    v.set(signed16(raw) * scale, now, raw < 0x7ffd || raw >= 0x8000);
}
struct Diagnostics {
    uint32_t frames = 0, dynamicMessages = 0, packetDrops = 0;
    uint32_t busErrors = 0, missed = 0, overruns = 0, rxErrors = 0, txErrors = 0;
    uint32_t lastFrame = 0, lastFault = 0, statusAt = 0;
    bool frameSeen = false, faultSeen = false, canReady = false, statusValid = false;
    int canState = -1;
    bool healthy(uint32_t now) const {
        return canReady && statusValid && uint32_t(now - statusAt) <= 1000 && canState == 1 && rxErrors < 96 && txErrors < 96 &&
               (!faultSeen || uint32_t(now - lastFault) > FRESH_MS);
    }
};
struct Trip {
    double seconds = 0, meters = 0, liters = 0, maxSpeedMs = 0, maxRpm = 0;
    double bestMetersPerLiter = 0, bestRpm = 0;
    bool partial = false, rpmSeen = false, speedSeen = false;
};
struct Snapshot { Data data; Diagnostics diagnostics; Trip trip; };

class Model {
    struct Packet {
        bool used = false;
        uint32_t id = 0, updated = 0;
        uint8_t sequence = 0, next = 0, size = 0;
        uint8_t bytes[26]{};
    } packets[8]{};
    bool tickSeen = false, previousValid = false, runningSeen = false;
    uint32_t lastTick = 0;
    double previousSpeed = 0, previousFlow = 0;
    double steadySeconds = 0, steadyMeters = 0, steadyLiters = 0, anchorRpm = 0, anchorSpeed = 0;
    void clearSteady() { steadySeconds = steadyMeters = steadyLiters = 0; }
    void dynamic(uint8_t source, const uint8_t *p, uint32_t now) {
        auto &d = state.data;
        if (p[0] >= 253 || !d.engine.accepts(source, p[0])) return;
        unsignedValue(d.oilPa, u16(p + 1), 100, now);
        unsignedValue(d.oilK, u16(p + 3), 0.1, now);
        unsignedValue(d.coolantK, u16(p + 5), 0.01, now);
        signedValue(d.voltage, u16(p + 7), 0.01, now);
        signedValue(d.flowLph, u16(p + 9), 0.1, now);
        if (d.flowLph.available && d.flowLph.value < 0) d.flowLph.available = false;
        d.hoursSec.set(u32(p + 11), now, u32(p + 11) < 0xfffffffdu);
        d.status1.set(u16(p + 20), now, u16(p + 20) < 0xfffd);
        d.status2.set(u16(p + 22), now, u16(p + 22) < 0xfffd);
        d.load.set(signed8(p[24]), now, signed8(p[24]) >= -100 && signed8(p[24]) <= 100);
        d.torque.set(signed8(p[25]), now, signed8(p[25]) >= -100 && signed8(p[25]) <= 100);
        ++state.diagnostics.dynamicMessages;
    }
    void fastPacket(uint32_t id, const uint8_t *p, size_t len, uint32_t now) {
        if (len < 2) { ++state.diagnostics.packetDrops; return; }
        uint8_t sequence = p[0] >> 5, index = p[0] & 31;
        Packet *match = nullptr, *free = nullptr;
        for (auto &packet : packets) {
            if (packet.used && uint32_t(now - packet.updated) > 250) {
                packet.used = false; ++state.diagnostics.packetDrops;
            }
            if (packet.used && packet.id == id && packet.sequence == sequence) match = &packet;
            if (!packet.used) free = &packet;
        }
        if (index == 0) {
            if (match) { match->used = false; ++state.diagnostics.packetDrops; }
            if (p[1] != 26 || len != 8) { ++state.diagnostics.packetDrops; return; }
            if (!match) match = free;
            if (!match) { ++state.diagnostics.packetDrops; return; }
            *match = Packet{}; match->used = true; match->id = id;
            match->sequence = sequence; match->updated = now; match->next = 1; match->size = 6;
            std::memcpy(match->bytes, p + 2, 6);
            return;
        }
        if (!match || index != match->next) {
            if (match) match->used = false;
            ++state.diagnostics.packetDrops; return;
        }
        size_t count = 26 - match->size;
        if (count > 7) count = 7;
        if (len < count + 1) { match->used = false; ++state.diagnostics.packetDrops; return; }
        std::memcpy(match->bytes + match->size, p + 1, count);
        match->size += count; ++match->next; match->updated = now;
        if (match->size == 26) { dynamic(uint8_t(id), match->bytes, now); match->used = false; }
    }
public:
    Snapshot state{};
    void resetTrip() { state.trip = Trip{}; previousValid = false; runningSeen = false; clearSteady(); }
    void resetSources() {
        state.data = Data{}; resetTrip();
        for (auto &p : packets) p = Packet{};
    }
    void frame(uint32_t id, const uint8_t *p, size_t len, uint32_t now) {
        if (id > 0x1fffffff || len > 8 || !p) return;
        auto &diag = state.diagnostics;
        ++diag.frames; diag.lastFrame = now; diag.frameSeen = true;
        const uint32_t pgn = pgnOf(id); const uint8_t source = uint8_t(id);
        if (source >= 254) return;
        auto &d = state.data;
        if (pgn == 127489) { fastPacket(id, p, len, now); return; }
        if (len != 8) return;
        if (pgn == 127488 && p[0] < 253 && d.engine.accepts(source, p[0])) {
            unsignedValue(d.rpm, u16(p + 1), 0.25, now);
            d.trim.set(signed8(p[5]), now, p[5] < 125 || p[5] >= 128);
        } else if (pgn == 129026 && d.gps.accepts(source)) {
            unsignedValue(d.speedMs, u16(p + 4), 0.01, now);
            unsignedValue(d.courseRad, u16(p + 2), 0.0001, now);
            if ((p[1] & 3) != 0 || d.courseRad.value >= 6.28318531) d.courseRad.available = false;
        } else if (pgn == 129025 && d.gps.accepts(source)) {
            const int32_t lat = signed32(u32(p)), lon = signed32(u32(p + 4));
            d.latitude.set(lat * 1e-7, now, lat >= -900000000 && lat <= 900000000);
            d.longitude.set(lon * 1e-7, now, lon >= -1800000000 && lon <= 1800000000);
        } else if (pgn == 127505 && (p[0] >> 4) == 0 && d.tank.accepts(source, p[0] & 15)) {
            signedValue(d.levelPct, u16(p + 1), 0.004, now);
            if (d.levelPct.value < 0 || d.levelPct.value > 100) d.levelPct.available = false;
            d.capacityL.set(u32(p + 3) * 0.1, now, u32(p + 3) < 0xfffffffdu);
        } else if (pgn == 130310 && d.water.accepts(source)) {
            unsignedValue(d.waterK, u16(p + 1), 0.01, now);
        } else if (pgn == 130312 && p[2] == 0 && d.water.accepts(source)) {
            unsignedValue(d.waterK, u16(p + 3), 0.01, now);
        }
    }
    void canStatus(bool valid, bool ready, int canState, uint32_t errors, uint32_t missed,
                   uint32_t overruns, uint32_t rec, uint32_t tec, uint32_t now) {
        auto &d = state.diagnostics;
        if (valid && (errors != d.busErrors || missed != d.missed || overruns != d.overruns || canState != 1)) {
            d.lastFault = now; d.faultSeen = true;
        }
        // Controller queue loss breaks continuity; bus errors alone may have
        // been retransmitted and must not discard an otherwise complete packet.
        if (valid && (missed != d.missed || overruns != d.overruns)) {
            for (auto &packet : packets) if (packet.used) {
                packet.used = false; ++d.packetDrops;
            }
        }
        d.statusValid = valid; d.canReady = ready; d.statusAt = now; d.canState = canState;
        if (valid) { d.busErrors = errors; d.missed = missed; d.overruns = overruns; d.rxErrors = rec; d.txErrors = tec; }
    }
    void tick(uint32_t now) {
        auto &d = state.data; auto &t = state.trip;
        const uint32_t elapsed = uint32_t(now - lastTick);
        const bool running = d.rpm.fresh(now) && d.rpm.value > 0;
        const bool valid = running && state.diagnostics.healthy(now) && d.flowLph.fresh(now) && d.speedMs.fresh(now);
        if (running) {
            runningSeen = true; t.rpmSeen = true;
            if (d.rpm.value > t.maxRpm) t.maxRpm = d.rpm.value;
            if (d.speedMs.fresh(now)) {
                t.speedSeen = true;
                if (d.speedMs.value > t.maxSpeedMs) t.maxSpeedMs = d.speedMs.value;
            }
        }
        if (tickSeen && runningSeen && (!d.rpm.fresh(now) || (running && !valid) || (running && elapsed > 1000))) t.partial = true;
        if (tickSeen && elapsed <= 1000 && previousValid && valid) {
            double seconds = elapsed / 1000.0;
            double meters = (previousSpeed + d.speedMs.value) * 0.5 * seconds;
            double liters = (previousFlow + d.flowLph.value) * 0.5 * seconds / 3600;
            t.seconds += seconds; t.meters += meters; t.liters += liters;
            if (d.speedMs.value > 1 && d.flowLph.value > 0.3) {
                if (!steadySeconds || std::fabs(d.rpm.value - anchorRpm) > 100 || std::fabs(d.speedMs.value - anchorSpeed) > 0.5) {
                    clearSteady(); anchorRpm = d.rpm.value; anchorSpeed = d.speedMs.value;
                }
                steadySeconds += seconds; steadyMeters += meters; steadyLiters += liters;
                if (steadySeconds >= 5 && steadyLiters > 0 && steadyMeters / steadyLiters > t.bestMetersPerLiter) {
                    t.bestMetersPerLiter = steadyMeters / steadyLiters; t.bestRpm = d.rpm.value;
                }
            } else clearSteady();
        } else clearSteady();
        previousValid = valid; previousSpeed = d.speedMs.value; previousFlow = d.flowLph.value;
        tickSeen = true; lastTick = now;
    }
};
} // namespace live
