#pragma once

#include "../../src/simulated_gauge/SimulatedEngineData.h"
#include <math.h>

// Runs on a host or through the demo's serial 't' command. No hardware writes.
inline bool simulatedGaugeChecks() {
    EngineSimulator simulator;
    const auto &idle = simulator.update(0);
    if (idle.speedMph != 0 || idle.tripFuelGallons != 0 || idle.economyMpg != 0) return false;
    simulator.update(8000);
    const auto beforeCruise = simulator.update(8000);
    const auto cruise = simulator.update(28000);
    if (fabs(cruise.tripDistanceMiles - beforeCruise.tripDistanceMiles - 42.0 * 20 / 3600) > 0.00001) return false;
    if (fabs(cruise.tripFuelGallons - beforeCruise.tripFuelGallons - 14.3 * 20 / 3600) > 0.00001) return false;
    if (cruise.bestEconomyMpg <= 0 || cruise.maxSpeedMph < 41.9F) return false;
    simulator.setScenario(DemoScenario::Offline);
    const auto offline = simulator.update(33000);
    if (offline.tripDistanceMiles != cruise.tripDistanceMiles ||
        offline.tripFuelGallons != cruise.tripFuelGallons || offline.dataAgeMs != 5000) return false;
    simulator.setScenario(DemoScenario::Normal);
    const auto resumed = simulator.update(34000);
    if (resumed.tripFuelGallons != offline.tripFuelGallons || resumed.dataAgeMs != 0) return false;
    simulator.resetTrip();
    const auto reset = simulator.update(34000);
    if (reset.tripFuelGallons != 0 || reset.tripDistanceMiles != 0 || reset.tripElapsedMs != 0 ||
        fabsf(reset.fuelLevelPercent - resumed.fuelLevelPercent) > 0.0001F) return false;
    simulator.setScenario(DemoScenario::Warning);
    if (!simulator.update(35000).warning) return false;
    simulator.setScenario(DemoScenario::Unavailable);
    if (simulator.update(36000).warning || simulator.update(37000).tripFuelGallons <= 0) return false;
    EngineSimulator rollover;
    rollover.update(UINT32_MAX - 499);
    if (rollover.update(500).tripElapsedMs != 1000) return false;
    return true;
}
