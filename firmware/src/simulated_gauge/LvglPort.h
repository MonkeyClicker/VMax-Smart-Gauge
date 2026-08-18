#pragma once

#include <esp_display_panel.hpp>
#include <lvgl.h>

bool startLvgl(esp_panel::board::Board &board);
bool lockLvgl(uint32_t timeoutMs = 1000);
void unlockLvgl();

