# Tarsis HMI Simulator

Qt6/C++ HMI simulator for UAV telemetry — portfolio project built in preparation for Indra's Tarsis UAV program.

## Status

🚧 **Work in progress**

| Module | Status |
|--------|--------|
| Qt6 + CMake skeleton | ✅ Done |
| `MainWindow` class (Q_OBJECT, signals & slots) | ✅ Done |
| C++ modern fundamentals (RAII, smart pointers) | ✅ Done — see `cpp-fundamentals-refresher/` |
| UDP telemetry reception & parsing | ✅ Done |
| TCP critical command channel (RTL) | ✅ Done |
| MQTT status publishing (ONLINE/OFFLINE) | ✅ Done |
| DDS conceptual study | ⏳ Planned |
| Logging & error handling | ⏳ Planned |
| PECAL documentation review | ⏳ Planned |

## Tech stack

- C++17
- Qt6 (Widgets, Network)
- libmosquitto (C client) — wrapped in a custom RAII C++ class, not Qt's own MQTT module
- CMake + pkg-config
- Git / GitHub

## MQTT design note

The HMI publishes its lifecycle status (`ONLINE` on startup, `OFFLINE` on window close) to topic `tarsis/hmi/status`, using QoS 1 (broker confirms delivery) and `retain=true` (late subscribers get the current status immediately). Implemented with `libmosquitto` rather than the official Qt MQTT module, since the latter isn't packaged in standard Ubuntu repositories and would require building from source — `libmosquitto` is the official client of the same Mosquitto broker already used for testing, available directly via `apt`.

## Known limitations (intentional, documented technical debt)

- Broker IP/port and TCP command channel IP/port are currently hardcoded for this prototype phase.
- Mosquitto broker configured with `allow_anonymous true` for this isolated test setup — not production-appropriate as-is.

## Build

**Prerequisites:** Qt6 (Widgets, Network), `libmosquitto-dev`, `pkg-config`.

```bash
cmake -S . -B build
cmake --build build
./build/tarsis-hmi-simulator
```

## Project structure

```
tarsis-hmi-simulator/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── mainwindow.h
│   ├── mainwindow.cpp
│   ├── mosquittomqttpublisher.h
│   └── mosquittomqttpublisher.cpp
└── cpp-fundamentals-refresher/   # C++ onboarding exercises (RAII, smart pointers)
```

## Test infrastructure

UDP telemetry is generated on the test rig (Kali), not on the development machine, for realism. Since WSL2's default NAT networking blocks inbound connections from the LAN, `tools/start_telemetry_bridge.sh` opens an SSH reverse tunnel and a local TCP→UDP translator (socat) to bridge it through. TCP and MQTT don't need this, since those connections originate from the HMI itself.