/**
 * @file ecu_startup_monitor.h
 * @brief Header file for Engine ECU Startup Monitoring
 */

#ifndef ECU_STARTUP_MONITOR_H
#define ECU_STARTUP_MONITOR_H

#include <stdint.h>
#include <stdbool.h>

/* Module ID for DET reporting */
#define MODULE_ID_STARTUP_MONITOR   0x100

/* Startup phase definitions */
typedef enum {
    STARTUP_PHASE_INIT = 0,
    STARTUP_PHASE_BSW_INIT,
    STARTUP_PHASE_RTE_START,
    STARTUP_PHASE_APP_INIT,
    STARTUP_PHASE_RUNNING,
    STARTUP_PHASE_ERROR
} StartupPhase_t;

/* Error codes for startup failures */
typedef enum {
    STARTUP_ERROR_NONE = 0,
    STARTUP_ERROR_CLOCK_FAIL,
    STARTUP_ERROR_RAM_TEST_FAIL,
    STARTUP_ERROR_FLASH_CRC_FAIL,
    STARTUP_ERROR_BSW_INIT_FAIL,
    STARTUP_ERROR_RTE_START_FAIL,
    STARTUP_ERROR_APP_INIT_FAIL,
    STARTUP_ERROR_WATCHDOG_RESET,
    STARTUP_ERROR_STACK_OVERFLOW
} StartupError_t;

/* Startup monitoring structure */
typedef struct {
    StartupPhase_t current_phase;
    StartupError_t last_error;
    uint32_t boot_count;
    uint32_t error_count;
    uint32_t last_reset_reason;
    uint32_t startup_timestamp;
} StartupMonitor_t;

/* Function prototypes */
void StartupMonitor_Init(void);
void StartupMonitor_SetPhase(StartupPhase_t phase);
void StartupMonitor_ReportError(StartupError_t error);
StartupMonitor_t* StartupMonitor_GetStatus(void);
bool StartupMonitor_CheckCriticalSystems(void);
void StartupMonitor_EmergencyRecovery(void);

#endif /* ECU_STARTUP_MONITOR_H */