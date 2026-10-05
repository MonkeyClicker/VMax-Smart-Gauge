#pragma once

#include <stdint.h>
#include <math.h>

struct TimedDouble {
    double value = NAN;
    uint32_t updatedMs = 0;
    bool seen = false;
    bool available = false;

    void set(double newValue, uint32_t nowMs) {
        value = newValue;
        updatedMs = nowMs;
        seen = true;
        available = true;
    }

    void invalidate(uint32_t nowMs) {
        updatedMs = nowMs;
        seen = true;
        available = false;
    }

    bool fresh(uint32_t nowMs, uint32_t timeoutMs) const {
        return seen && available && (uint32_t)(nowMs - updatedMs) <= timeoutMs;
    }
};

struct TimedInt {
    int32_t value = 0;
    uint32_t updatedMs = 0;
    bool seen = false;
    bool available = false;

    void set(int32_t newValue, uint32_t nowMs) {
        value = newValue;
        updatedMs = nowMs;
        seen = true;
        available = true;
    }

    void invalidate(uint32_t nowMs) {
        updatedMs = nowMs;
        seen = true;
        available = false;
    }

    bool fresh(uint32_t nowMs, uint32_t timeoutMs) const {
        return seen && available && (uint32_t)(nowMs - updatedMs) <= timeoutMs;
    }
};

struct EngineData {
    uint8_t instance = 0;
    uint8_t sourceAddress = 0xFF;
    bool detected = false;

    TimedDouble rpm;
    TimedInt trimPercent;

    // Added when PGN 127489 fast-packet decoding is implemented.
    TimedDouble coolantTempK;
    TimedDouble alternatorVoltage;
    TimedDouble fuelRateLph;
    TimedDouble engineHoursSec;
    TimedInt engineLoadPercent;
    TimedInt engineTorquePercent;
};

struct NavigationData {
    uint8_t sourceAddress = 0xFF;
    bool detected = false;
    TimedDouble speedOverGroundMs;
    TimedDouble courseOverGroundRad;
};

struct FuelTankData {
    uint8_t instance = 0;
    uint8_t sourceAddress = 0xFF;
    bool detected = false;

    TimedDouble levelPercent;
    TimedDouble capacityL;
};

struct BoatData {
    EngineData engine;
    NavigationData navigation;
    FuelTankData fuelTank;
};
