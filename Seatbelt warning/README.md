# AUTOSAR Seatbelt Warning System (Simulation Case Study)

This project is a simplified AUTOSAR Classic style simulation of a seatbelt warning system focusing on prevention of false triggers (ASIL-B case). It includes interface software components (SWCs), a warning logic state machine, calibration handling, and a demo scenario to illustrate debounce, gating, and suppression mechanisms.

## Features
- Asymmetric seatbelt latch debounce (50 ms latch, 500 ms unlatch)
- Occupancy debounce (300 ms)
- Door grace period (2.0 s) and speed/ignition gating
- Warning arbitration: Off / Visual / AudioVisual (simplified)
- Basic diagnostics stubs (Dem events for stuck/stale examples)
- Calibration structure with CRC placeholder
- Simulation timeline demonstrating chatter immunity and correct warning activation

## Directory Layout
```
src/
  application/        SWC logic (sensors + warning)
  rte/                Lightweight RTE shim (signals, validity, timestamps)
  bsw/                Basic software stubs (Dem, NvM, Crc)
  platform/           Simulation main + scheduler
  config/             (reserved for future ARXML or calibration specifics)
tests/                Unit tests for debounce logic
docs/                 Safety case and design docs
```

## Build & Run
Requires Clang or GCC (Makefile uses `clang`).

```sh
make           # build simulation and tests
make run       # run 10-second scenario simulation
make test      # execute unit tests
make clean     # remove build artifacts
```

Sample simulation output line (every 100 ms):
```
[TIME   500] IGN=1 SPD=12 OCC=1 LATCH=1 DOOR=1 GRACE=1500 WARN=0
```
Fields: time, ignition, speed(km/h), occupancy(0 empty /1 occupied), latch(1 latched /0 unlatched), door(1 closed), remaining grace ms, warning level.

## Scenario Timeline
- 0 ms: Ignition ON, door closed (grace starts), belt latched, seat occupied
- 500 ms: Speed rises above threshold (still in door grace)
- 2000 ms: 40 ms unlatch chatter (ignored by debounce)
- 4000 ms: Sustained unlatch begins
- 4700 ms: Warning activates after 500 ms off-delay
- 6000 ms: Re-latch; clears warning after 50 ms
- 8000 ms: Seat becomes empty; warning suppressed after occupancy debounce
- 9000 ms: Speed to 0; gating removes warning

## Tests
`tests/test_debounce.c` covers:
- Unlatch chatter immunity (< 500 ms)
- Sustained unlatch detection
- Occupancy debounce enforcement

Extend with additional tests for gating or diagnostics as needed.

## Safety Case
See `docs/safety/seatbelt_false_trigger_case.md` for detailed HARA, safety goals, requirements, architecture, timing, diagnostics, and acceptance metrics.

## Next Steps / Extensions
- Add ARXML definitions for ports and runnables
- Integrate a real unit test framework (Unity / GoogleTest)
- Expand Dem handling (freeze frames, passed transitions)
- Add plausibility checks (e.g., occupancy vs. speed correlation)
- Introduce CAN abstraction for vehicle state inputs

## Disclaimer
This is a didactic simulation and not production AUTOSAR code. RTE, BSW, and diagnostics are minimal placeholders for illustration.
