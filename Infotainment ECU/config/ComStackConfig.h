/**
 * @file ComStackConfig.h
 * @brief Communication Stack Configuration for Power Management
 * @details CAN, Ethernet, and diagnostic communication setup
 * @author Battery Drain Case Study
 * @date November 2024
 */

#ifndef COM_STACK_CONFIG_H
#define COM_STACK_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

//=============================================================================
// CAN Configuration for Power Management
//=============================================================================

/**
 * @brief CAN Message IDs for Power Management
 */
#define CAN_MSG_POWER_STATE         0x100
#define CAN_MSG_WAKEUP_REQUEST      0x101
#define CAN_MSG_SLEEP_REQUEST       0x102
#define CAN_MSG_DIAGNOSTIC_DATA     0x103
#define CAN_MSG_BATTERY_STATUS      0x104

/**
 * @brief Network Management (NM) Configuration
 */
#define NM_NODE_ID                  0x10
#define NM_NETWORK_TIMEOUT_TIME     5000    // 5 seconds
#define NM_REPEAT_MESSAGE_TIME      500     // 500 ms
#define NM_WAIT_BUS_SLEEP_TIME      1000    // 1 second

/**
 * @brief CAN Network States
 */
typedef enum {
    CAN_NM_STATE_BUS_OFF = 0,
    CAN_NM_STATE_BUS_SLEEP = 1,
    CAN_NM_STATE_PREPARE_SLEEP = 2,
    CAN_NM_STATE_READY_SLEEP = 3,
    CAN_NM_STATE_NORMAL_OPERATION = 4,
    CAN_NM_STATE_REPEAT_MESSAGE = 5,
    CAN_NM_STATE_NETWORK_MODE = 6
} CanNmState_t;

/**
 * @brief Power management CAN messages structure
 */
typedef struct {
    uint32_t messageId;
    uint8_t data[8];
    uint8_t dlc;
    bool cyclic;
    uint32_t cycletime_ms;
    bool wakeupCapable;
} PowerCanMessage_t;

//=============================================================================
// Diagnostic Communication Configuration (UDS over CAN)
//=============================================================================

/**
 * @brief UDS Service IDs for Power Management
 */
#define UDS_SID_DIAGNOSTIC_SESSION_CONTROL    0x10
#define UDS_SID_ECU_RESET                     0x11
#define UDS_SID_READ_DATA_BY_IDENTIFIER       0x22
#define UDS_SID_WRITE_DATA_BY_IDENTIFIER      0x2E
#define UDS_SID_ROUTINE_CONTROL               0x31

/**
 * @brief Power Management DIDs (Data Identifiers)
 */
#define DID_POWER_STATE                       0xF010
#define DID_BATTERY_VOLTAGE                   0xF011
#define DID_POWER_CONSUMPTION                 0xF012
#define DID_WAKE_UP_SOURCES                   0xF013
#define DID_SLEEP_MODE_CONFIG                 0xF014

/**
 * @brief Routine Control IDs for Power Management
 */
#define RID_FORCE_SLEEP_MODE                  0x0101
#define RID_WAKE_UP_SYSTEM                    0x0102
#define RID_POWER_CONSUMPTION_TEST            0x0103

//=============================================================================
// Ethernet Configuration (for diagnostics and updates)
//=============================================================================

/**
 * @brief Ethernet Configuration
 */
#define ETH_MAC_ADDRESS                       {0x02, 0x00, 0x00, 0x00, 0x00, 0x01}
#define ETH_IP_ADDRESS                        {192, 168, 1, 100}
#define ETH_SUBNET_MASK                       {255, 255, 255, 0}
#define ETH_GATEWAY                           {192, 168, 1, 1}

/**
 * @brief DoIP (Diagnostics over IP) Configuration
 */
#define DOIP_UDP_PORT                         13400
#define DOIP_TCP_PORT                         13400
#define DOIP_LOGICAL_ADDRESS                  0x1000
#define DOIP_FUNCTION_ADDRESS                 0xDF00

//=============================================================================
// Communication Matrix Configuration
//=============================================================================

/**
 * @brief Power management communication signals
 */
typedef struct {
    // Transmitted signals (ECU -> Network)
    struct {
        uint8_t powerState;           // Current power state
        uint16_t batteryVoltage_mV;   // Battery voltage in mV
        uint32_t powerConsumption_uA; // Power consumption in μA
        uint8_t systemHealth;         // System health status
        uint16_t diagnosticData;      // Diagnostic trouble codes
    } tx;
    
    // Received signals (Network -> ECU)
    struct {
        uint8_t wakeupRequest;        // Wake-up request from other ECUs
        uint8_t sleepRequest;         // Sleep request from other ECUs
        uint8_t ignitionState;        // Ignition switch state
        uint8_t vehicleSpeed;         // Vehicle speed (affects power policy)
        uint16_t externalCommands;    // External power management commands
    } rx;
} PowerComMatrix_t;

//=============================================================================
// Network Management Configuration
//=============================================================================

/**
 * @brief Network Management Configuration
 * CRITICAL: Improper NM configuration is a common cause of battery drain
 */
typedef struct {
    bool nmEnabled;                   // Enable network management
    uint32_t nmTimeoutTime_ms;        // Time before entering sleep
    uint32_t nmWaitBusSleepTime_ms;   // Wait time before bus sleep
    uint32_t nmRepeatMessageTime_ms;  // NM message repeat interval
    bool nmPassiveModeEnabled;        // Passive mode (listen only)
    bool nmImmediateTxMode;           // Immediate transmission mode
    
    // POTENTIAL BATTERY DRAIN SOURCES:
    bool nmCarWakeUpFilterEnabled;    // Car wake-up filter (can cause issues)
    bool nmAllNmMessagesKeepAwake;    // Keep awake on all NM messages (DANGEROUS!)
    bool nmCoordinatorSyncSupport;    // Coordinator synchronization
    uint32_t nmCoordinatorTimeout_ms; // Coordinator timeout
} NmConfiguration_t;

//=============================================================================
// Communication Schedule Configuration
//=============================================================================

/**
 * @brief Communication schedule for different power states
 */
typedef struct {
    // Normal operation schedule
    struct {
        uint32_t powerStateMsgCycle_ms;     // 100ms
        uint32_t batteryStatusCycle_ms;     // 1000ms  
        uint32_t diagnosticCycle_ms;        // 5000ms
        uint32_t nmMessageCycle_ms;         // 500ms
    } normalOperation;
    
    // Prepare sleep schedule
    struct {
        uint32_t powerStateMsgCycle_ms;     // 50ms (faster updates)
        uint32_t batteryStatusCycle_ms;     // 500ms
        uint32_t diagnosticCycle_ms;        // Disabled
        uint32_t nmMessageCycle_ms;         // 200ms
    } prepareSleep;
    
    // Sleep mode schedule  
    struct {
        bool powerStateMsgEnabled;          // false (stop transmission)
        bool batteryStatusEnabled;          // false (stop transmission)
        bool diagnosticEnabled;             // false (stop transmission)
        bool nmMessageEnabled;              // false (stop transmission)
        
        // Wake-up message monitoring only
        bool wakeupMsgMonitoring;           // true (keep CAN RX active)
        uint32_t wakeupCheckInterval_ms;    // 100ms
    } sleepMode;
} ComScheduleConfig_t;

//=============================================================================
// Function Declarations
//=============================================================================

/**
 * @brief Initialize communication stack for power management
 */
bool ComStack_Init(void);

/**
 * @brief Deinitialize communication stack (for sleep mode)
 */
void ComStack_DeInit(void);

/**
 * @brief Main communication task
 */
void ComStack_MainFunction(void);

/**
 * @brief Prepare communication stack for sleep mode
 */
void ComStack_PrepareSleep(void);

/**
 * @brief Wake up communication stack from sleep mode  
 */
void ComStack_WakeUp(void);

/**
 * @brief Send power state message
 */
void ComStack_SendPowerState(uint8_t powerState, uint16_t batteryVoltage, uint32_t consumption);

/**
 * @brief Send wake-up request to other ECUs
 */
void ComStack_SendWakeUpRequest(uint8_t targetEcu);

/**
 * @brief Send sleep request to other ECUs
 */
void ComStack_SendSleepRequest(void);

/**
 * @brief Handle received wake-up request
 */
void ComStack_OnWakeUpRequest(uint8_t sourceEcu);

/**
 * @brief Handle received sleep request
 */
void ComStack_OnSleepRequest(uint8_t sourceEcu);

/**
 * @brief Configure network management parameters
 */
void ComStack_ConfigureNM(const NmConfiguration_t* config);

/**
 * @brief Set communication schedule based on power state
 */
void ComStack_SetSchedule(PowerState_t powerState);

/**
 * @brief Check if network allows sleep mode entry
 */
bool ComStack_IsNetworkSleepReady(void);

/**
 * @brief Get network management state
 */
CanNmState_t ComStack_GetNmState(void);

//=============================================================================
// Critical Battery Drain Prevention Macros
//=============================================================================

/**
 * @brief Safety checks to prevent communication-related battery drain
 */
#define COMSTACK_CHECK_SLEEP_CONDITIONS() \
    do { \
        /* Ensure all cyclic messages are stopped */ \
        if (ComStack_GetNmState() != CAN_NM_STATE_BUS_SLEEP) { \
            /* WARNING: Network not ready for sleep! */ \
        } \
        /* Verify no active diagnostic sessions */ \
        if (Dcm_GetSessionType() != DCM_DEFAULT_SESSION) { \
            /* WARNING: Diagnostic session active! */ \
        } \
        /* Check for pending communication requests */ \
        if (Com_GetStatus() != COM_UNINIT) { \
            /* WARNING: COM stack still active! */ \
        } \
    } while(0)

/**
 * @brief Force communication stack to sleep mode
 * Use only when normal sleep transition fails
 */
#define COMSTACK_FORCE_SLEEP() \
    do { \
        Com_IpduGroupStop(COM_ALL_IPDU_GROUPS); \
        CanIf_SetControllerMode(0, CANIF_CS_SLEEP); \
        /* Stop all communication timers */ \
        /* Disable all interrupts except wake-up */ \
    } while(0)

#endif // COM_STACK_CONFIG_H