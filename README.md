# Tarsis HMI Simulator

Qt6/C++ HMI simulator for UAV telemetry — portfolio project built in preparation for Indra's Tarsis UAV program.

## Status

✅ **Core HMI feature-complete** — DDS conceptual study and PECAL documentation review remain.

| Module                                                         | Status                                     |
| ---------------------------------------------------------------| ------------------------------------------ |
| Qt6 + CMake skeleton                                            | ✅ Done                                     |
| `MainWindow` class (Q_OBJECT, signals & slots)                  | ✅ Done                                     |
| C++ modern fundamentals (RAII, smart pointers)                  | ✅ Done — see `cpp-fundamentals-refresher/` |
| UDP telemetry reception & parsing                                | ✅ Done                                     |
| TCP critical command channel (RTL)                               | ✅ Done                                     |
| MQTT status publishing (ONLINE/OFFLINE)                          | ✅ Done                                     |
| Logging (dual-sink, battery alarm threshold)                     | ✅ Done                                     |
| Full HMI UI (telemetry cards, status badges, alarm/mission banners, RTL/Reconnect) | ✅ Done |
| MQTT background-thread connection monitoring                    | ✅ Done                                     |
| Alarm banner (blink, acknowledge/mute)                            | ✅ Done                                     |
| DDS conceptual study                                              | ⏳ Planned                                  |
| PECAL documentation review                                       | ⏳ Planned                                  |

## Tech stack

- C++17
- Qt6 (Widgets, Network)
- libmosquitto (C client) — wrapped in a custom RAII C++ class, not Qt's own MQTT module
- spdlog — dual-sink logging (`general.log` + `alarms.log`)
- CMake + pkg-config
- Git / GitHub

## MQTT design note

The HMI publishes its lifecycle status (`ONLINE` on startup, `OFFLINE` on window close) to topic `tarsis/hmi/status`, using QoS 1 (broker confirms delivery) and `retain=true` (late subscribers get the current status immediately). Implemented with `libmosquitto` rather than the official Qt MQTT module, since the latter isn't packaged in standard Ubuntu repositories and would require building from source — `libmosquitto` is the official client of the same Mosquitto broker already used for testing, available directly via `apt`.

## Logging design note

The HMI uses a single `spdlog` logger registered as the process-wide default (set up once in `main()`, before `QApplication`), writing to two independent file sinks:

- **`general.log`** — every telemetry reading (`info` level), regardless of value.
- **`alarms.log`** — filtered at `warning` level and above: only battery-critical readings and operator actions land here, isolated from routine noise.

Battery readings below **10%** are logged as `warning`, on every single reading below threshold — not just on the first crossing — so the alarm log can reconstruct exactly how long the aircraft was in a critical state before the operator acted. When the operator presses RTL, a separate `critical`-level line is written once, recording that a human decision was made, distinct from the repeated automated warnings above it.

Both sinks share the same timestamped pattern (`[date time] [logger] [level] message`) and flush to disk via `spdlog::shutdown()`, called after the Qt event loop (`app.exec()`) returns — not before, since the logger must stay alive for the entire lifetime of the application.


## Threading design note

[#threading-design-note](#threading-design-note)

MQTT connection state is updated from mosquitto's native callbacks (`mosquitto_connect_callback_set` / `mosquitto_disconnect_callback_set`), which run on a background thread managed by `mosquitto_loop_start`/`stop` — not on Qt's main thread. Qt widgets are never touched directly from that callback thread; instead, the callback only writes to a `std::atomic<bool>` connection flag. A `QTimer` running on the Qt main thread polls that flag periodically and updates the MQTT status badge from there. This keeps every UI mutation on the thread that owns the UI, while `std::atomic<bool>` guarantees the flag itself is never read mid-write across threads.


## Alarm banner design note

[#alarm-banner-design-note](#alarm-banner-design-note)

The alarm banner always reserves its layout space — its visibility never toggles, only its style (color, text) changes — so the rest of the HMI doesn't jump when an alarm appears or clears. A `QTimer` drives the blink while the alarm is active and unacknowledged. Pressing RTL transitions the banner into an acknowledged/muted state, distinct from a fully cleared alarm, so the operator's action is visually distinguishable from the system simply recovering on its own.

## Known limitations (intentional, documented technical debt)

- Broker IP/port and TCP command channel IP/port are currently hardcoded for this prototype phase.
- Mosquitto broker configured with `allow_anonymous true` for this isolated test setup — not production-appropriate as-is.
- The RTL acknowledgment log line reads directly from the `QLabel` display text (e.g. `"Battery: 39 %"`) rather than the raw parsed telemetry value — a deliberate trade-off to avoid introducing extra member variables under time pressure, at the cost of coupling the log message format to the UI's display string.

## Build

**Prerequisites:** Qt6 (Widgets, Network), `libmosquitto-dev`, `libspdlog-dev`, `pkg-config`.

```bash
cmake -S . -B build
cmake --build build
./build/tarsis-hmi-simulator
```

Log files are written at runtime to `logs/general.log` and `logs/alarms.log` (both gitignored) — not included in the repository, generated automatically on first run.


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
