#include "FuelInventory.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <initializer_list>
static void near(double a, double b) { assert(std::fabs(a - b) < 1e-8); }
static void tick(live::FuelInventory &fuel, live::Data &data, uint32_t now, double flow = 36, bool healthy = true) {
    data.flowLph.set(flow, now);
    live::Diagnostics can; can.canReady = can.statusValid = true; can.canState = healthy ? 1 : 0; can.statusAt = now;
    fuel.update(data, can, now);
}
int main() {
    live::FuelInventory fuel; live::Data data;
    uint32_t amount = 17;
    assert(live::parseInitialGallons("0", 60, amount) && amount == 0);
    assert(live::parseInitialGallons("60", 60, amount) && amount == 60);
    for (const char *bad : {"", "61", "-1", "1.5", "NaN", " 1", "10000"}) {
        assert(!live::parseInitialGallons(bad, 60, amount)); assert(amount == 60);
    }
    assert(!live::parseInitialGallons("1", 0, amount));
    assert(!fuel.setInitial(30)); assert(!fuel.known);
    assert(fuel.setCapacity(60, data, 0)); assert(!fuel.known && !fuel.reading(0).seen);
    assert(fuel.setInitial(30)); near(fuel.reading(0).value, 30);
    for (uint32_t now = 100; now <= 10100; now += 100) tick(fuel, data, now);
    near(fuel.gallons, 30 - 0.1 / 3.785411784); assert(!fuel.incomplete);
    const double before = fuel.gallons;
    tick(fuel, data, 10200, 36, false); near(fuel.gallons, before); assert(fuel.incomplete);
    tick(fuel, data, 10300); near(fuel.gallons, before); // Never bridge the bad interval.
    assert(!fuel.setCapacity(20, data, 10300)); assert(fuel.capacity == 60);
    assert(fuel.setInitial(0)); tick(fuel, data, 10400); near(fuel.gallons, 0); assert(fuel.known);
    fuel.dirty = false; tick(fuel, data, 10450); assert(!fuel.dirty); // Empty stays zero without repeated flash writes.
    assert(fuel.setInitial(40)); assert(!fuel.incomplete);
    // Fresh network level replaces the manual amount and is the displayed authority.
    data.tank.source = 8; data.tank.instance = 1; data.levelPct.set(50, 10500);
    tick(fuel, data, 10500); assert(fuel.networkActive); near(fuel.gallons, 30); near(fuel.reading(10500).value, 30);
    assert(fuel.setInitial(10)); tick(fuel, data, 10500); near(fuel.gallons, 30); // Same cached fresh packet still wins after reset.
    for (uint32_t now = 10600; now <= 15600; now += 100) tick(fuel, data, now);
    assert(!fuel.networkActive); near(fuel.gallons, 30 - 0.051 / 3.785411784);
    near(fuel.reading(15600).value, fuel.gallons); // No reversion to the older 10/40-gallon entries.
    data.levelPct.set(0, 15700); tick(fuel, data, 15700);
    assert(fuel.networkActive && fuel.known); near(fuel.reading(15700).value, 0);
    data.levelPct.set(100, 15800); tick(fuel, data, 15800);
    assert(fuel.setCapacity(20, data, 15800)); tick(fuel, data, 15800); near(fuel.gallons, 20);
    // Invalid/unavailable network values cannot overwrite a valid estimate.
    data.levelPct.set(999, 15900); tick(fuel, data, 15900, 0); assert(!fuel.networkActive && fuel.gallons <= 20);
    data.levelPct.set(50, 16000, false); const double saved = fuel.gallons;
    tick(fuel, data, 16000, 0); near(fuel.gallons, saved);
    auto record = fuel.record(); assert(record.valid());
    live::FuelInventory restored; assert(restored.restore(record));
    near(restored.gallons, fuel.gallons); assert(restored.known && restored.incomplete && !restored.networkActive);
    record.gallons = 999; assert(!restored.restore(record)); near(restored.gallons, fuel.gallons);
    record = fuel.record(); record.schema = 99; assert(!restored.restore(record));
    // Long gaps, missing flow and negative flow pause; zero flow and zero RPM are valid.
    live::FuelInventory gap; assert(gap.setCapacity(60, data, 0)); assert(gap.setInitial(20));
    tick(gap, data, 100); tick(gap, data, 3000); near(gap.gallons, 20); assert(gap.incomplete);
    tick(gap, data, 3100, -1); near(gap.gallons, 20);
    data.flowLph.available = false;
    live::Diagnostics can; can.canReady = can.statusValid = true; can.canState = 1; can.statusAt = 3200;
    data.rpm.set(0, 3200); gap.update(data, can, 3200); near(gap.gallons, 20);
    live::FuelInventory rollover; data.levelPct = live::Value{};
    assert(rollover.setCapacity(60, data, 0)); assert(rollover.setInitial(10));
    tick(rollover, data, 0xffffffc0); tick(rollover, data, 36);
    near(rollover.gallons, 10 - 0.001 / 3.785411784);
    puts("Fuel inventory tests passed");
}
