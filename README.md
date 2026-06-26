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
| MQTT integration (Qt as client) | ⏳ Planned |
| Logging & error handling | ⏳ Planned |
| PECAL documentation review | ⏳ Planned |

## Tech stack

- C++17
- Qt6 (Widgets)
- CMake
- Git / GitHub

## Build

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
│   └── mainwindow.cpp
└── cpp-fundamentals-refresher/   # C++ onboarding exercises (RAII, smart pointers)
```