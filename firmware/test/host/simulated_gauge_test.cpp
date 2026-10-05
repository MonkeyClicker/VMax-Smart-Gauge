#include "simulated_gauge_checks.h"
#include <assert.h>
#include <stdio.h>

int main() {
    assert(simulatedGaugeChecks());
    puts("Simulated gauge checks passed");
}
