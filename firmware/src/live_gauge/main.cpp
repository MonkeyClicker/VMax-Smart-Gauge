#include <Arduino.h>
#include <atomic>
#include <cstdarg>
#include <driver/twai.h>
#include <esp_display_panel.hpp>
#include <esp_system.h>
#include <esp_timer.h>
#include <freertos/queue.h>
#include "LiveModel.h"
#include "ScannerLogging.h"
#include "GaugeUi.h"
#include "../simulated_gauge/LvglPort.h"

using namespace esp_panel::board;
using namespace esp_panel::drivers;
namespace {
Board board;
live::Model model;
QueueHandle_t displayQueue = nullptr, reportQueue = nullptr, rawQueue = nullptr;
std::atomic<bool> rawEnabled{false}, resetTrip{false}, resetSources{false};
std::atomic<uint32_t> rawDrops{0}, writeFailures{0};
struct RawFrame { uint64_t timeUs; uint32_t sequence, id; uint8_t length, bytes[8]; };
scanner::LogControl logControl;
scanner::LogBudget logBudget;
uint32_t logSequence = 0, rawSequence = 0, skippedRecords = 0;

void serviceControl() {
    logControl.observeConnection(bool(Serial));
    for (unsigned i = 0; i < 32 && Serial.available(); ++i) logControl.command(Serial.read());
    rawEnabled.store(logControl.raw && logControl.connected);
}
struct UsbTransport {
    uint32_t nowMs() { return millis(); }
    void service() { serviceControl(); }
    bool connected() { return logControl.connected; }
    int space() { return Serial.availableForWrite(); }
    size_t write(const char *data, size_t size) { return Serial.write(reinterpret_cast<const uint8_t *>(data), size); }
    void yield() { vTaskDelay(pdMS_TO_TICKS(1)); }
};
bool logLine(const char *format, ...) {
    serviceControl();
    if (!logControl.connected || logBudget.expired(millis())) { ++skippedRecords; return false; }
    char payload[384], record[448];
    va_list args; va_start(args, format);
    int n = vsnprintf(payload, sizeof(payload), format, args); va_end(args);
    if (n < 0 || size_t(n) >= sizeof(payload)) { ++writeFailures; return false; }
    int length = scanner::formatRecord(record, sizeof(record), ++logSequence, payload);
    if (length < 0) { ++writeFailures; return false; }
    UsbTransport transport;
    if (!scanner::writeRecord(transport, record, size_t(length), logBudget)) { ++writeFailures; return false; }
    return true;
}
void loggerTask(void *) {
    live::Snapshot snapshot;
    while (true) {
        serviceControl(); logBudget = scanner::LogBudget{millis(), 250};
        if (!logControl.connected) { vTaskDelay(pdMS_TO_TICKS(10)); continue; }
        if (logControl.metadataPending) {
            const uint32_t request = logControl.metadataRequest;
            const bool complete = logLine("BOOT firmware=live-gauge-%s recordFormat=scanner-diag-3 build=%s_%s reset=%d TX=15 RX=16 bitrate=250000 mode=LISTEN_ONLY rxQueue=128 rawQueue=512 rawTime=uptime_us64 commands=r(raw-on),s(raw-off),i(info)",
                live::VERSION, __DATE__, __TIME__, int(esp_reset_reason()));
            logControl.metadataResult(complete, request);
            if (!complete) { vTaskDelay(pdMS_TO_TICKS(1)); continue; }
        }
        if (logControl.modePending) {
            logControl.modePending = false;
            if (!logLine("MODE raw=%u rawDrops=%lu", logControl.raw, (unsigned long)rawDrops.load())) logControl.modePending = true;
        }
        if (!logBudget.expired(millis()) && xQueueReceive(reportQueue, &snapshot, 0) == pdTRUE) {
            const auto &d = snapshot.diagnostics;
            logLine("LIVE ms=%lu frames=%lu dynamic=%lu packetsDropped=%lu canReady=%u statusValid=%u state=%d busErrors=%lu missed=%lu overruns=%lu REC=%lu TEC=%lu raw=%u rawDrops=%lu writeFailures=%lu skippedRecords=%lu heap=%u",
                (unsigned long)d.statusAt, (unsigned long)d.frames, (unsigned long)d.dynamicMessages,
                (unsigned long)d.packetDrops, d.canReady, d.statusValid, d.canState,
                (unsigned long)d.busErrors, (unsigned long)d.missed, (unsigned long)d.overruns,
                (unsigned long)d.rxErrors, (unsigned long)d.txErrors, logControl.raw,
                (unsigned long)rawDrops.load(), (unsigned long)writeFailures.load(), (unsigned long)skippedRecords, ESP.getFreeHeap());
        }
        RawFrame raw;
        for (unsigned i = 0; i < 32 && !logBudget.expired(millis()) && logControl.connected && xQueueReceive(rawQueue, &raw, 0) == pdTRUE; ++i) {
            char hex[17]; for (unsigned j = 0; j < raw.length; ++j) snprintf(hex + j * 2, 3, "%02X", raw.bytes[j]);
            hex[raw.length * 2] = 0;
            logLine("RAW seq=%lu us=%llu id=%08lX dlc=%u data=%s", (unsigned long)raw.sequence,
                (unsigned long long)raw.timeUs, (unsigned long)raw.id, raw.length, hex);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
void receiveTask(void *) {
    uint32_t lastPublish = 0, lastReport = 0;
    while (true) {
        if (resetSources.exchange(false)) model.resetSources();
        else if (resetTrip.exchange(false)) model.resetTrip();
        // A dedicated receiver drains bursts without waiting for display or USB.
        twai_message_t message{};
        for (unsigned i = 0; i < 128; ++i) {
            const auto result = twai_receive(&message, i == 0 ? pdMS_TO_TICKS(5) : 0);
            if (result != ESP_OK) break;
            if (!message.extd || message.rtr || message.data_length_code > 8) continue;
            const uint32_t now = millis();
            model.frame(message.identifier, message.data, message.data_length_code, now);
            if (rawEnabled.load()) {
                RawFrame raw{}; raw.sequence = ++rawSequence; raw.timeUs = uint64_t(esp_timer_get_time()); raw.id = message.identifier;
                raw.length = message.data_length_code; memcpy(raw.bytes, message.data, raw.length);
                if (xQueueSend(rawQueue, &raw, 0) != pdTRUE) ++rawDrops;
            }
        }
        const uint32_t now = millis();
        if (uint32_t(now - lastPublish) >= 100) {
            uint32_t raised = 0;
            twai_read_alerts(&raised, 0); // Counters/state below determine health.
            twai_status_info_t can{};
            const bool valid = twai_get_status_info(&can) == ESP_OK;
            model.canStatus(valid, true, int(can.state), can.bus_error_count, can.rx_missed_count,
                can.rx_overrun_count, can.rx_error_counter, can.tx_error_counter, now);
            model.tick(now); xQueueOverwrite(displayQueue, &model.state); lastPublish = now;
            if (uint32_t(now - lastReport) >= 5000) { xQueueOverwrite(reportQueue, &model.state); lastReport = now; }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
[[noreturn]] void fatal(const char *message, bool uiReady = false) {
    Serial.println(message);
    if (uiReady && lockLvgl()) { liveui::fault(message); unlockLvgl(); }
    while (true) delay(1000);
}
bool startCan() {
    twai_general_config_t general = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_15, GPIO_NUM_16, TWAI_MODE_LISTEN_ONLY);
    general.rx_queue_len = 128; general.tx_queue_len = 0;
    general.alerts_enabled = TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL |
        TWAI_ALERT_ABOVE_ERR_WARN | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_OFF;
    twai_timing_config_t timing = TWAI_TIMING_CONFIG_250KBITS();
    twai_filter_config_t filter = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    if (twai_driver_install(&general, &timing, &filter) != ESP_OK) return false;
    if (twai_start() == ESP_OK) return true;
    twai_driver_uninstall(); return false;
}
} // namespace

void setup() {
    Serial.begin(115200); Serial.setTxTimeoutMs(10);
    if (!ESP.getPsramSize()) fatal("PSRAM unavailable - expected OPI PSRAM");
    if (!board.init()) fatal("Display initialization failed");
    auto *lcd = board.getLCD(); if (!lcd) fatal("LCD unavailable");
    auto *bus = lcd->getBus();
    if (bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB)
        static_cast<BusRGB *>(bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
    if (!board.begin() || !startLvgl(board)) fatal("Display startup failed");
    if (!lockLvgl()) fatal("Display lock failed");
    liveui::start(); unlockLvgl();
    displayQueue = xQueueCreate(1, sizeof(live::Snapshot));
    reportQueue = xQueueCreate(1, sizeof(live::Snapshot));
    rawQueue = xQueueCreate(512, sizeof(RawFrame));
    if (!displayQueue || !reportQueue || !rawQueue) fatal("Receive/log queue allocation failed", true);
    if (!startCan()) fatal("CAN receiver startup failed", true);
    if (xTaskCreatePinnedToCore(receiveTask, "live_can", 8192, nullptr, 5, nullptr, 0) != pdPASS)
        fatal("CAN receive task failed", true);
    if (xTaskCreatePinnedToCore(loggerTask, "live_log", 8192, nullptr, 1, nullptr, 0) != pdPASS)
        fatal("USB logging task failed", true);
}
void loop() {
    static live::Snapshot snapshot;
    xQueueReceive(displayQueue, &snapshot, 0);
    if (lockLvgl(50)) {
        liveui::update(snapshot, millis(), rawDrops.load(), writeFailures.load(), rawEnabled.load());
        if (liveui::takeSourceReset()) resetSources.store(true);
        if (liveui::takeTripReset()) resetTrip.store(true);
        unlockLvgl();
    }
    delay(100);
}
