# ABS Malfunction Detection Simulation

This directory contains a PC-based simulation of the AUTOSAR ABS malfunction detection system that can run on any computer with a C compiler.

## 🚀 Quick Start

### Build and Run
```bash
cd simulation/
make run
```

### Manual Build
```bash
gcc -Wall -std=c99 simulation_main.c -o abs_simulation -lm
./abs_simulation
```

## 🔧 What This Simulation Does

### Real-time Monitoring
- Simulates 4 wheel speed sensors (FL, FR, RL, RR)
- Monitors for calibration drift and speed inconsistencies
- Detects malfunctions in real-time with visual output

### Malfunction Scenarios
1. **Normal Operation** (Steps 0-49): All sensors working correctly
2. **Miscalibration Injection** (Step 50): Introduces 15% calibration error in Front Left sensor
3. **Detection & Confirmation** (Steps 50+): System detects and confirms malfunction

### Output Example
```
Step 000 | FL:61.2 FR:58.9 RL:59.7 RR:60.3 | Malfunctions: None
Step 010 | FL:59.8 FR:61.1 RL:60.5 RR:59.2 | Malfunctions: None
...
🔧 Introducing miscalibration in Front Left sensor...
Step 050 | FL:68.9 FR:60.2 RL:59.8 RR:60.1 | Malfunctions: None
Step 060 | FL:69.1 FR:59.7 RL:60.4 RR:59.9 | Malfunctions: [FL:Miscalibration(15.0)]
```

## 🎯 Key Features Demonstrated

### Malfunction Detection Algorithms
- **Calibration Drift**: Detects when correction factors exceed 10% deviation
- **Speed Plausibility**: Compares individual wheel speeds against median reference
- **Acceleration Validation**: Monitors for implausible acceleration values
- **Debouncing**: Confirms malfunctions over multiple cycles

### Realistic Simulation
- Random sensor noise (±2 km/h)
- Proper calibration application
- Acceleration calculation
- Quality factor assessment

## 🛠️ Build Requirements

### System Requirements
- GCC compiler (or any C99-compatible compiler)
- Math library support (-lm)
- POSIX-compatible system (Linux/macOS/WSL)

### No Dependencies
- No external libraries required
- Standard C library only
- Cross-platform compatible

## 📊 Understanding the Output

### Wheel Speed Format
`FL:61.2` = Front Left wheel at 61.2 km/h

### Malfunction Format
`[FL:Miscalibration(15.0)]` = Front Left sensor has 15.0% calibration error

### Malfunction Types
- **None**: No malfunction detected
- **Miscalibration**: Calibration drift >10%
- **Speed Difference**: Speed deviation >20 km/h from median
- **Acceleration Error**: Acceleration >15 m/s²

## 🔬 Customization Options

### Modify Thresholds
Edit `simulation_main.c` and change:
```c
if (calibrationDrift > 10.0f) {     // Calibration threshold
if (speedDifference > 20.0f) {      // Speed difference threshold  
if (fabsf(acceleration) > 15.0f) {  // Acceleration threshold
```

### Add More Scenarios
Extend `SimulateMiscalibration()` function to introduce:
- Multiple sensor failures
- Speed sensor noise
- Different calibration errors
- Dynamic malfunction scenarios

### Modify Vehicle Conditions
Change simulation parameters:
```c
static float32 g_VehicleSpeed = 60.0f;  // Base vehicle speed
// Add braking, acceleration, cornering scenarios
```

## 🎮 Interactive Features

The simulation automatically:
1. Runs normal operation for 5 seconds
2. Introduces Front Left sensor miscalibration
3. Shows real-time detection and confirmation
4. Completes after 20 seconds of simulation

## 📈 Expected Behavior

### Phase 1 (Steps 0-49): Normal Operation
- All wheel speeds around 60 km/h ±2 km/h noise
- No malfunctions detected
- All sensors show 100% quality factor

### Phase 2 (Steps 50+): Malfunction Scenario  
- Front Left sensor reads ~15% higher (69 km/h vs 60 km/h)
- System detects calibration drift after confirmation period
- Malfunction status shows: `[FL:Miscalibration(15.0)]`

## 🚗 Real-World Correlation

This simulation demonstrates the core algorithms used in actual automotive ABS systems:
- Real-time sensor monitoring
- Cross-validation between wheels
- Statistical reference calculation
- Malfunction confirmation logic
- Diagnostic trouble code generation

The simplified PC version maintains the essential detection logic while removing hardware dependencies.