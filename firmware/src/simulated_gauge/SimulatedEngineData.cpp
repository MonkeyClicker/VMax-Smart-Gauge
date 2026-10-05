#include "SimulatedEngineData.h"
#include <math.h>
#include <limits.h>

const SimulatedEngineData &EngineSimulator::update(uint32_t nowMs) {
    const uint32_t deltaMs = started_ ? nowMs - previousMs_ : 0;
    started_ = true;
    previousMs_ = nowMs;
    elapsedMs_ += deltaMs;
    data_.tripElapsedMs += deltaMs;
    const float seconds = static_cast<float>(elapsedMs_ / 1000.0);
    const float cycle = fmodf(seconds, 45.0F);
    const float throttle = cycle < 8 ? cycle / 8 : cycle < 30 ? 1 : 1 - (cycle - 30) / 15;
    const bool fresh = data_.scenario != DemoScenario::Offline;
    if (fresh) {
        data_.rpm = 700 + throttle * 3800 + sinf(seconds * 2.3F) * 35;
        data_.trimPercent = 12 + throttle * 26;
        data_.coolantTempF = 126 + throttle * 22 + sinf(seconds * 0.15F) * 2;
        data_.oilPressurePsi = 24 + throttle * 38;
        data_.batteryVoltage = 13.8F + throttle * 0.45F;
        data_.fuelFlowGph = 0.8F + throttle * throttle * 13.5F;
        data_.speedMph = throttle * 42;
        data_.courseDegrees = 92 + sinf(seconds * 0.08F) * 8;
        data_.engineHours = 124.7F + seconds / 3600;
        data_.warning = data_.scenario == DemoScenario::Warning;
        if (data_.warning) data_.coolantTempF = 195 + sinf(seconds) * 2;
        data_.dataAgeMs = 0;
        // Never integrate across a gap in simulated input.
        if (previousFresh_) {
            const double hours = deltaMs / 3600000.0;
            data_.tripDistanceMiles += (previousSpeed_ + data_.speedMph) * 0.5 * hours;
            const double fuel = (previousFlow_ + data_.fuelFlowGph) * 0.5 * hours;
            data_.tripFuelGallons += fuel;
            totalFuelGallons_ += fuel;
        }
        data_.economyMpg = data_.speedMph / data_.fuelFlowGph;
        data_.maxSpeedMph = fmaxf(data_.maxSpeedMph, data_.speedMph);
        data_.maxRpm = fmaxf(data_.maxRpm, data_.rpm);
        if (data_.rpm >= 2000 && !data_.warning) {
            if (data_.economyMpg > data_.bestEconomyMpg) {
                data_.bestEconomyMpg = data_.economyMpg;
                data_.bestRpm = data_.rpm;
                data_.bestTrimPercent = data_.trimPercent;
            }
            const int index = static_cast<int>((data_.rpm - 2000) / 500);
            if (index >= 0 && index < 6) {
                auto &sample = data_.cruise[index];
                if (!sample.observed || data_.economyMpg > sample.economyMpg)
                    sample = {data_.rpm, data_.speedMph, data_.fuelFlowGph,
                              data_.economyMpg, data_.trimPercent, true};
            }
        }
        previousSpeed_ = data_.speedMph;
        previousFlow_ = data_.fuelFlowGph;
    } else {
        data_.dataAgeMs = deltaMs > UINT32_MAX - data_.dataAgeMs ?
                          UINT32_MAX : data_.dataAgeMs + deltaMs;
        data_.warning = false;
    }
    // Demo tank only: 60 US gal, starting at 68%.
    data_.fuelLevelPercent = fmaxf(0, 68 - totalFuelGallons_ / 60 * 100);
    previousFresh_ = fresh;
    return data_;
}

void EngineSimulator::setScenario(DemoScenario scenario) {
    data_.scenario = scenario;
    previousFresh_ = false;
}

void EngineSimulator::resetTrip() {
    data_.tripElapsedMs = 0;
    data_.tripDistanceMiles = 0;
    data_.tripFuelGallons = 0;
    data_.maxSpeedMph = 0;
    data_.maxRpm = 0;
    data_.bestEconomyMpg = 0;
    data_.bestRpm = 0;
    data_.bestTrimPercent = 0;
    for (auto &sample : data_.cruise) sample = {};
    previousFresh_ = false;
}
