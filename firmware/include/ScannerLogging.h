#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "ScannerDiagnostics.h"

namespace scanner {
struct LogControl {
    uint32_t metadataRequest = 0;
    bool raw = false;
    bool metadataPending = true;
    bool modePending = false;
    bool connected = false;
    void observeConnection(bool value) {
        if (value && !connected) { metadataPending = true; ++metadataRequest; }
        connected = value;
    }
    void command(int value) {
        if (value == 'r' || value == 's') {
            raw = value == 'r'; modePending = true;
        } else if (value == 'i') { metadataPending = true; ++metadataRequest; }
    }
    void metadataResult(bool complete, uint32_t request) {
        if (complete && request == metadataRequest) metadataPending = false;
    }
};

// Clock subtraction remains valid across millis() rollover.
struct LogBudget {
    uint32_t start = 0;
    uint32_t duration = 250;
    bool expired(uint32_t now) const { return uint32_t(now - start) >= duration; }
};

inline int formatRecord(char *out, size_t capacity, uint32_t sequence, const char *payload) {
    const int bodyLength = snprintf(out, capacity, "\n@%lu %s", static_cast<unsigned long>(sequence), payload);
    if (bodyLength < 0 || size_t(bodyLength) >= capacity) return -1;
    // Protect the @ marker, decimal sequence, separator and payload together.
    const uint32_t hash = checksum(out + 1, size_t(bodyLength) - 1);
    const int tailLength = snprintf(out + bodyLength, capacity - size_t(bodyLength),
        " *%08lX\n", static_cast<unsigned long>(hash));
    if (tailLength < 0 || size_t(tailLength) >= capacity - size_t(bodyLength)) return -1;
    return bodyLength + tailLength;
}

// Transport services commands during partial writes, without recursive logging.
// Both the record deadline and the shared batch deadline must remain open.
template <typename Transport>
bool writeRecord(Transport &transport, const char *data, size_t length, const LogBudget &batch) {
    const LogBudget record{transport.nowMs(), 250};
    size_t written = 0;
    while (written < length) {
        transport.service();
        const uint32_t now = transport.nowMs();
        if (!transport.connected() || record.expired(now) || batch.expired(now)) return false;
        const int space = transport.space();
        if (space > 0) {
            const size_t remaining = length - written;
            const size_t chunk = remaining < size_t(space) ? remaining : size_t(space);
            written += transport.write(data + written, chunk);
        }
        transport.yield();
    }
    return true;
}
}
