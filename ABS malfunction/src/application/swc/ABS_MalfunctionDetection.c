/**
 * @file ABS_MalfunctionDetection.c
 * @brief ABS Malfunction Detection Implementation
 * @author Generated for ABS Malfunction Detection System
 */

#include "ABS_MalfunctionDetection.h"
#include "SpeedSensor_Interface.h"
#include <string.h>
#include <math.h>

/* Local data structures */
static ABS_MalfunctionStatus_t g_MalfunctionStatus[WHEEL_MAX];
static ABS_DetectionParameters_t g_DetectionParams;
static ABS_VehicleData_t g_VehicleData;
static ABS_SystemState_t g_SystemState;
static boolean g_ABS_Initialized = FALSE;

/* Debounce counters for malfunction detection */
static uint16 g_DebounceCounters[WHEEL_MAX];
static uint8 g_ConsecutiveErrorCount[WHEEL_MAX];

/* Internal function prototypes */
static void ABS_InitDefaultParameters(void);
static Std_ReturnType ABS_ProcessWheelMalfunctionDetection(WheelPosition_t wheelPos);
static boolean ABS_CheckCalibrationDrift(WheelPosition_t wheelPos, float32* driftPercentage);
static boolean ABS_CheckSpeedPlausibility(WheelPosition_t wheelPos, float32* deviation);
static boolean ABS_CheckAccelerationPlausibility(WheelPosition_t wheelPos, float32* acceleration);
static void ABS_UpdateMalfunctionStatus(WheelPosition_t wheelPos, ABS_MalfunctionType_t type, 
                                       ABS_MalfunctionSeverity_t severity, float32 deviation);
static void ABS_ProcessDebouncing(WheelPosition_t wheelPos);
static ABS_MalfunctionSeverity_t ABS_DetermineSeverity(ABS_MalfunctionType_t type, float32 deviation);
static void ABS_UpdateSystemState(void);
static float32 ABS_CalculateMedianSpeed(void);

/**
 * @brief Initialize ABS malfunction detection system
 */
Std_ReturnType ABS_MalfunctionDetection_Init(void)
{
    uint8 wheelIdx;
    
    if (g_ABS_Initialized == FALSE)
    {
        /* Initialize malfunction status for all wheels */
        for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
        {
            memset(&g_MalfunctionStatus[wheelIdx], 0, sizeof(ABS_MalfunctionStatus_t));
            g_MalfunctionStatus[wheelIdx].malfunctionType = ABS_MALFUNCTION_NONE;
            g_MalfunctionStatus[wheelIdx].severity = ABS_SEVERITY_NONE;
            g_MalfunctionStatus[wheelIdx].affectedWheel = (WheelPosition_t)wheelIdx;
            g_MalfunctionStatus[wheelIdx].isActive = FALSE;
            g_MalfunctionStatus[wheelIdx].confirmedMalfunction = FALSE;
            
            g_DebounceCounters[wheelIdx] = 0;
            g_ConsecutiveErrorCount[wheelIdx] = 0;
        }
        
        /* Initialize default detection parameters */
        ABS_InitDefaultParameters();
        
        /* Initialize vehicle data */
        memset(&g_VehicleData, 0, sizeof(ABS_VehicleData_t));
        g_SystemState = ABS_STATE_MONITORING;
        
        g_ABS_Initialized = TRUE;
    }
    
    return E_OK;
}

/**
 * @brief Deinitialize ABS malfunction detection system
 */
Std_ReturnType ABS_MalfunctionDetection_DeInit(void)
{
    g_ABS_Initialized = FALSE;
    g_SystemState = ABS_STATE_INACTIVE;
    return E_OK;
}

/**
 * @brief Main processing function for ABS malfunction detection
 */
Std_ReturnType ABS_MalfunctionDetection_MainFunction(void)
{
    Std_ReturnType retVal = E_OK;
    uint8 wheelIdx;
    
    if (g_ABS_Initialized == TRUE)
    {
        /* Process malfunction detection for all wheels */
        for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
        {
            ABS_ProcessWheelMalfunctionDetection((WheelPosition_t)wheelIdx);
            ABS_ProcessDebouncing((WheelPosition_t)wheelIdx);
        }
        
        /* Update overall system state */
        ABS_UpdateSystemState();
    }
    else
    {
        retVal = E_NOT_OK;
    }
    
    return retVal;
}

/**
 * @brief Update vehicle data for malfunction detection
 */
Std_ReturnType ABS_UpdateVehicleData(const ABS_VehicleData_t* vehicleData)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((vehicleData != NULL_PTR) && (g_ABS_Initialized == TRUE))
    {
        g_VehicleData = *vehicleData;
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Check for speed sensor miscalibration
 */
Std_ReturnType ABS_CheckSpeedSensorCalibration(WheelPosition_t wheelPos, boolean* isMiscalibrated)
{
    Std_ReturnType retVal = E_NOT_OK;
    float32 driftPercentage;
    
    if ((wheelPos < WHEEL_MAX) && (isMiscalibrated != NULL_PTR) && (g_ABS_Initialized == TRUE))
    {
        *isMiscalibrated = ABS_CheckCalibrationDrift(wheelPos, &driftPercentage);
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Detect excessive speed differences between wheels
 */
Std_ReturnType ABS_DetectSpeedDifferences(boolean* excessiveDifference, WheelPosition_t* affectedWheel)
{
    Std_ReturnType retVal = E_NOT_OK;
    float32 referenceSpeed;
    float32 speedDifference;
    uint8 wheelIdx;
    
    if ((excessiveDifference != NULL_PTR) && (affectedWheel != NULL_PTR) && (g_ABS_Initialized == TRUE))
    {
        *excessiveDifference = FALSE;
        referenceSpeed = ABS_CalculateMedianSpeed();
        
        /* Check each wheel speed against reference */
        for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
        {
            if (g_VehicleData.wheelSpeeds[wheelIdx].speedValid == TRUE)
            {
                speedDifference = fabsf(g_VehicleData.wheelSpeeds[wheelIdx].wheelSpeed - referenceSpeed);
                
                if (speedDifference > g_DetectionParams.speedDifferenceThreshold)
                {
                    *excessiveDifference = TRUE;
                    *affectedWheel = (WheelPosition_t)wheelIdx;
                    break;
                }
            }
        }
        
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Validate speed plausibility based on acceleration
 */
Std_ReturnType ABS_ValidateSpeedPlausibility(WheelPosition_t wheelPos, boolean* isPlausible)
{
    Std_ReturnType retVal = E_NOT_OK;
    float32 deviation;
    
    if ((wheelPos < WHEEL_MAX) && (isPlausible != NULL_PTR) && (g_ABS_Initialized == TRUE))
    {
        *isPlausible = ABS_CheckSpeedPlausibility(wheelPos, &deviation);
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Get current malfunction status
 */
Std_ReturnType ABS_GetMalfunctionStatus(WheelPosition_t wheelPos, ABS_MalfunctionStatus_t* status)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((wheelPos < WHEEL_MAX) && (status != NULL_PTR) && (g_ABS_Initialized == TRUE))
    {
        *status = g_MalfunctionStatus[wheelPos];
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Clear malfunction status
 */
Std_ReturnType ABS_ClearMalfunctionStatus(WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((wheelPos < WHEEL_MAX) && (g_ABS_Initialized == TRUE))
    {
        g_MalfunctionStatus[wheelPos].isActive = FALSE;
        g_MalfunctionStatus[wheelPos].confirmedMalfunction = FALSE;
        g_MalfunctionStatus[wheelPos].malfunctionType = ABS_MALFUNCTION_NONE;
        g_MalfunctionStatus[wheelPos].severity = ABS_SEVERITY_NONE;
        g_DebounceCounters[wheelPos] = 0;
        g_ConsecutiveErrorCount[wheelPos] = 0;
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Set detection parameters
 */
Std_ReturnType ABS_SetDetectionParameters(const ABS_DetectionParameters_t* params)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((params != NULL_PTR) && (g_ABS_Initialized == TRUE))
    {
        g_DetectionParams = *params;
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Get detection parameters
 */
Std_ReturnType ABS_GetDetectionParameters(ABS_DetectionParameters_t* params)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((params != NULL_PTR) && (g_ABS_Initialized == TRUE))
    {
        *params = g_DetectionParams;
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Check overall ABS system health
 */
Std_ReturnType ABS_CheckSystemHealth(boolean* systemHealthy, ABS_SystemState_t* systemState)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint8 wheelIdx;
    uint8 malfunctionCount = 0;
    
    if ((systemHealthy != NULL_PTR) && (systemState != NULL_PTR) && (g_ABS_Initialized == TRUE))
    {
        /* Count active malfunctions */
        for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
        {
            if (g_MalfunctionStatus[wheelIdx].isActive == TRUE)
            {
                malfunctionCount++;
            }
        }
        
        /* Determine system health */
        *systemHealthy = (malfunctionCount == 0);
        *systemState = g_SystemState;
        
        retVal = E_OK;
    }
    
    return retVal;
}

/* Internal Functions */

/**
 * @brief Initialize default detection parameters
 */
static void ABS_InitDefaultParameters(void)
{
    g_DetectionParams.speedDifferenceThreshold = ABS_MAX_SPEED_DIFFERENCE;
    g_DetectionParams.accelerationThreshold = ABS_MAX_ACCELERATION;
    g_DetectionParams.calibrationDriftThreshold = ABS_CALIBRATION_DRIFT_LIMIT;
    g_DetectionParams.debounceTimeMs = ABS_DEBOUNCE_TIME_MS;
    g_DetectionParams.consecutiveErrorsThreshold = ABS_CONSECUTIVE_ERRORS_MAX;
    g_DetectionParams.enableMiscalibrationDetection = TRUE;
    g_DetectionParams.enableSpeedPlausibilityCheck = TRUE;
    g_DetectionParams.enableAccelerationCheck = TRUE;
}

/**
 * @brief Process malfunction detection for a specific wheel
 */
static Std_ReturnType ABS_ProcessWheelMalfunctionDetection(WheelPosition_t wheelPos)
{
    float32 deviation;
    boolean malfunctionDetected = FALSE;
    ABS_MalfunctionType_t malfunctionType = ABS_MALFUNCTION_NONE;
    ABS_MalfunctionSeverity_t severity = ABS_SEVERITY_NONE;
    
    /* Check calibration drift */
    if (g_DetectionParams.enableMiscalibrationDetection == TRUE)
    {
        if (ABS_CheckCalibrationDrift(wheelPos, &deviation) == TRUE)
        {
            malfunctionDetected = TRUE;
            malfunctionType = ABS_MALFUNCTION_SPEED_SENSOR_MISCALIBRATION;
            severity = ABS_DetermineSeverity(malfunctionType, deviation);
        }
    }
    
    /* Check speed plausibility */
    if ((g_DetectionParams.enableSpeedPlausibilityCheck == TRUE) && (malfunctionDetected == FALSE))
    {
        if (ABS_CheckSpeedPlausibility(wheelPos, &deviation) == FALSE)
        {
            malfunctionDetected = TRUE;
            malfunctionType = ABS_MALFUNCTION_SPEED_DIFFERENCE_EXCESSIVE;
            severity = ABS_DetermineSeverity(malfunctionType, deviation);
        }
    }
    
    /* Check acceleration plausibility */
    if ((g_DetectionParams.enableAccelerationCheck == TRUE) && (malfunctionDetected == FALSE))
    {
        if (ABS_CheckAccelerationPlausibility(wheelPos, &deviation) == FALSE)
        {
            malfunctionDetected = TRUE;
            malfunctionType = ABS_MALFUNCTION_ACCELERATION_IMPLAUSIBLE;
            severity = ABS_DetermineSeverity(malfunctionType, deviation);
        }
    }
    
    /* Update malfunction status */
    if (malfunctionDetected == TRUE)
    {
        ABS_UpdateMalfunctionStatus(wheelPos, malfunctionType, severity, deviation);
        g_ConsecutiveErrorCount[wheelPos]++;
    }
    else
    {
        /* Reset consecutive error count if no malfunction */
        g_ConsecutiveErrorCount[wheelPos] = 0;
        
        /* Clear malfunction if it was temporary */
        if (g_MalfunctionStatus[wheelPos].isActive == TRUE)
        {
            g_DebounceCounters[wheelPos] = 0;
        }
    }
    
    return E_OK;
}

/**
 * @brief Check for calibration drift
 */
static boolean ABS_CheckCalibrationDrift(WheelPosition_t wheelPos, float32* driftPercentage)
{
    SpeedSensorCalibration_t calibration;
    boolean isDrifted = FALSE;
    float32 expectedCorrectionFactor = 1.0f; /* Ideal correction factor */
    
    if (SpeedSensor_GetCalibration(wheelPos, &calibration) == E_OK)
    {
        /* Calculate drift percentage */
        *driftPercentage = fabsf((calibration.correctionFactor - expectedCorrectionFactor) / 
                                expectedCorrectionFactor) * 100.0f;
        
        /* Check if drift exceeds threshold */
        if (*driftPercentage > g_DetectionParams.calibrationDriftThreshold)
        {
            isDrifted = TRUE;
        }
    }
    
    return isDrifted;
}

/**
 * @brief Check speed plausibility
 */
static boolean ABS_CheckSpeedPlausibility(WheelPosition_t wheelPos, float32* deviation)
{
    boolean isPlausible = TRUE;
    float32 referenceSpeed;
    float32 wheelSpeed;
    
    if (g_VehicleData.wheelSpeeds[wheelPos].speedValid == TRUE)
    {
        wheelSpeed = g_VehicleData.wheelSpeeds[wheelPos].wheelSpeed;
        referenceSpeed = ABS_CalculateMedianSpeed();
        
        *deviation = fabsf(wheelSpeed - referenceSpeed);
        
        /* Check if speed deviation exceeds threshold */
        if (*deviation > g_DetectionParams.speedDifferenceThreshold)
        {
            isPlausible = FALSE;
        }
    }
    else
    {
        isPlausible = FALSE;
        *deviation = 0.0f;
    }
    
    return isPlausible;
}

/**
 * @brief Check acceleration plausibility
 */
static boolean ABS_CheckAccelerationPlausibility(WheelPosition_t wheelPos, float32* acceleration)
{
    boolean isPlausible = TRUE;
    
    if (g_VehicleData.wheelSpeeds[wheelPos].speedValid == TRUE)
    {
        *acceleration = fabsf(g_VehicleData.wheelSpeeds[wheelPos].accelerationX);
        
        /* Check if acceleration exceeds threshold (unless braking) */
        if ((*acceleration > g_DetectionParams.accelerationThreshold) && 
            (g_VehicleData.brakePedalPressed == FALSE))
        {
            isPlausible = FALSE;
        }
    }
    else
    {
        isPlausible = FALSE;
        *acceleration = 0.0f;
    }
    
    return isPlausible;
}

/**
 * @brief Update malfunction status
 */
static void ABS_UpdateMalfunctionStatus(WheelPosition_t wheelPos, ABS_MalfunctionType_t type, 
                                       ABS_MalfunctionSeverity_t severity, float32 deviation)
{
    g_MalfunctionStatus[wheelPos].malfunctionType = type;
    g_MalfunctionStatus[wheelPos].severity = severity;
    g_MalfunctionStatus[wheelPos].isActive = TRUE;
    g_MalfunctionStatus[wheelPos].deviationValue = deviation;
    g_MalfunctionStatus[wheelPos].detectionTimestamp = 0; /* Should be actual timestamp */
    g_MalfunctionStatus[wheelPos].occurrenceCount++;
}

/**
 * @brief Process debouncing logic
 */
static void ABS_ProcessDebouncing(WheelPosition_t wheelPos)
{
    if (g_MalfunctionStatus[wheelPos].isActive == TRUE)
    {
        g_DebounceCounters[wheelPos] += ABS_DETECTION_CYCLE_MS;
        
        /* Confirm malfunction after debounce time */
        if (g_DebounceCounters[wheelPos] >= g_DetectionParams.debounceTimeMs)
        {
            g_MalfunctionStatus[wheelPos].confirmedMalfunction = TRUE;
        }
    }
    else
    {
        /* Reset debounce counter */
        g_DebounceCounters[wheelPos] = 0;
        g_MalfunctionStatus[wheelPos].confirmedMalfunction = FALSE;
    }
}

/**
 * @brief Determine malfunction severity
 */
static ABS_MalfunctionSeverity_t ABS_DetermineSeverity(ABS_MalfunctionType_t type, float32 deviation)
{
    ABS_MalfunctionSeverity_t severity = ABS_SEVERITY_LOW;
    
    switch (type)
    {
        case ABS_MALFUNCTION_SPEED_SENSOR_MISCALIBRATION:
            if (deviation > 15.0f) severity = ABS_SEVERITY_CRITICAL;
            else if (deviation > 10.0f) severity = ABS_SEVERITY_HIGH;
            else if (deviation > 5.0f) severity = ABS_SEVERITY_MEDIUM;
            else severity = ABS_SEVERITY_LOW;
            break;
            
        case ABS_MALFUNCTION_SPEED_DIFFERENCE_EXCESSIVE:
            if (deviation > 50.0f) severity = ABS_SEVERITY_CRITICAL;
            else if (deviation > 30.0f) severity = ABS_SEVERITY_HIGH;
            else if (deviation > 20.0f) severity = ABS_SEVERITY_MEDIUM;
            else severity = ABS_SEVERITY_LOW;
            break;
            
        case ABS_MALFUNCTION_ACCELERATION_IMPLAUSIBLE:
            if (deviation > 20.0f) severity = ABS_SEVERITY_CRITICAL;
            else if (deviation > 15.0f) severity = ABS_SEVERITY_HIGH;
            else severity = ABS_SEVERITY_MEDIUM;
            break;
            
        default:
            severity = ABS_SEVERITY_LOW;
            break;
    }
    
    return severity;
}

/**
 * @brief Update overall system state
 */
static void ABS_UpdateSystemState(void)
{
    uint8 wheelIdx;
    uint8 criticalMalfunctions = 0;
    uint8 activeMalfunctions = 0;
    
    /* Count malfunctions by severity */
    for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
    {
        if (g_MalfunctionStatus[wheelIdx].confirmedMalfunction == TRUE)
        {
            activeMalfunctions++;
            
            if (g_MalfunctionStatus[wheelIdx].severity >= ABS_SEVERITY_HIGH)
            {
                criticalMalfunctions++;
            }
        }
    }
    
    /* Determine system state */
    if (criticalMalfunctions > 0)
    {
        g_SystemState = ABS_STATE_MALFUNCTION;
    }
    else if (activeMalfunctions > 0)
    {
        g_SystemState = ABS_STATE_DEGRADED;
    }
    else
    {
        g_SystemState = ABS_STATE_MONITORING;
    }
}

/**
 * @brief Calculate median speed from all valid wheel speeds
 */
static float32 ABS_CalculateMedianSpeed(void)
{
    float32 validSpeeds[WHEEL_MAX];
    uint8 validCount = 0;
    uint8 wheelIdx;
    float32 medianSpeed = 0.0f;
    
    /* Collect valid speeds */
    for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
    {
        if (g_VehicleData.wheelSpeeds[wheelIdx].speedValid == TRUE)
        {
            validSpeeds[validCount] = g_VehicleData.wheelSpeeds[wheelIdx].wheelSpeed;
            validCount++;
        }
    }
    
    /* Calculate median (simplified for 4 wheels) */
    if (validCount >= 2)
    {
        /* Simple sorting for small array */
        for (uint8 i = 0; i < validCount - 1; i++)
        {
            for (uint8 j = 0; j < validCount - i - 1; j++)
            {
                if (validSpeeds[j] > validSpeeds[j + 1])
                {
                    float32 temp = validSpeeds[j];
                    validSpeeds[j] = validSpeeds[j + 1];
                    validSpeeds[j + 1] = temp;
                }
            }
        }
        
        /* Get median value */
        if (validCount % 2 == 0)
        {
            medianSpeed = (validSpeeds[validCount/2 - 1] + validSpeeds[validCount/2]) / 2.0f;
        }
        else
        {
            medianSpeed = validSpeeds[validCount/2];
        }
    }
    
    return medianSpeed;
}

/* RTE Runnable Functions */

/**
 * @brief RTE Runnable for main malfunction detection
 */
void RE_ABS_MalfunctionDetection_MainCyclic(void)
{
    ABS_MalfunctionDetection_MainFunction();
    
    /* Send malfunction status via RTE */
    ABS_MalfunctionStatus_t status;
    
    if (ABS_GetMalfunctionStatus(WHEEL_FRONT_LEFT, &status) == E_OK)
    {
        Rte_Write_MalfunctionStatus_FL_status(&status);
    }
    
    if (ABS_GetMalfunctionStatus(WHEEL_FRONT_RIGHT, &status) == E_OK)
    {
        Rte_Write_MalfunctionStatus_FR_status(&status);
    }
    
    if (ABS_GetMalfunctionStatus(WHEEL_REAR_LEFT, &status) == E_OK)
    {
        Rte_Write_MalfunctionStatus_RL_status(&status);
    }
    
    if (ABS_GetMalfunctionStatus(WHEEL_REAR_RIGHT, &status) == E_OK)
    {
        Rte_Write_MalfunctionStatus_RR_status(&status);
    }
    
    /* Send system state */
    Rte_Write_SystemState_state(&g_SystemState);
}

/**
 * @brief RTE Runnable for speed plausibility check
 */
void RE_ABS_MalfunctionDetection_SpeedPlausibility(void)
{
    /* This can be called on-demand for specific plausibility checks */
}

/**
 * @brief RTE Runnable for calibration check
 */
void RE_ABS_MalfunctionDetection_CalibrationCheck(void)
{
    /* This can be called on-demand for calibration validation */
}