/**
 * @file SpeedSensor_Swc.c
 * @brief AUTOSAR Software Component Implementation for Speed Sensor
 * @author Generated for ABS Malfunction Detection System
 */

#include "SpeedSensor_Interface.h"
#include "Std_Types.h"
#include <string.h>

/* Local data storage for speed sensors */
static SpeedSensorData_t g_SpeedSensorData[WHEEL_MAX];
static boolean g_SpeedSensor_Initialized = FALSE;

/* Internal function prototypes */
static Std_ReturnType SpeedSensor_ProcessRawData(WheelPosition_t wheelPos);
static Std_ReturnType SpeedSensor_CalculateSpeed(WheelPosition_t wheelPos);
static Std_ReturnType SpeedSensor_ValidateSpeedData(WheelPosition_t wheelPos);
static void SpeedSensor_UpdateDiagnostics(WheelPosition_t wheelPos);

/**
 * @brief Initialize speed sensor interface
 */
Std_ReturnType SpeedSensor_Init(void)
{
    Std_ReturnType retVal = E_OK;
    uint8 wheelIdx;
    
    if (g_SpeedSensor_Initialized == FALSE)
    {
        /* Initialize all wheel sensor data */
        for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
        {
            memset(&g_SpeedSensorData[wheelIdx], 0, sizeof(SpeedSensorData_t));
            
            /* Set default values */
            g_SpeedSensorData[wheelIdx].wheelPosition = (WheelPosition_t)wheelIdx;
            g_SpeedSensorData[wheelIdx].rawData.status = SENSOR_STATUS_INVALID;
            g_SpeedSensorData[wheelIdx].rawData.dataValid = FALSE;
            g_SpeedSensorData[wheelIdx].speedData.speedValid = FALSE;
            
            /* Default calibration parameters */
            g_SpeedSensorData[wheelIdx].calibration.correctionFactor = 1.0f;
            g_SpeedSensorData[wheelIdx].calibration.offsetValue = 0.0f;
            g_SpeedSensorData[wheelIdx].calibration.pulsesPerRevolution = 60; /* Typical ABS sensor */
            g_SpeedSensorData[wheelIdx].calibration.wheelCircumference = 2.1f; /* Meters */
            g_SpeedSensorData[wheelIdx].calibration.calibrationValid = TRUE;
        }
        
        g_SpeedSensor_Initialized = TRUE;
    }
    
    return retVal;
}

/**
 * @brief Deinitialize speed sensor interface
 */
Std_ReturnType SpeedSensor_DeInit(void)
{
    g_SpeedSensor_Initialized = FALSE;
    return E_OK;
}

/**
 * @brief Main processing function - called cyclically by RTE
 */
Std_ReturnType SpeedSensor_MainFunction(void)
{
    Std_ReturnType retVal = E_OK;
    uint8 wheelIdx;
    
    if (g_SpeedSensor_Initialized == TRUE)
    {
        /* Process all wheel sensors */
        for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
        {
            if (SpeedSensor_ProcessRawData((WheelPosition_t)wheelIdx) == E_OK)
            {
                SpeedSensor_CalculateSpeed((WheelPosition_t)wheelIdx);
                SpeedSensor_ValidateSpeedData((WheelPosition_t)wheelIdx);
                SpeedSensor_UpdateDiagnostics((WheelPosition_t)wheelIdx);
            }
        }
    }
    else
    {
        retVal = E_NOT_OK;
    }
    
    return retVal;
}

/**
 * @brief Read raw speed sensor data for specific wheel
 */
Std_ReturnType SpeedSensor_ReadRawData(WheelPosition_t wheelPos, SpeedSensorRawData_t* rawData)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((wheelPos < WHEEL_MAX) && (rawData != NULL_PTR) && (g_SpeedSensor_Initialized == TRUE))
    {
        *rawData = g_SpeedSensorData[wheelPos].rawData;
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Get calculated speed data for specific wheel
 */
Std_ReturnType SpeedSensor_GetSpeedData(WheelPosition_t wheelPos, SpeedData_t* speedData)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((wheelPos < WHEEL_MAX) && (speedData != NULL_PTR) && (g_SpeedSensor_Initialized == TRUE))
    {
        *speedData = g_SpeedSensorData[wheelPos].speedData;
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Set calibration parameters for speed sensor
 */
Std_ReturnType SpeedSensor_SetCalibration(WheelPosition_t wheelPos, const SpeedSensorCalibration_t* calibration)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((wheelPos < WHEEL_MAX) && (calibration != NULL_PTR) && (g_SpeedSensor_Initialized == TRUE))
    {
        /* Validate calibration parameters */
        if ((calibration->correctionFactor > 0.5f) && (calibration->correctionFactor < 2.0f) &&
            (calibration->pulsesPerRevolution > 0) && (calibration->wheelCircumference > 0.0f))
        {
            g_SpeedSensorData[wheelPos].calibration = *calibration;
            g_SpeedSensorData[wheelPos].calibration.calibrationValid = TRUE;
            
            /* Increment calibration cycle counter */
            g_SpeedSensorData[wheelPos].diagnostics.calibrationCycles++;
            
            retVal = E_OK;
        }
    }
    
    return retVal;
}

/**
 * @brief Get calibration parameters for speed sensor
 */
Std_ReturnType SpeedSensor_GetCalibration(WheelPosition_t wheelPos, SpeedSensorCalibration_t* calibration)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((wheelPos < WHEEL_MAX) && (calibration != NULL_PTR) && (g_SpeedSensor_Initialized == TRUE))
    {
        *calibration = g_SpeedSensorData[wheelPos].calibration;
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Perform calibration validation
 */
Std_ReturnType SpeedSensor_ValidateCalibration(WheelPosition_t wheelPos, boolean* isValid)
{
    Std_ReturnType retVal = E_NOT_OK;
    SpeedSensorCalibration_t* cal;
    
    if ((wheelPos < WHEEL_MAX) && (isValid != NULL_PTR) && (g_SpeedSensor_Initialized == TRUE))
    {
        cal = &g_SpeedSensorData[wheelPos].calibration;
        
        /* Check calibration parameter ranges */
        *isValid = (cal->correctionFactor >= 0.8f) && (cal->correctionFactor <= 1.2f) &&
                   (cal->pulsesPerRevolution >= 30) && (cal->pulsesPerRevolution <= 120) &&
                   (cal->wheelCircumference >= 1.5f) && (cal->wheelCircumference <= 3.0f) &&
                   (cal->calibrationValid == TRUE);
        
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Get sensor diagnostic information
 */
Std_ReturnType SpeedSensor_GetDiagnostics(WheelPosition_t wheelPos, SpeedSensorDiagnostics_t* diagnostics)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((wheelPos < WHEEL_MAX) && (diagnostics != NULL_PTR) && (g_SpeedSensor_Initialized == TRUE))
    {
        *diagnostics = g_SpeedSensorData[wheelPos].diagnostics;
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Clear sensor error counters
 */
Std_ReturnType SpeedSensor_ClearErrors(WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((wheelPos < WHEEL_MAX) && (g_SpeedSensor_Initialized == TRUE))
    {
        g_SpeedSensorData[wheelPos].diagnostics.errorCount = 0;
        g_SpeedSensorData[wheelPos].diagnostics.lastErrorTimestamp = 0;
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Check if all speed sensors are functional
 */
Std_ReturnType SpeedSensor_CheckAllSensors(boolean* allSensorsOk)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint8 wheelIdx;
    
    if ((allSensorsOk != NULL_PTR) && (g_SpeedSensor_Initialized == TRUE))
    {
        *allSensorsOk = TRUE;
        
        /* Check all wheel sensors */
        for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
        {
            if ((g_SpeedSensorData[wheelIdx].rawData.status != SENSOR_STATUS_OK) ||
                (g_SpeedSensorData[wheelIdx].speedData.speedValid != TRUE))
            {
                *allSensorsOk = FALSE;
                break;
            }
        }
        
        retVal = E_OK;
    }
    
    return retVal;
}

/* Internal Functions */

/**
 * @brief Process raw sensor data from hardware
 */
static Std_ReturnType SpeedSensor_ProcessRawData(WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_OK;
    SpeedSensorRawData_t rawData;
    
    /* Read raw data from hardware layer via RTE */
    switch (wheelPos)
    {
        case WHEEL_FRONT_LEFT:
            retVal = Rte_Read_RawSensorData_FL_rawData(&rawData);
            break;
        case WHEEL_FRONT_RIGHT:
            retVal = Rte_Read_RawSensorData_FR_rawData(&rawData);
            break;
        case WHEEL_REAR_LEFT:
            retVal = Rte_Read_RawSensorData_RL_rawData(&rawData);
            break;
        case WHEEL_REAR_RIGHT:
            retVal = Rte_Read_RawSensorData_RR_rawData(&rawData);
            break;
        default:
            retVal = E_NOT_OK;
            break;
    }
    
    if (retVal == E_OK)
    {
        g_SpeedSensorData[wheelPos].rawData = rawData;
    }
    
    return retVal;
}

/**
 * @brief Calculate speed from raw sensor data
 */
static Std_ReturnType SpeedSensor_CalculateSpeed(WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_OK;
    SpeedSensorData_t* sensorData = &g_SpeedSensorData[wheelPos];
    float32 rawSpeed;
    float32 timeInSeconds;
    
    /* Calculate raw speed from pulse count and time interval */
    if ((sensorData->rawData.timeInterval > 0) && 
        (sensorData->calibration.pulsesPerRevolution > 0))
    {
        timeInSeconds = (float32)sensorData->rawData.timeInterval / 1000.0f;
        
        /* Calculate RPM first */
        float32 rpm = ((float32)sensorData->rawData.pulseCount / 
                      (float32)sensorData->calibration.pulsesPerRevolution) / timeInSeconds * 60.0f;
        
        /* Convert to linear speed in km/h */
        rawSpeed = rpm * sensorData->calibration.wheelCircumference * 60.0f / 1000.0f;
        
        /* Apply calibration correction */
        sensorData->speedData.wheelSpeedRaw = rawSpeed;
        sensorData->speedData.wheelSpeed = rawSpeed * sensorData->calibration.correctionFactor + 
                                          sensorData->calibration.offsetValue;
        
        /* Calculate acceleration (simple difference) */
        static float32 lastSpeed[WHEEL_MAX] = {0.0f};
        sensorData->speedData.accelerationX = (sensorData->speedData.wheelSpeed - lastSpeed[wheelPos]) / 
                                             (SPEED_SENSOR_SAMPLE_RATE_MS / 1000.0f);
        lastSpeed[wheelPos] = sensorData->speedData.wheelSpeed;
        
        /* Update diagnostics */
        sensorData->diagnostics.totalPulseCount += sensorData->rawData.pulseCount;
    }
    else
    {
        sensorData->speedData.wheelSpeed = 0.0f;
        sensorData->speedData.wheelSpeedRaw = 0.0f;
        sensorData->speedData.accelerationX = 0.0f;
    }
    
    return retVal;
}

/**
 * @brief Validate calculated speed data
 */
static Std_ReturnType SpeedSensor_ValidateSpeedData(WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_OK;
    SpeedSensorData_t* sensorData = &g_SpeedSensorData[wheelPos];
    boolean speedValid = TRUE;
    uint8 qualityFactor = 100;
    
    /* Check speed range */
    if ((sensorData->speedData.wheelSpeed < 0.0f) || 
        (sensorData->speedData.wheelSpeed > MAX_WHEEL_SPEED_KMH))
    {
        speedValid = FALSE;
        qualityFactor = 0;
    }
    
    /* Check sensor status */
    if (sensorData->rawData.status != SENSOR_STATUS_OK)
    {
        speedValid = FALSE;
        qualityFactor = 0;
    }
    
    /* Check calibration validity */
    if (sensorData->calibration.calibrationValid != TRUE)
    {
        speedValid = FALSE;
        qualityFactor = (qualityFactor > 50) ? 50 : qualityFactor;
    }
    
    /* Check for unreasonable acceleration */
    if ((sensorData->speedData.accelerationX > 20.0f) || 
        (sensorData->speedData.accelerationX < -20.0f))
    {
        qualityFactor = (qualityFactor > 30) ? 30 : qualityFactor;
    }
    
    sensorData->speedData.speedValid = speedValid;
    sensorData->speedData.qualityFactor = qualityFactor;
    
    return retVal;
}

/**
 * @brief Update diagnostic information
 */
static void SpeedSensor_UpdateDiagnostics(WheelPosition_t wheelPos)
{
    SpeedSensorData_t* sensorData = &g_SpeedSensorData[wheelPos];
    
    /* Update error count if sensor has issues */
    if ((sensorData->rawData.status != SENSOR_STATUS_OK) || 
        (sensorData->speedData.speedValid != TRUE))
    {
        sensorData->diagnostics.errorCount++;
        sensorData->diagnostics.lastErrorTimestamp = 0; /* Should be actual timestamp */
    }
    
    sensorData->diagnostics.lastStatus = sensorData->rawData.status;
}

/* RTE Runnable Functions */

/**
 * @brief RTE Runnable for cyclic speed sensor processing
 */
void RE_SpeedSensor_MainCyclic(void)
{
    SpeedSensor_MainFunction();
    
    /* Send speed data to other SWCs via RTE */
    SpeedData_t speedData;
    
    if (SpeedSensor_GetSpeedData(WHEEL_FRONT_LEFT, &speedData) == E_OK)
    {
        Rte_Write_SpeedData_FL_speedData(&speedData);
    }
    
    if (SpeedSensor_GetSpeedData(WHEEL_FRONT_RIGHT, &speedData) == E_OK)
    {
        Rte_Write_SpeedData_FR_speedData(&speedData);
    }
    
    if (SpeedSensor_GetSpeedData(WHEEL_REAR_LEFT, &speedData) == E_OK)
    {
        Rte_Write_SpeedData_RL_speedData(&speedData);
    }
    
    if (SpeedSensor_GetSpeedData(WHEEL_REAR_RIGHT, &speedData) == E_OK)
    {
        Rte_Write_SpeedData_RR_speedData(&speedData);
    }
}

/**
 * @brief RTE Runnable for speed sensor calibration
 */
void RE_SpeedSensor_Calibration(void)
{
    /* Calibration operations handled via client-server interface */
    /* This runnable can be triggered by diagnostic requests */
}

/**
 * @brief RTE Runnable for speed sensor diagnostics
 */
void RE_SpeedSensor_Diagnostics(void)
{
    /* Diagnostic operations handled via client-server interface */
    /* This runnable can be triggered by diagnostic requests */
}