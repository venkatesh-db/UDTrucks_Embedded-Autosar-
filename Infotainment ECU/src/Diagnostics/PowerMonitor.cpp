/**
 * @file PowerMonitor.cpp
 * @brief Power Monitoring and Debugging Tools Implementation
 * @details Real-time monitoring, anomaly detection, and battery drain debugging
 * @author Battery Drain Case Study
 * @date November 2024
 */

#include "PowerMonitor.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <ctime>
#include <cmath>

//=============================================================================
// PowerMonitor Implementation
//=============================================================================

PowerMonitor::PowerMonitor() :
    powerManager(nullptr),
    infotainmentSystem(nullptr),
    measurementIndex(0),
    measurementCount(0),
    anomalyCount(0),
    measurementInterval_ms(1000),
    continuousLogging(false),
    anomalyDetection(true),
    realTimeAlerts(true),
    sleepThreshold_uA(THRESHOLD_SLEEP),
    standbyThreshold_uA(THRESHOLD_STANDBY),
    activeThreshold_uA(THRESHOLD_ACTIVE),
    criticalThreshold_uA(THRESHOLD_CRITICAL)
{
    memset(&analysisReport, 0, sizeof(analysisReport));
    memset(measurements, 0, sizeof(measurements));
    memset(currentAnomalies, 0, sizeof(currentAnomalies));
}

PowerMonitor::~PowerMonitor() {
    stopLogging();
}

bool PowerMonitor::initialize(PowerManager* pm, InfotainmentSystem* is) {
    powerManager = pm;
    infotainmentSystem = is;
    
    if (!pm || !is) {
        return false;
    }
    
    // Clear previous data
    clearMeasurements();
    
    std::cout << "Power Monitor initialized successfully" << std::endl;
    std::cout << "Monitoring thresholds:" << std::endl;
    std::cout << "  Sleep Mode: < " << sleepThreshold_uA/1000 << " mA" << std::endl;
    std::cout << "  Standby Mode: < " << standbyThreshold_uA/1000 << " mA" << std::endl;
    std::cout << "  Active Mode: < " << activeThreshold_uA/1000 << " mA" << std::endl;
    std::cout << "  Critical Threshold: > " << criticalThreshold_uA/1000 << " mA" << std::endl;
    
    return true;
}

void PowerMonitor::monitoringTask() {
    if (!powerManager || !infotainmentSystem) return;
    
    static uint32_t lastMeasurement = 0;
    uint32_t currentTime = clock();
    
    if (continuousLogging && 
        (currentTime - lastMeasurement) >= measurementInterval_ms) {
        
        PowerMeasurement_t measurement = takeMeasurement();
        
        if (anomalyDetection) {
            detectAnomalies(measurement);
        }
        
        lastMeasurement = currentTime;
    }
}

void PowerMonitor::startLogging(uint32_t interval_ms) {
    measurementInterval_ms = interval_ms;
    continuousLogging = true;
    
    std::cout << "Started continuous power logging (interval: " 
              << interval_ms << " ms)" << std::endl;
}

void PowerMonitor::stopLogging() {
    continuousLogging = false;
    
    if (measurementCount > 0) {
        updateAnalysisReport();
        std::cout << "Stopped power logging. Total measurements: " 
                  << measurementCount << std::endl;
    }
}

PowerMeasurement_t PowerMonitor::takeMeasurement() {
    PowerMeasurement_t measurement;
    
    measurement.timestamp_ms = clock();
    measurement.consumption_uA = getCurrentConsumption();
    measurement.batteryVoltage_mV = powerManager->getStatistics().batteryVoltage_mV;
    measurement.powerState = powerManager->getCurrentState();
    measurement.subsystemMask = getSubsystemMask();
    
    // Store measurement
    if (measurementCount < MAX_MEASUREMENTS) {
        measurements[measurementIndex] = measurement;
        measurementIndex = (measurementIndex + 1) % MAX_MEASUREMENTS;
        measurementCount++;
    } else {
        // Circular buffer - overwrite oldest
        measurements[measurementIndex] = measurement;
        measurementIndex = (measurementIndex + 1) % MAX_MEASUREMENTS;
    }
    
    return measurement;
}

PowerAnalysisReport_t PowerMonitor::generateReport() {
    updateAnalysisReport();
    return analysisReport;
}

void PowerMonitor::analyzeAnomalies() {
    anomalyCount = 0;
    
    // Check recent measurements for anomalies
    uint32_t recentCount = std::min(measurementCount, 100u); // Last 100 measurements
    
    for (uint32_t i = 0; i < recentCount; i++) {
        uint32_t idx = (measurementIndex - 1 - i + MAX_MEASUREMENTS) % MAX_MEASUREMENTS;
        detectAnomalies(measurements[idx]);
    }
    
    std::cout << "Anomaly analysis complete. Found " << anomalyCount 
              << " anomalies in recent measurements." << std::endl;
}

uint32_t PowerMonitor::getCurrentConsumption() const {
    if (!powerManager) return 0;
    
    // Get base consumption from power manager
    uint32_t baseConsumption = powerManager->getCurrentConsumption();
    
    // Add infotainment system consumption
    if (infotainmentSystem) {
        baseConsumption += infotainmentSystem->getTotalPowerConsumption();
    }
    
    return baseConsumption;
}

uint32_t PowerMonitor::getEstimatedBatteryLife(uint32_t batteryCapacity_mAh) const {
    uint32_t currentConsumption_mA = getCurrentConsumption() / 1000;
    
    if (currentConsumption_mA == 0) {
        return UINT32_MAX; // Infinite if no consumption
    }
    
    return batteryCapacity_mAh / currentConsumption_mA; // Hours remaining
}

bool PowerMonitor::isInProperSleepMode() const {
    if (!powerManager) return false;
    
    PowerState_t state = powerManager->getCurrentState();
    uint32_t consumption = getCurrentConsumption();
    
    return (state == POWER_STATE_SLEEP) && (consumption <= sleepThreshold_uA);
}

void PowerMonitor::printPowerDashboard() {
    if (!powerManager || !infotainmentSystem) {
        std::cout << "Power Monitor not initialized" << std::endl;
        return;
    }
    
    // Clear screen (ANSI escape code)
    std::cout << "\033[2J\033[1;1H";
    
    std::cout << "=== REAL-TIME POWER DASHBOARD ===" << std::endl;
    std::cout << "Timestamp: " << clock() << " ms" << std::endl << std::endl;
    
    // Current power state
    PowerState_t state = powerManager->getCurrentState();
    const char* stateStr = "";
    switch (state) {
        case POWER_STATE_OFF: stateStr = "OFF"; break;
        case POWER_STATE_RESET: stateStr = "RESET"; break;
        case POWER_STATE_STARTUP: stateStr = "STARTUP"; break;
        case POWER_STATE_RUN: stateStr = "RUN"; break;
        case POWER_STATE_SLEEP_PREPARE: stateStr = "SLEEP_PREPARE"; break;
        case POWER_STATE_SLEEP: stateStr = "SLEEP"; break;
        case POWER_STATE_SHUTDOWN: stateStr = "SHUTDOWN"; break;
    }
    
    std::cout << "Power State: " << stateStr << std::endl;
    
    // Power consumption
    uint32_t consumption_uA = getCurrentConsumption();
    uint32_t consumption_mA = consumption_uA / 1000;
    
    std::cout << "Current Consumption: " << consumption_mA << " mA (" 
              << consumption_uA << " μA)" << std::endl;
    
    // Battery status
    const PowerStats_t& stats = powerManager->getStatistics();
    std::cout << "Battery Voltage: " << stats.batteryVoltage_mV << " mV" << std::endl;
    
    // Estimated battery life
    uint32_t batteryLife = getEstimatedBatteryLife(70000); // 70Ah typical car battery
    std::cout << "Estimated Battery Life: ";
    if (batteryLife > 8760) { // > 1 year
        std::cout << "> 1 year" << std::endl;
    } else {
        std::cout << batteryLife << " hours" << std::endl;
    }
    
    // Active subsystems
    std::cout << std::endl << "Active Subsystems:" << std::endl;
    uint32_t subsystemMask = getSubsystemMask();
    if (subsystemMask & SUBSYSTEM_AUDIO) std::cout << "  • Audio System" << std::endl;
    if (subsystemMask & SUBSYSTEM_DISPLAY) std::cout << "  • Display System" << std::endl;
    if (subsystemMask & SUBSYSTEM_BLUETOOTH) std::cout << "  • Bluetooth" << std::endl;
    if (subsystemMask & SUBSYSTEM_WIFI) std::cout << "  • WiFi" << std::endl;
    if (subsystemMask & SUBSYSTEM_GPS) std::cout << "  • GPS/Navigation" << std::endl;
    if (subsystemMask & SUBSYSTEM_MAINTENANCE) std::cout << "  • Maintenance Tasks" << std::endl;
    if (subsystemMask & SUBSYSTEM_DIAGNOSTICS) std::cout << "  • Diagnostics" << std::endl;
    if (subsystemMask & SUBSYSTEM_UPDATES) std::cout << "  • Software Updates" << std::endl;
    
    if (subsystemMask == 0) {
        std::cout << "  None (Proper Sleep Mode)" << std::endl;
    }
    
    // Power consumption breakdown
    std::cout << std::endl << "Power Consumption Breakdown:" << std::endl;
    if (infotainmentSystem->getAudioSystem()) {
        uint32_t audioPower = infotainmentSystem->getAudioSystem()->getCurrentConsumption();
        std::cout << "  Audio: " << audioPower/1000 << " mA" << std::endl;
    }
    if (infotainmentSystem->getDisplaySystem()) {
        uint32_t displayPower = infotainmentSystem->getDisplaySystem()->getCurrentConsumption();
        std::cout << "  Display: " << displayPower/1000 << " mA" << std::endl;
    }
    if (infotainmentSystem->getBluetoothSystem()) {
        uint32_t btPower = infotainmentSystem->getBluetoothSystem()->getCurrentConsumption();
        std::cout << "  Bluetooth: " << btPower/1000 << " mA" << std::endl;
    }
    if (infotainmentSystem->getWifiSystem()) {
        uint32_t wifiPower = infotainmentSystem->getWifiSystem()->getCurrentConsumption();
        std::cout << "  WiFi: " << wifiPower/1000 << " mA" << std::endl;
    }
    if (infotainmentSystem->getNavigationSystem()) {
        uint32_t gpsPower = infotainmentSystem->getNavigationSystem()->getCurrentConsumption();
        std::cout << "  GPS: " << gpsPower/1000 << " mA" << std::endl;
    }
    
    // Alerts
    std::cout << std::endl << "Status:" << std::endl;
    if (consumption_uA > criticalThreshold_uA) {
        std::cout << "  🚨 CRITICAL: Excessive power consumption!" << std::endl;
    } else if (state == POWER_STATE_SLEEP && consumption_uA > sleepThreshold_uA) {
        std::cout << "  ⚠️  WARNING: High consumption in sleep mode" << std::endl;
    } else if (state == POWER_STATE_RUN && consumption_uA > activeThreshold_uA) {
        std::cout << "  ⚠️  WARNING: High consumption in active mode" << std::endl;
    } else {
        std::cout << "  ✅ Normal operation" << std::endl;
    }
    
    // Sleep mode analysis
    if (state == POWER_STATE_SLEEP) {
        if (isInProperSleepMode()) {
            std::cout << "  ✅ Proper sleep mode active" << std::endl;
        } else {
            std::cout << "  ❌ Improper sleep mode - subsystems active!" << std::endl;
        }
    }
    
    std::cout << std::endl << "Press Ctrl+C to stop monitoring..." << std::endl;
}

void PowerMonitor::printAnalysisReport() {
    PowerAnalysisReport_t report = generateReport();
    
    std::cout << "\n=== POWER ANALYSIS REPORT ===" << std::endl;
    std::cout << "Measurement Period: " << measurementCount << " samples" << std::endl;
    std::cout << "Total Energy Consumed: " << report.totalEnergy_mAh << " mAh" << std::endl;
    std::cout << "Average Consumption: " << report.averageConsumption_mA << " mA" << std::endl;
    std::cout << "Peak Consumption: " << report.peakConsumption_mA << " mA" << std::endl;
    std::cout << "Sleep Mode Usage: " << report.sleepModePercentage << "%" << std::endl;
    std::cout << "Wake-up Events: " << report.wakeupCount << std::endl;
    std::cout << "Detected Anomalies: " << report.anomalyCount << std::endl;
    std::cout << "Estimated Battery Life: " << report.estimatedBatteryLife_hours << " hours" << std::endl;
    
    // Recommendations
    std::cout << "\nRecommendations:" << std::endl;
    if (report.sleepModePercentage < 80) {
        std::cout << "  • Increase sleep mode usage (currently " << report.sleepModePercentage << "%)" << std::endl;
    }
    if (report.averageConsumption_mA > 100) {
        std::cout << "  • Investigate high average consumption (" << report.averageConsumption_mA << " mA)" << std::endl;
    }
    if (report.anomalyCount > 0) {
        std::cout << "  • Address " << report.anomalyCount << " detected anomalies" << std::endl;
    }
    if (report.wakeupCount > 20) {
        std::cout << "  • Reduce wake-up frequency (" << report.wakeupCount << " events)" << std::endl;
    }
}

bool PowerMonitor::exportToCSV(const char* filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    // Write CSV header
    file << "Timestamp_ms,Consumption_uA,Consumption_mA,Battery_mV,Power_State,";
    file << "Audio_Active,Display_Active,BT_Active,WiFi_Active,GPS_Active,";
    file << "Maintenance_Active,Diagnostics_Active,Updates_Active" << std::endl;
    
    // Write measurement data
    for (uint32_t i = 0; i < measurementCount; i++) {
        const PowerMeasurement_t& m = measurements[i];
        
        file << m.timestamp_ms << ","
             << m.consumption_uA << ","
             << m.consumption_uA/1000 << ","
             << m.batteryVoltage_mV << ","
             << static_cast<int>(m.powerState) << ","
             << ((m.subsystemMask & SUBSYSTEM_AUDIO) ? 1 : 0) << ","
             << ((m.subsystemMask & SUBSYSTEM_DISPLAY) ? 1 : 0) << ","
             << ((m.subsystemMask & SUBSYSTEM_BLUETOOTH) ? 1 : 0) << ","
             << ((m.subsystemMask & SUBSYSTEM_WIFI) ? 1 : 0) << ","
             << ((m.subsystemMask & SUBSYSTEM_GPS) ? 1 : 0) << ","
             << ((m.subsystemMask & SUBSYSTEM_MAINTENANCE) ? 1 : 0) << ","
             << ((m.subsystemMask & SUBSYSTEM_DIAGNOSTICS) ? 1 : 0) << ","
             << ((m.subsystemMask & SUBSYSTEM_UPDATES) ? 1 : 0) << std::endl;
    }
    
    file.close();
    std::cout << "Exported " << measurementCount << " measurements to " << filename << std::endl;
    return true;
}

void PowerMonitor::configureThresholds(uint32_t sleep_uA, uint32_t standby_uA, 
                                     uint32_t active_uA, uint32_t critical_uA) {
    sleepThreshold_uA = sleep_uA;
    standbyThreshold_uA = standby_uA;
    activeThreshold_uA = active_uA;
    criticalThreshold_uA = critical_uA;
    
    std::cout << "Updated power thresholds:" << std::endl;
    std::cout << "  Sleep: " << sleep_uA/1000 << " mA" << std::endl;
    std::cout << "  Standby: " << standby_uA/1000 << " mA" << std::endl;
    std::cout << "  Active: " << active_uA/1000 << " mA" << std::endl;
    std::cout << "  Critical: " << critical_uA/1000 << " mA" << std::endl;
}

void PowerMonitor::clearMeasurements() {
    measurementIndex = 0;
    measurementCount = 0;
    anomalyCount = 0;
    memset(&analysisReport, 0, sizeof(analysisReport));
    memset(measurements, 0, sizeof(measurements));
    memset(currentAnomalies, 0, sizeof(currentAnomalies));
}

void PowerMonitor::detectAnomalies(const PowerMeasurement_t& measurement) {
    if (anomalyCount >= 10) return; // Max 10 anomalies tracked
    
    // Anomaly 1: Excessive consumption in sleep mode
    if (measurement.powerState == POWER_STATE_SLEEP && 
        measurement.consumption_uA > sleepThreshold_uA) {
        logAnomaly(ANOMALY_EXCESSIVE_CONSUMPTION, 
                  "High consumption in sleep mode");
    }
    
    // Anomaly 2: Failed sleep entry (multiple subsystems active)
    uint32_t activeSubsystems = __builtin_popcount(measurement.subsystemMask);
    if (measurement.powerState == POWER_STATE_SLEEP && activeSubsystems > 2) {
        logAnomaly(ANOMALY_FAILED_SLEEP_ENTRY, 
                  "Multiple subsystems active during sleep");
    }
    
    // Anomaly 3: Critical consumption level
    if (measurement.consumption_uA > criticalThreshold_uA) {
        logAnomaly(ANOMALY_EXCESSIVE_CONSUMPTION, 
                  "Critical power consumption level");
    }
    
    // Anomaly 4: Battery voltage drop
    static uint32_t lastVoltage = measurement.batteryVoltage_mV;
    if (measurement.batteryVoltage_mV < lastVoltage - 500) { // 0.5V drop
        logAnomaly(ANOMALY_BATTERY_VOLTAGE_DROP, 
                  "Significant battery voltage drop");
    }
    lastVoltage = measurement.batteryVoltage_mV;
}

void PowerMonitor::updateAnalysisReport() {
    if (measurementCount == 0) return;
    
    uint64_t totalConsumption = 0;
    uint32_t peakConsumption = 0;
    uint32_t sleepModeCount = 0;
    uint32_t totalEnergy = 0;
    
    for (uint32_t i = 0; i < measurementCount; i++) {
        const PowerMeasurement_t& m = measurements[i];
        
        totalConsumption += m.consumption_uA;
        
        if (m.consumption_uA > peakConsumption) {
            peakConsumption = m.consumption_uA;
        }
        
        if (m.powerState == POWER_STATE_SLEEP) {
            sleepModeCount++;
        }
        
        // Calculate energy (simplified: assume 1 second intervals)
        totalEnergy += (m.consumption_uA / 1000) / 3600; // Convert to mAh
    }
    
    analysisReport.measurementCount = measurementCount;
    analysisReport.averageConsumption_mA = (totalConsumption / measurementCount) / 1000;
    analysisReport.peakConsumption_mA = peakConsumption / 1000;
    analysisReport.sleepModePercentage = (sleepModeCount * 100) / measurementCount;
    analysisReport.totalEnergy_mAh = totalEnergy;
    analysisReport.anomalyCount = anomalyCount;
    
    // Estimate battery life with 70Ah battery
    if (analysisReport.averageConsumption_mA > 0) {
        analysisReport.estimatedBatteryLife_hours = 70000 / analysisReport.averageConsumption_mA;
    } else {
        analysisReport.estimatedBatteryLife_hours = UINT32_MAX;
    }
}

uint32_t PowerMonitor::getSubsystemMask() const {
    uint32_t mask = 0;
    
    if (!infotainmentSystem) return mask;
    
    // Check each subsystem
    AudioSubsystem* audio = infotainmentSystem->getAudioSystem();
    if (audio && audio->getCurrentConsumption() > 5000) { // > 5mA
        mask |= SUBSYSTEM_AUDIO;
    }
    
    DisplaySubsystem* display = infotainmentSystem->getDisplaySystem();
    if (display && display->getCurrentConsumption() > 5000) {
        mask |= SUBSYSTEM_DISPLAY;
    }
    
    BluetoothSubsystem* bluetooth = infotainmentSystem->getBluetoothSystem();
    if (bluetooth && bluetooth->getCurrentConsumption() > 2000) { // > 2mA
        mask |= SUBSYSTEM_BLUETOOTH;
    }
    
    WiFiSubsystem* wifi = infotainmentSystem->getWifiSystem();
    if (wifi && wifi->getCurrentConsumption() > 5000) { // > 5mA
        mask |= SUBSYSTEM_WIFI;
    }
    
    NavigationSubsystem* gps = infotainmentSystem->getNavigationSystem();
    if (gps && gps->getCurrentConsumption() > 10000) { // > 10mA
        mask |= SUBSYSTEM_GPS;
    }
    
    // Check system-level tasks (simplified)
    if (infotainmentSystem->getTotalPowerConsumption() > 1000000) { // > 1A indicates heavy processing
        mask |= SUBSYSTEM_MAINTENANCE;
    }
    
    return mask;
}

void PowerMonitor::logAnomaly(PowerAnomaly_t anomaly, const char* description) {
    if (anomalyCount < 10) {
        currentAnomalies[anomalyCount++] = anomaly;
        
        if (realTimeAlerts) {
            std::cout << "⚠️  ANOMALY DETECTED: " << description 
                      << " (Type: " << static_cast<int>(anomaly) << ")" << std::endl;
        }
    }
}