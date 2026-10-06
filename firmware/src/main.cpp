#include <Arduino.h>
#include <driver/twai.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "Scanner.h"
#include "scanner/ScannerScreen.h"

namespace {
constexpr gpio_num_t CAN_TX_PIN = GPIO_NUM_15;
constexpr gpio_num_t CAN_RX_PIN = GPIO_NUM_16;
constexpr uint32_t SERIAL_BAUD = 115200;

struct Snapshot {
    BoatData boat;
    scanner::Stats stats;
    twai_status_info_t can{};
    bool canStatusValid = false;
    uint32_t nowMs = 0;
};

Snapshot current;
QueueHandle_t snapshotQueue = nullptr;
uint32_t lastReportMs = 0;
uint32_t lastScreenMs = 0;
ScannerScreenStatus screenStatus;

void printValue(const TimedDouble &value, uint32_t nowMs, const char *units) {
    if (!value.seen) Serial.print("UNKNOWN");
    else if (!value.available) Serial.print("NA");
    else if (!value.fresh(nowMs, scanner::RATE_WINDOW_MS)) Serial.print("STALE");
    else Serial.printf("%.3f%s", value.value, units);
}

// Serial backpressure can block this task, but never the CAN receive loop.
void loggerTask(void *) {
    Snapshot snapshot;
    while (true) {
        if (xQueueReceive(snapshotQueue, &snapshot, portMAX_DELAY) != pdTRUE) continue;
        const auto &boat = snapshot.boat;
        Serial.printf("\n--- NMEA SNAPSHOT timeMs=%lu ---\n", static_cast<unsigned long>(snapshot.nowMs));
        if (boat.engine.detected) {
            Serial.printf("ENGINE src=%u inst=%u rpm=", boat.engine.sourceAddress, boat.engine.instance);
            printValue(boat.engine.rpm, snapshot.nowMs, "");
            Serial.print(" trim=");
            const auto &trim = boat.engine.trimPercent;
            if (!trim.seen) Serial.println("UNKNOWN");
            else if (!trim.available) Serial.println("NA");
            else if (!trim.fresh(snapshot.nowMs, scanner::RATE_WINDOW_MS)) Serial.println("STALE");
            else Serial.printf("%ld%%\n", static_cast<long>(trim.value));
        }
        if (boat.navigation.detected) {
            Serial.printf("GPS src=%u sog=", boat.navigation.sourceAddress);
            printValue(boat.navigation.speedOverGroundMs, snapshot.nowMs, " m/s");
            Serial.print(" cog=");
            printValue(boat.navigation.courseOverGroundRad, snapshot.nowMs, " rad");
            Serial.println();
        }
        if (boat.fuelTank.detected) {
            Serial.printf("FUEL src=%u inst=%u level=", boat.fuelTank.sourceAddress, boat.fuelTank.instance);
            printValue(boat.fuelTank.levelPercent, snapshot.nowMs, "%");
            Serial.print(" capacity=");
            printValue(boat.fuelTank.capacityL, snapshot.nowMs, " L");
            Serial.println();
        }
        Serial.println("PGN      SRC   FRAMES     FRAME_RATE_HZ  AGE_MS");
        for (const auto &stat : snapshot.stats.entries) {
            if (!stat.used) continue;
            Serial.printf("%-8lu %-5u %-10lu %.2f %lu\n",
                          static_cast<unsigned long>(stat.pgn), stat.source,
                          static_cast<unsigned long>(stat.count), snapshot.stats.rate(stat, snapshot.nowMs),
                          static_cast<unsigned long>(uint32_t(snapshot.nowMs - stat.lastMs)));
        }
        Serial.printf("STATS evictions=%lu (counts reset when a pair is evicted)\n",
                      static_cast<unsigned long>(snapshot.stats.evictions));
        if (snapshot.canStatusValid) {
            Serial.printf("CAN state=%d queued=%lu missed=%lu overruns=%lu busErrors=%lu rxErrors=%lu\n",
                          static_cast<int>(snapshot.can.state),
                          static_cast<unsigned long>(snapshot.can.msgs_to_rx),
                          static_cast<unsigned long>(snapshot.can.rx_missed_count),
                          static_cast<unsigned long>(snapshot.can.rx_overrun_count),
                          static_cast<unsigned long>(snapshot.can.bus_error_count),
                          static_cast<unsigned long>(snapshot.can.rx_error_counter));
        } else Serial.println("CAN status unavailable");
        Serial.printf("SCREEN refreshes=%lu free heap=%u free PSRAM=%u\n",
                      static_cast<unsigned long>(scannerScreenRefreshCount()), ESP.getFreeHeap(), ESP.getFreePsram());
    }
}

bool startCan() {
    twai_general_config_t general = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_LISTEN_ONLY);
    general.rx_queue_len = 64;
    general.tx_queue_len = 0;
    twai_timing_config_t timing = TWAI_TIMING_CONFIG_250KBITS();
    twai_filter_config_t filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    esp_err_t err = twai_driver_install(&general, &timing, &filter);
    if (err != ESP_OK) {
        Serial.printf("TWAI driver install failed: %d\n", err);
        return false;
    }
    err = twai_start();
    if (err != ESP_OK) {
        Serial.printf("TWAI start failed: %d\n", err);
        twai_driver_uninstall();
        return false;
    }
    return true;
}
} // namespace

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(1500);
    Serial.println("V MAX Smart Gauge - NMEA 2000 Scanner");
    Serial.println("CAN TX=GPIO15 RX=GPIO16, 250 kbit/s, LISTEN ONLY");
    if (!startScannerScreen()) {
        Serial.println("Display initialization FAILED. Scanner stopped.");
        while (true) delay(1000);
    }
    snapshotQueue = xQueueCreate(1, sizeof(Snapshot));
    if (!snapshotQueue || xTaskCreate(loggerTask, "scanner_log", 8192, nullptr, 1, nullptr) != pdPASS) {
        Serial.println("Logger initialization FAILED. Scanner stopped.");
        while (true) delay(1000);
    }
    if (!startCan()) {
        Serial.println("CAN initialization FAILED. Scanner stopped.");
        while (true) delay(1000);
    }
    lastReportMs = millis();
    current.stats.resetWindow(lastReportMs);
    Serial.println("CAN initialized. Five-second diagnostic snapshots; frame counts, not reassembled messages.");
}

void loop() {
    twai_message_t message{};
    if (twai_receive(&message, pdMS_TO_TICKS(20)) == ESP_OK && message.extd && !message.rtr) {
        const uint32_t nowMs = millis();
        const uint32_t pgn = scanner::extractPgn(message.identifier);
        const uint8_t source = message.identifier & 0xFF;
        current.stats.record(pgn, source, nowMs);
        ++screenStatus.frames;
        screenStatus.lastFrameMs = nowMs;
        scanner::decode(current.boat, pgn, source, message.data, message.data_length_code, nowMs);
    }
    const uint32_t nowMs = millis();
    if (uint32_t(nowMs - lastScreenMs) >= 80) {
        lastScreenMs = nowMs;
        screenStatus.heartbeatMs = nowMs;
        twai_status_info_t can{};
        screenStatus.canStatusValid = twai_get_status_info(&can) == ESP_OK;
        screenStatus.canRunning = screenStatus.canStatusValid && can.state == TWAI_STATE_RUNNING;
        screenStatus.busErrors = can.bus_error_count;
        screenStatus.losses = can.rx_missed_count + can.rx_overrun_count;
        screenStatus.trackedPgns = 0;
        for (size_t i = 0; i < scanner::MAX_TRACKED_PGNS; ++i) {
            if (!current.stats.entries[i].used) continue;
            bool duplicate = false;
            for (size_t j = 0; j < i; ++j)
                if (current.stats.entries[j].used && current.stats.entries[j].pgn == current.stats.entries[i].pgn)
                    duplicate = true;
            if (!duplicate) ++screenStatus.trackedPgns;
        }
        publishScannerScreen(screenStatus);
    }
    if (uint32_t(nowMs - lastReportMs) >= scanner::RATE_WINDOW_MS) {
        current.nowMs = nowMs;
        current.canStatusValid = twai_get_status_info(&current.can) == ESP_OK;
        // A slow host gets the latest snapshot; it never stalls CAN reception.
        xQueueOverwrite(snapshotQueue, &current);
        current.stats.resetWindow(nowMs);
        lastReportMs = nowMs;
        // Serial diagnostics run in loggerTask so USB backpressure cannot stall reception.
    }
}
