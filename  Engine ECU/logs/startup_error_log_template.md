# Engine ECU Startup Issue - Error Log Template
# Date: 2024-11-07
# Software Version: [INSERT VERSION]
# Hardware Revision: [INSERT HW REV]

## Issue Description
ECU fails to start after software update. No communication on CAN bus.

## System Information
- ECU Part Number: [INSERT P/N]
- Software Version Before Update: [INSERT VERSION]
- Software Version After Update: [INSERT VERSION]
- Hardware Revision: [INSERT HW REV]
- Calibration Set: [INSERT CAL VERSION]

## Observed Symptoms
- [ ] No CAN communication
- [ ] LED indicators not functioning
- [ ] Watchdog reset loop
- [ ] Stuck in bootloader
- [ ] Application not starting
- [ ] Other: _______________

## Diagnostic Results

### Power Supply
- Supply voltage: ___V (nominal: 12V ±10%)
- Current draw: ___mA
- Ripple: ___mV

### Communication Interface
- CAN High: ___V
- CAN Low: ___V  
- CAN differential: ___V (should be ~2V)
- Termination resistance: ___Ω (should be 60Ω total)

### Hardware Tests
- Crystal oscillator frequency: ___MHz
- Reset pin status: ___
- Boot mode pins: ___
- JTAG/SWD connection: ___

### Memory Tests
- Flash CRC: PASS/FAIL
- RAM test: PASS/FAIL
- EEPROM/NVM: PASS/FAIL
- Stack usage: ___%

### Software Analysis
- Bootloader version: ___
- Application signature: VALID/INVALID
- Configuration CRC: PASS/FAIL
- Calibration checksum: PASS/FAIL

## Error Codes Detected
[List any DTCs or internal error codes]

## Recovery Actions Attempted
- [ ] Power cycle
- [ ] Hard reset
- [ ] Bootloader recovery mode
- [ ] Configuration reset
- [ ] Software rollback
- [ ] Flash reprogram

## Root Cause Analysis
[To be filled after investigation]

## Resolution
[To be filled after fix is implemented]

## Prevention Measures
[To be filled to prevent future occurrences]