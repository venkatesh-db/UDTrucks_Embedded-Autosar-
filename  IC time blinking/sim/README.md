# IC Time Blinking – Stress Repro + Robust Fix (Simulation)

This small C++17 simulation reproduces an AUTOSAR-like IC time display flicker/blink under stress (ISR bursts and late COM sync), then shows a robust pattern (double buffer + exclusive area + grace before blanking) that eliminates the flicker.

It’s not using AUTOSAR libs; it mirrors the architecture so you can map the ideas to your SW-Cs and OS alarms.

## Concepts

- Producer (TimeUpdateRunnable, 100 ms): updates cached time (HH:MM:SS) and validity based on latest COM sync age.
- Consumer (DisplayRunnable, 50 ms): reads cached time and decides to display or blank.
- Stress:
  - ISR-like extra delays on producer.
  - COM sync occasionally late or dropped.
- Naive path:
  - Single buffer; blanks immediately when validity times out.
  - Susceptible to race/timeout flicker.
- Robust path:
  - Double-buffered cache (coherent snapshots), exclusive area, monotonic timestamp.
  - Grace period before blanking (debounce invalidity), keeping last good value briefly.

## Build and Run

Requires: macOS or Linux with `g++` supporting C++17.

```sh
# build
make -C sim

# run
./sim/ic_time_blink_sim
```

Optional tuning via environment variables:

```sh
# timeout before display considers time invalid (ms)
SIM_TIMEOUT_MS=300 \
# grace period before blanking once invalidity detected (ms)
SIM_GRACE_MS=200 \
./sim/ic_time_blink_sim
```

## Expected Output

You’ll see two sections:

- Naive: higher Blank events count.
- Robust: significantly fewer or zero blank events under the same stress.

Example (numbers vary per run):

```
=== Naive ===
Produced: 100
Consumed: 200
Blank events: 27
Invalid transitions: 0

=== Robust ===
Produced: 100
Consumed: 200
Blank events: 0
Invalid transitions: 5
```

`Invalid transitions` in Robust indicates how often the consumer first noticed invalidity; the grace prevents immediate blanking.

## Mapping to AUTOSAR

- "ExclusiveArea" → SchM_Enter/Exit or Rte_Enter/Exit on a dedicated exclusive area for the time cache.
- Double buffer → write to inactive buffer and atomically flip index (e.g., via atomic or exclusive area).
- Producer alarm period (100 ms) should be less than or equal to display update rate (50–100 ms) so consumer always has fresh data.
- Use a monotonic counter (OsCounter) to timestamp last update; avoid sources that reset during diagnostics.
- Grace debounce: Only blank after validity is false continuously for N cycles (e.g., 200–500 ms) to avoid flicker due to single missed frames.

## Next Steps for Your Project

- Ensure a single writer SW-C updates the time cache.
- Protect the cache with an exclusive area.
- Apply a grace window before blanking; keep last valid visible briefly.
- Verify OS alarm jitter under stress (trace) and adjust priorities if producer misses deadlines.
- If validity depends on COM time sync, choose a timeout >> period (e.g., >3x cycle) to reduce false invalids.
