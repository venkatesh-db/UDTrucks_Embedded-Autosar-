/**
 * @file main.cpp
 * @brief AUTOSAR Infotainment ECU Battery Drain Case Study
 * @details Real-world overnight battery drain scenarios with debugging tools
 * @version 1.0.0
 * @date November 2024
 * @author Battery Drain Case Study Team
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>
#include <string>

#include "src/PowerManager/PowerManager.h"
#include "src/InfotainmentSystem/InfotainmentSystem.h"
#include "src/Diagnostics/PowerMonitor.h"

// Global flag for graceful shutdown
volatile bool g_running = true;
PowerMonitor* g_monitor = nullptr;

/**
 * @brief Signal handler for graceful shutdown
 */
void signalHandler(int signal) {
    std::cout << "\n\nReceived signal " << signal << " - shutting down gracefully..." << std::endl;
    g_running = false;
    
    if (g_monitor) {
        g_monitor->stopLogging();
        std::cout << "Power monitoring stopped." << std::endl;
    }
}

// Forward declarations
void runInteractiveMode(PowerManager& pm, InfotainmentSystem& is, PowerMonitor& monitor);
void runScenarios(PowerManager& pm, InfotainmentSystem& is, PowerMonitor& monitor);
void runDashboard(PowerManager& pm, InfotainmentSystem& is, PowerMonitor& monitor);
void runSimulation(PowerManager& pm, InfotainmentSystem& is, PowerMonitor& monitor);
void printUsage(const char* programName);

int main(int argc, char* argv[]) {
    std::cout << "=== AUTOSAR Infotainment ECU Battery Drain Case Study ===" << std::endl;
    std::cout << "Real-world overnight battery drain scenarios and debugging tools\n" << std::endl;
    
    // Set up signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        // Initialize core components
        PowerManager powerManager;
        InfotainmentSystem infotainmentSystem;
        PowerMonitor powerMonitor;
        
        g_monitor = &powerMonitor;
        
        // Configure power management
        PowerConfig_t config = {
            .sleepTimeout_ms = 300000,        // 5 minutes
            .deepSleepTimeout_ms = 1800000,   // 30 minutes
            .wakeupSources = WAKEUP_IGNITION | WAKEUP_CAN_NETWORK | WAKEUP_USER_INPUT,
            .enablePeriodicWakeup = true,
            .periodicWakeupInterval_ms = 3600000, // 1 hour
            .enableNetworkWakeup = true,
            .enableRemoteWakeup = false
        };
        
        // Initialize systems
        std::cout << "Initializing power manager..." << std::endl;
        if (!powerManager.initialize(config)) {
            std::cerr << "Failed to initialize power manager!" << std::endl;
            return 1;
        }
        
        std::cout << "Initializing infotainment system..." << std::endl;
        if (!infotainmentSystem.initialize(&powerManager)) {
            std::cerr << "Failed to initialize infotainment system!" << std::endl;
            return 1;
        }
        
        std::cout << "Initializing power monitor..." << std::endl;
        if (!powerMonitor.initialize(&powerManager, &infotainmentSystem)) {
            std::cerr << "Failed to initialize power monitor!" << std::endl;
            return 1;
        }
        
        std::cout << "All systems initialized successfully!\n" << std::endl;
        
        // Parse command line arguments
        std::string mode = "interactive";
        if (argc > 1) {
            mode = argv[1];
        }
        
        // Run based on mode
        if (mode == "scenarios") {
            runScenarios(powerManager, infotainmentSystem, powerMonitor);
        }
        else if (mode == "dashboard") {
            runDashboard(powerManager, infotainmentSystem, powerMonitor);
        }
        else if (mode == "simulation") {
            runSimulation(powerManager, infotainmentSystem, powerMonitor);
        }
        else if (mode == "interactive" || mode == "help" || mode == "--help") {
            if (mode == "help" || mode == "--help") {
                printUsage(argv[0]);
                return 0;
            }
            runInteractiveMode(powerManager, infotainmentSystem, powerMonitor);
        }
        else {
            std::cerr << "Unknown mode: " << mode << std::endl;
            printUsage(argv[0]);
            return 1;
        }
        
    }
    catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown exception occurred!" << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Battery Drain Case Study Complete ===" << std::endl;
    return 0;
}

void runInteractiveMode(PowerManager& pm, InfotainmentSystem& is, PowerMonitor& monitor) {
    std::cout << "=== Interactive Mode ===" << std::endl;
    std::cout << "Choose an option:" << std::endl;
    std::cout << "1. Run battery drain scenarios" << std::endl;
    std::cout << "2. Run real-time dashboard" << std::endl;
    std::cout << "3. Run vehicle simulation" << std::endl;
    std::cout << "4. Run power consumption test" << std::endl;
    std::cout << "5. Export power data to CSV" << std::endl;
    std::cout << "0. Exit" << std::endl;
    
    while (g_running) {
        std::cout << "\nEnter choice (0-5): ";
        std::string input;
        std::getline(std::cin, input);
        
        if (input.empty()) continue;
        
        int choice = 0;
        try {
            choice = std::stoi(input);
        } catch (...) {
            std::cout << "Invalid input! Please enter a number 0-5." << std::endl;
            continue;
        }
        
        switch (choice) {
            case 0:
                g_running = false;
                break;
                
            case 1:
                runScenarios(pm, is, monitor);
                break;
                
            case 2:
                std::cout << "Starting dashboard (Press Ctrl+C to stop)..." << std::endl;
                runDashboard(pm, is, monitor);
                break;
                
            case 3:
                runSimulation(pm, is, monitor);
                break;
                
            case 4: {
                std::cout << "Running power consumption test..." << std::endl;
                monitor.startLogging(1000); // 1 second interval
                
                // Simulate some activity
                pm.setIgnitionState(true);
                std::this_thread::sleep_for(std::chrono::seconds(5));
                
                pm.setIgnitionState(false);
                std::this_thread::sleep_for(std::chrono::seconds(10));
                
                monitor.stopLogging();
                monitor.printAnalysisReport();
                break;
            }
            
            case 5: {
                std::cout << "Exporting power data..." << std::endl;
                if (monitor.exportToCSV("power_data.csv")) {
                    std::cout << "Data exported to power_data.csv" << std::endl;
                } else {
                    std::cout << "Export failed!" << std::endl;
                }
                break;
            }
            
            default:
                std::cout << "Invalid choice!" << std::endl;
                break;
        }
    }
}

void runScenarios(PowerManager& pm, InfotainmentSystem& is, PowerMonitor& monitor) {
    std::cout << "\n=== Running Battery Drain Scenarios ===" << std::endl;
    
    monitor.startLogging(1000);
    
    // Scenario 1: Audio DSP Stuck On
    std::cout << "\n--- SCENARIO 1: Audio DSP Stuck On ---" << std::endl;
    AudioSubsystem* audio = is.getAudioSystem();
    if (audio) {
        audio->enableDspAlwaysOn(true);
        audio->enableBackgroundProcessing(true);
        pm.setIgnitionState(false);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        uint32_t consumption = monitor.getCurrentConsumption();
        std::cout << "Bug reproduced - Consumption: " << consumption/1000 << " mA" << std::endl;
        
        audio->enterLowPowerMode();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        consumption = monitor.getCurrentConsumption();
        std::cout << "After fix - Consumption: " << consumption/1000 << " mA" << std::endl;
    }
    
    // Scenario 2: Display Never Sleeps
    std::cout << "\n--- SCENARIO 2: Display Never Sleeps ---" << std::endl;
    DisplaySubsystem* display = is.getDisplaySystem();
    if (display) {
        display->setAlwaysOn(true);
        display->enableAnimations(true);
        pm.setIgnitionState(false);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        uint32_t consumption = monitor.getCurrentConsumption();
        std::cout << "Bug reproduced - Consumption: " << consumption/1000 << " mA" << std::endl;
        
        display->enterLowPowerMode();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        consumption = monitor.getCurrentConsumption();
        std::cout << "After fix - Consumption: " << consumption/1000 << " mA" << std::endl;
    }
    
    // Scenario 3: Bluetooth Continuous Scanning
    std::cout << "\n--- SCENARIO 3: Bluetooth Continuous Scanning ---" << std::endl;
    BluetoothSubsystem* bluetooth = is.getBluetoothSystem();
    if (bluetooth) {
        bluetooth->enableContinuousScanning(true);
        bluetooth->enableHighPowerMode(true);
        pm.setIgnitionState(false);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        uint32_t consumption = monitor.getCurrentConsumption();
        std::cout << "Bug reproduced - Consumption: " << consumption/1000 << " mA" << std::endl;
        
        bluetooth->enterLowPowerMode();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        consumption = monitor.getCurrentConsumption();
        std::cout << "After fix - Consumption: " << consumption/1000 << " mA" << std::endl;
    }
    
    monitor.stopLogging();
    std::cout << "\nAll scenarios completed!" << std::endl;
    monitor.printAnalysisReport();
}

void runDashboard(PowerManager& pm, InfotainmentSystem& is, PowerMonitor& monitor) {
    monitor.startLogging(1000);
    pm.setIgnitionState(true);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    while (g_running) {
        pm.mainTask();
        is.mainTask();
        monitor.monitoringTask();
        
        static int counter = 0;
        if (++counter >= 20) { // Update every 2 seconds
            monitor.printPowerDashboard();
            counter = 0;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    monitor.stopLogging();
}

void runSimulation(PowerManager& pm, InfotainmentSystem& is, PowerMonitor& monitor) {
    std::cout << "\n=== Vehicle Operation Simulation ===" << std::endl;
    
    monitor.startLogging(1000);
    
    std::cout << "Simulating ignition on..." << std::endl;
    pm.setIgnitionState(true);
    pm.registerUserActivity();
    
    for (int i = 0; i < 10 && g_running; i++) {
        pm.mainTask();
        is.mainTask();
        monitor.monitoringTask();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "Simulating ignition off..." << std::endl;
    pm.setIgnitionState(false);
    
    for (int i = 0; i < 15 && g_running; i++) {
        pm.mainTask();
        is.mainTask();
        monitor.monitoringTask();
        
        if (pm.getCurrentState() == POWER_STATE_SLEEP) {
            std::cout << "Sleep mode entered after " << i << " seconds" << std::endl;
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    monitor.stopLogging();
    monitor.printAnalysisReport();
    
    // Estimate overnight battery drain
    uint32_t currentConsumption_mA = monitor.getCurrentConsumption() / 1000;
    uint32_t overnightDrain_mAh = (currentConsumption_mA * 8); // 8 hours
    
    std::cout << "\n=== Overnight Battery Drain Estimation ===" << std::endl;
    std::cout << "Current consumption: " << currentConsumption_mA << " mA" << std::endl;
    std::cout << "Estimated 8-hour drain: " << overnightDrain_mAh << " mAh" << std::endl;
    
    if (overnightDrain_mAh > 2000) {
        std::cout << "🚨 CRITICAL: Battery will be drained overnight!" << std::endl;
    } else if (overnightDrain_mAh > 500) {
        std::cout << "⚠️  WARNING: High overnight battery drain" << std::endl;
    } else if (overnightDrain_mAh > 80) {
        std::cout << "ℹ️  INFO: Moderate overnight battery drain" << std::endl;
    } else {
        std::cout << "✅ GOOD: Low overnight battery drain" << std::endl;
    }
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [mode]" << std::endl;
    std::cout << std::endl;
    std::cout << "Modes:" << std::endl;
    std::cout << "  interactive  Interactive mode with menu (default)" << std::endl;
    std::cout << "  scenarios    Run all battery drain scenarios" << std::endl;
    std::cout << "  dashboard    Real-time power monitoring dashboard" << std::endl;
    std::cout << "  simulation   Vehicle operation simulation" << std::endl;
    std::cout << "  help         Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << "                  # Interactive mode" << std::endl;
    std::cout << "  " << programName << " scenarios        # Run battery scenarios" << std::endl;
    std::cout << "  " << programName << " dashboard        # Real-time monitoring" << std::endl;
}