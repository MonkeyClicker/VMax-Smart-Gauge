#pragma once
#include "LiveModel.h"
namespace liveui {
void start();
void update(const live::Snapshot &snapshot, uint32_t now, uint32_t rawDrops, uint32_t logFailures, bool rawActive);
bool takeTripReset();
bool takeSourceReset();
void fault(const char *message);
}
