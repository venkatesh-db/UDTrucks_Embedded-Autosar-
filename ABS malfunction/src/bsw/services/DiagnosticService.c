/**
 * @file DiagnosticService.c
 * @brief UDS Diagnostic Services Implementation
 * @author Generated for ABS Malfunction Detection System
 */

#include "DiagnosticService.h"
#include "SpeedSensor_Interface.h"
#include "ABS_MalfunctionDetection.h"
#include "CalibrationManager.h"
#include <string.h>

/* Local data structures */
static DTCInfo_t g_DTCTable[DIAG_MAX_DTC_COUNT];
static uint8 g_ActiveDTCCount = 0;
static DiagSession_t g_CurrentSession = DIAG_SESSION_DEFAULT;
static boolean g_DiagnosticService_Initialized = FALSE;

/* UDS Service Handler Function Pointer Type */
typedef Std_ReturnType (*UDSServiceHandler_t)(const UDSMessage_t* request, UDSMessage_t* response);

/* UDS Service Handler Table */
typedef struct {
    uint8 serviceId;
    UDSServiceHandler_t handler;
} UDSServiceEntry_t;

/* Service handler table */
static const UDSServiceEntry_t g_UDSServiceTable[] = {
    {UDS_SID_DIAGNOSTIC_SESSION_CONTROL, UDS_DiagnosticSessionControl},
    {UDS_SID_ECU_RESET, UDS_ECUReset},
    {UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION, UDS_ClearDiagnosticInformation},
    {UDS_SID_READ_DTC_INFORMATION, UDS_ReadDTCInformation},
    {UDS_SID_READ_DATA_BY_IDENTIFIER, UDS_ReadDataByIdentifier},
    {UDS_SID_WRITE_DATA_BY_IDENTIFIER, UDS_WriteDataByIdentifier},
    {UDS_SID_IO_CONTROL_BY_IDENTIFIER, UDS_InputOutputControlByIdentifier},
    {UDS_SID_ROUTINE_CONTROL, UDS_RoutineControl}
};

#define UDS_SERVICE_TABLE_SIZE (sizeof(g_UDSServiceTable) / sizeof(UDSServiceEntry_t))

/* Internal function prototypes */
static Std_ReturnType DiagnosticService_FindDTC(uint32 dtcNumber, uint8* index);
static Std_ReturnType DiagnosticService_AddDTC(uint32 dtcNumber, WheelPosition_t wheelPos, ABS_MalfunctionType_t type);
static Std_ReturnType DiagnosticService_UpdateDTCStatus(uint32 dtcNumber, boolean active);
static UDSServiceHandler_t DiagnosticService_GetServiceHandler(uint8 serviceId);
static void DiagnosticService_PrepareErrorResponse(UDSMessage_t* response, uint8 serviceId, uint8 nrc);
static uint32 DiagnosticService_GetDTCForMalfunction(ABS_MalfunctionType_t type, WheelPosition_t wheelPos);
static void DiagnosticService_MonitorMalfunctions(void);

/**
 * @brief Initialize diagnostic service
 */
Std_ReturnType DiagnosticService_Init(void)
{
    uint8 i;
    
    if (g_DiagnosticService_Initialized == FALSE)
    {
        /* Initialize DTC table */
        for (i = 0; i < DIAG_MAX_DTC_COUNT; i++)
        {
            memset(&g_DTCTable[i], 0, sizeof(DTCInfo_t));
        }
        
        g_ActiveDTCCount = 0;
        g_CurrentSession = DIAG_SESSION_DEFAULT;
        g_DiagnosticService_Initialized = TRUE;
    }
    
    return E_OK;
}

/**
 * @brief Deinitialize diagnostic service
 */
Std_ReturnType DiagnosticService_DeInit(void)
{
    g_DiagnosticService_Initialized = FALSE;
    return E_OK;
}

/**
 * @brief Main processing function for diagnostic service
 */
Std_ReturnType DiagnosticService_MainFunction(void)
{
    if (g_DiagnosticService_Initialized == TRUE)
    {
        /* Monitor ABS malfunctions and update DTCs */
        DiagnosticService_MonitorMalfunctions();
    }
    
    return E_OK;
}

/**
 * @brief Process UDS request
 */
Std_ReturnType DiagnosticService_ProcessUDSRequest(const UDSMessage_t* request, UDSMessage_t* response)
{
    Std_ReturnType retVal = E_NOT_OK;
    UDSServiceHandler_t handler;
    
    if ((request != NULL_PTR) && (response != NULL_PTR) && (g_DiagnosticService_Initialized == TRUE))
    {
        /* Find service handler */
        handler = DiagnosticService_GetServiceHandler(request->serviceId);
        
        if (handler != NULL_PTR)
        {
            /* Call service handler */
            retVal = handler(request, response);
        }
        else
        {
            /* Service not supported */
            DiagnosticService_PrepareErrorResponse(response, request->serviceId, UDS_NRC_SERVICE_NOT_SUPPORTED);
            retVal = E_OK; /* Response prepared, even if error */
        }
    }
    
    return retVal;
}

/**
 * @brief Set DTC status
 */
Std_ReturnType DiagnosticService_SetDTC(uint32 dtcNumber, boolean active, WheelPosition_t wheelPos)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint8 dtcIndex;
    
    if (g_DiagnosticService_Initialized == TRUE)
    {
        /* Find existing DTC */
        if (DiagnosticService_FindDTC(dtcNumber, &dtcIndex) == E_OK)
        {
            /* Update existing DTC */
            retVal = DiagnosticService_UpdateDTCStatus(dtcNumber, active);
        }
        else if (active == TRUE)
        {
            /* Add new DTC */
            retVal = DiagnosticService_AddDTC(dtcNumber, wheelPos, ABS_MALFUNCTION_NONE);
        }
    }
    
    return retVal;
}

/**
 * @brief Clear DTC
 */
Std_ReturnType DiagnosticService_ClearDTC(uint32 dtcNumber)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint8 dtcIndex;
    
    if (g_DiagnosticService_Initialized == TRUE)
    {
        if (DiagnosticService_FindDTC(dtcNumber, &dtcIndex) == E_OK)
        {
            /* Clear DTC status */
            memset(&g_DTCTable[dtcIndex].status, 0, sizeof(DTCStatus_t));
            g_DTCTable[dtcIndex].status.testNotCompletedSinceLastClear = 1;
            retVal = E_OK;
        }
    }
    
    return retVal;
}

/**
 * @brief Clear all DTCs
 */
Std_ReturnType DiagnosticService_ClearAllDTCs(void)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint8 i;
    
    if (g_DiagnosticService_Initialized == TRUE)
    {
        /* Clear all DTCs */
        for (i = 0; i < DIAG_MAX_DTC_COUNT; i++)
        {
            if (g_DTCTable[i].dtcNumber != 0)
            {
                memset(&g_DTCTable[i].status, 0, sizeof(DTCStatus_t));
                g_DTCTable[i].status.testNotCompletedSinceLastClear = 1;
            }
        }
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Get DTC information
 */
Std_ReturnType DiagnosticService_GetDTCInfo(uint32 dtcNumber, DTCInfo_t* dtcInfo)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint8 dtcIndex;
    
    if ((dtcInfo != NULL_PTR) && (g_DiagnosticService_Initialized == TRUE))
    {
        if (DiagnosticService_FindDTC(dtcNumber, &dtcIndex) == E_OK)
        {
            *dtcInfo = g_DTCTable[dtcIndex];
            retVal = E_OK;
        }
    }
    
    return retVal;
}

/**
 * @brief Get all active DTCs
 */
Std_ReturnType DiagnosticService_GetActiveDTCs(uint32* dtcList, uint8* dtcCount, uint8 maxDtcs)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint8 i, count = 0;
    
    if ((dtcList != NULL_PTR) && (dtcCount != NULL_PTR) && (g_DiagnosticService_Initialized == TRUE))
    {
        /* Collect active DTCs */
        for (i = 0; (i < DIAG_MAX_DTC_COUNT) && (count < maxDtcs); i++)
        {
            if ((g_DTCTable[i].dtcNumber != 0) && 
                ((g_DTCTable[i].status.testFailed) || (g_DTCTable[i].status.confirmedDTC)))
            {
                dtcList[count] = g_DTCTable[i].dtcNumber;
                count++;
            }
        }
        
        *dtcCount = count;
        retVal = E_OK;
    }
    
    return retVal;
}

/* UDS Service Handlers */

/**
 * @brief UDS Diagnostic Session Control (0x10)
 */
Std_ReturnType UDS_DiagnosticSessionControl(const UDSMessage_t* request, UDSMessage_t* response)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((request->requestDataLength >= 1) && (response->maxResponseLength >= 6))
    {
        DiagSession_t requestedSession = (DiagSession_t)request->requestData[0];
        
        /* Validate session type */
        if ((requestedSession >= DIAG_SESSION_DEFAULT) && (requestedSession <= DIAG_SESSION_SAFETY_SYSTEM))
        {
            g_CurrentSession = requestedSession;
            
            /* Prepare positive response */
            response->responseData[0] = UDS_SID_DIAGNOSTIC_SESSION_CONTROL + 0x40; /* Positive response */
            response->responseData[1] = requestedSession;
            response->responseData[2] = 0x00; /* P2 Server Max timing high byte */
            response->responseData[3] = 0x32; /* P2 Server Max timing low byte (50ms) */
            response->responseData[4] = 0x01; /* P2* Server Max timing high byte */
            response->responseData[5] = 0xF4; /* P2* Server Max timing low byte (500ms) */
            response->responseDataLength = 6;
            
            retVal = E_OK;
        }
        else
        {
            DiagnosticService_PrepareErrorResponse(response, UDS_SID_DIAGNOSTIC_SESSION_CONTROL, UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
            retVal = E_OK;
        }
    }
    else
    {
        DiagnosticService_PrepareErrorResponse(response, UDS_SID_DIAGNOSTIC_SESSION_CONTROL, UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief UDS ECU Reset (0x11)
 */
Std_ReturnType UDS_ECUReset(const UDSMessage_t* request, UDSMessage_t* response)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((request->requestDataLength >= 1) && (response->maxResponseLength >= 2))
    {
        uint8 resetType = request->requestData[0];
        
        /* Only allow hard reset in programming session */
        if ((resetType == 0x01) && (g_CurrentSession == DIAG_SESSION_PROGRAMMING))
        {
            /* Prepare positive response */
            response->responseData[0] = UDS_SID_ECU_RESET + 0x40;
            response->responseData[1] = resetType;
            response->responseDataLength = 2;
            
            /* Note: Actual reset would be performed after sending response */
            retVal = E_OK;
        }
        else
        {
            DiagnosticService_PrepareErrorResponse(response, UDS_SID_ECU_RESET, UDS_NRC_CONDITIONS_NOT_CORRECT);
            retVal = E_OK;
        }
    }
    else
    {
        DiagnosticService_PrepareErrorResponse(response, UDS_SID_ECU_RESET, UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief UDS Clear Diagnostic Information (0x14)
 */
Std_ReturnType UDS_ClearDiagnosticInformation(const UDSMessage_t* request, UDSMessage_t* response)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((request->requestDataLength >= 3) && (response->maxResponseLength >= 1))
    {
        uint32 dtcGroupToDelete = ((uint32)request->requestData[0] << 16) |
                                 ((uint32)request->requestData[1] << 8) |
                                 (uint32)request->requestData[2];
        
        /* Clear all DTCs if group is 0xFFFFFF */
        if (dtcGroupToDelete == 0xFFFFFFU)
        {
            if (DiagnosticService_ClearAllDTCs() == E_OK)
            {
                /* Clear malfunction status in ABS system */
                ABS_ClearMalfunctionStatus(WHEEL_FRONT_LEFT);
                ABS_ClearMalfunctionStatus(WHEEL_FRONT_RIGHT);
                ABS_ClearMalfunctionStatus(WHEEL_REAR_LEFT);
                ABS_ClearMalfunctionStatus(WHEEL_REAR_RIGHT);
                
                /* Prepare positive response */
                response->responseData[0] = UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION + 0x40;
                response->responseDataLength = 1;
                retVal = E_OK;
            }
        }
        else
        {
            /* Clear specific DTC */
            if (DiagnosticService_ClearDTC(dtcGroupToDelete) == E_OK)
            {
                response->responseData[0] = UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION + 0x40;
                response->responseDataLength = 1;
                retVal = E_OK;
            }
        }
        
        if (retVal != E_OK)
        {
            DiagnosticService_PrepareErrorResponse(response, UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION, UDS_NRC_REQUEST_OUT_OF_RANGE);
            retVal = E_OK;
        }
    }
    else
    {
        DiagnosticService_PrepareErrorResponse(response, UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION, UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief UDS Read DTC Information (0x19)
 */
Std_ReturnType UDS_ReadDTCInformation(const UDSMessage_t* request, UDSMessage_t* response)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((request->requestDataLength >= 1) && (response->maxResponseLength >= 3))
    {
        uint8 subFunction = request->requestData[0];
        
        response->responseData[0] = UDS_SID_READ_DTC_INFORMATION + 0x40;
        response->responseData[1] = subFunction;
        response->responseDataLength = 2;
        
        switch (subFunction)
        {
            case 0x02: /* reportDTCByStatusMask */
                if (request->requestDataLength >= 2)
                {
                    uint8 statusMask = request->requestData[1];
                    uint8 dtcCount = 0;
                    uint16 responseIndex = 3;
                    
                    response->responseData[2] = statusMask; /* Status availability mask */
                    
                    /* Add DTCs matching status mask */
                    for (uint8 i = 0; (i < DIAG_MAX_DTC_COUNT) && (responseIndex < (response->maxResponseLength - 4)); i++)
                    {
                        if (g_DTCTable[i].dtcNumber != 0)
                        {
                            uint8 dtcStatus = *(uint8*)&g_DTCTable[i].status;
                            
                            if (dtcStatus & statusMask)
                            {
                                /* Add DTC to response */
                                response->responseData[responseIndex++] = (uint8)(g_DTCTable[i].dtcNumber >> 16);
                                response->responseData[responseIndex++] = (uint8)(g_DTCTable[i].dtcNumber >> 8);
                                response->responseData[responseIndex++] = (uint8)(g_DTCTable[i].dtcNumber);
                                response->responseData[responseIndex++] = dtcStatus;
                                dtcCount++;
                            }
                        }
                    }
                    
                    response->responseDataLength = responseIndex;
                    retVal = E_OK;
                }
                break;
                
            case 0x0A: /* reportSupportedDTC */
                {
                    uint16 responseIndex = 2;
                    
                    /* Add all supported DTCs */
                    for (uint8 i = 0; (i < DIAG_MAX_DTC_COUNT) && (responseIndex < (response->maxResponseLength - 3)); i++)
                    {
                        if (g_DTCTable[i].dtcNumber != 0)
                        {
                            response->responseData[responseIndex++] = (uint8)(g_DTCTable[i].dtcNumber >> 16);
                            response->responseData[responseIndex++] = (uint8)(g_DTCTable[i].dtcNumber >> 8);
                            response->responseData[responseIndex++] = (uint8)(g_DTCTable[i].dtcNumber);
                        }
                    }
                    
                    response->responseDataLength = responseIndex;
                    retVal = E_OK;
                }
                break;
                
            default:
                DiagnosticService_PrepareErrorResponse(response, UDS_SID_READ_DTC_INFORMATION, UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
                retVal = E_OK;
                break;
        }
    }
    else
    {
        DiagnosticService_PrepareErrorResponse(response, UDS_SID_READ_DTC_INFORMATION, UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief UDS Read Data By Identifier (0x22)
 */
Std_ReturnType UDS_ReadDataByIdentifier(const UDSMessage_t* request, UDSMessage_t* response)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((request->requestDataLength >= 2) && (response->maxResponseLength >= 3))
    {
        uint16 dataId = ((uint16)request->requestData[0] << 8) | request->requestData[1];
        uint16 dataLength = 0;
        
        response->responseData[0] = UDS_SID_READ_DATA_BY_IDENTIFIER + 0x40;
        response->responseData[1] = request->requestData[0]; /* DID high byte */
        response->responseData[2] = request->requestData[1]; /* DID low byte */
        
        /* Handle different DIDs */
        if ((dataId >= DID_SPEED_SENSOR_FL_DATA) && (dataId <= DID_SPEED_SENSOR_RR_DATA))
        {
            retVal = DID_ReadSpeedSensorData(dataId, &response->responseData[3], &dataLength);
        }
        else if ((dataId >= DID_CALIBRATION_FL_PARAMS) && (dataId <= DID_CALIBRATION_RR_PARAMS))
        {
            retVal = DID_ReadCalibrationParams(dataId, &response->responseData[3], &dataLength);
        }
        else if (dataId == DID_ABS_SYSTEM_STATUS)
        {
            retVal = DID_ReadABSSystemStatus(&response->responseData[3], &dataLength);
        }
        else if (dataId == DID_MALFUNCTION_COUNTER)
        {
            retVal = DID_ReadMalfunctionCounter(&response->responseData[3], &dataLength);
        }
        else
        {
            DiagnosticService_PrepareErrorResponse(response, UDS_SID_READ_DATA_BY_IDENTIFIER, UDS_NRC_REQUEST_OUT_OF_RANGE);
            retVal = E_OK;
        }
        
        if (retVal == E_OK)
        {
            response->responseDataLength = 3 + dataLength;
        }
    }
    else
    {
        DiagnosticService_PrepareErrorResponse(response, UDS_SID_READ_DATA_BY_IDENTIFIER, UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief UDS Write Data By Identifier (0x2E)
 */
Std_ReturnType UDS_WriteDataByIdentifier(const UDSMessage_t* request, UDSMessage_t* response)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((request->requestDataLength >= 3) && (response->maxResponseLength >= 3))
    {
        uint16 dataId = ((uint16)request->requestData[0] << 8) | request->requestData[1];
        
        /* Only allow write in extended diagnostic session */
        if (g_CurrentSession == DIAG_SESSION_EXTENDED)
        {
            if ((dataId >= DID_CALIBRATION_FL_PARAMS) && (dataId <= DID_CALIBRATION_RR_PARAMS))
            {
                retVal = DID_WriteCalibrationParams(dataId, &request->requestData[2], request->requestDataLength - 2);
                
                if (retVal == E_OK)
                {
                    response->responseData[0] = UDS_SID_WRITE_DATA_BY_IDENTIFIER + 0x40;
                    response->responseData[1] = request->requestData[0];
                    response->responseData[2] = request->requestData[1];
                    response->responseDataLength = 3;
                }
            }
            else
            {
                DiagnosticService_PrepareErrorResponse(response, UDS_SID_WRITE_DATA_BY_IDENTIFIER, UDS_NRC_REQUEST_OUT_OF_RANGE);
                retVal = E_OK;
            }
        }
        else
        {
            DiagnosticService_PrepareErrorResponse(response, UDS_SID_WRITE_DATA_BY_IDENTIFIER, UDS_NRC_CONDITIONS_NOT_CORRECT);
            retVal = E_OK;
        }
    }
    else
    {
        DiagnosticService_PrepareErrorResponse(response, UDS_SID_WRITE_DATA_BY_IDENTIFIER, UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief UDS Input Output Control By Identifier (0x2F)
 */
Std_ReturnType UDS_InputOutputControlByIdentifier(const UDSMessage_t* request, UDSMessage_t* response)
{
    /* Not implemented for this application */
    DiagnosticService_PrepareErrorResponse(response, UDS_SID_IO_CONTROL_BY_IDENTIFIER, UDS_NRC_SERVICE_NOT_SUPPORTED);
    return E_OK;
}

/**
 * @brief UDS Routine Control (0x31)
 */
Std_ReturnType UDS_RoutineControl(const UDSMessage_t* request, UDSMessage_t* response)
{
    Std_ReturnType retVal = E_NOT_OK;
    
    if ((request->requestDataLength >= 4) && (response->maxResponseLength >= 4))
    {
        uint8 subFunction = request->requestData[0];
        uint16 routineId = ((uint16)request->requestData[1] << 8) | request->requestData[2];
        
        if (g_CurrentSession == DIAG_SESSION_EXTENDED)
        {
            response->responseData[0] = UDS_SID_ROUTINE_CONTROL + 0x40;
            response->responseData[1] = subFunction;
            response->responseData[2] = request->requestData[1];
            response->responseData[3] = request->requestData[2];
            response->responseDataLength = 4;
            
            if (subFunction == 0x01) /* Start routine */
            {
                if ((routineId >= RID_START_CALIBRATION_FL) && (routineId <= RID_START_CALIBRATION_RR))
                {
                    uint16 responseLength = 0;
                    retVal = RID_StartCalibration(routineId, &request->requestData[3], 
                                                request->requestDataLength - 3, 
                                                &response->responseData[4], &responseLength);
                    response->responseDataLength += responseLength;
                }
                else if (routineId == RID_VALIDATE_CALIBRATION)
                {
                    uint16 responseLength = 0;
                    retVal = RID_ValidateCalibration(&request->requestData[3], request->requestDataLength - 3,
                                                   &response->responseData[4], &responseLength);
                    response->responseDataLength += responseLength;
                }
                else if (routineId == RID_RESET_CALIBRATION_ALL)
                {
                    uint16 responseLength = 0;
                    retVal = RID_ResetCalibrationAll(&response->responseData[4], &responseLength);
                    response->responseDataLength += responseLength;
                }
                else if (routineId == RID_ABS_SELF_TEST)
                {
                    uint16 responseLength = 0;
                    retVal = RID_ABSSelfTest(&response->responseData[4], &responseLength);
                    response->responseDataLength += responseLength;
                }
                else
                {
                    DiagnosticService_PrepareErrorResponse(response, UDS_SID_ROUTINE_CONTROL, UDS_NRC_REQUEST_OUT_OF_RANGE);
                    retVal = E_OK;
                }
            }
            else
            {
                DiagnosticService_PrepareErrorResponse(response, UDS_SID_ROUTINE_CONTROL, UDS_NRC_SUBFUNCTION_NOT_SUPPORTED);
                retVal = E_OK;
            }
        }
        else
        {
            DiagnosticService_PrepareErrorResponse(response, UDS_SID_ROUTINE_CONTROL, UDS_NRC_CONDITIONS_NOT_CORRECT);
            retVal = E_OK;
        }
    }
    else
    {
        DiagnosticService_PrepareErrorResponse(response, UDS_SID_ROUTINE_CONTROL, UDS_NRC_INCORRECT_MESSAGE_LENGTH);
        retVal = E_OK;
    }
    
    return retVal;
}

/* Internal Functions */

/**
 * @brief Find DTC in table
 */
static Std_ReturnType DiagnosticService_FindDTC(uint32 dtcNumber, uint8* index)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint8 i;
    
    for (i = 0; i < DIAG_MAX_DTC_COUNT; i++)
    {
        if (g_DTCTable[i].dtcNumber == dtcNumber)
        {
            *index = i;
            retVal = E_OK;
            break;
        }
    }
    
    return retVal;
}

/**
 * @brief Add DTC to table
 */
static Std_ReturnType DiagnosticService_AddDTC(uint32 dtcNumber, WheelPosition_t wheelPos, ABS_MalfunctionType_t type)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint8 i;
    
    /* Find empty slot */
    for (i = 0; i < DIAG_MAX_DTC_COUNT; i++)
    {
        if (g_DTCTable[i].dtcNumber == 0)
        {
            g_DTCTable[i].dtcNumber = dtcNumber;
            g_DTCTable[i].affectedWheel = wheelPos;
            g_DTCTable[i].malfunctionType = type;
            g_DTCTable[i].status.testFailed = 1;
            g_DTCTable[i].status.testFailedThisOperationCycle = 1;
            g_DTCTable[i].status.pendingDTC = 1;
            g_DTCTable[i].occurrenceCount = 1;
            g_DTCTable[i].firstFailureTimestamp = 0; /* Should be actual timestamp */
            g_DTCTable[i].lastFailureTimestamp = 0;
            
            if (g_ActiveDTCCount < DIAG_MAX_DTC_COUNT)
            {
                g_ActiveDTCCount++;
            }
            
            retVal = E_OK;
            break;
        }
    }
    
    return retVal;
}

/**
 * @brief Update DTC status
 */
static Std_ReturnType DiagnosticService_UpdateDTCStatus(uint32 dtcNumber, boolean active)
{
    Std_ReturnType retVal = E_NOT_OK;
    uint8 dtcIndex;
    
    if (DiagnosticService_FindDTC(dtcNumber, &dtcIndex) == E_OK)
    {
        if (active == TRUE)
        {
            g_DTCTable[dtcIndex].status.testFailed = 1;
            g_DTCTable[dtcIndex].status.testFailedThisOperationCycle = 1;
            g_DTCTable[dtcIndex].occurrenceCount++;
            g_DTCTable[dtcIndex].lastFailureTimestamp = 0; /* Should be actual timestamp */
            
            /* Confirm DTC after multiple occurrences */
            if (g_DTCTable[dtcIndex].occurrenceCount >= 3)
            {
                g_DTCTable[dtcIndex].status.confirmedDTC = 1;
            }
        }
        else
        {
            g_DTCTable[dtcIndex].status.testFailed = 0;
        }
        
        retVal = E_OK;
    }
    
    return retVal;
}

/**
 * @brief Get service handler for service ID
 */
static UDSServiceHandler_t DiagnosticService_GetServiceHandler(uint8 serviceId)
{
    UDSServiceHandler_t handler = NULL_PTR;
    uint8 i;
    
    for (i = 0; i < UDS_SERVICE_TABLE_SIZE; i++)
    {
        if (g_UDSServiceTable[i].serviceId == serviceId)
        {
            handler = g_UDSServiceTable[i].handler;
            break;
        }
    }
    
    return handler;
}

/**
 * @brief Prepare error response
 */
static void DiagnosticService_PrepareErrorResponse(UDSMessage_t* response, uint8 serviceId, uint8 nrc)
{
    response->responseData[0] = 0x7F; /* Negative response */
    response->responseData[1] = serviceId;
    response->responseData[2] = nrc;
    response->responseDataLength = 3;
}

/**
 * @brief Get DTC for malfunction type and wheel position
 */
static uint32 DiagnosticService_GetDTCForMalfunction(ABS_MalfunctionType_t type, WheelPosition_t wheelPos)
{
    uint32 dtcNumber = 0;
    
    switch (type)
    {
        case ABS_MALFUNCTION_SPEED_SENSOR_MISCALIBRATION:
            switch (wheelPos)
            {
                case WHEEL_FRONT_LEFT: dtcNumber = DTC_SPEED_SENSOR_FL_MISCALIBRATED; break;
                case WHEEL_FRONT_RIGHT: dtcNumber = DTC_SPEED_SENSOR_FR_MISCALIBRATED; break;
                case WHEEL_REAR_LEFT: dtcNumber = DTC_SPEED_SENSOR_RL_MISCALIBRATED; break;
                case WHEEL_REAR_RIGHT: dtcNumber = DTC_SPEED_SENSOR_RR_MISCALIBRATED; break;
                default: break;
            }
            break;
            
        case ABS_MALFUNCTION_SPEED_SENSOR_FAILURE:
            switch (wheelPos)
            {
                case WHEEL_FRONT_LEFT: dtcNumber = DTC_SPEED_SENSOR_FL_FAILURE; break;
                case WHEEL_FRONT_RIGHT: dtcNumber = DTC_SPEED_SENSOR_FR_FAILURE; break;
                case WHEEL_REAR_LEFT: dtcNumber = DTC_SPEED_SENSOR_RL_FAILURE; break;
                case WHEEL_REAR_RIGHT: dtcNumber = DTC_SPEED_SENSOR_RR_FAILURE; break;
                default: break;
            }
            break;
            
        case ABS_MALFUNCTION_SPEED_DIFFERENCE_EXCESSIVE:
            dtcNumber = DTC_SPEED_PLAUSIBILITY_ERROR;
            break;
            
        default:
            dtcNumber = DTC_ABS_SYSTEM_MALFUNCTION;
            break;
    }
    
    return dtcNumber;
}

/**
 * @brief Monitor ABS malfunctions and update DTCs
 */
static void DiagnosticService_MonitorMalfunctions(void)
{
    ABS_MalfunctionStatus_t malfunctionStatus;
    uint8 wheelIdx;
    
    /* Check malfunction status for all wheels */
    for (wheelIdx = 0; wheelIdx < WHEEL_MAX; wheelIdx++)
    {
        if (ABS_GetMalfunctionStatus((WheelPosition_t)wheelIdx, &malfunctionStatus) == E_OK)
        {
            if (malfunctionStatus.confirmedMalfunction == TRUE)
            {
                uint32 dtcNumber = DiagnosticService_GetDTCForMalfunction(malfunctionStatus.malfunctionType, 
                                                                        malfunctionStatus.affectedWheel);
                
                if (dtcNumber != 0)
                {
                    DiagnosticService_SetDTC(dtcNumber, TRUE, malfunctionStatus.affectedWheel);
                }
            }
        }
    }
}

/* RTE Runnable Functions */

/**
 * @brief RTE Runnable for main diagnostic processing
 */
void RE_DiagnosticService_MainCyclic(void)
{
    DiagnosticService_MainFunction();
}

/**
 * @brief RTE Runnable for UDS processing
 */
void RE_DiagnosticService_UDSProcessing(void)
{
    UDSMessage_t request, response;
    
    /* Read UDS request from RTE */
    if (Rte_Read_UDSRequest_message(&request) == E_OK)
    {
        /* Process UDS request */
        if (DiagnosticService_ProcessUDSRequest(&request, &response) == E_OK)
        {
            /* Send UDS response via RTE */
            Rte_Write_UDSResponse_message(&response);
        }
    }
}

/**
 * @brief RTE Runnable for DTC management
 */
void RE_DiagnosticService_DTCManager(void)
{
    /* DTC management handled in main function */
}