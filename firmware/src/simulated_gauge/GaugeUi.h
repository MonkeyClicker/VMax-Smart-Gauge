#pragma once

#include "SimulatedEngineData.h"

// Call under the LVGL lock. Touch callbacks queue changes for the next update.
void createGaugeUi(EngineSimulator &simulator);
void updateGaugeUi(uint32_t nowMs);
void handleGaugeUiCommand(char command);
