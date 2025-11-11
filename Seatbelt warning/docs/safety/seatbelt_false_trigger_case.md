# AUTOSAR Safety Case: Seatbelt Warning False Trigger (ASIL-B)

Scope: Prevent and mitigate false activation of the seatbelt warning when the belt is latched or the seat is unoccupied. Applies to Driver Seat Warning.

## 1) Assumptions & Context
- Sensors:
  - Seatbelt latch microswitch (digital) → may chatter; may be read via GPIO or CAN gateway.
  - Seat occupancy (weight mat/classifier) → analog/filtered; may provide validity.
  - Door-ajar, ignition state, vehicle speed from Vehicle State.
- Schedulers: 10 ms base cyc task for input processing; 100 ms for HMI update.
- Platform: AUTOSAR Classic; ASIL-B target; MISRA C; static memory; no dynamic allocation.
- False trigger definition: Audible/visual warning asserted with seatbelt latched OR seat unoccupied for more than allowed transient time.

## 2) HARA Summary
- Hazard: Driver distraction and habituation from false warning.
- S/E/C: S2–S3, E4, C2 → ASIL-B (see full HARA in work items).
- Safety concept: Limit unjustified warnings via filtering, gating, plausibility; detect sensor faults and degrade safely.

## 3) Safety Goals (SG)
- SG1: The system shall not assert seatbelt warning when the driver seatbelt is latched, beyond a transient filter time.
- SG2: The system shall not assert seatbelt warning when the driver seat is unoccupied, beyond a transient filter time.
- SG3: The system shall detect input signal faults (stuck, out-of-range, stale) affecting seatbelt/occupancy and prevent nuisance warnings while informing diagnostics.
- SG4: The system shall gate warnings to operating states where they are relevant (ignition ON, speed above threshold, door closed grace period elapsed).

## 4) Functional Safety Requirements (FSR)
- FSR1 (Filtering): Debounce seatbelt latch with asymmetric on/off delays: on-delay ≤ 50 ms, off-delay ≥ 500 ms (configurable in NVRAM). Occupancy debounce ≥ 300 ms.
- FSR2 (Plausibility): Suppress warning if seat occupancy = not-occupied OR unknown. Only assert warning when occupancy = occupied AND belt = unlatched (after debounces).
- FSR3 (Gating): Assert warning only if Ignition = ON, VehicleSpeed ≥ 10 km/h (configurable), and a DoorCloseGrace timer of 2.0 s has elapsed since last door-ajar = closed.
- FSR4 (Validity & Timeouts): Treat any input with invalid flag or stale timestamp (> 200 ms for latch, > 500 ms for occupancy, > 300 ms for vehicle state) as unknown; in unknown state, do not assert warning; raise DTC per TSR7.
- FSR5 (Timing): End-to-end latency from belt status change to warning update ≤ 100 ms (95th percentile) and ≤ 200 ms (worst-case), measured under max load.
- FSR6 (Fail-safe Behavior): On conflicting inputs (e.g., occupancy=empty and belt=unlatched), prefer no warning and record a plausibility DTC.
- FSR7 (Configuration Integrity): Verify calibration CRC at startup; if invalid, use safe defaults and log DTC; warn timings remain within safe ranges.

## 5) Technical Safety Requirements (TSR) and Allocation
- SWC: Seatbelt_Sensor_IF
  - Reads raw latch via Rte_Read_SeatbeltLatch_Raw and/or IoHwAb.
  - Implements asymmetric debounce; outputs SeatbeltLatch_Filtered (boolean) + Validity + Timestamp.
  - Detects stuck-at via activity monitor (min toggle expectation or long-constant watchdog).
- SWC: Occupancy_Sensor_IF
  - Reads occupancy classification; debounces; outputs Occupancy_Filtered (enum: Empty/Occupied/Unknown) + Validity + Timestamp.
- SWC: VehicleState_IF
  - Provides IgnitionState, VehicleSpeed, DoorAjar, with validity + timestamps; staleness detection for CAN data (alive counter, CRC).
- SWC: SeatbeltWarning_Logic
  - Runnable: R_SBW_10ms (periodic 10 ms). State machine implementing gating, grace timers, and warning arbitration.
  - Outputs Rte_Write_SBW_WarningRequest (enum: Off/Visual/AudioVisual) to HMI.
  - Maintains door-close grace timer, ignition transition handling, and chime cadence.
- BSW: Dem (Diagnostics), NVRAM (calibration), WdgM (supervision), IoHwAb (I/O), Com (CAN), Crc.

AUTOSAR Interfaces (examples):
- Sender-Receiver:
  - Rte_Read_Seatbelt_Sensor_IF_LatchFiltered(boolean, validity, timestamp)
  - Rte_Read_Occupancy_Sensor_IF_Status(enum, validity, timestamp)
  - Rte_Read_VehicleState_IF_Speed_kph(uint16, validity, timestamp)
  - Rte_Read_VehicleState_IF_Ignition(enum)
  - Rte_Read_VehicleState_IF_DoorAjar(boolean)
  - Rte_Write_SBW_HMI_WarningRequest(enum)
- Diagnostics:
  - Dem_ReportErrorStatus(DTC_SEATBELT_STUCK, DEM_EVENT_STATUS_PREFAILED/PASSED)
  - Dem_ReportErrorStatus(DTC_OCCUPANCY_INVALID, ...)
  - Dem_ReportErrorStatus(DTC_PLAUSIBILITY_CONFLICT, ...)

## 6) Logic Design
### 6.1 State Machine (SeatbeltWarning_Logic)
States:
- Idle: WarningRequest=Off; exit when Ign=ON & Speed ≥ Vmin & DoorGrace elapsed.
- Monitor: Evaluate Occupancy & Latch (debounced/valid) each 10 ms.
- WarningActive: When Occupancy=Occupied AND Latch=Unlatched; generate visual + cadence for audible per OEM rules.
- InhibitFault: Enter on invalid/stale inputs; suppress warning; raise DTCs; exit when inputs valid for 1 s.

Transitions:
- Idle → Monitor: Ign=ON, Speed ≥ Vmin, DoorGrace=0.
- Monitor → WarningActive: Occupied & Unlatched confirmed.
- WarningActive → Monitor: Latched confirmed OR Occupancy transitions to Empty/Unknown.
- Any → InhibitFault: Any required input invalid/stale; InhibitFault → Idle/Monitor after recovery grace.

### 6.2 Debounce Pseudocode (C-like)
```c
// 10 ms runnable
static uint16_t unlatch_on_delay_ms = 500;  // config
static uint16_t latch_on_delay_ms   = 50;   // config

void DebounceLatch(bool raw, uint32_t dt_ms) {
  if (raw) {
    latch_timer = MIN(latch_timer + dt_ms, latch_on_delay_ms);
    unlatch_timer = 0;
  } else {
    unlatch_timer = MIN(unlatch_timer + dt_ms, unlatch_on_delay_ms);
    latch_timer = 0;
  }
  if (latch_timer >= latch_on_delay_ms) filtered = true;
  else if (unlatch_timer >= unlatch_on_delay_ms) filtered = false;
}
```

## 7) Failure Modes, Diagnostics, and Safety Mechanisms
- Failure Modes:
  - Latch sensor stuck-at-0/1, chattering, intermittent opens.
  - Occupancy sensor misclassification, invalid flag, stale sampling.
  - CAN frame loss, CRC errors, wrong scaling.
- Detection:
  - Staleness monitors per signal; alive counters; CRC via Crc library; DEM events.
  - Range/plausibility checks (e.g., Occupancy=Empty but seat load > threshold if available).
- Reactions:
  - Enter InhibitFault; suppress warning; store DTC with freeze frame (timestamps, signal states, speed).
  - Fall back to safe defaults for calibration.

Target Diagnostic Coverage (ASIL-B typical):
- Sensor path DC ≥ 60% via staleness + stuck-at detection + plausibility.

## 8) Timing & Scheduling
- Seatbelt_Sensor_IF, Occupancy_Sensor_IF, VehicleState_IF: 10 ms periodic runnables; WCET budget ≤ 2 ms each.
- SeatbeltWarning_Logic: 10 ms periodic; WCET ≤ 3 ms; end-to-end budget ≤ 100 ms.
- HMI/Buzzer output update: 100 ms periodic; cadence managed with counters.
- WdgM supervision cycle: 100 ms; alive supervision of 10 ms tasks.

## 9) Test & Validation Strategy
- Unit Tests (Unity/GoogleTest with RTE mocks):
  - Debounce edge cases (noise bursts up to 40 ms must not trigger; ≥ 500 ms unlatch must trigger).
  - State machine gating (door grace, speed thresholds).
  - Fault handling transitions (invalid → inhibit → recover after 1 s valid).
- Integration (SiL/HiL):
  - Inject chattering on latch (5–20 ms pulses) while latched; verify no warning.
  - Simulate CAN dropouts and CRC errors; verify inhibit and DTCs.
  - Measure latency and WCET under maximum load.
- Fault Injection:
  - Stuck-at 0/1, stale timestamps, oscillations; verify detection and reaction.
- Traceability:
  - SG → FSR → TSR → Test Cases mapping maintained in requirements tool (IDs mirrored in code comments).

## 10) Verification Metrics (Acceptance)
- False Positive Activation Time: ≤ 1 s per 10 h of driving (95th percentile fleet metric).
- Noise Immunity: Any latch noise burst ≤ 40 ms shall not change filtered state.
- Latency: ≤ 100 ms (95th) and ≤ 200 ms (max) belt-change-to-warning-change.
- Diagnostic Coverage: ≥ 60% for identified failure modes on sensor paths.
- DTC Behavior: Pre-Failed within 300 ms of persistent invalid/stale; set to Passed within 1 s of stable recovery.

## 11) Implementation Notes (AUTOSAR)
- Use sender-receiver ports; do not modify RTE-generated code.
- Keep calibration in NVRAM with versioning + CRC; expose via CDD or NvM blocks.
- Use Dem event IDs and freeze frames; align with OEM DTC mapping.
- Respect freedom-from-interference: bound stack/CPU; avoid blocking calls in 10 ms runnables.
- Document timing in ARXML (runnables, events, periods) and ensure RTE generation aligns.
