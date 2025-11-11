/**
 * @file ABS_MalfunctionDetection.h
 * @brief ABS Malfunction Detection Interface
 * @author Generated for ABS Malfunction Detection System
 */

#ifndef ABS_MALFUNCTIONDETECTION_H
#define ABS_MALFUNCTIONDETECTION_H

#include "Std_Types.h"
#include "SpeedSensor_Types.h"

/* ABS Malfunction Types */
typedef enum {
    ABS_MALFUNCTION_NONE = 0,
    ABS_MALFUNCTION_SPEED_SENSOR_MISCALIBRATION = 1,
    ABS_MALFUNCTION_SPEED_SENSOR_FAILURE = 2,
    ABS_MALFUNCTION_WHEEL_SLIP_EXCESSIVE = 3,
    ABS_MALFUNCTION_SPEED_DIFFERENCE_EXCESSIVE = 4,
    ABS_MALFUNCTION_ACCELERATION_IMPLAUSIBLE = 5,
    ABS_MALFUNCTION_CALIBRATION_DRIFT = 6,
    ABS_MALFUNCTION_SYSTEM_ERROR = 7
} ABS_MalfunctionType_t;

/* ABS Malfunction Severity */
typedef enum {
    ABS_SEVERITY_NONE = 0,
    ABS_SEVERITY_LOW = 1,
    ABS_SEVERITY_MEDIUM = 2,
    ABS_SEVERITY_HIGH = 3,
    ABS_SEVERITY_CRITICAL = 4
} ABS_MalfunctionSeverity_t;

/* ABS Malfunction Status */
typedef struct {
    ABS_MalfunctionType_t malfunctionType;
    ABS_MalfunctionSeverity_t severity;
    WheelPosition_t affectedWheel;
    boolean isActive;
    uint32 detectionTimestamp;
    uint16 occurrenceCount;
    float32 deviationValue;      /* Magnitude of deviation from expected */
    boolean confirmedMalfunction; /* True after debouncing */
} ABS_MalfunctionStatus_t;

/* ABS System State */
typedef enum {
    ABS_STATE_INACTIVE = 0,
    ABS_STATE_MONITORING = 1,
    ABS_STATE_INTERVENTION = 2,
    ABS_STATE_MALFUNCTION = 3,
    ABS_STATE_DEGRADED = 4
} ABS_SystemState_t;

/* ABS Detection Parameters */
typedef struct {
    float32 speedDifferenceThreshold;     /* km/h */
    float32 accelerationThreshold;        /* m/s² */
    float32 calibrationDriftThreshold;    /* % */
    uint16 debounceTimeMs;               /* Debounce time */
    uint8 consecutiveErrorsThreshold;     /* Number of consecutive errors */
    boolean enableMiscalibrationDetection;
    boolean enableSpeedPlausibilityCheck;
    boolean enableAccelerationCheck;
} ABS_DetectionParameters_t;

/* ABS Vehicle Data */
typedef struct {
    SpeedData_t wheelSpeeds[WHEEL_MAX];
    float32 vehicleReferenceSpeed;
    float32 longitudinalAcceleration;
    float32 lateralAcceleration;
    boolean brakePedalPressed;
    boolean vehicleStabilityActive;
    ABS_SystemState_t systemState;
} ABS_VehicleData_t;

/* Function prototypes */

/**
 * @brief Initialize ABS malfunction detection system
 */
Std_ReturnType ABS_MalfunctionDetection_Init(void);

/**
 * @brief Deinitialize ABS malfunction detection system
 */
Std_ReturnType ABS_MalfunctionDetection_DeInit(void);

/**
 * @brief Main processing function for ABS malfunction detection
 */
Std_ReturnType ABS_MalfunctionDetection_MainFunction(void);

/**
 * @brief Update vehicle data for malfunction detection
 */
Std_ReturnType ABS_UpdateVehicleData(const ABS_VehicleData_t* vehicleData);

/**
 * @brief Check for speed sensor miscalibration
 */
Std_ReturnType ABS_CheckSpeedSensorCalibration(WheelPosition_t wheelPos, boolean* isMiscalibrated);

/**
 * @brief Detect excessive speed differences between wheels
 */
Std_ReturnType ABS_DetectSpeedDifferences(boolean* excessiveDifference, WheelPosition_t* affectedWheel);

/**
 * @brief Validate speed plausibility based on acceleration
 */
Std_ReturnType ABS_ValidateSpeedPlausibility(WheelPosition_t wheelPos, boolean* isPlausible);

/**
 * @brief Get current malfunction status
 */
Std_ReturnType ABS_GetMalfunctionStatus(WheelPosition_t wheelPos, ABS_MalfunctionStatus_t* status);

/**
 * @brief Clear malfunction status
 */
Std_ReturnType ABS_ClearMalfunctionStatus(WheelPosition_t wheelPos);

/**
 * @brief Set detection parameters
 */
Std_ReturnType ABS_SetDetectionParameters(const ABS_DetectionParameters_t* params);

/**
 * @brief Get detection parameters
 */
Std_ReturnType ABS_GetDetectionParameters(ABS_DetectionParameters_t* params);

/**
 * @brief Check overall ABS system health
 */
Std_ReturnType ABS_CheckSystemHealth(boolean* systemHealthy, ABS_SystemState_t* systemState);

/* RTE Interface Functions */
void RE_ABS_MalfunctionDetection_MainCyclic(void);
void RE_ABS_MalfunctionDetection_SpeedPlausibility(void);
void RE_ABS_MalfunctionDetection_CalibrationCheck(void);

/* Port Interface for RTE */
extern Std_ReturnType Rte_Read_VehicleData_vehicleData(ABS_VehicleData_t* data);
extern Std_ReturnType Rte_Write_MalfunctionStatus_FL_status(const ABS_MalfunctionStatus_t* status);
extern Std_ReturnType Rte_Write_MalfunctionStatus_FR_status(const ABS_MalfunctionStatus_t* status);
extern Std_ReturnType Rte_Write_MalfunctionStatus_RL_status(const ABS_MalfunctionStatus_t* status);
extern Std_ReturnType Rte_Write_MalfunctionStatus_RR_status(const ABS_MalfunctionStatus_t* status);
extern Std_ReturnType Rte_Write_SystemState_state(const ABS_SystemState_t* state);

/* Constants */
#define ABS_DETECTION_CYCLE_MS          20U      /* 50 Hz processing */
#define ABS_MAX_SPEED_DIFFERENCE        30.0f    /* km/h */
#define ABS_MAX_ACCELERATION           15.0f     /* m/s² */
#define ABS_CALIBRATION_DRIFT_LIMIT    10.0f     /* % */
#define ABS_DEBOUNCE_TIME_MS           100U      /* 100ms debounce */
#define ABS_CONSECUTIVE_ERRORS_MAX     5U        /* Max consecutive errors */

#endif /* ABS_MALFUNCTIONDETECTION_H */