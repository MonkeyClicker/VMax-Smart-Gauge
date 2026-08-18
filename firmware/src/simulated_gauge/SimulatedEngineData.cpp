#include "SimulatedEngineData.h"

#include <math.h>

const SimulatedEngineData &EngineSimulator::update(uint32_t nowMs) {
    const float seconds = nowMs / 1000.0F;
    const float cycle = fmodf(seconds, 45.0F);
    float throttle = 0.0F;
    if (cycle < 8.0F) {
        throttle = cycle / 8.0F;
    } else if (cycle < 30.0F) {
        throttle = 1.0F;
    } else {
        throttle = 1.0F - ((cycle - 30.0F) / 15.0F);
    }

    data_.rpm = 700.0F + throttle * 3800.0F + sinf(seconds * 2.3F) * 35.0F;
    data_.trimPercent = 12.0F + throttle * 26.0F;
    data_.coolantTempF = 126.0F + throttle * 22.0F + sinf(seconds * 0.15F) * 2.0F;
    data_.oilPressurePsi = 24.0F + throttle * 38.0F;
    data_.batteryVoltage = 13.8F + throttle * 0.45F;
    data_.fuelFlowGph = 0.8F + throttle * throttle * 13.5F;
    data_.speedMph = throttle * 42.0F;
    data_.engineHours = 124.7F + seconds / 3600.0F;
    data_.warning = false;
    return data_;
}
