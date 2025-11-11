/**
 * @file PowerMonitor.h
 * @brief Power Monitoring and Debugging Tools for AUTOSAR Infotainment ECU
 * @details Real-time power consumption tracking, diagnostics, and analysis
 * @author Battery Drain Case Study
 * @date November 2024
 */

#ifndef POWER_MONITOR_H
#define POWER_MONITOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../PowerManager/PowerManager.h"
#include "../InfotainmentSystem/InfotainmentSystem.h"

/**
 * @brief Power measurement data point
 */
typedef struct {
    uint32_t timestamp_ms;
    uint32_t consumption_uA;
    uint32_t batteryVoltage_mV;
    PowerState_t powerState;
    uint32_t subsystemMask;        /**< Bitmask of active subsystems */
} PowerMeasurement_t;

/**
 * @brief Power consumption thresholds
 */
typedef enum {
    THRESHOLD_SLEEP = 10000,       /**< 10mA - Maximum in sleep mode */
    THRESHOLD_STANDBY = 200000,    /**< 200mA - Maximum in standby */
    THRESHOLD_ACTIVE = 3000000,    /**< 3A - Maximum in active mode */
    THRESHOLD_CRITICAL = 5000000   /**< 5A - Critical consumption */
} PowerThreshold_t;

/**
 * @brief Subsystem activity flags
 */
typedef enum {
    SUBSYSTEM_AUDIO = 0x01,
    SUBSYSTEM_DISPLAY = 0x02,
    SUBSYSTEM_BLUETOOTH = 0x04,
    SUBSYSTEM_WIFI = 0x08,
    SUBSYSTEM_GPS = 0x10,
    SUBSYSTEM_MAINTENANCE = 0x20,
    SUBSYSTEM_DIAGNOSTICS = 0x40,
    SUBSYSTEM_UPDATES = 0x80
} SubsystemFlags_t;

/**
 * @brief Power anomaly types
 */
typedef enum {
    ANOMALY_NONE = 0,
    ANOMALY_EXCESSIVE_CONSUMPTION,
    ANOMALY_FAILED_SLEEP_ENTRY,
    ANOMALY_FREQUENT_WAKEUPS,
    ANOMALY_STUCK_SUBSYSTEM,
    ANOMALY_BATTERY_VOLTAGE_DROP,
    ANOMALY_THERMAL_ISSUE
} PowerAnomaly_t;

/**
 * @brief Power analysis report
 */
typedef struct {
    uint32_t measurementCount;
    uint32_t totalEnergy_mAh;
    uint32_t averageConsumption_mA;
    uint32_t peakConsumption_mA;
    uint32_t sleepModePercentage;
    uint32_t wakeupCount;
    uint32_t anomalyCount;
    PowerAnomaly_t mostCommonAnomaly;
    uint32_t estimatedBatteryLife_hours;
} PowerAnalysisReport_t;

/**
 * @brief Real-time power monitor class
 */
class PowerMonitor {
private:
    PowerManager* powerManager;
    InfotainmentSystem* infotainmentSystem;
    
    // Measurement storage
    static const uint32_t MAX_MEASUREMENTS = 10000;
    PowerMeasurement_t measurements[MAX_MEASUREMENTS];
    uint32_t measurementIndex;
    uint32_t measurementCount;
    
    // Analysis data
    PowerAnalysisReport_t analysisReport;
    PowerAnomaly_t currentAnomalies[10];
    uint32_t anomalyCount;
    
    // Configuration
    uint32_t measurementInterval_ms;
    bool continuousLogging;
    bool anomalyDetection;
    bool realTimeAlerts;
    
    // Thresholds
    uint32_t sleepThreshold_uA;
    uint32_t standbyThreshold_uA;
    uint32_t activeThreshold_uA;
    uint32_t criticalThreshold_uA;
    
    // Internal methods
    void detectAnomalies(const PowerMeasurement_t& measurement);
    void updateAnalysisReport();
    uint32_t getSubsystemMask() const;
    void logAnomaly(PowerAnomaly_t anomaly, const char* description);

public:
    /**
     * @brief Constructor
     */
    PowerMonitor();
    
    /**
     * @brief Destructor
     */
    ~PowerMonitor();
    
    /**
     * @brief Initialize power monitor
     * @param pm Power manager instance
     * @param is Infotainment system instance
     * @return true if successful
     */
    bool initialize(PowerManager* pm, InfotainmentSystem* is);
    
    /**
     * @brief Main monitoring task - call cyclically
     */
    void monitoringTask();
    
    /**
     * @brief Start continuous power logging
     * @param interval_ms Measurement interval in milliseconds
     */
    void startLogging(uint32_t interval_ms = 1000);
    
    /**
     * @brief Stop continuous power logging
     */
    void stopLogging();
    
    /**
     * @brief Take single power measurement
     * @return Measurement data
     */
    PowerMeasurement_t takeMeasurement();
    
    /**
     * @brief Generate comprehensive power analysis report
     * @return Analysis report
     */
    PowerAnalysisReport_t generateReport();
    
    /**
     * @brief Detect and analyze power anomalies
     */
    void analyzeAnomalies();
    
    /**
     * @brief Get current power consumption
     * @return Current consumption in microamps
     */
    uint32_t getCurrentConsumption() const;
    
    /**
     * @brief Get estimated battery life remaining
     * @param batteryCapacity_mAh Battery capacity
     * @return Estimated hours remaining
     */
    uint32_t getEstimatedBatteryLife(uint32_t batteryCapacity_mAh) const;
    
    /**
     * @brief Check if system is in proper sleep mode
     * @return true if in proper sleep
     */
    bool isInProperSleepMode() const;
    
    /**
     * @brief Get list of active subsystems
     * @return Bitmask of active subsystems
     */
    uint32_t getActiveSubsystems() const { return getSubsystemMask(); }
    
    /**
     * @brief Print real-time power dashboard
     */
    void printPowerDashboard();
    
    /**
     * @brief Print detailed analysis report
     */
    void printAnalysisReport();
    
    /**
     * @brief Export measurements to CSV format
     * @param filename Output filename
     * @return true if successful
     */
    bool exportToCSV(const char* filename);
    
    /**
     * @brief Configure anomaly detection thresholds
     */
    void configureThresholds(uint32_t sleep_uA, uint32_t standby_uA, 
                           uint32_t active_uA, uint32_t critical_uA);
    
    /**
     * @brief Enable/disable real-time alerts
     */
    void enableRealTimeAlerts(bool enable) { realTimeAlerts = enable; }
    
    /**
     * @brief Clear measurement history
     */
    void clearMeasurements();
};

/**
 * @brief Sleep mode analyzer class
 */
class SleepModeAnalyzer {
private:
    PowerManager* powerManager;
    InfotainmentSystem* infotainmentSystem;
    
    struct SleepAttempt {
        uint32_t timestamp_ms;
        bool successful;
        uint32_t blockerMask;
        char blockerDescription[128];
    };
    
    static const uint32_t MAX_SLEEP_ATTEMPTS = 1000;
    SleepAttempt sleepAttempts[MAX_SLEEP_ATTEMPTS];
    uint32_t attemptIndex;
    uint32_t attemptCount;

public:
    SleepModeAnalyzer();
    
    bool initialize(PowerManager* pm, InfotainmentSystem* is);
    void analyzeSleepAttempt();
    void printSleepAnalysis();
    uint32_t getSleepSuccessRate() const;
    void identifySleepBlockers();
};

/**
 * @brief Wake-up event analyzer class
 */
class WakeupAnalyzer {
private:
    PowerManager* powerManager;
    
    struct WakeupEvent {
        uint32_t timestamp_ms;
        WakeupSource_t source;
        uint32_t sleepDuration_ms;
        bool validWakeup;
    };
    
    static const uint32_t MAX_WAKEUP_EVENTS = 1000;
    WakeupEvent wakeupEvents[MAX_WAKEUP_EVENTS];
    uint32_t eventIndex;
    uint32_t eventCount;

public:
    WakeupAnalyzer();
    
    bool initialize(PowerManager* pm);
    void recordWakeupEvent(WakeupSource_t source, uint32_t sleepDuration_ms);
    void analyzeWakeupPattern();
    void printWakeupAnalysis();
    uint32_t getWakeupFrequency() const;
    WakeupSource_t getMostCommonWakeupSource() const;
};

/**
 * @brief Battery health analyzer class
 */
class BatteryHealthAnalyzer {
private:
    struct BatteryReading {
        uint32_t timestamp_ms;
        uint32_t voltage_mV;
        uint32_t current_mA;
        int8_t temperature_C;
        uint32_t capacity_mAh;
    };
    
    static const uint32_t MAX_BATTERY_READINGS = 1000;
    BatteryReading batteryReadings[MAX_BATTERY_READINGS];
    uint32_t readingIndex;
    uint32_t readingCount;
    
    uint32_t nominalCapacity_mAh;
    uint32_t currentCapacity_mAh;

public:
    BatteryHealthAnalyzer();
    
    void recordBatteryReading(uint32_t voltage_mV, uint32_t current_mA, 
                             int8_t temperature_C);
    void analyzeBatteryHealth();
    uint32_t getBatteryHealthPercentage() const;
    uint32_t getEstimatedLifetime_cycles() const;
    void printBatteryHealthReport();
};

#endif // POWER_MONITOR_H