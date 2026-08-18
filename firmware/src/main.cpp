#include <Arduino.h>
#include <driver/twai.h>

#include "BoatData.h"

namespace {

constexpr gpio_num_t CAN_TX_PIN = GPIO_NUM_15;
constexpr gpio_num_t CAN_RX_PIN = GPIO_NUM_16;
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t STATS_INTERVAL_MS = 5000;
constexpr size_t MAX_TRACKED_PGNS = 64;

BoatData boat;

struct PgnStat {
    uint32_t pgn = 0;
    uint8_t source = 0xFF;
    uint32_t count = 0;
    uint32_t firstMs = 0;
    uint32_t lastMs = 0;
    bool used = false;
};

PgnStat pgnStats[MAX_TRACKED_PGNS];
uint32_t lastStatsPrintMs = 0;

uint32_t extractPgn(uint32_t canId) {
    const uint8_t dataPage = (canId >> 24) & 0x01;
    const uint8_t pduFormat = (canId >> 16) & 0xFF;
    const uint8_t pduSpecific = (canId >> 8) & 0xFF;

    if (pduFormat < 240) {
        return (static_cast<uint32_t>(dataPage) << 16) |
               (static_cast<uint32_t>(pduFormat) << 8);
    }

    return (static_cast<uint32_t>(dataPage) << 16) |
           (static_cast<uint32_t>(pduFormat) << 8) |
           pduSpecific;
}

uint8_t extractSource(uint32_t canId) {
    return canId & 0xFF;
}

PgnStat* findOrCreateStat(uint32_t pgn, uint8_t source, uint32_t nowMs) {
    for (auto &stat : pgnStats) {
        if (stat.used && stat.pgn == pgn && stat.source == source) {
            return &stat;
        }
    }

    for (auto &stat : pgnStats) {
        if (!stat.used) {
            stat.used = true;
            stat.pgn = pgn;
            stat.source = source;
            stat.count = 0;
            stat.firstMs = nowMs;
            stat.lastMs = nowMs;
            return &stat;
        }
    }

    return nullptr;
}

void recordPgn(uint32_t pgn, uint8_t source, uint32_t nowMs) {
    PgnStat* stat = findOrCreateStat(pgn, source, nowMs);
    if (!stat) return;

    ++stat->count;
    stat->lastMs = nowMs;
}

void parseEngineRapid(const twai_message_t &msg, uint8_t source, uint32_t nowMs) {
    // PGN 127488 - Engine Parameters, Rapid Update
    // Byte 0: engine instance
    // Bytes 1-2: engine speed, 0.25 RPM/bit, little-endian
    // Byte 5: tilt/trim, signed percent
    if (msg.data_length_code < 6) return;

    const uint8_t instance = msg.data[0];
    const uint16_t rawRpm = static_cast<uint16_t>(msg.data[1]) |
                            (static_cast<uint16_t>(msg.data[2]) << 8);

    boat.engine.detected = true;
    boat.engine.instance = instance;
    boat.engine.sourceAddress = source;

    if (rawRpm != 0xFFFF) {
        boat.engine.rpm.set(rawRpm * 0.25, nowMs);
    }

    const int8_t rawTrim = static_cast<int8_t>(msg.data[5]);
    if (rawTrim != INT8_MAX) {
        boat.engine.trimPercent.set(rawTrim, nowMs);
    }

    Serial.printf("ENGINE pgn=127488 src=%u inst=%u rpm=", source, instance);
    if (rawRpm == 0xFFFF) {
        Serial.print("NA");
    } else {
        Serial.printf("%.0f", boat.engine.rpm.value);
    }

    Serial.print(" trim=");
    if (rawTrim == INT8_MAX) {
        Serial.println("NA");
    } else {
        Serial.printf("%d%%\n", rawTrim);
    }
}

void parseCogSogRapid(const twai_message_t &msg, uint8_t source, uint32_t nowMs) {
    // PGN 129026 - COG & SOG, Rapid Update
    // Bytes 2-3: COG, 0.0001 rad/bit
    // Bytes 4-5: SOG, 0.01 m/s/bit
    if (msg.data_length_code < 6) return;

    const uint16_t rawCog = static_cast<uint16_t>(msg.data[2]) |
                            (static_cast<uint16_t>(msg.data[3]) << 8);
    const uint16_t rawSog = static_cast<uint16_t>(msg.data[4]) |
                            (static_cast<uint16_t>(msg.data[5]) << 8);

    if (rawCog != 0xFFFF) {
        boat.navigation.courseOverGroundRad.set(rawCog * 0.0001, nowMs);
    }
    if (rawSog != 0xFFFF) {
        boat.navigation.speedOverGroundMs.set(rawSog * 0.01, nowMs);
    }

    Serial.printf("GPS pgn=129026 src=%u sog=", source);
    if (rawSog == 0xFFFF) {
        Serial.print("NA");
    } else {
        Serial.printf("%.2f m/s", boat.navigation.speedOverGroundMs.value);
    }

    Serial.print(" cog=");
    if (rawCog == 0xFFFF) {
        Serial.println("NA");
    } else {
        Serial.printf("%.4f rad\n", boat.navigation.courseOverGroundRad.value);
    }
}

void parseFluidLevel(const twai_message_t &msg, uint8_t source, uint32_t nowMs) {
    // PGN 127505 - Fluid Level
    // Byte 0 low nibble: instance, high nibble: fluid type
    // Bytes 1-2: level, 0.004 percent/bit
    // Bytes 3-6: capacity, 0.1 L/bit
    if (msg.data_length_code < 7) return;

    const uint8_t instance = msg.data[0] & 0x0F;
    const uint8_t fluidType = (msg.data[0] >> 4) & 0x0F;

    // NMEA 2000 fluid type 0 = fuel.
    if (fluidType != 0) return;

    const uint16_t rawLevel = static_cast<uint16_t>(msg.data[1]) |
                              (static_cast<uint16_t>(msg.data[2]) << 8);
    const uint32_t rawCapacity = static_cast<uint32_t>(msg.data[3]) |
                                 (static_cast<uint32_t>(msg.data[4]) << 8) |
                                 (static_cast<uint32_t>(msg.data[5]) << 16) |
                                 (static_cast<uint32_t>(msg.data[6]) << 24);

    boat.fuelTank.detected = true;
    boat.fuelTank.instance = instance;
    boat.fuelTank.sourceAddress = source;

    if (rawLevel != 0xFFFF) {
        boat.fuelTank.levelPercent.set(rawLevel * 0.004, nowMs);
    }
    if (rawCapacity != 0xFFFFFFFF) {
        boat.fuelTank.capacityL.set(rawCapacity * 0.1, nowMs);
    }

    Serial.printf("FUEL pgn=127505 src=%u inst=%u level=", source, instance);
    if (rawLevel == 0xFFFF) {
        Serial.print("NA");
    } else {
        Serial.printf("%.1f%%", boat.fuelTank.levelPercent.value);
    }

    Serial.print(" capacity=");
    if (rawCapacity == 0xFFFFFFFF) {
        Serial.println("NA");
    } else {
        Serial.printf("%.1f L\n", boat.fuelTank.capacityL.value);
    }
}

void handleFrame(const twai_message_t &msg) {
    if (!msg.extd || msg.rtr) return;

    const uint32_t nowMs = millis();
    const uint32_t pgn = extractPgn(msg.identifier);
    const uint8_t source = extractSource(msg.identifier);

    recordPgn(pgn, source, nowMs);

    switch (pgn) {
        case 127488:
            parseEngineRapid(msg, source, nowMs);
            break;
        case 127505:
            parseFluidLevel(msg, source, nowMs);
            break;
        case 129026:
            parseCogSogRapid(msg, source, nowMs);
            break;
        default:
            break;
    }
}

void printStats() {
    Serial.println();
    Serial.println("--- NMEA 2000 PGN/SOURCE STATS ---");
    Serial.println("PGN      SRC   COUNT      RATE(Hz approx)");

    for (const auto &stat : pgnStats) {
        if (!stat.used) continue;

        double rateHz = 0.0;
        if (stat.count > 1 && stat.lastMs > stat.firstMs) {
            rateHz = (stat.count - 1) * 1000.0 /
                     static_cast<double>(stat.lastMs - stat.firstMs);
        }

        Serial.printf("%-8lu %-5u %-10lu %.2f\n",
                      static_cast<unsigned long>(stat.pgn),
                      stat.source,
                      static_cast<unsigned long>(stat.count),
                      rateHz);
    }

    Serial.println("----------------------------------");
    Serial.println();
}

bool startCan() {
    twai_general_config_t general = TWAI_GENERAL_CONFIG_DEFAULT(
        CAN_TX_PIN,
        CAN_RX_PIN,
        TWAI_MODE_LISTEN_ONLY
    );

    // NMEA 2000 uses 250 kbit/s CAN.
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

    Serial.println();
    Serial.println("V MAX Smart Gauge - Milestone 1 NMEA 2000 Scanner");
    Serial.println("Waveshare ESP32-S3-Touch-LCD-5");
    Serial.println("CAN TX=GPIO15 RX=GPIO16, 250 kbit/s, LISTEN ONLY");

    if (!startCan()) {
        Serial.println("CAN initialization FAILED. Scanner stopped.");
        while (true) delay(1000);
    }

    Serial.println("CAN initialized. Waiting for NMEA 2000 traffic...");
}

void loop() {
    twai_message_t message{};

    if (twai_receive(&message, pdMS_TO_TICKS(20)) == ESP_OK) {
        handleFrame(message);
    }

    const uint32_t nowMs = millis();
    if ((uint32_t)(nowMs - lastStatsPrintMs) >= STATS_INTERVAL_MS) {
        lastStatsPrintMs = nowMs;
        printStats();
    }
}
