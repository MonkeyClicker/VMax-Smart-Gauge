#pragma once

#include <Arduino.h>

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
};

class EngineSimulator {
public:
    const SimulatedEngineData &update(uint32_t nowMs);
private:
    SimulatedEngineData data_{};
};

