/**
 * @file InfotainmentSystem.cpp
 * @brief Main Infotainment System Implementation with Battery Drain Scenarios
 * @details Shows common battery drain issues in infotainment systems
 * @author Battery Drain Case Study  
 * @date November 2024
 */

#include "InfotainmentSystem.h"
#include <cstring>
#include <ctime>

// Simulate system time
static uint32_t getSystemTime_ms() {
    return static_cast<uint32_t>(clock() * 1000 / CLOCKS_PER_SEC);
}

// Simulate hardware register access
static void writeHardwareRegister(uint32_t address, uint32_t value) {
    (void)address; (void)value;
}

static uint32_t readHardwareRegister(uint32_t address) {
    (void)address;
    return 0x12345678;
}

//=============================================================================
// AudioSubsystem Implementation
//=============================================================================

AudioSubsystem::AudioSubsystem() :
    currentState(AUDIO_OFF),
    currentSource(MEDIA_SOURCE_NONE),
    volume(50),
    equalizerActive(false),
    noiseReductionActive(false),
    surroundSoundActive(false),
    dspAlwaysOn(false),
    backgroundAudioProc(false),
    continuousDecoding(false)
{
}

AudioSubsystem::~AudioSubsystem() {
    shutdown();
}

bool AudioSubsystem::initialize() {
    // Initialize audio hardware
    writeHardwareRegister(0x60000000, 0x00000001); // Enable audio controller
    writeHardwareRegister(0x60000004, volume);      // Set initial volume
    
    currentState = AUDIO_STANDBY;
    return true;
}

void AudioSubsystem::update() {
    // BATTERY DRAIN BUG #1: DSP Always On
    if (dspAlwaysOn) {
        // BUG: DSP remains active even when no audio is playing
        // This consumes ~50mA continuously
        writeHardwareRegister(0x60000008, 0x00000001); // Keep DSP active
    }
    
    // BATTERY DRAIN BUG #2: Background Audio Processing
    if (backgroundAudioProc && currentState == AUDIO_OFF) {
        // BUG: Audio processing continues even when audio is off
        // This consumes ~30mA continuously
        writeHardwareRegister(0x6000000C, 0x00000001); // Background processing
    }
    
    // BATTERY DRAIN BUG #3: Continuous Media Decoding
    if (continuousDecoding && currentSource != MEDIA_SOURCE_NONE) {
        // BUG: Media decoder stays active even when paused
        // This consumes ~40mA continuously
        writeHardwareRegister(0x60000010, 0x00000001); // Continuous decoding
    }
    
    // PROPER IMPLEMENTATION: Only process when needed
    if (currentState == AUDIO_PLAYING) {
        // Normal audio processing when actually playing
        writeHardwareRegister(0x60000014, 0x00000001);
    } else if (currentState == AUDIO_OFF) {
        // PROPER: Turn off processing when not needed
        writeHardwareRegister(0x60000008, 0x00000000); // Disable DSP
        writeHardwareRegister(0x6000000C, 0x00000000); // Disable background proc
        writeHardwareRegister(0x60000010, 0x00000000); // Disable decoding
    }
}

void AudioSubsystem::enterLowPowerMode() {
    // Properly shutdown audio subsystem for sleep
    currentState = AUDIO_OFF;
    dspAlwaysOn = false;         // CRITICAL: Turn off DSP
    backgroundAudioProc = false;  // CRITICAL: Stop background processing
    continuousDecoding = false;   // CRITICAL: Stop continuous decoding
    
    writeHardwareRegister(0x60000000, 0x00000000); // Disable audio controller
}

void AudioSubsystem::exitLowPowerMode() {
    // Restore audio subsystem from sleep
    initialize();
    currentState = AUDIO_STANDBY;
}

uint32_t AudioSubsystem::getCurrentConsumption() const {
    uint32_t consumption = 0;
    
    switch (currentState) {
        case AUDIO_OFF:
            consumption = 1000; // 1mA baseline
            break;
        case AUDIO_STANDBY:
            consumption = 5000; // 5mA in standby
            break;
        case AUDIO_PLAYING:
            consumption = 35000; // 35mA when playing
            break;
        case AUDIO_PROCESSING:
            consumption = 50000; // 50mA when processing
            break;
    }
    
    // Add consumption from problematic features
    if (dspAlwaysOn) consumption += 50000;        // +50mA for always-on DSP
    if (backgroundAudioProc) consumption += 30000; // +30mA for background processing
    if (continuousDecoding) consumption += 40000;  // +40mA for continuous decoding
    
    return consumption;
}

void AudioSubsystem::shutdown() {
    // Shutdown audio hardware and DSP
    writeHardwareRegister(0x100, 0x0); // Power down audio controller
    writeHardwareRegister(0x104, 0x0); // Disable DSP
    
    currentState = AUDIO_OFF;
    dspAlwaysOn = false;
    backgroundAudioProc = false;
    continuousDecoding = false;
}

//=============================================================================
// DisplaySubsystem Implementation  
//=============================================================================

DisplaySubsystem::DisplaySubsystem() :
    currentState(DISPLAY_OFF),
    brightness(80),
    backlightOn(false),
    lastUserInteraction(0),
    backlightTimeout(30000), // 30 seconds
    alwaysOn(false),
    animationsRunning(false),
    backgroundRendering(false)
{
}

DisplaySubsystem::~DisplaySubsystem() {
    shutdown();
}

bool DisplaySubsystem::initialize() {
    writeHardwareRegister(0x70000000, 0x00000001); // Enable display controller
    writeHardwareRegister(0x70000004, brightness);  // Set brightness
    
    currentState = DISPLAY_ON;
    backlightOn = true;
    lastUserInteraction = getSystemTime_ms();
    
    return true;
}

void DisplaySubsystem::update() {
    uint32_t currentTime = getSystemTime_ms();
    
    // BATTERY DRAIN BUG #4: Always On Display
    if (alwaysOn) {
        // BUG: Display never turns off, even during sleep
        // This consumes ~200mA continuously
        currentState = DISPLAY_ON;
        backlightOn = true;
        writeHardwareRegister(0x70000008, 0x00000001); // Keep backlight on
        return; // Skip timeout logic
    }
    
    // BATTERY DRAIN BUG #5: Continuous Animations
    if (animationsRunning) {
        // BUG: Animations run even when user not present
        // This consumes ~50mA continuously
        writeHardwareRegister(0x7000000C, 0x00000001); // Animation processing
    }
    
    // BATTERY DRAIN BUG #6: Background Rendering
    if (backgroundRendering && currentState == DISPLAY_OFF) {
        // BUG: Display rendering continues even when display is off
        // This consumes ~30mA continuously  
        writeHardwareRegister(0x70000010, 0x00000001); // Background rendering
    }
    
    // PROPER IMPLEMENTATION: Timeout handling
    if (!alwaysOn && (currentTime - lastUserInteraction) > backlightTimeout) {
        if (currentState != DISPLAY_OFF) {
            currentState = DISPLAY_OFF;
            backlightOn = false;
            animationsRunning = false;     // PROPER: Stop animations
            backgroundRendering = false;   // PROPER: Stop background rendering
            writeHardwareRegister(0x70000008, 0x00000000); // Turn off backlight
            writeHardwareRegister(0x7000000C, 0x00000000); // Stop animations
            writeHardwareRegister(0x70000010, 0x00000000); // Stop rendering
        }
    }
}

void DisplaySubsystem::enterLowPowerMode() {
    currentState = DISPLAY_OFF;
    backlightOn = false;
    alwaysOn = false;            // CRITICAL: Ensure display can turn off
    animationsRunning = false;   // CRITICAL: Stop animations
    backgroundRendering = false; // CRITICAL: Stop background rendering
    
    writeHardwareRegister(0x70000000, 0x00000000); // Disable display controller
}

uint32_t DisplaySubsystem::getCurrentConsumption() const {
    uint32_t consumption = 0;
    
    switch (currentState) {
        case DISPLAY_OFF:
            consumption = 2000; // 2mA for controller
            break;
        case DISPLAY_DIMMED:
            consumption = 50000; // 50mA dimmed
            break;
        case DISPLAY_ON:
            consumption = 150000; // 150mA normal
            break;
        case DISPLAY_FULL_BRIGHTNESS:
            consumption = 250000; // 250mA full brightness
            break;
    }
    
    // Add consumption from problematic features
    if (alwaysOn && currentState == DISPLAY_OFF) consumption += 200000; // +200mA if always on
    if (animationsRunning) consumption += 50000;   // +50mA for animations
    if (backgroundRendering) consumption += 30000; // +30mA for background rendering
    
    return consumption;
}

void DisplaySubsystem::exitLowPowerMode() {
    if (currentState == DISPLAY_OFF) {
        currentState = DISPLAY_DIMMED;
        writeHardwareRegister(0x200, 0x1); // Power on display
    }
}

void DisplaySubsystem::shutdown() {
    currentState = DISPLAY_OFF;
    alwaysOn = false;
    animationsRunning = false;
    backgroundRendering = false;
    writeHardwareRegister(0x200, 0x0); // Power down display
    writeHardwareRegister(0x204, 0x0); // Disable GPU
}

//=============================================================================
// BluetoothSubsystem Implementation
//=============================================================================

BluetoothSubsystem::BluetoothSubsystem() :
    currentState(CONN_DISABLED),
    discoverable(false),
    scanning(false),
    lastScanTime(0),
    scanInterval(30000), // 30 seconds
    continuousScanning(false),
    highPowerMode(false),
    backgroundSync(false)
{
}

BluetoothSubsystem::~BluetoothSubsystem() {
    shutdown();
}

bool BluetoothSubsystem::initialize() {
    writeHardwareRegister(0x80000000, 0x00000001); // Enable BT controller
    currentState = CONN_SCANNING;
    return true;
}

void BluetoothSubsystem::update() {
    uint32_t currentTime = getSystemTime_ms();
    
    // BATTERY DRAIN BUG #7: Continuous Scanning
    if (continuousScanning) {
        // BUG: Bluetooth never stops scanning for devices
        // This consumes ~30mA continuously
        scanning = true;
        writeHardwareRegister(0x80000004, 0x00000001); // Continuous scan
        return; // Skip normal scan logic
    }
    
    // BATTERY DRAIN BUG #8: High Power Mode Always On
    if (highPowerMode) {
        // BUG: Bluetooth uses maximum transmission power always
        // This consumes ~50mA instead of ~20mA
        writeHardwareRegister(0x80000008, 0x000000FF); // Max power
    }
    
    // BATTERY DRAIN BUG #9: Background Synchronization
    if (backgroundSync && currentState != CONN_ACTIVE) {
        // BUG: Background sync runs even when not connected
        // This consumes ~25mA continuously
        writeHardwareRegister(0x8000000C, 0x00000001); // Background sync
    }
    
    // PROPER IMPLEMENTATION: Periodic scanning
    if (!continuousScanning) {
        if (scanning && (currentTime - lastScanTime) > 10000) { // Scan for 10s
            scanning = false;
            writeHardwareRegister(0x80000004, 0x00000000); // Stop scan
        } else if (!scanning && (currentTime - lastScanTime) > scanInterval) {
            scanning = true;
            lastScanTime = currentTime;
            writeHardwareRegister(0x80000004, 0x00000001); // Start scan
        }
    }
}

void BluetoothSubsystem::enterLowPowerMode() {
    currentState = CONN_DISABLED;
    scanning = false;
    discoverable = false;
    continuousScanning = false;  // CRITICAL: Stop continuous scanning
    highPowerMode = false;       // CRITICAL: Use low power mode
    backgroundSync = false;      // CRITICAL: Stop background sync
    
    writeHardwareRegister(0x80000000, 0x00000000); // Disable BT controller
}

uint32_t BluetoothSubsystem::getCurrentConsumption() const {
    uint32_t consumption = 0;
    
    switch (currentState) {
        case CONN_DISABLED:
            consumption = 500; // 0.5mA disabled
            break;
        case CONN_SCANNING:
            consumption = 20000; // 20mA scanning
            break;
        case CONN_CONNECTED:
            consumption = 15000; // 15mA connected
            break;
        case CONN_ACTIVE:
            consumption = 25000; // 25mA active transfer
            break;
    }
    
    // Add consumption from problematic features
    if (continuousScanning) consumption += 30000;  // +30mA continuous scan
    if (highPowerMode) consumption += 30000;       // +30mA high power
    if (backgroundSync) consumption += 25000;      // +25mA background sync
    
    return consumption;
}

void BluetoothSubsystem::exitLowPowerMode() {
    if (currentState == CONN_DISABLED) {
        currentState = CONN_SCANNING;
    }
}

void BluetoothSubsystem::shutdown() {
    currentState = CONN_DISABLED;
    discoverable = false;
    scanning = false;
    continuousScanning = false;
    highPowerMode = false;
    backgroundSync = false;
}

//=============================================================================
// WiFiSubsystem Implementation
//=============================================================================

WiFiSubsystem::WiFiSubsystem() :
    currentState(CONN_DISABLED),
    scanning(false),
    hotspotMode(false),
    lastScanTime(0),
    scanInterval(60000), // 60 seconds
    continuousScanning(false),
    hotspotAlwaysOn(false),
    backgroundUpdates(false)
{
}

void WiFiSubsystem::update() {
    // BATTERY DRAIN BUG #10: Continuous WiFi Scanning
    if (continuousScanning) {
        // BUG: WiFi never stops scanning for networks
        // This consumes ~100mA continuously
        scanning = true;
        writeHardwareRegister(0x90000004, 0x00000001); // Continuous WiFi scan
    }
    
    // BATTERY DRAIN BUG #11: Hotspot Always On
    if (hotspotAlwaysOn) {
        // BUG: WiFi hotspot remains active even when not needed
        // This consumes ~150mA continuously
        hotspotMode = true;
        writeHardwareRegister(0x90000008, 0x00000001); // Hotspot always on
    }
    
    // BATTERY DRAIN BUG #12: Background Updates
    if (backgroundUpdates) {
        // BUG: System tries to download updates in background
        // This consumes ~80mA when active
        writeHardwareRegister(0x9000000C, 0x00000001); // Background updates
    }
}

void WiFiSubsystem::enterLowPowerMode() {
    currentState = CONN_DISABLED;
    scanning = false;
    hotspotMode = false;
    continuousScanning = false;  // CRITICAL: Stop continuous scanning
    hotspotAlwaysOn = false;     // CRITICAL: Turn off hotspot
    backgroundUpdates = false;   // CRITICAL: Stop background updates
    
    writeHardwareRegister(0x90000000, 0x00000000); // Disable WiFi
}

uint32_t WiFiSubsystem::getCurrentConsumption() const {
    uint32_t consumption = 0;
    
    switch (currentState) {
        case CONN_DISABLED:
            consumption = 1000; // 1mA disabled
            break;
        case CONN_SCANNING:
            consumption = 80000; // 80mA scanning
            break;
        case CONN_CONNECTED:
            consumption = 50000; // 50mA connected
            break;
        case CONN_ACTIVE:
            consumption = 120000; // 120mA active transfer
            break;
    }
    
    // Add consumption from problematic features
    if (continuousScanning) consumption += 100000; // +100mA continuous scan
    if (hotspotAlwaysOn) consumption += 150000;    // +150mA hotspot
    if (backgroundUpdates) consumption += 80000;   // +80mA updates
    
    return consumption;
}

bool WiFiSubsystem::initialize() {
    currentState = CONN_DISABLED;
    scanning = false;
    continuousScanning = false;
    hotspotAlwaysOn = false;
    backgroundUpdates = false;
    return true;
}

void WiFiSubsystem::exitLowPowerMode() {
    if (currentState == CONN_DISABLED) {
        currentState = CONN_SCANNING;
    }
}

void WiFiSubsystem::shutdown() {
    currentState = CONN_DISABLED;
    scanning = false;
    continuousScanning = false;
    hotspotAlwaysOn = false;
    backgroundUpdates = false;
}

//=============================================================================
// NavigationSubsystem Implementation
//=============================================================================

NavigationSubsystem::NavigationSubsystem() :
    gpsActive(false),
    navigationActive(false),
    lastPositionUpdate(0),
    updateInterval(1000), // 1 second
    alwaysTracking(false),
    backgroundLogging(false),
    highAccuracyMode(false)
{
}

void NavigationSubsystem::update() {
    // BATTERY DRAIN BUG #13: GPS Always Tracking
    if (alwaysTracking) {
        // BUG: GPS remains active even when navigation not in use
        // This consumes ~80mA continuously
        gpsActive = true;
        writeHardwareRegister(0xA0000004, 0x00000001); // GPS always on
    }
    
    // BATTERY DRAIN BUG #14: Background Location Logging
    if (backgroundLogging && !navigationActive) {
        // BUG: GPS logging continues in background
        // This consumes ~60mA continuously
        writeHardwareRegister(0xA0000008, 0x00000001); // Background logging
    }
    
    // BATTERY DRAIN BUG #15: High Accuracy Mode Always On
    if (highAccuracyMode) {
        // BUG: GPS uses maximum accuracy mode always
        // This consumes ~120mA instead of ~80mA
        writeHardwareRegister(0xA000000C, 0x000000FF); // High accuracy
    }
}

void NavigationSubsystem::enterLowPowerMode() {
    gpsActive = false;
    navigationActive = false;
    alwaysTracking = false;      // CRITICAL: Stop GPS tracking
    backgroundLogging = false;   // CRITICAL: Stop background logging
    highAccuracyMode = false;    // CRITICAL: Use low power GPS mode
    
    writeHardwareRegister(0xA0000000, 0x00000000); // Disable GPS
}

uint32_t NavigationSubsystem::getCurrentConsumption() const {
    uint32_t consumption = 0;
    
    if (gpsActive) {
        consumption = 80000; // 80mA for GPS
    } else {
        consumption = 1000; // 1mA when off
    }
    
    // Add consumption from problematic features
    if (alwaysTracking) consumption += 80000;      // +80mA always tracking
    if (backgroundLogging) consumption += 60000;   // +60mA background logging  
    if (highAccuracyMode) consumption += 40000;    // +40mA high accuracy
    
    return consumption;
}

bool NavigationSubsystem::initialize() {
    gpsActive = false;
    navigationActive = false;
    lastPositionUpdate = 0;
    updateInterval = 1000;
    alwaysTracking = false;
    backgroundLogging = false;
    highAccuracyMode = false;
    return true;
}

void NavigationSubsystem::exitLowPowerMode() {
    // Wake up GPS if needed
    if (navigationActive) {
        gpsActive = true;
    }
}

//=============================================================================
// InfotainmentSystem Implementation
//=============================================================================

InfotainmentSystem::InfotainmentSystem() :
    powerManager(nullptr),
    audioSystem(nullptr),
    displaySystem(nullptr),
    bluetoothSystem(nullptr),
    wifiSystem(nullptr),
    navigationSystem(nullptr),
    systemInitialized(false),
    lastMaintenanceTask(0),
    maintenanceTaskActive(false),
    diagnosticsRunning(false),
    updateInProgress(false)
{
}

InfotainmentSystem::~InfotainmentSystem() {
    shutdown();
}

bool InfotainmentSystem::initialize(PowerManager* pm) {
    powerManager = pm;
    
    // Create subsystems
    audioSystem = new AudioSubsystem();
    displaySystem = new DisplaySubsystem();
    bluetoothSystem = new BluetoothSubsystem();
    wifiSystem = new WiFiSubsystem();
    navigationSystem = new NavigationSubsystem();
    
    // Initialize all subsystems
    if (!audioSystem->initialize() ||
        !displaySystem->initialize() ||
        !bluetoothSystem->initialize() ||
        !wifiSystem->initialize() ||
        !navigationSystem->initialize()) {
        return false;
    }
    
    systemInitialized = true;
    lastMaintenanceTask = getSystemTime_ms();
    
    return true;
}

void InfotainmentSystem::mainTask() {
    if (!systemInitialized) return;
    
    uint32_t currentTime = getSystemTime_ms();
    
    // Update all subsystems
    audioSystem->update();
    displaySystem->update();
    bluetoothSystem->update(); 
    wifiSystem->update();
    navigationSystem->update();
    
    // BATTERY DRAIN BUG #16: Maintenance Task Always Running
    if (maintenanceTaskActive) {
        // BUG: Maintenance task runs continuously even during sleep
        // This consumes ~20mA continuously
        runMaintenanceTask();
    } else {
        // PROPER: Run maintenance periodically
        if ((currentTime - lastMaintenanceTask) > 300000) { // Every 5 minutes
            runMaintenanceTask();
            lastMaintenanceTask = currentTime;
        }
    }
    
    // BATTERY DRAIN BUG #17: Diagnostics Always Running
    if (diagnosticsRunning) {
        // BUG: Diagnostic routines run continuously
        // This consumes ~15mA continuously
        runDiagnostics();
    }
    
    // BATTERY DRAIN BUG #18: Update Process Stuck
    if (updateInProgress) {
        // BUG: Update process doesn't complete, system stays active
        // This prevents sleep mode entry
        powerManager->setBackgroundTaskActive(true); // Prevents sleep
    }
    
    // Communicate power consumption to power manager
    uint32_t totalConsumption = getTotalPowerConsumption();
    
    // Set power manager flags based on subsystem states
    powerManager->setAudioProcessingActive(
        audioSystem->getState() != AUDIO_OFF ||
        audioSystem->getCurrentConsumption() > 10000
    );
    
    powerManager->setDisplayBacklight(displaySystem->isBacklightOn());
    powerManager->setBluetoothScan(bluetoothSystem->isScanning());
    powerManager->setWifiScan(wifiSystem->isScanning());
    powerManager->setGpsActive(navigationSystem->isGpsActive());
    powerManager->setBackgroundTaskActive(maintenanceTaskActive || updateInProgress);
}

void InfotainmentSystem::enterLowPowerMode() {
    if (!systemInitialized) return;
    
    // Properly shutdown all subsystems for sleep
    audioSystem->enterLowPowerMode();
    displaySystem->enterLowPowerMode();
    bluetoothSystem->enterLowPowerMode();
    wifiSystem->enterLowPowerMode();
    navigationSystem->enterLowPowerMode();
    
    // Stop problematic background tasks
    maintenanceTaskActive = false;  // CRITICAL: Stop maintenance
    diagnosticsRunning = false;     // CRITICAL: Stop diagnostics
    updateInProgress = false;       // CRITICAL: Complete/abort updates
}

void InfotainmentSystem::exitLowPowerMode() {
    if (!systemInitialized) return;
    
    // Restore subsystems from sleep
    audioSystem->exitLowPowerMode();
    displaySystem->exitLowPowerMode();
    bluetoothSystem->exitLowPowerMode();
    wifiSystem->exitLowPowerMode();
    navigationSystem->exitLowPowerMode();
}

uint32_t InfotainmentSystem::getTotalPowerConsumption() const {
    if (!systemInitialized) return 0;
    
    uint32_t total = 0;
    
    total += audioSystem->getCurrentConsumption();
    total += displaySystem->getCurrentConsumption();
    total += bluetoothSystem->getCurrentConsumption();
    total += wifiSystem->getCurrentConsumption();
    total += navigationSystem->getCurrentConsumption();
    
    // Add consumption from system-level tasks
    if (maintenanceTaskActive) total += 20000;  // +20mA maintenance
    if (diagnosticsRunning) total += 15000;     // +15mA diagnostics
    if (updateInProgress) total += 50000;       // +50mA updates
    
    return total;
}

void InfotainmentSystem::runMaintenanceTask() {
    // Simulate maintenance task
    writeHardwareRegister(0xB0000000, 0x00000001);
}

void InfotainmentSystem::runDiagnostics() {
    // Simulate diagnostic routines
    writeHardwareRegister(0xB0000004, 0x00000001);
}

bool InfotainmentSystem::isSystemHealthy() const {
    return systemInitialized && 
           getTotalPowerConsumption() < 500000; // Less than 500mA
}

void InfotainmentSystem::shutdown() {
    if (audioSystem) audioSystem->shutdown();
    if (displaySystem) displaySystem->shutdown();
    if (bluetoothSystem) bluetoothSystem->shutdown();
    if (wifiSystem) wifiSystem->shutdown();
    
    systemInitialized = false;
}