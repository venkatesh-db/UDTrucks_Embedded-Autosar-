/**
 * @file README.md
 * @brief AUTOSAR ABS Malfunction Detection System Documentation
 * @author Generated for ABS Malfunction Detection System
 */

# AUTOSAR ABS Malfunction Detection System

## Overview

This project implements a comprehensive AUTOSAR-compliant system for detecting Anti-lock Braking System (ABS) malfunctions caused by miscalibrated speed sensors. The system provides real-time monitoring, diagnosis, and calibration management for wheel speed sensors in automotive applications.

## System Architecture

### Components

The system consists of several interconnected AUTOSAR software components:

#### 1. Speed Sensor Software Component (`SpeedSensorSwc`)
- **Purpose**: Processes raw speed sensor data and provides calibrated wheel speed information
- **Location**: `src/application/swc/SpeedSensor_Swc.c`
- **Key Features**:
  - Real-time speed calculation from pulse data
  - Calibration parameter management
  - Data validation and quality assessment
  - Support for 4 wheel positions (FL, FR, RL, RR)

#### 2. ABS Malfunction Detection Component (`ABSMalfunctionDetectionSwc`)
- **Purpose**: Monitors speed sensor data for malfunctions and inconsistencies
- **Location**: `src/application/swc/ABS_MalfunctionDetection.c`
- **Key Features**:
  - Speed sensor miscalibration detection
  - Excessive speed difference detection
  - Acceleration plausibility checks
  - Malfunction severity assessment
  - Debouncing and confirmation logic

#### 3. Calibration Manager Service (`CalibrationManagerSwc`)
- **Purpose**: Manages speed sensor calibration parameters and procedures
- **Location**: `src/bsw/services/CalibrationManager.c`
- **Key Features**:
  - Automatic and manual calibration procedures
  - NVM storage and retrieval
  - Calibration history tracking
  - Factory reset capabilities
  - Real-time calibration validation

#### 4. Diagnostic Service (`DiagnosticServiceSwc`)
- **Purpose**: Provides UDS diagnostic services and DTC management
- **Location**: `src/bsw/services/DiagnosticService.c`
- **Key Features**:
  - UDS protocol support (0x10, 0x11, 0x14, 0x19, 0x22, 0x2E, 0x31)
  - Diagnostic Trouble Code (DTC) management
  - Data Identifier (DID) read/write operations
  - Routine control for calibration procedures

### Data Types and Interfaces

#### Key Data Structures
- `SpeedSensorData_t`: Complete speed sensor data including raw, processed, and diagnostic information
- `ABS_MalfunctionStatus_t`: Malfunction detection results and severity information
- `CalibrationSession_t`: Calibration procedure status and results
- `DTCInfo_t`: Diagnostic trouble code information

#### AUTOSAR Interfaces
- **Sender-Receiver Interfaces**: For real-time data exchange between components
- **Client-Server Interfaces**: For service-oriented operations (calibration, diagnostics)
- **Port Interfaces**: Defined in ARXML configuration files

## Malfunction Detection Algorithms

### 1. Speed Sensor Miscalibration Detection
- Monitors calibration drift over time
- Compares correction factors against acceptable ranges
- Triggers DTC when drift exceeds threshold (default: 10%)

### 2. Speed Plausibility Checks
- Validates wheel speeds against vehicle reference speed
- Detects excessive speed differences between wheels
- Configurable thresholds for different driving conditions

### 3. Acceleration Validation
- Monitors wheel acceleration for implausible values
- Considers braking conditions and vehicle dynamics
- Prevents false alarms during normal ABS operation

### 4. Cross-Wheel Validation
- Compares speeds across all four wheels
- Uses median filtering for robust reference calculation
- Identifies individual wheel sensor failures

## Diagnostic Trouble Codes (DTCs)

The system generates the following DTCs:

| DTC Code | Description | Affected Component |
|----------|-------------|-------------------|
| 0xC14100 | Speed Sensor FL Miscalibrated | Front Left Wheel |
| 0xC14101 | Speed Sensor FR Miscalibrated | Front Right Wheel |
| 0xC14102 | Speed Sensor RL Miscalibrated | Rear Left Wheel |
| 0xC14103 | Speed Sensor RR Miscalibrated | Rear Right Wheel |
| 0xC14200-0xC14203 | Speed Sensor Failure | Individual Wheels |
| 0xC14300 | ABS System Malfunction | System Level |
| 0xC14400 | Speed Plausibility Error | Cross-Wheel |

## UDS Diagnostic Services

### Supported Services
- **0x10**: Diagnostic Session Control
- **0x11**: ECU Reset
- **0x14**: Clear Diagnostic Information
- **0x19**: Read DTC Information
- **0x22**: Read Data By Identifier
- **0x2E**: Write Data By Identifier
- **0x31**: Routine Control

### Data Identifiers (DIDs)
- **0xF100-0xF103**: Speed sensor data for each wheel
- **0xF110-0xF113**: Calibration parameters for each wheel
- **0xF120**: ABS system status
- **0xF121**: Malfunction counters

### Routine Identifiers (RIDs)
- **0x0201-0x0204**: Start calibration for each wheel
- **0x0210**: Validate all calibrations
- **0x0220**: Reset all calibrations to factory defaults
- **0x0230**: ABS self-test procedure

## Configuration

### Calibration Parameters
```c
typedef struct {
    float32 correctionFactor;   // Typical range: 0.8 - 1.2
    float32 offsetValue;        // Speed offset in km/h
    uint16 pulsesPerRevolution; // Sensor-specific (typically 60)
    float32 wheelCircumference; // In meters (typically 2.1m)
    boolean calibrationValid;   // Validation flag
} SpeedSensorCalibration_t;
```

### Detection Thresholds
- **Speed Difference Threshold**: 30 km/h (configurable)
- **Acceleration Threshold**: 15 m/s² (configurable)
- **Calibration Drift Limit**: 10% (configurable)
- **Debounce Time**: 100ms (configurable)

## AUTOSAR Configuration

### ARXML Files
- `config/arxml/SoftwareComponents.arxml`: Component definitions and interfaces
- `config/arxml/SystemDescription.arxml`: System composition and mappings

### RTE Configuration
The system uses the AUTOSAR Runtime Environment (RTE) for:
- Inter-component communication
- Timing synchronization
- Data consistency management
- Service orchestration

## Build and Integration

### Directory Structure
```
/ABS malfunction/
├── src/
│   ├── application/swc/     # Application software components
│   └── bsw/
│       ├── services/        # Basic software services
│       └── ecual/          # ECU abstraction layer
├── include/                 # Header files
├── config/arxml/           # AUTOSAR configuration
└── docs/                   # Documentation
```

### Dependencies
- AUTOSAR BSW modules (RTE, COM, NvM, Dem)
- Hardware abstraction layer for speed sensors
- CAN communication stack for diagnostics
- Non-volatile memory management

### Integration Steps
1. Configure AUTOSAR BSW stack
2. Implement hardware-specific sensor drivers
3. Configure RTE generation based on ARXML files
4. Integrate calibration data storage (EEPROM/Flash)
5. Set up diagnostic communication (CAN/Ethernet)
6. Perform system validation and testing

## Testing and Validation

### Unit Testing
- Individual component functionality
- Data processing algorithms
- Calibration procedures
- Diagnostic service handlers

### Integration Testing
- Component communication via RTE
- End-to-end data flow validation
- Timing and synchronization verification
- Error handling and recovery

### System Testing
- Real-world malfunction scenarios
- Calibration accuracy validation
- Diagnostic communication compliance
- Performance under various conditions

### Safety Validation
- ASIL compliance verification
- Failure mode analysis
- Robustness testing
- Functional safety requirements

## Calibration Procedures

### Automatic Calibration
1. Collect speed samples during normal driving
2. Compare with reference speed (GPS or other sensors)
3. Calculate correction factors using statistical methods
4. Validate results against acceptable ranges
5. Apply and store calibration if valid

### Manual Calibration
1. Set vehicle to known speed condition
2. Collect sensor data over specified time period
3. Calculate calibration based on known reference
4. Validate and apply corrections
5. Update NVM storage

### Factory Calibration Reset
- Restores default calibration parameters
- Clears calibration history
- Sets validation flags appropriately
- Triggers recalibration procedures

## Maintenance and Diagnostics

### Preventive Maintenance
- Regular calibration validation checks
- Periodic accuracy assessments
- History analysis for degradation trends
- Proactive recalibration scheduling

### Diagnostic Procedures
- Real-time malfunction monitoring
- Historical data analysis
- Comparative wheel performance assessment
- System health reporting

### Service Procedures
- DTC reading and clearing
- Calibration parameter adjustment
- Performance validation testing
- Software version management

## Safety Considerations

### Functional Safety (ISO 26262)
- ASIL B compliance for speed sensor functions
- Fail-safe behavior on malfunction detection
- Redundancy and cross-checking mechanisms
- Safe state transitions and error handling

### Cybersecurity
- Secure diagnostic communication protocols
- Authentication for calibration modifications
- Protection against malicious interference
- Audit trail for diagnostic activities

## Performance Characteristics

### Timing Requirements
- Speed sensor processing: 10ms cycle time
- Malfunction detection: 20ms cycle time
- Diagnostic response: < 100ms
- Calibration procedures: < 30 seconds

### Resource Usage
- RAM: Approximately 4KB per wheel sensor
- Flash: ~32KB for application code
- CPU Load: < 5% on typical automotive MCU
- NVM: 64 bytes per wheel for calibration data

## Future Enhancements

### Potential Improvements
- Machine learning-based malfunction detection
- Enhanced calibration algorithms
- Predictive maintenance capabilities
- Integration with vehicle-to-everything (V2X) systems
- Advanced sensor fusion techniques

### Technology Roadmap
- Support for next-generation speed sensors
- Integration with autonomous driving systems
- Enhanced cybersecurity measures
- Cloud-based diagnostic capabilities
- Over-the-air calibration updates

## References

- AUTOSAR Release 4.3 Specification
- ISO 26262 Functional Safety Standard
- ISO 14229 UDS Specification
- SAE J1979 OBD-II Standard
- Automotive safety and security best practices

---

**Note**: This system is designed for automotive safety-critical applications. Proper validation, testing, and certification are required before deployment in production vehicles.