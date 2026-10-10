#include <Arduino.h>
#include <atomic>
#include <stdarg.h>
#include <esp_system.h>
#include "ScannerDiagnostics.h"
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

constexpr uint32_t CAN_ALERTS = TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL |
    TWAI_ALERT_ABOVE_ERR_WARN | TWAI_ALERT_BELOW_ERR_WARN | TWAI_ALERT_ERR_PASS |
    TWAI_ALERT_ERR_ACTIVE | TWAI_ALERT_BUS_OFF | TWAI_ALERT_BUS_RECOVERED;
struct RawFrame {
    uint32_t sequence, timeUs, identifier;
    uint8_t length, data[8];
};
struct Snapshot {
    BoatData boat;
    scanner::Stats stats;
    twai_status_info_t can{};
    bool canStatusValid = false;
    bool deltaValid = false;
    uint32_t nowMs = 0, sequence = 0, elapsedMs = 0;
    uint32_t busDelta = 0, missedDelta = 0, overrunDelta = 0;
    uint32_t alerts = 0, queueHighWater = 0, maxLoopUs = 0, receiveFailures = 0;
    uint32_t rawDrops = 0, heap = 0, psram = 0;
    uint32_t screenRefreshes = 0;
    uint32_t alertPollFailures = 0, statusFailures = 0;

};

Snapshot current;
QueueHandle_t snapshotQueue = nullptr;
uint32_t lastReportMs = 0;
uint32_t lastScreenMs = 0;
ScannerScreenStatus screenStatus;
QueueHandle_t rawQueue = nullptr;
std::atomic<bool> rawEnabled{false};
std::atomic<uint32_t> rawDrops{0};
uint32_t rawSequence = 0, snapshotSequence = 0;
uint32_t alertMask = 0, queueHighWater = 0, maxLoopUs = 0, receiveFailures = 0;
uint32_t alertPollFailures = 0, statusFailures = 0;
uint32_t previousLoopStartUs = 0;
bool loopStarted = false;
twai_status_info_t previousCan{};
bool previousCanValid = false;
uint32_t lastBusErrors = 0, lastErrorMs = 0;
bool busBaselineValid = false, recentBusErrors = false;
uint32_t logSequence = 0, incompleteWrites = 0, formatFailures = 0;

// One logger owns serial after setup. Every record is bounded, numbered and
// checksummed. A slow/disconnected host cannot hold the CAN receive task.
void logLine(const char *format, ...) {
    char payload[384], record[448];
    va_list args;
    va_start(args, format);
    const int n = vsnprintf(payload, sizeof(payload), format, args);
    va_end(args);
    if (n < 0 || size_t(n) >= sizeof(payload)) { ++formatFailures; return; }
    const uint32_t sequence = ++logSequence;
    const int length = snprintf(record, sizeof(record), "\n@%lu %s *%08lX\n",
        static_cast<unsigned long>(sequence), payload,
        static_cast<unsigned long>(scanner::checksum(payload, size_t(n))));
    if (length < 0 || size_t(length) >= sizeof(record)) { ++formatFailures; return; }
    size_t written = 0;
    const uint32_t start = millis();
    while (written < size_t(length) && uint32_t(millis() - start) < 250) {
        const int space = Serial.availableForWrite();
        if (!Serial || space <= 0) { vTaskDelay(pdMS_TO_TICKS(1)); continue; }
        const size_t remaining = size_t(length) - written;
        const size_t chunk = remaining < size_t(space) ? remaining : size_t(space);
        written += Serial.write(reinterpret_cast<const uint8_t *>(record) + written, chunk);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    if (written != size_t(length)) ++incompleteWrites;
}

void describeValue(char *out, size_t size, const TimedDouble &value, uint32_t nowMs) {
    if (!value.seen) snprintf(out, size, "UNKNOWN");
    else if (!value.available) snprintf(out, size, "NA");
    else if (!value.fresh(nowMs, scanner::RATE_WINDOW_MS)) snprintf(out, size, "STALE");
    else snprintf(out, size, "%.3f", value.value);
}


// Snapshot sequence gaps expose the latest-value mailbox overwriting old reports.
void loggerTask(void *) {
    logLine("BOOT firmware=scanner-diag-2 build=%s_%s reset=%d sdk=%s TX=15 RX=16 bitrate=250000 mode=LISTEN_ONLY rxQueue=64 rawQueue=256", __DATE__, __TIME__, int(esp_reset_reason()), ESP.getSdkVersion());
    const twai_timing_config_t timing = TWAI_TIMING_CONFIG_250KBITS();
    logLine("TIMING brp=%lu tseg1=%u tseg2=%u sjw=%u triple=%u commands=r(raw-on),s(raw-off)",
        static_cast<unsigned long>(timing.brp), timing.tseg_1, timing.tseg_2, timing.sjw, timing.triple_sampling);
    Snapshot snapshot;
    uint32_t lastSnapshot = 0, overwritten = 0;
    while (true) {
        while (Serial.available()) {
            const int command = Serial.read();
            if (command == 'r' || command == 's') {
                rawEnabled.store(command == 'r');
                logLine("MODE raw=%u rawDrops=%lu", rawEnabled.load(), static_cast<unsigned long>(rawDrops.load()));
            }
        }
        if (xQueueReceive(snapshotQueue, &snapshot, 0) == pdTRUE) {
            overwritten += snapshot.sequence - lastSnapshot - 1;
            lastSnapshot = snapshot.sequence;
            logLine("SNAP begin=%lu ms=%lu elapsedMs=%lu", static_cast<unsigned long>(snapshot.sequence), static_cast<unsigned long>(snapshot.nowMs), static_cast<unsigned long>(snapshot.elapsedMs));
            char rpm[32], sog[32], cog[32], fuel[32], capacity[32], trim[32];
            describeValue(rpm, sizeof(rpm), snapshot.boat.engine.rpm, snapshot.nowMs);
            describeValue(sog, sizeof(sog), snapshot.boat.navigation.speedOverGroundMs, snapshot.nowMs);
            describeValue(cog, sizeof(cog), snapshot.boat.navigation.courseOverGroundRad, snapshot.nowMs);
            describeValue(fuel, sizeof(fuel), snapshot.boat.fuelTank.levelPercent, snapshot.nowMs);
            describeValue(capacity, sizeof(capacity), snapshot.boat.fuelTank.capacityL, snapshot.nowMs);
            const auto &t = snapshot.boat.engine.trimPercent;
            if (!t.seen) snprintf(trim, sizeof(trim), "UNKNOWN");
            else if (!t.available) snprintf(trim, sizeof(trim), "NA");
            else if (!t.fresh(snapshot.nowMs, scanner::RATE_WINDOW_MS)) snprintf(trim, sizeof(trim), "STALE");
            else snprintf(trim, sizeof(trim), "%ld", static_cast<long>(t.value));
            if (snapshot.boat.engine.detected) logLine("ENGINE src=%u inst=%u rpm=%s trimPct=%s", snapshot.boat.engine.sourceAddress, snapshot.boat.engine.instance, rpm, trim);
            if (snapshot.boat.navigation.detected) logLine("GPS src=%u sogMs=%s cogRad=%s", snapshot.boat.navigation.sourceAddress, sog, cog);
            if (snapshot.boat.fuelTank.detected) logLine("FUEL src=%u inst=%u levelPct=%s capacityL=%s", snapshot.boat.fuelTank.sourceAddress, snapshot.boat.fuelTank.instance, fuel, capacity);
            for (const auto &stat : snapshot.stats.entries) if (stat.used)
                logLine("PGN pgn=%lu src=%u frames=%lu frameHz=%.2f ageMs=%lu", static_cast<unsigned long>(stat.pgn), stat.source, static_cast<unsigned long>(stat.count), snapshot.stats.rate(stat, snapshot.nowMs), static_cast<unsigned long>(uint32_t(snapshot.nowMs - stat.lastMs)));
            if (snapshot.canStatusValid) {
                const auto &c = snapshot.can;
                logLine("CAN state=%d queued=%lu highWater=%lu busTotal=%lu busDelta=%lu busHz=%.2f deltaValid=%u REC=%lu TEC=%lu", int(c.state), static_cast<unsigned long>(c.msgs_to_rx), static_cast<unsigned long>(snapshot.queueHighWater), static_cast<unsigned long>(c.bus_error_count), static_cast<unsigned long>(snapshot.busDelta), scanner::perSecond(snapshot.busDelta, snapshot.elapsedMs), snapshot.deltaValid, static_cast<unsigned long>(c.rx_error_counter), static_cast<unsigned long>(c.tx_error_counter));
                logLine("LOSS missed=%lu delta=%lu overruns=%lu deltaOverrun=%lu receiveFailures=%lu maxLoopUs=%lu alerts=0x%08lX", static_cast<unsigned long>(c.rx_missed_count), static_cast<unsigned long>(snapshot.missedDelta), static_cast<unsigned long>(c.rx_overrun_count), static_cast<unsigned long>(snapshot.overrunDelta), static_cast<unsigned long>(snapshot.receiveFailures), static_cast<unsigned long>(snapshot.maxLoopUs), static_cast<unsigned long>(snapshot.alerts));
            } else logLine("CAN status=UNAVAILABLE");
            logLine("LOG incompleteWrites=%lu formatFailures=%lu overwrittenSnapshots=%lu raw=%u rawDrops=%lu alertPollFailures=%lu statusFailures=%lu", static_cast<unsigned long>(incompleteWrites), static_cast<unsigned long>(formatFailures), static_cast<unsigned long>(overwritten), rawEnabled.load(), static_cast<unsigned long>(snapshot.rawDrops), static_cast<unsigned long>(snapshot.alertPollFailures), static_cast<unsigned long>(snapshot.statusFailures));
            logLine("SNAP end=%lu heap=%lu psram=%lu refreshes=%lu evictions=%lu", static_cast<unsigned long>(snapshot.sequence), static_cast<unsigned long>(snapshot.heap), static_cast<unsigned long>(snapshot.psram), static_cast<unsigned long>(snapshot.screenRefreshes), static_cast<unsigned long>(snapshot.stats.evictions));
        }
        // Bound each raw batch so commands and snapshots cannot be starved.
        RawFrame frame;
        for (unsigned i = 0; i < 16 && xQueueReceive(rawQueue, &frame, 0) == pdTRUE; ++i) {
            char hex[17];
            for (unsigned j = 0; j < frame.length; ++j) snprintf(hex + j * 2, 3, "%02X", frame.data[j]);
            hex[frame.length * 2] = 0;
            logLine("RAW seq=%lu us=%lu id=%08lX dlc=%u data=%s", static_cast<unsigned long>(frame.sequence), static_cast<unsigned long>(frame.timeUs), static_cast<unsigned long>(frame.identifier), frame.length, hex);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

bool startCan() {
    twai_general_config_t general = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_LISTEN_ONLY);
    general.rx_queue_len = 64;
    general.alerts_enabled = CAN_ALERTS;
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
    Serial.setTxTimeoutMs(10);
    delay(1500);
    Serial.println("V MAX Smart Gauge - NMEA 2000 Scanner");
    Serial.println("CAN TX=GPIO15 RX=GPIO16, 250 kbit/s, LISTEN ONLY");
    if (!startScannerScreen()) {
        Serial.println("Display initialization FAILED. Scanner stopped.");
        while (true) delay(1000);
    }
    snapshotQueue = xQueueCreate(1, sizeof(Snapshot));
    rawQueue = xQueueCreate(256, sizeof(RawFrame));
    if (!snapshotQueue || !rawQueue) {
        Serial.println("Logger initialization FAILED. Scanner stopped.");
        while (true) delay(1000);
    }
    if (!startCan()) {
        Serial.println("CAN initialization FAILED. Scanner stopped.");
        while (true) delay(1000);
    }
    previousCanValid = twai_get_status_info(&previousCan) == ESP_OK;
    if (previousCanValid) { lastBusErrors = previousCan.bus_error_count; busBaselineValid = true; }
    lastReportMs = millis();
    current.stats.resetWindow(lastReportMs);
    Serial.println("CAN initialized. Numbered diagnostic records; frame counts, not reassembled messages.");
    if (xTaskCreate(loggerTask, "scanner_log", 8192, nullptr, 1, nullptr) != pdPASS) {
        Serial.println("Logger task FAILED. Scanner stopped.");
        while (true) delay(1000);
    }
}

void loop() {
    const uint32_t loopStartUs = micros();
    if (loopStarted) {
        const uint32_t gap = loopStartUs - previousLoopStartUs;
        if (gap > maxLoopUs) maxLoopUs = gap;
    }
    previousLoopStartUs = loopStartUs; loopStarted = true;
    twai_message_t message{};
    const esp_err_t received = twai_receive(&message, pdMS_TO_TICKS(20));
    if (received != ESP_OK && received != ESP_ERR_TIMEOUT) ++receiveFailures;
    if (received == ESP_OK && message.extd && !message.rtr) {
        if (rawEnabled.load()) {
            RawFrame raw{};
            raw.sequence = ++rawSequence; raw.timeUs = micros(); raw.identifier = message.identifier;
            raw.length = message.data_length_code > 8 ? 8 : message.data_length_code;
            memcpy(raw.data, message.data, raw.length);
            if (xQueueSend(rawQueue, &raw, 0) != pdTRUE) ++rawDrops;
        }
        const uint32_t nowMs = millis();
        const uint32_t pgn = scanner::extractPgn(message.identifier);
        const uint8_t source = message.identifier & 0xFF;
        current.stats.record(pgn, source, nowMs);
        ++screenStatus.frames;
        screenStatus.lastFrameMs = nowMs;
        scanner::decode(current.boat, pgn, source, message.data, message.data_length_code, nowMs);
    }
    uint32_t raised = 0;
    const esp_err_t alertResult = twai_read_alerts(&raised, 0);
    if (alertResult == ESP_OK) alertMask |= raised;
    else if (alertResult != ESP_ERR_TIMEOUT) ++alertPollFailures;
    // Sample queue occupancy after each receive, before draining the next frame.
    twai_status_info_t sampled{};
    const bool sampledValid = twai_get_status_info(&sampled) == ESP_OK;
    if (!sampledValid) ++statusFailures;
    if (sampledValid && sampled.msgs_to_rx > queueHighWater) queueHighWater = sampled.msgs_to_rx;
    const uint32_t nowMs = millis();
    if (sampledValid) {
        if (busBaselineValid && sampled.bus_error_count != lastBusErrors) {
            recentBusErrors = true; lastErrorMs = nowMs;
        }
        lastBusErrors = sampled.bus_error_count; busBaselineValid = true;
    }
    if (uint32_t(nowMs - lastScreenMs) >= 80) {
        lastScreenMs = nowMs;
        screenStatus.heartbeatMs = nowMs;
        const twai_status_info_t &can = sampled;
        screenStatus.canStatusValid = sampledValid;
        screenStatus.recentBusErrors = recentBusErrors && uint32_t(nowMs - lastErrorMs) < scanner::RATE_WINDOW_MS;
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
    const uint32_t loopUs = micros() - loopStartUs;
    if (loopUs > maxLoopUs) maxLoopUs = loopUs;
    if (uint32_t(nowMs - lastReportMs) >= scanner::RATE_WINDOW_MS) {
        current.nowMs = nowMs;
        current.canStatusValid = sampledValid;
        current.can = sampled;
        current.sequence = ++snapshotSequence;
        current.elapsedMs = nowMs - lastReportMs;
        current.busDelta = current.missedDelta = current.overrunDelta = 0;
        current.deltaValid = sampledValid && previousCanValid;
        if (current.deltaValid) {
            current.busDelta = scanner::counterDelta(sampled.bus_error_count, previousCan.bus_error_count);
            current.missedDelta = scanner::counterDelta(sampled.rx_missed_count, previousCan.rx_missed_count);
            current.overrunDelta = scanner::counterDelta(sampled.rx_overrun_count, previousCan.rx_overrun_count);
        }
        previousCanValid = sampledValid; previousCan = sampled;
        current.alerts = alertMask; current.queueHighWater = queueHighWater;
        current.maxLoopUs = maxLoopUs; current.receiveFailures = receiveFailures;
        current.rawDrops = rawDrops.load(); current.heap = ESP.getFreeHeap(); current.psram = ESP.getFreePsram();
        current.screenRefreshes = scannerScreenRefreshCount();
        current.alertPollFailures = alertPollFailures; current.statusFailures = statusFailures;
        alertMask = queueHighWater = maxLoopUs = 0;
        // A slow host gets the latest snapshot; it never stalls CAN reception.
        xQueueOverwrite(snapshotQueue, &current);
        current.stats.resetWindow(nowMs);
        lastReportMs = nowMs;
        // Serial diagnostics run in loggerTask so USB backpressure cannot stall reception.
    }
}
