#pragma once
#include <stdint.h>

struct ScannerScreenStatus {
    uint32_t heartbeatMs = 0;
    uint32_t lastFrameMs = 0;
    uint64_t frames = 0;
    uint32_t trackedPgns = 0;
    uint32_t busErrors = 0;
    uint32_t losses = 0;
    bool canRunning = false;
    bool canStatusValid = false;
};

bool startScannerScreen();
// Nonblocking latest-value mailbox; never waits for rendering.
void publishScannerScreen(const ScannerScreenStatus &status);
uint32_t scannerScreenRefreshCount();
