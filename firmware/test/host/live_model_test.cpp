#include "LiveModel.h"
#include "TankCapacity.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

static void near(double a, double b, double epsilon = 1e-7) { assert(std::fabs(a - b) < epsilon); }
static uint32_t id(uint32_t pgn, uint8_t source = 2) { return 0x18000000 | (pgn << 8) | source; }
static void put16(uint8_t *p, uint16_t value) { p[0] = value; p[1] = value >> 8; }
static void packet(live::Model &m, const uint8_t *payload, uint8_t source = 2, uint32_t now = 100, uint8_t seq = 0) {
    uint8_t p[8]{}; p[0] = seq << 5; p[1] = 26; memcpy(p + 2, payload, 6);
    m.frame(id(127489, source), p, 8, now);
    for (unsigned n = 1; n <= 3; ++n) {
        memset(p, 0xff, 8); p[0] = (seq << 5) | n;
        size_t offset = 6 + (n - 1) * 7, count = 26 - offset; if (count > 7) count = 7;
        memcpy(p + 1, payload + offset, count); m.frame(id(127489, source), p, 8, now + n);
    }
}
static void inputs(live::Model &m, uint32_t now, double rpm = 750, double speed = 5, double flow = 36) {
    auto &d = m.state.data; d.rpm.set(rpm, now); d.speedMs.set(speed, now); d.flowLph.set(flow, now);
    m.canStatus(true, true, 1, 0, 0, 0, 0, 0, now); m.tick(now);
}
int main(int argc, char **argv) {
    uint32_t gallons = 60;
    for (const char *invalid : {"", "0", "-1", "60.5", "NaN", " 60", "60 ", "10000", "42949672960"}) {
        assert(!live::parseTankGallons(invalid, gallons)); assert(gallons == 60);
    }
    assert(!live::parseTankGallons(nullptr, gallons));
    assert(live::parseTankGallons("9999", gallons) && gallons == 9999);
    assert(live::parseTankGallons("0060", gallons) && gallons == 60);
    live::Value level; level.set(37.5, 100);
    auto remaining = live::remainingTankGallons(level, gallons);
    near(remaining.value, 22.5); assert(remaining.fresh(100) && !remaining.fresh(5101));
    assert(!live::remainingTankGallons(level, 0).seen);
    level.set(0, 100); remaining = live::remainingTankGallons(level, gallons);
    assert(remaining.available); near(remaining.value, 0);
    level.set(100, 100); near(live::remainingTankGallons(level, gallons).value, 60);
    level.set(101, 100); assert(!live::remainingTankGallons(level, gallons).available);
    level.set(50, 100, false); remaining = live::remainingTankGallons(level, gallons);
    assert(remaining.seen && !remaining.available && remaining.updated == 100);
    live::Value v; assert(!v.seen && !v.fresh(0));
    v.set(0, 0xfffffff0); assert(v.fresh(16)); assert(!v.fresh(6000));
    v.set(5, 100, false); assert(v.seen && !v.available);
    live::Model m;
    uint8_t rapid[8] = {1, 0xb8, 0x0b, 0xff, 0xff, 0xf6, 0xff, 0xff};
    m.frame(id(127488), rapid, 8, 100); near(m.state.data.rpm.value, 750); near(m.state.data.trim.value, -10);
    rapid[0] = 0; m.frame(id(127488, 3), rapid, 8, 101);
    assert(m.state.data.engine.source == 2 && m.state.data.engine.instance == 1);
    rapid[0] = 1; rapid[1] = 0xfd; rapid[2] = 0xff;
    m.frame(id(127488), rapid, 8, 110); assert(!m.state.data.rpm.available);
    rapid[1] = rapid[2] = 0; m.frame(id(127488), rapid, 8, 120); assert(m.state.data.rpm.available); near(m.state.data.rpm.value, 0);
    uint8_t dyn[26]; memset(dyn, 0xff, sizeof(dyn)); dyn[0] = 1;
    put16(dyn + 1, 4000); put16(dyn + 5, 32015); put16(dyn + 7, 1420); put16(dyn + 9, 36);
    dyn[11] = 0x10; dyn[12] = 0x0e; dyn[13] = dyn[14] = 0;
    put16(dyn + 20, 0); put16(dyn + 22, 0xffe0);
    packet(m, dyn); auto &d = m.state.data;
    assert(m.state.diagnostics.dynamicMessages == 1);
    near(d.oilPa.value, 400000); near(d.coolantK.value, 320.15); near(d.voltage.value, 14.2);
    near(d.flowLph.value, 3.6); near(d.hoursSec.value, 3600);
    assert(!d.oilK.available && !d.warning(200)); // Captured Yamaha reserved bits do not trigger shutdown.
    put16(dyn + 20, 2); packet(m, dyn, 2, 200); assert(d.critical(210));
    put16(dyn + 9, 0x7ffe); packet(m, dyn, 2, 300); assert(!d.flowLph.available);
    put16(dyn + 9, 0xffff); packet(m, dyn, 2, 400); assert(!d.flowLph.available); // Negative flow is invalid.
    // Timeout, missing index, noninitial fragments, truncation: never publish partial data.
    live::Model broken;
    uint8_t first[8] = {0, 26, 1, 0, 0, 0, 0, 0};
    uint8_t continuation[8] = {2, 0, 0, 0, 0, 0, 0, 0};
    broken.frame(id(127489), first, 8, 10); broken.frame(id(127489), continuation, 8, 20);
    assert(!broken.state.data.engine.selected && broken.state.diagnostics.packetDrops == 1);
    broken.frame(id(127489), first, 8, 100); continuation[0] = 1;
    broken.frame(id(127489), continuation, 8, 351); assert(!broken.state.data.engine.selected);
    broken.frame(id(127489), first, 7, 400); assert(!broken.state.data.engine.selected);
    // A different source/sequence cannot complete this packet.
    broken.frame(id(127489), first, 8, 500); broken.frame(id(127489, 3), continuation, 8, 501);
    continuation[0] = 0x21; broken.frame(id(127489), continuation, 8, 502);
    assert(!broken.state.data.engine.selected);
    uint8_t gps[8] = {0xff, 0xfc, 0x10, 0x27, 0xf4, 1, 0xff, 0xff};
    m.frame(id(129026, 0), gps, 8, 700); near(d.speedMs.value, 5); near(d.courseRad.value, 1);
    gps[1] = 0xfd; m.frame(id(129026, 0), gps, 8, 701); assert(!d.courseRad.available && d.speedMs.available);
    gps[4] = 0xfe; gps[5] = 0xff; m.frame(id(129026, 0), gps, 8, 702); assert(!d.speedMs.available);
    uint8_t temp[8] = {0, 0, 1, 0x73, 0x75, 0xff, 0xff, 0xff};
    m.frame(id(130312, 0), temp, 8, 800); assert(!d.waterK.seen); // Air must not become water.
    temp[2] = 0; m.frame(id(130312, 0), temp, 8, 801); near(d.waterK.value, 300.67);
    uint8_t fuel[8] = {0, 0x70, 0x30, 0xe8, 3, 0, 0, 0xff};
    m.frame(id(127505, 8), fuel, 8, 900); near(d.levelPct.value, 49.6); near(d.capacityL.value, 100);
    m.resetSources(); assert(!m.state.data.engine.selected && !m.state.data.coolantK.seen && !m.state.data.gps.selected);
    live::Model trip;
    for (uint32_t now = 100; now <= 10100; now += 100) inputs(trip, now);
    near(trip.state.trip.seconds, 10); near(trip.state.trip.meters, 50); near(trip.state.trip.liters, 0.1);
    near(trip.state.trip.bestMetersPerLiter, 500); assert(!trip.state.trip.partial);
    assert(trip.state.trip.rpmSeen && trip.state.trip.speedSeen);
    trip.canStatus(true, true, 1, 1, 0, 0, 0, 0, 10200); trip.tick(10200);
    near(trip.state.trip.liters, 0.1); assert(trip.state.trip.partial);
    inputs(trip, 10300); near(trip.state.trip.liters, 0.1); // Recovery requires five quiet seconds.
    trip.resetTrip(); assert(!trip.state.trip.partial && trip.state.trip.meters == 0);
    live::Model gap; inputs(gap, 100); inputs(gap, 5000); near(gap.state.trip.liters, 0); assert(gap.state.trip.partial);
    live::Model off; inputs(off, 100, 0); inputs(off, 200, 0); near(off.state.trip.seconds, 0); assert(!off.state.trip.partial);
    assert(!off.state.trip.rpmSeen && !off.state.trip.speedSeen);
    live::Model rollover; inputs(rollover, 0xffffffc0); inputs(rollover, 36); near(rollover.state.trip.seconds, 0.1);
    // New invalid start must invalidate the previous incomplete packet with the same key.
    live::Model restarted;
    restarted.frame(id(127489), first, 8, 100);
    restarted.frame(id(127489), first, 7, 110);
    continuation[0] = 1; restarted.frame(id(127489), continuation, 8, 111);
    assert(!restarted.state.data.engine.selected);
    // Queue loss invalidates assembly; bus errors alone do not fabricate missing fragments.
    live::Model lost;
    lost.frame(id(127489), first, 8, 100);
    lost.canStatus(true, true, 1, 0, 1, 0, 0, 0, 101);
    lost.frame(id(127489), continuation, 8, 102);
    assert(!lost.state.data.engine.selected && lost.state.diagnostics.packetDrops >= 2);
    live::Model passive;
    passive.canStatus(true, true, 1, 0, 0, 0, 96, 0, 100);
    assert(!passive.state.diagnostics.healthy(100));
    // Exact first dynamic message from the October 13 boat capture, retained in CI.
    live::Model captured;
    const uint8_t realFrames[][8] = {
        {0x00,0x1a,0x01,0x56,0x00,0xff,0xff,0xab},
        {0x01,0x7c,0xf3,0x04,0x00,0x00,0x68,0x72},
        {0x02,0x07,0x00,0xff,0xff,0xff,0xff,0xff},
        {0x03,0x00,0x00,0xe0,0xff,0x7f,0x7f,0xff}
    };
    for (unsigned i = 0; i < 4; ++i) captured.frame(0x09f20102, realFrames[i], 8, 100 + i);
    assert(captured.state.diagnostics.dynamicMessages == 1);
    near(captured.state.data.coolantK.value, 319.15);
    near(captured.state.data.voltage.value, 12.67);
    near(captured.state.data.flowLph.value, 0);
    assert(!captured.state.data.warning(110));
    // Replay the boat's actual raw capture when supplied; no capture required in CI.
    if (argc > 1) {
        std::ifstream file(argv[1]); assert(file.is_open()); std::string line; live::Model replay;
        double minK = 1e9, maxK = 0, maxRpm = 0; unsigned count = 0;
        while (std::getline(file, line)) {
            const char *raw = std::strstr(line.c_str(), "RAW seq="); if (!raw) continue;
            unsigned seq, cid, length; unsigned long long us; char hex[17]{};
            if (std::sscanf(raw, "RAW seq=%u us=%llu id=%x dlc=%u data=%16s", &seq, &us, &cid, &length, hex) != 5 || length > 8) continue;
            uint8_t bytes[8]{}; for (unsigned i = 0; i < length; ++i) { unsigned b; assert(std::sscanf(hex + i * 2, "%2x", &b) == 1); bytes[i] = b; }
            replay.frame(cid, bytes, length, uint32_t(us / 1000)); ++count;
            const auto &data = replay.state.data;
            if (data.coolantK.available) { minK = fmin(minK, data.coolantK.value); maxK = fmax(maxK, data.coolantK.value); }
            if (data.rpm.available) maxRpm = fmax(maxRpm, data.rpm.value);
        }
        assert(count == 5418); assert(replay.state.diagnostics.dynamicMessages == 120);
        near(minK, 319.15); near(maxK, 321.15); near(maxRpm, 1445.25);
        assert(replay.state.data.engine.source == 2 && replay.state.data.engine.instance == 1);
        assert(replay.state.data.gps.source == 0 && !replay.state.data.oilK.available);
        printf("Boat replay: %u frames, %u complete engine messages, coolant %.2f..%.2f K, max %.2f RPM\n", count, replay.state.diagnostics.dynamicMessages, minK, maxK, maxRpm);
    }
    puts("Live model tests passed");
}
