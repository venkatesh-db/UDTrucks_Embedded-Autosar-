/**
 * @file CalibrationManager.h
 * @brief Calibration Management Service for Speed Sensors
 * @author Generated for ABS Malfunction Detection System
 */

#ifndef CALIBRATIONMANAGER_H
#define CALIBRATIONMANAGER_H

#include "Std_Types.h"
#include "SpeedSensor_Types.h"

/* Calibration operation result codes */
typedef enum {
    CALIBRATION_RESULT_OK = 0,
    CALIBRATION_RESULT_NOT_OK = 1,
    CALIBRATION_RESULT_INVALID_PARAM = 2,
    CALIBRATION_RESULT_OUT_OF_RANGE = 3,
    CALIBRATION_RESULT_NVM_ERROR = 4,
    CALIBRATION_RESULT_VALIDATION_FAILED = 5,
    CALIBRATION_RESULT_IN_PROGRESS = 6
} CalibrationResult_t;

/* Calibration methods */
typedef enum {
    CALIBRATION_METHOD_MANUAL = 0,
    CALIBRATION_METHOD_AUTOMATIC = 1,
    CALIBRATION_METHOD_REFERENCE_BASED = 2,
    CALIBRATION_METHOD_GPS_BASED = 3,
    CALIBRATION_METHOD_FACTORY_RESET = 4
} CalibrationMethod_t;

/* Calibration state */
typedef enum {
    CALIBRATION_STATE_IDLE = 0,
    CALIBRATION_STATE_REQUESTED = 1,
    CALIBRATION_STATE_IN_PROGRESS = 2,
    CALIBRATION_STATE_COMPLETED = 3,
    CALIBRATION_STATE_FAILED = 4,
    CALIBRATION_STATE_CANCELLED = 5
} CalibrationState_t;

/* Calibration request parameters */
typedef struct {
    WheelPosition_t wheelPosition;
    CalibrationMethod_t method;
    float32 referenceSpeed;          /* Reference speed for calibration */
    float32 tolerancePercentage;     /* Acceptable tolerance in % */
    uint16 calibrationTimeMs;        /* Time duration for calibration */
    boolean forceCalibration;        /* Force calibration even if current is valid */
} CalibrationRequest_t;

/* Calibration session data */
typedef struct {
    CalibrationRequest_t request;
    CalibrationState_t state;
    CalibrationResult_t result;
    uint32 startTimestamp;
    uint32 endTimestamp;
    uint16 samplesCollected;
    float32 calculatedCorrectionFactor;
    float32 calculatedOffset;
    float32 measuredAccuracy;
    boolean sessionActive;
} CalibrationSession_t;

/* Calibration history entry */
typedef struct {
    uint32 timestamp;
    CalibrationMethod_t method;
    CalibrationResult_t result;
    float32 oldCorrectionFactor;
    float32 newCorrectionFactor;
    float32 accuracy;
} CalibrationHistoryEntry_t;

/* Calibration manager configuration */
typedef struct {
    uint16 maxCalibrationSamples;
    uint16 minCalibrationSamples;
    float32 maxCorrectionFactor;
    float32 minCorrectionFactor;
    float32 defaultTolerance;
    uint16 calibrationTimeoutMs;
    boolean enableAutoCalibration;
    uint16 autoCalibrationIntervalHours;
} CalibrationConfig_t;

/* Function prototypes */

/**
 * @brief Initialize calibration manager
 */
Std_ReturnType CalibrationManager_Init(void);

/**
 * @brief Deinitialize calibration manager
 */
Std_ReturnType CalibrationManager_DeInit(void);

/**
 * @brief Main function for calibration manager
 */
Std_ReturnType CalibrationManager_MainFunction(void);

/**
 * @brief Start calibration for specific wheel
 */
Std_ReturnType CalibrationManager_StartCalibration(const CalibrationRequest_t* request);

/**
 * @brief Cancel ongoing calibration
 */
Std_ReturnType CalibrationManager_CancelCalibration(WheelPosition_t wheelPos);

/**
 * @brief Get calibration session status
 */
Std_ReturnType CalibrationManager_GetSessionStatus(WheelPosition_t wheelPos, CalibrationSession_t* session);

/**
 * @brief Apply calculated calibration parameters
 */
Std_ReturnType CalibrationManager_ApplyCalibration(WheelPosition_t wheelPos, boolean saveToNvm);

/**
 * @brief Validate current calibration parameters
 */
Std_ReturnType CalibrationManager_ValidateCalibration(WheelPosition_t wheelPos, boolean* isValid, float32* accuracy);

/**
 * @brief Reset calibration to factory defaults
 */
Std_ReturnType CalibrationManager_ResetToFactory(WheelPosition_t wheelPos);

/**
 * @brief Load calibration from NVM
 */
Std_ReturnType CalibrationManager_LoadFromNvm(WheelPosition_t wheelPos);

/**
 * @brief Save calibration to NVM
 */
Std_ReturnType CalibrationManager_SaveToNvm(WheelPosition_t wheelPos);

/**
 * @brief Get calibration history
 */
Std_ReturnType CalibrationManager_GetHistory(WheelPosition_t wheelPos, CalibrationHistoryEntry_t* history, uint8* count);

/**
 * @brief Clear calibration history
 */
Std_ReturnType CalibrationManager_ClearHistory(WheelPosition_t wheelPos);

/**
 * @brief Set calibration configuration
 */
Std_ReturnType CalibrationManager_SetConfig(const CalibrationConfig_t* config);

/**
 * @brief Get calibration configuration
 */
Std_ReturnType CalibrationManager_GetConfig(CalibrationConfig_t* config);

/**
 * @brief Perform automatic calibration check
 */
Std_ReturnType CalibrationManager_AutoCalibrationCheck(void);

/* RTE Runnable Functions */
void RE_CalibrationManager_MainCyclic(void);
void RE_CalibrationManager_CalibrationProcess(void);
void RE_CalibrationManager_NvmManager(void);

/* Client-Server Interface Functions */
extern Std_ReturnType Rte_Call_NvmService_ReadBlock(uint16 blockId, void* dataPtr);
extern Std_ReturnType Rte_Call_NvmService_WriteBlock(uint16 blockId, const void* dataPtr);
extern Std_ReturnType Rte_Call_DiagnosticService_SetDTC(uint16 dtcId, boolean active);

/* Constants */
#define CALIBRATION_MAX_SAMPLES         1000U
#define CALIBRATION_MIN_SAMPLES         50U
#define CALIBRATION_TIMEOUT_MS          30000U    /* 30 seconds */
#define CALIBRATION_HISTORY_SIZE        10U
#define CALIBRATION_NVM_BLOCK_SIZE      64U
#define CALIBRATION_AUTO_INTERVAL_MS    3600000U  /* 1 hour */

/* NVM Block IDs for calibration data */
#define NVM_BLOCK_CALIBRATION_FL        0x1001U
#define NVM_BLOCK_CALIBRATION_FR        0x1002U
#define NVM_BLOCK_CALIBRATION_RL        0x1003U
#define NVM_BLOCK_CALIBRATION_RR        0x1004U

/* DTC codes for calibration issues */
#define DTC_CALIBRATION_FAILED          0xC14187U
#define DTC_CALIBRATION_OUT_OF_RANGE    0xC14287U
#define DTC_CALIBRATION_NVM_ERROR       0xC14387U

#endif /* CALIBRATIONMANAGER_H */