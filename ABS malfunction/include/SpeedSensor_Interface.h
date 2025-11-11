/**
 * @file SpeedSensor_Interface.h
 * @brief AUTOSAR Software Component Interface for Speed Sensor
 * @author Generated for ABS Malfunction Detection System
 */

#ifndef SPEEDSENSOR_INTERFACE_H
#define SPEEDSENSOR_INTERFACE_H

#include "Std_Types.h"
#include "SpeedSensor_Types.h"

/* AUTOSAR SWC Interface for Speed Sensor */

/**
 * @brief Initialize speed sensor interface
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType SpeedSensor_Init(void);

/**
 * @brief Deinitialize speed sensor interface
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType SpeedSensor_DeInit(void);

/**
 * @brief Main processing function - called cyclically by RTE
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType SpeedSensor_MainFunction(void);

/**
 * @brief Read raw speed sensor data for specific wheel
 * @param wheelPos Wheel position
 * @param rawData Pointer to store raw sensor data
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType SpeedSensor_ReadRawData(WheelPosition_t wheelPos, SpeedSensorRawData_t* rawData);

/**
 * @brief Get calculated speed data for specific wheel
 * @param wheelPos Wheel position
 * @param speedData Pointer to store calculated speed data
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType SpeedSensor_GetSpeedData(WheelPosition_t wheelPos, SpeedData_t* speedData);

/**
 * @brief Set calibration parameters for speed sensor
 * @param wheelPos Wheel position
 * @param calibration Calibration parameters
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType SpeedSensor_SetCalibration(WheelPosition_t wheelPos, const SpeedSensorCalibration_t* calibration);

/**
 * @brief Get calibration parameters for speed sensor
 * @param wheelPos Wheel position
 * @param calibration Pointer to store calibration parameters
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType SpeedSensor_GetCalibration(WheelPosition_t wheelPos, SpeedSensorCalibration_t* calibration);

/**
 * @brief Perform calibration validation
 * @param wheelPos Wheel position
 * @param isValid Pointer to store validation result
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType SpeedSensor_ValidateCalibration(WheelPosition_t wheelPos, boolean* isValid);

/**
 * @brief Get sensor diagnostic information
 * @param wheelPos Wheel position
 * @param diagnostics Pointer to store diagnostic data
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType SpeedSensor_GetDiagnostics(WheelPosition_t wheelPos, SpeedSensorDiagnostics_t* diagnostics);

/**
 * @brief Clear sensor error counters
 * @param wheelPos Wheel position
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType SpeedSensor_ClearErrors(WheelPosition_t wheelPos);

/**
 * @brief Check if all speed sensors are functional
 * @param allSensorsOk Pointer to store result
 * @return E_OK if successful, E_NOT_OK otherwise
 */
Std_ReturnType SpeedSensor_CheckAllSensors(boolean* allSensorsOk);

/* RTE Interface Functions */

/**
 * @brief RTE Runnable for cyclic speed sensor processing
 */
void RE_SpeedSensor_MainCyclic(void);

/**
 * @brief RTE Runnable for speed sensor calibration
 */
void RE_SpeedSensor_Calibration(void);

/**
 * @brief RTE Runnable for speed sensor diagnostics
 */
void RE_SpeedSensor_Diagnostics(void);

/* Port Interface Definitions */

/* Sender-Receiver Ports */
extern Std_ReturnType Rte_Write_SpeedData_FL_speedData(const SpeedData_t* data);
extern Std_ReturnType Rte_Write_SpeedData_FR_speedData(const SpeedData_t* data);
extern Std_ReturnType Rte_Write_SpeedData_RL_speedData(const SpeedData_t* data);
extern Std_ReturnType Rte_Write_SpeedData_RR_speedData(const SpeedData_t* data);

extern Std_ReturnType Rte_Read_RawSensorData_FL_rawData(SpeedSensorRawData_t* data);
extern Std_ReturnType Rte_Read_RawSensorData_FR_rawData(SpeedSensorRawData_t* data);
extern Std_ReturnType Rte_Read_RawSensorData_RL_rawData(SpeedSensorRawData_t* data);
extern Std_ReturnType Rte_Read_RawSensorData_RR_rawData(SpeedSensorRawData_t* data);

/* Client-Server Ports */
extern Std_ReturnType Rte_Call_CalibrationService_SetCalibration(WheelPosition_t wheelPos, const SpeedSensorCalibration_t* cal);
extern Std_ReturnType Rte_Call_CalibrationService_GetCalibration(WheelPosition_t wheelPos, SpeedSensorCalibration_t* cal);
extern Std_ReturnType Rte_Call_DiagnosticService_GetDiagnostics(WheelPosition_t wheelPos, SpeedSensorDiagnostics_t* diag);

#endif /* SPEEDSENSOR_INTERFACE_H */