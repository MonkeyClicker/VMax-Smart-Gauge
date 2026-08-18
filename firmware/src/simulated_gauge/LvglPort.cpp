#include "LvglPort.h"

#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

using esp_panel::drivers::LCD;
using esp_panel::drivers::Touch;
using esp_panel::drivers::TouchPoint;

namespace {
SemaphoreHandle_t lvglMutex = nullptr;
esp_timer_handle_t tickTimer = nullptr;
LCD *lcd = nullptr;
Touch *touch = nullptr;
lv_disp_draw_buf_t drawBuffer;
lv_disp_drv_t displayDriver;
lv_indev_drv_t touchDriver;
lv_color_t *buffer1 = nullptr;
lv_color_t *buffer2 = nullptr;

void tick(void *) { lv_tick_inc(2); }

void flush(lv_disp_drv_t *driver, const lv_area_t *area, lv_color_t *colors) {
    auto *panel = static_cast<LCD *>(driver->user_data);
    panel->drawBitmap(area->x1, area->y1,
                      area->x2 - area->x1 + 1,
                      area->y2 - area->y1 + 1,
                      reinterpret_cast<const uint8_t *>(colors));
    lv_disp_flush_ready(driver);
}

void readTouch(lv_indev_drv_t *driver, lv_indev_data_t *data) {
    auto *panel = static_cast<Touch *>(driver->user_data);
    TouchPoint point{};
    if (panel != nullptr && panel->readPoints(&point, 1, 0) > 0) {
        data->point.x = point.x;
        data->point.y = point.y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void lvglTask(void *) {
    while (true) {
        if (lockLvgl(1000)) {
            const uint32_t waitMs = lv_timer_handler();
            unlockLvgl();
            vTaskDelay(pdMS_TO_TICKS(constrain(waitMs, 2U, 20U)));
        }
    }
}
} // namespace

bool lockLvgl(uint32_t timeoutMs) {
    return lvglMutex != nullptr &&
           xSemaphoreTakeRecursive(lvglMutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}

void unlockLvgl() {
    if (lvglMutex != nullptr) xSemaphoreGiveRecursive(lvglMutex);
}

bool startLvgl(esp_panel::board::Board &board) {
    lcd = board.getLCD();
    touch = board.getTouch();
    if (lcd == nullptr) return false;

    lvglMutex = xSemaphoreCreateRecursiveMutex();
    if (lvglMutex == nullptr) return false;
    lv_init();

    const uint32_t pixels = lcd->getFrameWidth() * 40U;
    buffer1 = static_cast<lv_color_t *>(heap_caps_malloc(
        pixels * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    buffer2 = static_cast<lv_color_t *>(heap_caps_malloc(
        pixels * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    if (buffer1 == nullptr || buffer2 == nullptr) return false;

    lv_disp_draw_buf_init(&drawBuffer, buffer1, buffer2, pixels);
    lv_disp_drv_init(&displayDriver);
    displayDriver.hor_res = lcd->getFrameWidth();
    displayDriver.ver_res = lcd->getFrameHeight();
    displayDriver.flush_cb = flush;
    displayDriver.draw_buf = &drawBuffer;
    displayDriver.user_data = lcd;
    if (lv_disp_drv_register(&displayDriver) == nullptr) return false;

    if (touch != nullptr) {
        lv_indev_drv_init(&touchDriver);
        touchDriver.type = LV_INDEV_TYPE_POINTER;
        touchDriver.read_cb = readTouch;
        touchDriver.user_data = touch;
        lv_indev_drv_register(&touchDriver);
    }

    const esp_timer_create_args_t timerArgs = {
        .callback = tick,
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "lvgl_tick",
        .skip_unhandled_events = true,
    };
    if (esp_timer_create(&timerArgs, &tickTimer) != ESP_OK ||
        esp_timer_start_periodic(tickTimer, 2000) != ESP_OK) return false;

    return xTaskCreatePinnedToCore(lvglTask, "lvgl", 8192, nullptr, 2, nullptr, 1) == pdPASS;
}
