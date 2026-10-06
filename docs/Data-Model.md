# Data Model

## Goal

Keep NMEA parsing, application state, logging, and UI rendering separated. The display should consume a normalized internal data model instead of reading raw CAN frames directly.

## Primary Engine Data

Suggested fields:

```cpp
struct EngineData {
    uint8_t sourceAddress;
    uint8_t engineInstance;

    double rpm;
    double trimPercent;
    double coolantTempK;
    double alternatorVoltage;
    double fuelRateLph;
    double engineHoursSec;
    int8_t engineLoad;
    int8_t engineTorque;

    uint32_t rapidUpdatedMs;
    uint32_t dynamicUpdatedMs;
};
```

## Navigation Data

```cpp
struct NavigationData {
    double speedOverGroundMs;
    double courseOverGroundRad;
    double latitude;
    double longitude;

    uint32_t sogUpdatedMs;
    uint32_t positionUpdatedMs;
};
```

## Fuel Data

```cpp
struct FuelData {
    uint8_t tankInstance;
    double levelPercent;
    double capacityLiters;
    uint32_t updatedMs;
};
```

## System State

The planned [Fuel Configuration](Fuel-Configuration.md) feature adds configured tank
capacity and a persistent manual inventory alongside raw network FuelData. Keep
network capacity/percentage, configured capacity, manual remaining volume and selected
display source distinct. Track inventory baseline, quality and committed consumption
sequence independently of trip totals.

Track separately from engine values:

- CAN initialized
- NMEA traffic recently observed
- Yamaha engine source discovered
- Garmin/GPS source discovered
- SD mounted
- Wi-Fi state
- OTA service mode state
- firmware version/build

## Staleness

Every important measurement should retain a last-update timestamp. UI code should determine whether a value is fresh before rendering it as current.

Example categories:

- fresh
- stale
- unavailable

The stale threshold can vary by PGN/update rate. RPM data should become stale much sooner than engine hours or tank level.

## Units

Keep NMEA-native/SI units internally where practical and convert only at presentation/log-export boundaries.

Examples:

- speed: meters/second internally -> MPH or knots for display
- fuel rate: liters/hour internally -> GPH when configured
- temperature: Kelvin internally -> Fahrenheit/Celsius for display
- engine time: seconds internally -> hours for display

## Calculated Metrics

Derived values should not overwrite raw values.

Examples:

```text
speed MPH = SOG m/s * conversion factor
fuel GPH  = fuel L/h * conversion factor
MPG       = speed MPH / fuel GPH
```

Guard against division by near-zero fuel flow and unavailable inputs.

## Future Multi-Engine Support

The first UI is explicitly single-engine, but the data model should avoid making future multi-engine support impossible. Engine data can eventually be stored by engine instance/source without changing the UI requirements for version 1.
