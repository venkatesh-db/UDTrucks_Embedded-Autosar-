/**
 * @file PowerManager.h
 * @brief Power Management Module for AUTOSAR Infotainment ECU
 * @details Handles power states, sleep modes, and wake-up management
 * @author Battery Drain Case Study
 * @date November 2024
 */

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Power States according to AUTOSAR Power Management
 */
typedef enum {
    POWER_STATE_OFF = 0x00,           /**< ECU is powered off */
    POWER_STATE_RESET = 0x01,         /**< ECU is in reset state */
    POWER_STATE_STARTUP = 0x02,       /**< ECU is starting up */
    POWER_STATE_RUN = 0x03,           /**< Normal operation mode */
    POWER_STATE_SLEEP_PREPARE = 0x04, /**< Preparing for sleep */
    POWER_STATE_SLEEP = 0x05,         /**< Low power sleep mode */
    POWER_STATE_SHUTDOWN = 0x06       /**< Shutdown in progress */
} PowerState_t;

/**
 * @brief Wake-up sources for the infotainment ECU
 */
typedef enum {
    WAKEUP_NONE = 0x00,
    WAKEUP_CAN_NETWORK = 0x01,        /**< CAN network activity */
    WAKEUP_IGNITION = 0x02,           /**< Ignition switch */
    WAKEUP_USER_INPUT = 0x04,         /**< Touch screen or buttons */
    WAKEUP_TIMER = 0x08,              /**< Periodic timer */
    WAKEUP_BLUETOOTH = 0x10,          /**< Bluetooth connection */
    WAKEUP_WIFI = 0x20,               /**< WiFi activity */
    WAKEUP_USB = 0x40,                /**< USB connection */
    WAKEUP_EMERGENCY = 0x80           /**< Emergency call system */
} WakeupSource_t;

/**
 * @brief Power consumption levels in microamps
 */
typedef enum {
    CURRENT_ACTIVE = 2500000,         /**< Active mode: 2.5A */
    CURRENT_STANDBY = 150000,         /**< Standby mode: 150mA */
    CURRENT_SLEEP = 5000,             /**< Sleep mode: 5mA */
    CURRENT_DEEP_SLEEP = 500          /**< Deep sleep: 0.5mA */
} PowerConsumption_t;

/**
 * @brief Power management configuration structure
 */
typedef struct {
    uint32_t sleepTimeout_ms;         /**< Timeout before entering sleep */
    uint32_t deepSleepTimeout_ms;     /**< Timeout before deep sleep */
    uint32_t wakeupSources;           /**< Enabled wake-up sources */
    bool enablePeriodicWakeup;        /**< Enable periodic wake-up */
    uint32_t periodicWakeupInterval_ms; /**< Periodic wake-up interval */
    bool enableNetworkWakeup;         /**< Enable network wake-up */
    bool enableRemoteWakeup;          /**< Enable remote wake-up */
} PowerConfig_t;

/**
 * @brief Power management statistics
 */
typedef struct {
    uint32_t sleepEntryCount;
    uint32_t wakeupCount;
    uint32_t totalSleepTime_ms;
    uint32_t totalActiveTime_ms;
    uint32_t currentConsumption_uA;
    uint32_t batteryVoltage_mV;
    uint32_t powerCycles;
} PowerStats_t;

/**
 * @brief Power Manager Class
 */
class PowerManager {
private:
    PowerState_t currentState;
    PowerConfig_t config;
    PowerStats_t stats;
    uint32_t lastActivity_ms;
    uint32_t sleepEntryTime_ms;
    bool ignitionState;
    bool networkActive;
    
    // Problematic variables that can cause battery drain
    bool backgroundTaskActive;        /**< PROBLEM: May stay active */
    bool audioProcessingActive;       /**< PROBLEM: Audio keeps running */
    bool displayBacklightOn;          /**< PROBLEM: Display stays on */
    bool bluetoothScanActive;         /**< PROBLEM: Continuous BT scan */
    bool wifiScanActive;              /**< PROBLEM: Continuous WiFi scan */
    bool gpsActive;                   /**< PROBLEM: GPS stays active */
    
    // Internal methods
    void enterSleepMode();
    void exitSleepMode();
    void updatePowerConsumption();
    bool checkWakeupConditions();
    void shutdownNonEssentialSystems();
    void restoreNonEssentialSystems();

public:
    /**
     * @brief Constructor
     */
    PowerManager();
    
    /**
     * @brief Destructor
     */
    ~PowerManager();
    
    /**
     * @brief Initialize power manager
     * @param config Power management configuration
     * @return true if successful, false otherwise
     */
    bool initialize(const PowerConfig_t& config);
    
    /**
     * @brief Main power manager task - should be called cyclically
     */
    void mainTask();
    
    /**
     * @brief Set ignition state
     * @param state true if ignition is on, false otherwise
     */
    void setIgnitionState(bool state);
    
    /**
     * @brief Set network activity state
     * @param active true if network is active, false otherwise
     */
    void setNetworkActivity(bool active);
    
    /**
     * @brief Register user activity
     */
    void registerUserActivity();
    
    /**
     * @brief Force sleep mode entry
     */
    void forceSleep();
    
    /**
     * @brief Wake up from sleep mode
     * @param source Wake-up source
     */
    void wakeup(WakeupSource_t source);
    
    /**
     * @brief Get current power state
     * @return Current power state
     */
    PowerState_t getCurrentState() const { return currentState; }
    
    /**
     * @brief Get power statistics
     * @return Power statistics structure
     */
    const PowerStats_t& getStatistics() const { return stats; }
    
    /**
     * @brief Get current power consumption
     * @return Current consumption in microamps
     */
    uint32_t getCurrentConsumption() const { return stats.currentConsumption_uA; }
    
    /**
     * @brief Check if system should enter sleep mode
     * @return true if sleep conditions are met
     */
    bool shouldEnterSleep() const;
    
    // PROBLEMATIC METHODS - These can cause battery drain if misused
    /**
     * @brief Set background task state (POTENTIAL DRAIN SOURCE)
     * @param active true to keep background tasks active
     */
    void setBackgroundTaskActive(bool active) { backgroundTaskActive = active; }
    
    /**
     * @brief Set audio processing state (POTENTIAL DRAIN SOURCE)
     * @param active true to keep audio processing active
     */
    void setAudioProcessingActive(bool active) { audioProcessingActive = active; }
    
    /**
     * @brief Set display backlight state (POTENTIAL DRAIN SOURCE)
     * @param on true to keep display backlight on
     */
    void setDisplayBacklight(bool on) { displayBacklightOn = on; }
    
    /**
     * @brief Set Bluetooth scan state (POTENTIAL DRAIN SOURCE)
     * @param active true to keep Bluetooth scanning active
     */
    void setBluetoothScan(bool active) { bluetoothScanActive = active; }
    
    /**
     * @brief Set WiFi scan state (POTENTIAL DRAIN SOURCE)
     * @param active true to keep WiFi scanning active
     */
    void setWifiScan(bool active) { wifiScanActive = active; }
    
    /**
     * @brief Set GPS state (POTENTIAL DRAIN SOURCE)
     * @param active true to keep GPS active
     */
    void setGpsActive(bool active) { gpsActive = active; }
};

#endif // POWER_MANAGER_H