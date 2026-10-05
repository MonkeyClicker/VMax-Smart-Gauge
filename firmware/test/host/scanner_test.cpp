#include "Scanner.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>

void near(double actual, double expected) { assert(std::abs(actual - expected) < 0.000001); }
int main() {
    BoatData boat;
    uint8_t engine[8] = {0, 0xB8, 0x0B, 0, 0, 10, 0, 0};
    scanner::decode(boat, 127488, 17, engine, 8, 100);
    near(boat.engine.rpm.value, 750);
    assert(boat.engine.rpm.fresh(200, 1000));
    uint8_t zeroEngine[8]{};
    scanner::decode(boat, 127488, 17, zeroEngine, 8, 150);
    assert(boat.engine.rpm.available && boat.engine.rpm.fresh(150, 1000));
    near(boat.engine.rpm.value, 0);
    scanner::decode(boat, 127488, 17, engine, 8, 160);
    engine[1] = engine[2] = 0xFF;
    engine[5] = 0x7F;
    scanner::decode(boat, 127488, 17, engine, 8, 200);
    assert(boat.engine.rpm.seen && !boat.engine.rpm.available && !boat.engine.rpm.fresh(200, 1000));
    assert(!boat.engine.trimPercent.available);
    engine[1] = 0xB8; engine[2] = 0x0B; engine[5] = 0xF6;
    scanner::decode(boat, 127488, 17, engine, 8, 300);
    assert(boat.engine.rpm.available && boat.engine.trimPercent.value == -10);
    boat.engine.coolantTempK.set(333.15, 300);
    engine[0] = 1; engine[1] = engine[2] = 0xFF;
    scanner::decode(boat, 127488, 18, engine, 8, 400);
    assert(boat.engine.sourceAddress == 18 && boat.engine.instance == 1);
    assert(!boat.engine.rpm.available && std::isnan(boat.engine.rpm.value));
    assert(!boat.engine.coolantTempK.seen);
    boat.engine.coolantTempK.set(300, 400);
    engine[0] = 2;
    scanner::decode(boat, 127488, 18, engine, 8, 450);
    assert(!boat.engine.coolantTempK.seen);

    uint8_t fuel[8] = {0, 0xD4, 0x30, 0xD0, 0x07, 0, 0, 0};
    scanner::decode(boat, 127505, 22, fuel, 8, 100);
    near(boat.fuelTank.levelPercent.value, 50); near(boat.fuelTank.capacityL.value, 200);
    fuel[1] = 0xFF; fuel[2] = 0x7F;
    scanner::decode(boat, 127505, 22, fuel, 8, 200);
    assert(!boat.fuelTank.levelPercent.available);
    fuel[1] = 0x3C; fuel[2] = 0xF6;
    scanner::decode(boat, 127505, 22, fuel, 8, 300);
    near(boat.fuelTank.levelPercent.value, -10);
    for (int i = 3; i < 7; ++i) fuel[i] = 0xFF;
    scanner::decode(boat, 127505, 22, fuel, 8, 350);
    assert(boat.fuelTank.capacityL.seen && !boat.fuelTank.capacityL.available);
    assert(!boat.fuelTank.capacityL.fresh(350, 1000));
    fuel[0] = 1; fuel[1] = 0xFF; fuel[2] = 0x7F;
    for (int i = 3; i < 7; ++i) fuel[i] = 0xFF;
    scanner::decode(boat, 127505, 23, fuel, 8, 400);
    assert(boat.fuelTank.instance == 1 && boat.fuelTank.sourceAddress == 23);
    assert(!boat.fuelTank.levelPercent.available && !boat.fuelTank.capacityL.available);
    assert(std::isnan(boat.fuelTank.capacityL.value));
    fuel[0] = 0x11;
    scanner::decode(boat, 127505, 24, fuel, 8, 500);
    assert(boat.fuelTank.sourceAddress == 23); // Non-fuel tank ignored.

    uint8_t gps[8] = {0, 0, 0x10, 0x27, 0xE8, 0x03, 0, 0};
    scanner::decode(boat, 129026, 3, gps, 8, 100);
    near(boat.navigation.courseOverGroundRad.value, 1); near(boat.navigation.speedOverGroundMs.value, 10);
    for (int i = 2; i < 6; ++i) gps[i] = 0xFF;
    scanner::decode(boat, 129026, 3, gps, 8, 200);
    assert(!boat.navigation.courseOverGroundRad.available && !boat.navigation.speedOverGroundMs.available);
    scanner::decode(boat, 129026, 4, gps, 8, 300);
    assert(boat.navigation.sourceAddress == 4 && std::isnan(boat.navigation.speedOverGroundMs.value));
    BoatData empty;
    scanner::decode(empty, 127488, 17, engine, 5, 100);
    scanner::decode(empty, 127505, 22, fuel, 6, 100);
    scanner::decode(empty, 129026, 3, gps, 5, 100);
    assert(!empty.engine.detected && !empty.fuelTank.detected && !empty.navigation.detected);

    TimedDouble timed;
    timed.set(0, UINT32_MAX - 100);
    assert(timed.fresh(50, 151)); assert(!timed.fresh(50, 150));
    scanner::Stats stats;
    stats.resetWindow(100);
    stats.record(127488, 17, 100); stats.record(127488, 17, 1100);
    near(stats.rate(stats.entries[0], 2100), 1);
    near(stats.rate(stats.entries[0], 6100), 0);
    stats.resetWindow(6100);
    near(stats.rate(stats.entries[0], 7100), 0);
    stats.record(127488, 17, 7100);
    near(stats.rate(stats.entries[0], 8100), 0.5);
    stats.resetWindow(UINT32_MAX - 999);
    stats.record(127488, 17, UINT32_MAX - 500);
    stats.record(127488, 17, 0);
    near(stats.rate(stats.entries[0], 1000), 1);
    near(stats.rate(stats.entries[0], 6000), 0);

    scanner::Stats full;
    for (unsigned i = 0; i < 64; ++i) full.record(130000 + i, 17, i);
    full.record(127488, 18, 1000);
    assert(full.evictions == 1 && full.entries[0].pgn == 127488 && full.entries[0].count == 1);
    full.record(127488, 18, 1001);
    assert(full.evictions == 1 && full.entries[0].count == 2);
    scanner::Stats wrapped;
    for (unsigned i = 0; i < 64; ++i) wrapped.record(130000 + i, 17, UINT32_MAX - 100 + i);
    wrapped.record(127488, 18, 10);
    assert(wrapped.entries[0].pgn == 127488);
    assert(scanner::extractPgn((0x1F112u << 8) | 17) == 127250);
    assert(scanner::extractPgn((0xEA23u << 8) | 17) == 59904); // PDU1 destination excluded.
    std::puts("Scanner regression tests passed");
}
