#pragma once

#include <stdint.h>

enum class DemoScenario : uint8_t { Normal, Warning, Offline, Unavailable };

struct CruiseSample {
    float rpm = 0;
    float speedMph = 0;
    float fuelFlowGph = 0;
    float economyMpg = 0;
    float trimPercent = 0;
    bool observed = false;
};

struct SimulatedEngineData {
    float rpm = 750.0F;
    float trimPercent = 18.0F;
    float coolantTempF = 128.0F;
    float oilPressurePsi = 42.0F;
    float batteryVoltage = 14.2F;
    float fuelFlowGph = 1.1F;
    float speedMph = 0.0F;
    float engineHours = 124.7F;
    bool warning = false;
    DemoScenario scenario = DemoScenario::Normal;
    float fuelLevelPercent = 68;
    float economyMpg = 0;
    float courseDegrees = 92;
    uint64_t tripElapsedMs = 0;
    double tripDistanceMiles = 0;
    double tripFuelGallons = 0;
    float maxSpeedMph = 0;
    float maxRpm = 0;
    float bestEconomyMpg = 0;
    float bestRpm = 0;
    float bestTrimPercent = 0;
    uint32_t dataAgeMs = 0;
    CruiseSample cruise[6]{};
};

class EngineSimulator {
public:
    const SimulatedEngineData &update(uint32_t nowMs);
    void setScenario(DemoScenario scenario);
    void resetTrip();
private:
    SimulatedEngineData data_{};
    bool started_ = false;
    bool previousFresh_ = false;
    uint32_t previousMs_ = 0;
    uint64_t elapsedMs_ = 0;
    double totalFuelGallons_ = 0;
    float previousSpeed_ = 0;
    float previousFlow_ = 0;
};

