/**
 * @file SpeedSensor_Types.h
 * @brief Type definitions for speed sensor interface
 * @author Generated for ABS Malfunction Detection System
 */

#ifndef SPEEDSENSOR_TYPES_H
#define SPEEDSENSOR_TYPES_H

#include "Std_Types.h"

/* Speed sensor wheel positions */
typedef enum {
    WHEEL_FRONT_LEFT = 0,
    WHEEL_FRONT_RIGHT = 1,
    WHEEL_REAR_LEFT = 2,
    WHEEL_REAR_RIGHT = 3,
    WHEEL_MAX
} WheelPosition_t;

/* Speed sensor status */
typedef enum {
    SENSOR_STATUS_OK = 0,
    SENSOR_STATUS_SHORT_CIRCUIT = 1,
    SENSOR_STATUS_OPEN_CIRCUIT = 2,
    SENSOR_STATUS_OUT_OF_RANGE = 3,
    SENSOR_STATUS_CALIBRATION_ERROR = 4,
    SENSOR_STATUS_INVALID = 5
} SensorStatus_t;

/* Speed sensor raw data */
typedef struct {
    uint16 pulseCount;      /* Pulse count from sensor */
    uint16 timeInterval;    /* Time interval in ms */
    SensorStatus_t status;  /* Sensor hardware status */
    boolean dataValid;      /* Data validity flag */
} SpeedSensorRawData_t;

/* Calculated speed data */
typedef struct {
    float32 wheelSpeed;         /* Wheel speed in km/h */
    float32 wheelSpeedRaw;      /* Uncalibrated wheel speed */
    float32 accelerationX;      /* Wheel acceleration */
    boolean speedValid;         /* Speed calculation validity */
    uint8 qualityFactor;        /* Speed data quality (0-100) */
} SpeedData_t;

/* Calibration parameters for speed sensor */
typedef struct {
    float32 correctionFactor;   /* Speed correction factor */
    float32 offsetValue;        /* Speed offset correction */
    uint16 pulsesPerRevolution; /* Sensor pulses per wheel revolution */
    float32 wheelCircumference; /* Wheel circumference in meters */
    boolean calibrationValid;   /* Calibration validity flag */
    uint32 calibrationTimestamp; /* Last calibration timestamp */
} SpeedSensorCalibration_t;

/* Speed sensor diagnostic data */
typedef struct {
    uint32 totalPulseCount;     /* Total lifetime pulse count */
    uint16 errorCount;          /* Number of detected errors */
    uint16 calibrationCycles;   /* Number of calibration cycles */
    SensorStatus_t lastStatus;  /* Last recorded sensor status */
    uint32 lastErrorTimestamp;  /* Timestamp of last error */
} SpeedSensorDiagnostics_t;

/* Complete speed sensor data structure */
typedef struct {
    WheelPosition_t wheelPosition;
    SpeedSensorRawData_t rawData;
    SpeedData_t speedData;
    SpeedSensorCalibration_t calibration;
    SpeedSensorDiagnostics_t diagnostics;
} SpeedSensorData_t;

/* ABS system constants */
#define SPEED_SENSOR_SAMPLE_RATE_MS     10U    /* 100 Hz sampling */
#define MAX_WHEEL_SPEED_KMH            300.0f  /* Maximum expected speed */
#define MIN_WHEEL_SPEED_KMH            0.1f    /* Minimum measurable speed */
#define CALIBRATION_TOLERANCE          5.0f    /* Calibration tolerance in % */
#define SPEED_DIFFERENCE_THRESHOLD     20.0f   /* Speed difference threshold for ABS trigger */

#endif /* SPEEDSENSOR_TYPES_H */