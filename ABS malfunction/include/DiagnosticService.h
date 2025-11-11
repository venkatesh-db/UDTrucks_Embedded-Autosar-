/**
 * @file DiagnosticService.h
 * @brief UDS Diagnostic Services for ABS Malfunction Detection
 * @author Generated for ABS Malfunction Detection System
 */

#ifndef DIAGNOSTICSERVICE_H
#define DIAGNOSTICSERVICE_H

#include "Std_Types.h"
#include "SpeedSensor_Types.h"
#include "ABS_MalfunctionDetection.h"
#include "CalibrationManager.h"

/* UDS Service IDs */
#define UDS_SID_DIAGNOSTIC_SESSION_CONTROL      0x10U
#define UDS_SID_ECU_RESET                       0x11U
#define UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION    0x14U
#define UDS_SID_READ_DTC_INFORMATION            0x19U
#define UDS_SID_READ_DATA_BY_IDENTIFIER         0x22U
#define UDS_SID_WRITE_DATA_BY_IDENTIFIER        0x2EU
#define UDS_SID_IO_CONTROL_BY_IDENTIFIER        0x2FU
#define UDS_SID_ROUTINE_CONTROL                 0x31U

/* UDS Response Codes */
#define UDS_NRC_POSITIVE_RESPONSE               0x00U
#define UDS_NRC_GENERAL_REJECT                  0x10U
#define UDS_NRC_SERVICE_NOT_SUPPORTED           0x11U
#define UDS_NRC_SUBFUNCTION_NOT_SUPPORTED       0x12U
#define UDS_NRC_INCORRECT_MESSAGE_LENGTH        0x13U
#define UDS_NRC_CONDITIONS_NOT_CORRECT          0x22U
#define UDS_NRC_REQUEST_OUT_OF_RANGE            0x31U
#define UDS_NRC_SECURITY_ACCESS_DENIED          0x33U
#define UDS_NRC_REQUEST_SEQUENCE_ERROR          0x24U

/* Diagnostic Session Types */
typedef enum {
    DIAG_SESSION_DEFAULT = 0x01,
    DIAG_SESSION_PROGRAMMING = 0x02,
    DIAG_SESSION_EXTENDED = 0x03,
    DIAG_SESSION_SAFETY_SYSTEM = 0x04
} DiagSession_t;

/* DTC Status Byte */
typedef struct {
    uint8 testFailed : 1;
    uint8 testFailedThisOperationCycle : 1;
    uint8 pendingDTC : 1;
    uint8 confirmedDTC : 1;
    uint8 testNotCompletedSinceLastClear : 1;
    uint8 testFailedSinceLastClear : 1;
    uint8 testNotCompletedThisOperationCycle : 1;
    uint8 warningIndicatorRequested : 1;
} DTCStatus_t;

/* DTC Information */
typedef struct {
    uint32 dtcNumber;
    DTCStatus_t status;
    uint8 severity;
    uint8 functionalUnit;
    WheelPosition_t affectedWheel;
    uint32 occurrenceCount;
    uint32 firstFailureTimestamp;
    uint32 lastFailureTimestamp;
    ABS_MalfunctionType_t malfunctionType;
} DTCInfo_t;

/* Data Identifiers (DIDs) */
#define DID_SPEED_SENSOR_FL_DATA                0xF100U
#define DID_SPEED_SENSOR_FR_DATA                0xF101U
#define DID_SPEED_SENSOR_RL_DATA                0xF102U
#define DID_SPEED_SENSOR_RR_DATA                0xF103U
#define DID_CALIBRATION_FL_PARAMS               0xF110U
#define DID_CALIBRATION_FR_PARAMS               0xF111U
#define DID_CALIBRATION_RL_PARAMS               0xF112U
#define DID_CALIBRATION_RR_PARAMS               0xF113U
#define DID_ABS_SYSTEM_STATUS                   0xF120U
#define DID_MALFUNCTION_COUNTER                 0xF121U
#define DID_DIAGNOSTIC_SESSION_INFO             0xF1F0U

/* Routine Control Identifiers */
#define RID_START_CALIBRATION_FL                0x0201U
#define RID_START_CALIBRATION_FR                0x0202U
#define RID_START_CALIBRATION_RL                0x0203U
#define RID_START_CALIBRATION_RR                0x0204U
#define RID_VALIDATE_CALIBRATION                0x0210U
#define RID_RESET_CALIBRATION_ALL               0x0220U
#define RID_ABS_SELF_TEST                       0x0230U

/* DTC Codes for ABS System */
#define DTC_SPEED_SENSOR_FL_MISCALIBRATED       0xC14100U
#define DTC_SPEED_SENSOR_FR_MISCALIBRATED       0xC14101U
#define DTC_SPEED_SENSOR_RL_MISCALIBRATED       0xC14102U
#define DTC_SPEED_SENSOR_RR_MISCALIBRATED       0xC14103U
#define DTC_SPEED_SENSOR_FL_FAILURE             0xC14200U
#define DTC_SPEED_SENSOR_FR_FAILURE             0xC14201U
#define DTC_SPEED_SENSOR_RL_FAILURE             0xC14202U
#define DTC_SPEED_SENSOR_RR_FAILURE             0xC14203U
#define DTC_ABS_SYSTEM_MALFUNCTION              0xC14300U
#define DTC_SPEED_PLAUSIBILITY_ERROR            0xC14400U

/* UDS Message Structure */
typedef struct {
    uint8 serviceId;
    uint8 subFunction;
    uint16 dataIdentifier;
    uint16 routineIdentifier;
    uint8* requestData;
    uint16 requestDataLength;
    uint8* responseData;
    uint16 responseDataLength;
    uint16 maxResponseLength;
} UDSMessage_t;

/* Diagnostic Service Functions */

/**
 * @brief Initialize diagnostic service
 */
Std_ReturnType DiagnosticService_Init(void);

/**
 * @brief Deinitialize diagnostic service
 */
Std_ReturnType DiagnosticService_DeInit(void);

/**
 * @brief Main processing function for diagnostic service
 */
Std_ReturnType DiagnosticService_MainFunction(void);

/**
 * @brief Process UDS request
 */
Std_ReturnType DiagnosticService_ProcessUDSRequest(const UDSMessage_t* request, UDSMessage_t* response);

/**
 * @brief Set DTC status
 */
Std_ReturnType DiagnosticService_SetDTC(uint32 dtcNumber, boolean active, WheelPosition_t wheelPos);

/**
 * @brief Clear DTC
 */
Std_ReturnType DiagnosticService_ClearDTC(uint32 dtcNumber);

/**
 * @brief Clear all DTCs
 */
Std_ReturnType DiagnosticService_ClearAllDTCs(void);

/**
 * @brief Get DTC information
 */
Std_ReturnType DiagnosticService_GetDTCInfo(uint32 dtcNumber, DTCInfo_t* dtcInfo);

/**
 * @brief Get all active DTCs
 */
Std_ReturnType DiagnosticService_GetActiveDTCs(uint32* dtcList, uint8* dtcCount, uint8 maxDtcs);

/* UDS Service Handlers */
Std_ReturnType UDS_DiagnosticSessionControl(const UDSMessage_t* request, UDSMessage_t* response);
Std_ReturnType UDS_ECUReset(const UDSMessage_t* request, UDSMessage_t* response);
Std_ReturnType UDS_ClearDiagnosticInformation(const UDSMessage_t* request, UDSMessage_t* response);
Std_ReturnType UDS_ReadDTCInformation(const UDSMessage_t* request, UDSMessage_t* response);
Std_ReturnType UDS_ReadDataByIdentifier(const UDSMessage_t* request, UDSMessage_t* response);
Std_ReturnType UDS_WriteDataByIdentifier(const UDSMessage_t* request, UDSMessage_t* response);
Std_ReturnType UDS_InputOutputControlByIdentifier(const UDSMessage_t* request, UDSMessage_t* response);
Std_ReturnType UDS_RoutineControl(const UDSMessage_t* request, UDSMessage_t* response);

/* DID Read/Write Functions */
Std_ReturnType DID_ReadSpeedSensorData(uint16 did, uint8* data, uint16* length);
Std_ReturnType DID_ReadCalibrationParams(uint16 did, uint8* data, uint16* length);
Std_ReturnType DID_WriteCalibrationParams(uint16 did, const uint8* data, uint16 length);
Std_ReturnType DID_ReadABSSystemStatus(uint8* data, uint16* length);
Std_ReturnType DID_ReadMalfunctionCounter(uint8* data, uint16* length);

/* Routine Control Functions */
Std_ReturnType RID_StartCalibration(uint16 rid, const uint8* data, uint16 length, uint8* response, uint16* responseLength);
Std_ReturnType RID_ValidateCalibration(const uint8* data, uint16 length, uint8* response, uint16* responseLength);
Std_ReturnType RID_ResetCalibrationAll(uint8* response, uint16* responseLength);
Std_ReturnType RID_ABSSelfTest(uint8* response, uint16* responseLength);

/* RTE Interface Functions */
void RE_DiagnosticService_MainCyclic(void);
void RE_DiagnosticService_UDSProcessing(void);
void RE_DiagnosticService_DTCManager(void);

/* Port Interfaces */
extern Std_ReturnType Rte_Read_UDSRequest_message(UDSMessage_t* message);
extern Std_ReturnType Rte_Write_UDSResponse_message(const UDSMessage_t* message);
extern Std_ReturnType Rte_Read_DiagnosticSession_session(DiagSession_t* session);
extern Std_ReturnType Rte_Write_DiagnosticSession_session(const DiagSession_t* session);

/* Constants */
#define DIAG_MAX_DTC_COUNT                      32U
#define DIAG_MAX_REQUEST_LENGTH                 4095U
#define DIAG_MAX_RESPONSE_LENGTH                4095U
#define DIAG_SESSION_TIMEOUT_MS                 5000U

#endif /* DIAGNOSTICSERVICE_H */