#pragma once
#include <stdint.h>
#include <stddef.h>
namespace scanner {
inline uint32_t counterDelta(uint32_t current, uint32_t previous) { return current - previous; }
inline double perSecond(uint32_t count, uint32_t elapsedMs) {
    return elapsedMs ? count * 1000.0 / elapsedMs : 0.0;
}
inline uint32_t checksum(const char *text, size_t length) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; ++i) { hash ^= uint8_t(text[i]); hash *= 16777619u; }
    return hash;
}
}
