/**
 * @file CalibrationManager.c
 * @brief Calibration Management Service Implementation
 * @author Generated for ABS Malfunction Detection System
 */

#include "CalibrationManager.h"
#include "SpeedSensor_Interface.h"
#include <string.h>
#include <math.h>

/* Local data structures */
static CalibrationSession_t g_CalibrationSessions[WHEEL_MAX];
static CalibrationConfig_t g_CalibrationConfig;
static CalibrationHistoryEntry_t g_CalibrationHistory[WHEEL_MAX][CALIBRATION_HISTORY_SIZE];
static uint8 g_HistoryCount[WHEEL_MAX];
static boolean g_CalibrationManager_Initialized = FALSE;

/* Calibration sample data for ongoing calibrations */
typedef struct {
    float32 speedSamples[CALIBRATION_MAX_SAMPLES];
    float32 referenceSamples[CALIBRATION_MAX_SAMPLES];
    uint16 sampleCount;
    uint32 lastSampleTime;
} CalibrationSampleData_t;

static CalibrationSampleData_t g_SampleData[WHEEL_MAX];

/* Internal function prototypes */
static void CalibrationManager_InitDefaultConfig(void);
static Std_ReturnType CalibrationManager_ProcessCalibrationSession(WheelPosition_t wheelPos);
static Std_ReturnType CalibrationManager_CollectSample(WheelPosition_t wheelPos);
static Std_ReturnType CalibrationManager_CalculateCalibration(WheelPosition_t wheelPos);
static Std_ReturnType CalibrationManager_ValidateCalculatedCalibration(WheelPosition_t wheelPos, boolean* isValid);
static void CalibrationManager_AddHistoryEntry(WheelPosition_t wheelPos, const CalibrationHistoryEntry_t* entry);
static uint16 CalibrationManager_GetNvmBlockId(WheelPosition_t wheelPos);
static void CalibrationManager_SetSessionResult(WheelPosition_t wheelPos, CalibrationResult_t result);

/**
 * @brief Initialize calibration manager
 */
Std_ReturnType CalibrationManager_Init(void)
{
    uint8 wheelIdx;
    
    if (g_CalibrationManager_Initialized == FALSE)
    {
        /* Initialize calibration sessions */
        for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
        {
            memset(&g_CalibrationSessions[wheelIdx], 0, sizeof(CalibrationSession_t));
            g_CalibrationSessions[wheelIdx].state = CALIBRATION_STATE_IDLE;
            g_CalibrationSessions[wheelIdx].result = CALIBRATION_RESULT_OK;
            g_CalibrationSessions[wheelIdx].sessionActive = FALSE;
            
            /* Initialize sample data */
            memset(&g_SampleData[wheelIdx], 0, sizeof(CalibrationSampleData_t));
            
            /* Initialize history */
            memset(g_CalibrationHistory[wheelIdx], 0, 
                   sizeof(CalibrationHistoryEntry_t) * CALIBRATION_HISTORY_SIZE);
            g_HistoryCount[wheelIdx] = 0;
        }
        
        /* Initialize default configuration */
        CalibrationManager_InitDefaultConfig();
        
        /* Load calibration data from NVM */
        for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
        {
            CalibrationManager_LoadFromNvm((WheelPosition_t)wheelIdx);
        }
        
        g_CalibrationManager_Initialized = TRUE;
    }
    
    return E_OK;
}

/**
 * @brief Deinitialize calibration manager
 */
Std_ReturnType CalibrationManager_DeInit(void)
{
    uint8 wheelIdx;
    
    if (g_CalibrationManager_Initialized == TRUE)
    {
        /* Cancel any ongoing calibrations */
        for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
        {
            if (g_CalibrationSessions[wheelIdx].sessionActive == TRUE)
            {
                CalibrationManager_CancelCalibration((WheelPosition_t)wheelIdx);
            }
        }
        
        g_CalibrationManager_Initialized = FALSE;
    }
    
    return E_OK;
}

/**
 * @brief Main function for calibration manager
 */
Std_ReturnType CalibrationManager_MainFunction(void)
{
    Std_ReturnType retVal = E_OK;
    uint8 wheelIdx;
    
    if (g_CalibrationManager_Initialized == TRUE)
    {
        /* Process active calibration sessions */
        for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
        {
            if (g_CalibrationSessions[wheelIdx].sessionActive == TRUE)
            {
                CalibrationManager_ProcessCalibrationSession((WheelPosition_t)wheelIdx);
            }
        }
        
        /* Check for automatic calibration if enabled */
        if (g_CalibrationConfig.enableAutoCalibration == TRUE)
        {
            CalibrationManager_AutoCalibrationCheck();
        }
    }
    else
    {
        retVal = E_NOT_OK;
    }
    
    return retVal;
}

/**
 * @brief Start calibration for specific wheel
 */
Std_ReturnType CalibrationManager_StartCalibration(const CalibrationRequest_t* request)
{
    Std_ReturnType retVal = E_NOT_OK;
    WheelPosition_t wheelPos;
    CalibrationSession_t* session;
    
    if ((request != NULL_PTR) && (g_CalibrationManager_Initialized == TRUE))
    {
        wheelPos = request->wheelPosition;
        
        if (wheelPos < WHEEL_MAX)
        {
            session = &g_CalibrationSessions[wheelPos];
            
            /* Check if calibration is already in progress */
            if (session->sessionActive == FALSE)
            {
                /* Initialize calibration session */
                session->request = *request;
                session->state = CALIBRATION_STATE_REQUESTED;
                session->result = CALIBRATION_RESULT_IN_PROGRESS;
                session->startTimestamp = 0; /* Should be actual timestamp */
                session->samplesCollected = 0;
                session->sessionActive = TRUE;
                
                /* Initialize sample data */
                memset(&g_SampleData[wheelPos], 0, sizeof(CalibrationSampleData_t));
                
                retVal = E_OK;
            }
            else
            {
                retVal = CALIBRATION_RESULT_IN_PROGRESS;
            }
        }
        else
        {
            retVal = CALIBRATION_RESULT_INVALID_PARAM;
        }
    }
    
    return retVal;
}

/**
 * @brief Cancel ongoing calibration
 */
Std_ReturnType CalibrationManager_CancelCalibration(WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_NOT_OK;
    CalibrationSession_t* session;
    
    if ((wheelPos < WHEEL_MAX) && (g_CalibrationManager_Initialized == TRUE))
    {
        session = &g_CalibrationSessions[wheelPos];
        
        if (session->sessionActive == TRUE)
        {
            session->state = CALIBRATION_STATE_CANCELLED;
            session->result = CALIBRATION_RESULT_NOT_OK;
            session->sessionActive = FALSE;
            session->endTimestamp = 0; /* Should be actual timestamp */
            
            retVal = E_OK;
        }
    }
    
    return retVal;
}

/**
 * @brief Get calibration session status
 */
Std_ReturnType CalibrationManager_GetSessionStatus(WheelPosition_t wheelPos, CalibrationSession_t* session)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((wheelPos < WHEEL_MAX) && (session != NULL_PTR) && (g_CalibrationManager_Initialized == TRUE))
    {
        *session = g_CalibrationSessions[wheelPos];
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Apply calculated calibration parameters
 */
Std_ReturnType CalibrationManager_ApplyCalibration(WheelPosition_t wheelPos, boolean saveToNvm)
{
    Std_ReturnType retVal = E_NOT_OK;
    CalibrationSession_t* session;
    SpeedSensorCalibration_t calibration;
    
    if ((wheelPos < WHEEL_MAX) && (g_CalibrationManager_Initialized == TRUE))
    {
        session = &g_CalibrationSessions[wheelPos];
        
        if ((session->state == CALIBRATION_STATE_COMPLETED) && 
            (session->result == CALIBRATION_RESULT_OK))
        {
            /* Get current calibration */
            if (SpeedSensor_GetCalibration(wheelPos, &calibration) == E_OK)
            {
                /* Apply calculated values */
                calibration.correctionFactor = session->calculatedCorrectionFactor;
                calibration.offsetValue = session->calculatedOffset;
                calibration.calibrationTimestamp = session->endTimestamp;
                calibration.calibrationValid = TRUE;
                
                /* Set calibration */
                if (SpeedSensor_SetCalibration(wheelPos, &calibration) == E_OK)
                {
                    /* Save to NVM if requested */
                    if (saveToNvm == TRUE)
                    {
                        retVal = CalibrationManager_SaveToNvm(wheelPos);
                    }
                    else
                    {
                        retVal = E_OK;
                    }
                    
                    /* Add history entry */
                    CalibrationHistoryEntry_t historyEntry;
                    historyEntry.timestamp = session->endTimestamp;
                    historyEntry.method = session->request.method;
                    historyEntry.result = session->result;
                    historyEntry.oldCorrectionFactor = 1.0f; /* Should store old value */
                    historyEntry.newCorrectionFactor = session->calculatedCorrectionFactor;
                    historyEntry.accuracy = session->measuredAccuracy;
                    
                    CalibrationManager_AddHistoryEntry(wheelPos, &historyEntry);
                }
            }
        }
    }
    
    return retVal;
}

/**
 * @brief Validate current calibration parameters
 */
Std_ReturnType CalibrationManager_ValidateCalibration(WheelPosition_t wheelPos, boolean* isValid, float32* accuracy)
{
    Std_ReturnType retVal = E_NOT_OK;
    SpeedSensorCalibration_t calibration;
    
    if ((wheelPos < WHEEL_MAX) && (isValid != NULL_PTR) && (accuracy != NULL_PTR) && 
        (g_CalibrationManager_Initialized == TRUE))
    {
        if (SpeedSensor_GetCalibration(wheelPos, &calibration) == E_OK)
        {
            /* Validate calibration parameters */
            *isValid = (calibration.correctionFactor >= g_CalibrationConfig.minCorrectionFactor) &&
                       (calibration.correctionFactor <= g_CalibrationConfig.maxCorrectionFactor) &&
                       (calibration.calibrationValid == TRUE);
            
            /* Calculate accuracy based on correction factor deviation from 1.0 */
            float32 deviation = fabsf(calibration.correctionFactor - 1.0f);
            *accuracy = (1.0f - deviation) * 100.0f;
            if (*accuracy < 0.0f) *accuracy = 0.0f;
            
            retVal = E_OK;
        }
    }
    
    return retVal;
}

/**
 * @brief Reset calibration to factory defaults
 */
Std_ReturnType CalibrationManager_ResetToFactory(WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_NOT_OK;
    SpeedSensorCalibration_t calibration;
    
    if ((wheelPos < WHEEL_MAX) && (g_CalibrationManager_Initialized == TRUE))
    {
        /* Set factory default calibration */
        memset(&calibration, 0, sizeof(SpeedSensorCalibration_t));
        calibration.correctionFactor = 1.0f;
        calibration.offsetValue = 0.0f;
        calibration.pulsesPerRevolution = 60;  /* Default for ABS sensor */
        calibration.wheelCircumference = 2.1f; /* Default wheel circumference */
        calibration.calibrationValid = TRUE;
        calibration.calibrationTimestamp = 0; /* Should be actual timestamp */
        
        /* Apply factory calibration */
        if (SpeedSensor_SetCalibration(wheelPos, &calibration) == E_OK)
        {
            /* Save to NVM */
            retVal = CalibrationManager_SaveToNvm(wheelPos);
            
            /* Add history entry */
            CalibrationHistoryEntry_t historyEntry;
            historyEntry.timestamp = calibration.calibrationTimestamp;
            historyEntry.method = CALIBRATION_METHOD_FACTORY_RESET;
            historyEntry.result = CALIBRATION_RESULT_OK;
            historyEntry.oldCorrectionFactor = 0.0f; /* Unknown old value */
            historyEntry.newCorrectionFactor = 1.0f;
            historyEntry.accuracy = 100.0f;
            
            CalibrationManager_AddHistoryEntry(wheelPos, &historyEntry);
        }
    }
    
    return retVal;
}

/**
 * @brief Load calibration from NVM
 */
Std_ReturnType CalibrationManager_LoadFromNvm(WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint16 nvmBlockId;
    SpeedSensorCalibration_t calibration;
    
    if ((wheelPos < WHEEL_MAX) && (g_CalibrationManager_Initialized == TRUE))
    {
        nvmBlockId = CalibrationManager_GetNvmBlockId(wheelPos);
        
        /* Read calibration data from NVM */
        if (Rte_Call_NvmService_ReadBlock(nvmBlockId, &calibration) == NVM_REQ_OK)
        {
            /* Validate loaded calibration */
            if ((calibration.correctionFactor >= g_CalibrationConfig.minCorrectionFactor) &&
                (calibration.correctionFactor <= g_CalibrationConfig.maxCorrectionFactor) &&
                (calibration.calibrationValid == TRUE))
            {
                /* Apply loaded calibration */
                retVal = SpeedSensor_SetCalibration(wheelPos, &calibration);
            }
            else
            {
                /* Invalid calibration - reset to factory defaults */
                retVal = CalibrationManager_ResetToFactory(wheelPos);
            }
        }
        else
        {
            /* NVM read failed - set DTC and use factory defaults */
            Rte_Call_DiagnosticService_SetDTC(DTC_CALIBRATION_NVM_ERROR, TRUE);
            retVal = CalibrationManager_ResetToFactory(wheelPos);
        }
    }
    
    return retVal;
}

/**
 * @brief Save calibration to NVM
 */
Std_ReturnType CalibrationManager_SaveToNvm(WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint16 nvmBlockId;
    SpeedSensorCalibration_t calibration;
    
    if ((wheelPos < WHEEL_MAX) && (g_CalibrationManager_Initialized == TRUE))
    {
        nvmBlockId = CalibrationManager_GetNvmBlockId(wheelPos);
        
        /* Get current calibration */
        if (SpeedSensor_GetCalibration(wheelPos, &calibration) == E_OK)
        {
            /* Save calibration data to NVM */
            if (Rte_Call_NvmService_WriteBlock(nvmBlockId, &calibration) == NVM_REQ_OK)
            {
                retVal = E_OK;
            }
            else
            {
                /* NVM write failed - set DTC */
                Rte_Call_DiagnosticService_SetDTC(DTC_CALIBRATION_NVM_ERROR, TRUE);
                retVal = E_NOT_OK;
            }
        }
    }
    
    return retVal;
}

/**
 * @brief Get calibration history
 */
Std_ReturnType CalibrationManager_GetHistory(WheelPosition_t wheelPos, CalibrationHistoryEntry_t* history, uint8* count)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint8 i;
    
    if ((wheelPos < WHEEL_MAX) && (history != NULL_PTR) && (count != NULL_PTR) && 
        (g_CalibrationManager_Initialized == TRUE))
    {
        *count = g_HistoryCount[wheelPos];
        
        /* Copy history entries */
        for (i = 0; i < g_HistoryCount[wheelPos]; i++)
        {
            history[i] = g_CalibrationHistory[wheelPos][i];
        }
        
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Clear calibration history
 */
Std_ReturnType CalibrationManager_ClearHistory(WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((wheelPos < WHEEL_MAX) && (g_CalibrationManager_Initialized == TRUE))
    {
        memset(g_CalibrationHistory[wheelPos], 0, 
               sizeof(CalibrationHistoryEntry_t) * CALIBRATION_HISTORY_SIZE);
        g_HistoryCount[wheelPos] = 0;
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Set calibration configuration
 */
Std_ReturnType CalibrationManager_SetConfig(const CalibrationConfig_t* config)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((config != NULL_PTR) && (g_CalibrationManager_Initialized == TRUE))
    {
        g_CalibrationConfig = *config;
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Get calibration configuration
 */
Std_ReturnType CalibrationManager_GetConfig(CalibrationConfig_t* config)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((config != NULL_PTR) && (g_CalibrationManager_Initialized == TRUE))
    {
        *config = g_CalibrationConfig;
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Perform automatic calibration check
 */
Std_ReturnType CalibrationManager_AutoCalibrationCheck(void)
{
    Std_ReturnType retVal = E_OK;
    uint8 wheelIdx;
    boolean isValid;
    float32 accuracy;
    static uint32 lastAutoCheckTime = 0;
    uint32 currentTime = 0; /* Should be actual timestamp */
    
    /* Check if it's time for automatic calibration check */
    if ((currentTime - lastAutoCheckTime) >= CALIBRATION_AUTO_INTERVAL_MS)
    {
        lastAutoCheckTime = currentTime;
        
        /* Check calibration validity for all wheels */
        for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
        {
            if (CalibrationManager_ValidateCalibration((WheelPosition_t)wheelIdx, &isValid, &accuracy) == E_OK)
            {
                /* If calibration is invalid or accuracy is low, trigger automatic calibration */
                if ((isValid == FALSE) || (accuracy < 90.0f))
                {
                    CalibrationRequest_t request;
                    request.wheelPosition = (WheelPosition_t)wheelIdx;
                    request.method = CALIBRATION_METHOD_AUTOMATIC;
                    request.referenceSpeed = 50.0f; /* 50 km/h reference */
                    request.tolerancePercentage = g_CalibrationConfig.defaultTolerance;
                    request.calibrationTimeMs = 10000; /* 10 seconds */
                    request.forceCalibration = FALSE;
                    
                    CalibrationManager_StartCalibration(&request);
                }
            }
        }
    }
    
    return retVal;
}

/* Internal Functions */

/**
 * @brief Initialize default configuration
 */
static void CalibrationManager_InitDefaultConfig(void)
{
    g_CalibrationConfig.maxCalibrationSamples = CALIBRATION_MAX_SAMPLES;
    g_CalibrationConfig.minCalibrationSamples = CALIBRATION_MIN_SAMPLES;
    g_CalibrationConfig.maxCorrectionFactor = 1.5f;
    g_CalibrationConfig.minCorrectionFactor = 0.5f;
    g_CalibrationConfig.defaultTolerance = 2.0f;
    g_CalibrationConfig.calibrationTimeoutMs = CALIBRATION_TIMEOUT_MS;
    g_CalibrationConfig.enableAutoCalibration = TRUE;
    g_CalibrationConfig.autoCalibrationIntervalHours = 24; /* Daily check */
}

/**
 * @brief Process calibration session
 */
static Std_ReturnType CalibrationManager_ProcessCalibrationSession(WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_OK;
    CalibrationSession_t* session = &g_CalibrationSessions[wheelPos];
    uint32 currentTime = 0; /* Should be actual timestamp */
    
    switch (session->state)
    {
        case CALIBRATION_STATE_REQUESTED:
            session->state = CALIBRATION_STATE_IN_PROGRESS;
            break;
            
        case CALIBRATION_STATE_IN_PROGRESS:
            /* Collect calibration samples */
            if (CalibrationManager_CollectSample(wheelPos) == E_OK)
            {
                session->samplesCollected = g_SampleData[wheelPos].sampleCount;
                
                /* Check if enough samples collected */
                if (session->samplesCollected >= g_CalibrationConfig.minCalibrationSamples)
                {
                    /* Check timeout */
                    if ((currentTime - session->startTimestamp) >= session->request.calibrationTimeMs)
                    {
                        /* Calculate calibration */
                        if (CalibrationManager_CalculateCalibration(wheelPos) == E_OK)
                        {
                            session->state = CALIBRATION_STATE_COMPLETED;
                            CalibrationManager_SetSessionResult(wheelPos, CALIBRATION_RESULT_OK);
                        }
                        else
                        {
                            session->state = CALIBRATION_STATE_FAILED;
                            CalibrationManager_SetSessionResult(wheelPos, CALIBRATION_RESULT_VALIDATION_FAILED);
                        }
                    }
                }
            }
            
            /* Check for timeout */
            if ((currentTime - session->startTimestamp) >= g_CalibrationConfig.calibrationTimeoutMs)
            {
                session->state = CALIBRATION_STATE_FAILED;
                CalibrationManager_SetSessionResult(wheelPos, CALIBRATION_RESULT_NOT_OK);
            }
            break;
            
        case CALIBRATION_STATE_COMPLETED:
        case CALIBRATION_STATE_FAILED:
        case CALIBRATION_STATE_CANCELLED:
            /* End session */
            session->sessionActive = FALSE;
            session->endTimestamp = currentTime;
            break;
            
        default:
            break;
    }
    
    return retVal;
}

/**
 * @brief Collect calibration sample
 */
static Std_ReturnType CalibrationManager_CollectSample(WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_NOT_OK;
    SpeedData_t speedData;
    CalibrationSampleData_t* sampleData = &g_SampleData[wheelPos];
    CalibrationSession_t* session = &g_CalibrationSessions[wheelPos];
    
    /* Get current speed data */
    if (SpeedSensor_GetSpeedData(wheelPos, &speedData) == E_OK)
    {
        if ((speedData.speedValid == TRUE) && 
            (sampleData->sampleCount < g_CalibrationConfig.maxCalibrationSamples))
        {
            /* Store speed sample */
            sampleData->speedSamples[sampleData->sampleCount] = speedData.wheelSpeed;
            sampleData->referenceSamples[sampleData->sampleCount] = session->request.referenceSpeed;
            sampleData->sampleCount++;
            
            retVal = E_OK;
        }
    }
    
    return retVal;
}

/**
 * @brief Calculate calibration parameters
 */
static Std_ReturnType CalibrationManager_CalculateCalibration(WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_NOT_OK;
    CalibrationSampleData_t* sampleData = &g_SampleData[wheelPos];
    CalibrationSession_t* session = &g_CalibrationSessions[wheelPos];
    float32 sumSpeed = 0.0f;
    float32 sumReference = 0.0f;
    uint16 validSamples = 0;
    uint16 i;
    boolean isValid;
    
    /* Calculate average speeds */
    for (i = 0; i < sampleData->sampleCount; i++)
    {
        if ((sampleData->speedSamples[i] > 0.0f) && 
            (sampleData->referenceSamples[i] > 0.0f))
        {
            sumSpeed += sampleData->speedSamples[i];
            sumReference += sampleData->referenceSamples[i];
            validSamples++;
        }
    }
    
    if (validSamples >= g_CalibrationConfig.minCalibrationSamples)
    {
        float32 avgSpeed = sumSpeed / validSamples;
        float32 avgReference = sumReference / validSamples;
        
        /* Calculate correction factor */
        session->calculatedCorrectionFactor = avgReference / avgSpeed;
        session->calculatedOffset = 0.0f; /* Simple calibration - no offset */
        
        /* Calculate accuracy */
        float32 error = fabsf(avgSpeed - avgReference) / avgReference * 100.0f;
        session->measuredAccuracy = 100.0f - error;
        
        /* Validate calculated calibration */
        if (CalibrationManager_ValidateCalculatedCalibration(wheelPos, &isValid) == E_OK)
        {
            if (isValid == TRUE)
            {
                retVal = E_OK;
            }
        }
    }
    
    return retVal;
}

/**
 * @brief Validate calculated calibration
 */
static Std_ReturnType CalibrationManager_ValidateCalculatedCalibration(WheelPosition_t wheelPos, boolean* isValid)
{
    Std_ReturnType retVal = E_OK;
    CalibrationSession_t* session = &g_CalibrationSessions[wheelPos];
    
    *isValid = FALSE;
    
    /* Check if correction factor is within acceptable range */
    if ((session->calculatedCorrectionFactor >= g_CalibrationConfig.minCorrectionFactor) &&
        (session->calculatedCorrectionFactor <= g_CalibrationConfig.maxCorrectionFactor))
    {
        /* Check accuracy */
        if (session->measuredAccuracy >= (100.0f - session->request.tolerancePercentage))
        {
            *isValid = TRUE;
        }
    }
    
    return retVal;
}

/**
 * @brief Add entry to calibration history
 */
static void CalibrationManager_AddHistoryEntry(WheelPosition_t wheelPos, const CalibrationHistoryEntry_t* entry)
{
    uint8 insertIndex;
    
    if (g_HistoryCount[wheelPos] < CALIBRATION_HISTORY_SIZE)
    {
        /* Add to end of array */
        insertIndex = g_HistoryCount[wheelPos];
        g_HistoryCount[wheelPos]++;
    }
    else
    {
        /* Shift entries and add to end */
        for (uint8 i = 0; i < (CALIBRATION_HISTORY_SIZE - 1); i++)
        {
            g_CalibrationHistory[wheelPos][i] = g_CalibrationHistory[wheelPos][i + 1];
        }
        insertIndex = CALIBRATION_HISTORY_SIZE - 1;
    }
    
    g_CalibrationHistory[wheelPos][insertIndex] = *entry;
}

/**
 * @brief Get NVM block ID for wheel position
 */
static uint16 CalibrationManager_GetNvmBlockId(WheelPosition_t wheelPos)
{
    uint16 blockId = 0;
    
    switch (wheelPos)
    {
        case WHEEL_FRONT_LEFT:
            blockId = NVM_BLOCK_CALIBRATION_FL;
            break;
        case WHEEL_FRONT_RIGHT:
            blockId = NVM_BLOCK_CALIBRATION_FR;
            break;
        case WHEEL_REAR_LEFT:
            blockId = NVM_BLOCK_CALIBRATION_RL;
            break;
        case WHEEL_REAR_RIGHT:
            blockId = NVM_BLOCK_CALIBRATION_RR;
            break;
        default:
            blockId = 0;
            break;
    }
    
    return blockId;
}

/**
 * @brief Set calibration session result
 */
static void CalibrationManager_SetSessionResult(WheelPosition_t wheelPos, CalibrationResult_t result)
{
    g_CalibrationSessions[wheelPos].result = result;
    
    /* Set DTC if calibration failed */
    if (result != CALIBRATION_RESULT_OK)
    {
        if (result == CALIBRATION_RESULT_OUT_OF_RANGE)
        {
            Rte_Call_DiagnosticService_SetDTC(DTC_CALIBRATION_OUT_OF_RANGE, TRUE);
        }
        else
        {
            Rte_Call_DiagnosticService_SetDTC(DTC_CALIBRATION_FAILED, TRUE);
        }
    }
}

/* RTE Runnable Functions */

/**
 * @brief RTE Runnable for main calibration processing
 */
void RE_CalibrationManager_MainCyclic(void)
{
    CalibrationManager_MainFunction();
}

/**
 * @brief RTE Runnable for calibration process
 */
void RE_CalibrationManager_CalibrationProcess(void)
{
    /* This can be triggered by external requests */
}

/**
 * @brief RTE Runnable for NVM management
 */
void RE_CalibrationManager_NvmManager(void)
{
    /* This can be triggered for NVM save/load operations */
}