#include "ScannerDiagnostics.h"
#include <cassert>
#include <cstring>
#include <iostream>
int main() {
    assert(scanner::counterDelta(12, 7) == 5);
    assert(scanner::counterDelta(3, UINT32_MAX - 1) == 5);
    assert(scanner::perSecond(1520, 5000) == 304.0);
    assert(scanner::perSecond(1520, 0) == 0.0);
    assert(scanner::checksum("", 0) == 2166136261u);
    assert(scanner::checksum("hello", 5) == 0x4f9f2cabu);
    assert(scanner::checksum("hello", 5) != scanner::checksum("Hello", 5));
    std::cout << "Scanner diagnostics tests passed\n";
}
