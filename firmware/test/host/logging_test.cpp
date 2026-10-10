#include "ScannerLogging.h"
#include <cassert>
#include <cstring>
#include <string>
#include <iostream>

struct FakeUsb {
    scanner::LogControl control;
    uint32_t time = 0, stopAt = UINT32_MAX;
    size_t partial = 3;
    bool stalled = false;
    std::string bytes;
    uint32_t nowMs() { return time; }
    void service() {
        control.observeConnection(true);
        if (time >= stopAt) control.command('s');
    }
    bool connected() { return control.connected; }
    int space() { return stalled ? 0 : 10; }
    size_t write(const char *data, size_t size) {
        const size_t count = size < partial ? size : partial;
        bytes.append(data, count); return count;
    }
    void yield() { ++time; }
};

int main() {
    char record[128];
    const int length = scanner::formatRecord(record, sizeof(record), 1, "CAN busDelta=0");
    assert(length > 0);
    char *suffix = strstr(record, " *");
    assert(suffix);
    const auto hash = scanner::checksum(record + 1, size_t(suffix - record - 1));
    char expected[16]; snprintf(expected, sizeof(expected), " *%08lX\n", static_cast<unsigned long>(hash));
    assert(strcmp(suffix, expected) == 0);
    record[2] = '9';
    assert(scanner::checksum(record + 1, size_t(suffix - record - 1)) != hash);
    assert(scanner::formatRecord(record, 4, 1, "too long") == -1);

    FakeUsb usb;
    assert(scanner::writeRecord(usb, "abcdefghij", 10, {0, 250}));
    assert(usb.bytes == "abcdefghij"); // partial writes retained exactly
    usb = FakeUsb{};
    usb.stalled = true; usb.control.raw = true; usb.stopAt = 5;
    assert(!scanner::writeRecord(usb, "data", 4, {0, 30}));
    assert(!usb.control.raw && usb.control.modePending); // stop serviced inside stalled write
    assert(usb.time == 30); // shared batch budget, not 250 ms per record
    assert(!scanner::writeRecord(usb, "next", 4, {0, 30}));
    assert(usb.time == 30); // another record cannot extend the expired batch
    usb = FakeUsb{}; usb.time = UINT32_MAX - 10; usb.stalled = true;
    const uint32_t start = usb.time;
    assert(!scanner::writeRecord(usb, "data", 4, {start, 20}));
    assert(uint32_t(usb.time - start) == 20);

    scanner::LogControl control;
    control.observeConnection(false);
    assert(control.metadataPending);
    control.observeConnection(true);
    auto request = control.metadataRequest;
    control.metadataResult(false, request);
    assert(control.metadataPending); // failed metadata must retry
    control.metadataResult(true, request);
    assert(!control.metadataPending);
    control.observeConnection(false); control.observeConnection(true);
    assert(control.metadataPending); // reconnect resends startup context
    request = control.metadataRequest;
    control.command('i');
    control.metadataResult(true, request);
    assert(control.metadataPending); // new info request cannot be consumed by an old write
    control.metadataResult(true, control.metadataRequest);
    assert(!control.metadataPending);
    control.command('r'); assert(control.raw && control.modePending);
    control.command('s'); assert(!control.raw);
    std::cout << "Scanner logging transport tests passed\n";
}
