/**
 * @file InfotainmentSystem.h
 * @brief Main Infotainment System Module for AUTOSAR ECU
 * @details Manages audio, display, connectivity, and user interface
 * @author Battery Drain Case Study
 * @date November 2024
 */

#ifndef INFOTAINMENT_SYSTEM_H
#define INFOTAINMENT_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include "../PowerManager/PowerManager.h"

/**
 * @brief Audio system states
 */
typedef enum {
    AUDIO_OFF = 0,
    AUDIO_STANDBY = 1,
    AUDIO_PLAYING = 2,
    AUDIO_PROCESSING = 3
} AudioState_t;

/**
 * @brief Display states
 */
typedef enum {
    DISPLAY_OFF = 0,
    DISPLAY_DIMMED = 1,
    DISPLAY_ON = 2,
    DISPLAY_FULL_BRIGHTNESS = 3
} DisplayState_t;

/**
 * @brief Connectivity states
 */
typedef enum {
    CONN_DISABLED = 0,
    CONN_SCANNING = 1,
    CONN_CONNECTED = 2,
    CONN_ACTIVE = 3
} ConnectivityState_t;

/**
 * @brief Media source types
 */
typedef enum {
    MEDIA_SOURCE_NONE = 0,
    MEDIA_SOURCE_RADIO = 1,
    MEDIA_SOURCE_USB = 2,
    MEDIA_SOURCE_BLUETOOTH = 3,
    MEDIA_SOURCE_STREAMING = 4
} MediaSource_t;

/**
 * @brief Audio subsystem class
 */
class AudioSubsystem {
private:
    AudioState_t currentState;
    MediaSource_t currentSource;
    uint8_t volume;
    bool equalizerActive;
    bool noiseReductionActive;
    bool surroundSoundActive;
    
    // PROBLEMATIC: These can cause battery drain
    bool dspAlwaysOn;           /**< DSP processing always active */
    bool backgroundAudioProc;   /**< Background audio processing */
    bool continuousDecoding;    /**< Continuous media decoding */

public:
    AudioSubsystem();
    ~AudioSubsystem();
    
    bool initialize();
    void update();
    void shutdown();
    
    void setVolume(uint8_t vol);
    void setMediaSource(MediaSource_t source);
    void play();
    void pause();
    void stop();
    
    // Power management
    void enterLowPowerMode();
    void exitLowPowerMode();
    uint32_t getCurrentConsumption() const;
    
    // Getters
    AudioState_t getState() const { return currentState; }
    MediaSource_t getMediaSource() const { return currentSource; }
    uint8_t getVolume() const { return volume; }
    
    // PROBLEMATIC METHODS
    void enableDspAlwaysOn(bool enable) { dspAlwaysOn = enable; }
    void enableBackgroundProcessing(bool enable) { backgroundAudioProc = enable; }
    void enableContinuousDecoding(bool enable) { continuousDecoding = enable; }
};

/**
 * @brief Display subsystem class
 */
class DisplaySubsystem {
private:
    DisplayState_t currentState;
    uint8_t brightness;
    bool backlightOn;
    uint32_t lastUserInteraction;
    uint32_t backlightTimeout;
    
    // PROBLEMATIC: These can cause battery drain
    bool alwaysOn;              /**< Display never turns off */
    bool animationsRunning;     /**< Continuous animations */
    bool backgroundRendering;   /**< Background rendering active */

public:
    DisplaySubsystem();
    ~DisplaySubsystem();
    
    bool initialize();
    void update();
    void shutdown();
    
    void setBrightness(uint8_t brightness);
    void setBacklight(bool on);
    void registerUserInteraction();
    
    // Power management
    void enterLowPowerMode();
    void exitLowPowerMode();
    uint32_t getCurrentConsumption() const;
    
    // Getters
    DisplayState_t getState() const { return currentState; }
    uint8_t getBrightness() const { return brightness; }
    bool isBacklightOn() const { return backlightOn; }
    
    // PROBLEMATIC METHODS
    void setAlwaysOn(bool enable) { alwaysOn = enable; }
    void enableAnimations(bool enable) { animationsRunning = enable; }
    void enableBackgroundRendering(bool enable) { backgroundRendering = enable; }
};

/**
 * @brief Bluetooth subsystem class
 */
class BluetoothSubsystem {
private:
    ConnectivityState_t currentState;
    bool discoverable;
    bool scanning;
    uint32_t lastScanTime;
    uint32_t scanInterval;
    
    // PROBLEMATIC: These can cause battery drain
    bool continuousScanning;    /**< Never stops scanning */
    bool highPowerMode;         /**< Always use high power */
    bool backgroundSync;        /**< Background synchronization */

public:
    BluetoothSubsystem();
    ~BluetoothSubsystem();
    
    bool initialize();
    void update();
    void shutdown();
    
    void startScanning();
    void stopScanning();
    void setDiscoverable(bool discoverable);
    bool connect(const char* deviceAddress);
    void disconnect();
    
    // Power management
    void enterLowPowerMode();
    void exitLowPowerMode();
    uint32_t getCurrentConsumption() const;
    
    // Getters
    ConnectivityState_t getState() const { return currentState; }
    bool isScanning() const { return scanning; }
    bool isDiscoverable() const { return discoverable; }
    
    // PROBLEMATIC METHODS
    void enableContinuousScanning(bool enable) { continuousScanning = enable; }
    void enableHighPowerMode(bool enable) { highPowerMode = enable; }
    void enableBackgroundSync(bool enable) { backgroundSync = enable; }
};

/**
 * @brief WiFi subsystem class
 */
class WiFiSubsystem {
private:
    ConnectivityState_t currentState;
    bool scanning;
    bool hotspotMode;
    uint32_t lastScanTime;
    uint32_t scanInterval;
    
    // PROBLEMATIC: These can cause battery drain
    bool continuousScanning;    /**< Never stops WiFi scanning */
    bool hotspotAlwaysOn;       /**< Hotspot always active */
    bool backgroundUpdates;     /**< Background software updates */

public:
    WiFiSubsystem();
    ~WiFiSubsystem();
    
    bool initialize();
    void update();
    void shutdown();
    
    void startScanning();
    void stopScanning();
    void enableHotspot(bool enable);
    bool connect(const char* ssid, const char* password);
    void disconnect();
    
    // Power management
    void enterLowPowerMode();
    void exitLowPowerMode();
    uint32_t getCurrentConsumption() const;
    
    // Getters
    ConnectivityState_t getState() const { return currentState; }
    bool isScanning() const { return scanning; }
    bool isHotspotActive() const { return hotspotMode; }
    
    // PROBLEMATIC METHODS
    void enableContinuousScanning(bool enable) { continuousScanning = enable; }
    void enableHotspotAlwaysOn(bool enable) { hotspotAlwaysOn = enable; }
    void enableBackgroundUpdates(bool enable) { backgroundUpdates = enable; }
};

/**
 * @brief GPS/Navigation subsystem class
 */
class NavigationSubsystem {
private:
    bool gpsActive;
    bool navigationActive;
    uint32_t lastPositionUpdate;
    uint32_t updateInterval;
    
    // PROBLEMATIC: These can cause battery drain
    bool alwaysTracking;        /**< GPS always active */
    bool backgroundLogging;     /**< Background location logging */
    bool highAccuracyMode;      /**< Always high accuracy mode */

public:
    NavigationSubsystem();
    ~NavigationSubsystem();
    
    bool initialize();
    void update();
    void shutdown();
    
    void startGps();
    void stopGps();
    void startNavigation();
    void stopNavigation();
    
    // Power management
    void enterLowPowerMode();
    void exitLowPowerMode();
    uint32_t getCurrentConsumption() const;
    
    // Getters
    bool isGpsActive() const { return gpsActive; }
    bool isNavigationActive() const { return navigationActive; }
    
    // PROBLEMATIC METHODS
    void enableAlwaysTracking(bool enable) { alwaysTracking = enable; }
    void enableBackgroundLogging(bool enable) { backgroundLogging = enable; }
    void enableHighAccuracyMode(bool enable) { highAccuracyMode = enable; }
};

/**
 * @brief Main Infotainment System class
 */
class InfotainmentSystem {
private:
    PowerManager* powerManager;
    AudioSubsystem* audioSystem;
    DisplaySubsystem* displaySystem;
    BluetoothSubsystem* bluetoothSystem;
    WiFiSubsystem* wifiSystem;
    NavigationSubsystem* navigationSystem;
    
    bool systemInitialized;
    uint32_t lastMaintenanceTask;
    
    // PROBLEMATIC: Global flags that can prevent sleep
    bool maintenanceTaskActive;
    bool diagnosticsRunning;
    bool updateInProgress;

public:
    InfotainmentSystem();
    ~InfotainmentSystem();
    
    bool initialize(PowerManager* pm);
    void mainTask();
    void shutdown();
    
    // Subsystem access
    AudioSubsystem* getAudioSystem() { return audioSystem; }
    DisplaySubsystem* getDisplaySystem() { return displaySystem; }
    BluetoothSubsystem* getBluetoothSystem() { return bluetoothSystem; }
    WiFiSubsystem* getWifiSystem() { return wifiSystem; }
    NavigationSubsystem* getNavigationSystem() { return navigationSystem; }
    
    // Power management
    void enterLowPowerMode();
    void exitLowPowerMode();
    uint32_t getTotalPowerConsumption() const;
    
    // System health
    void runMaintenanceTask();
    void runDiagnostics();
    bool isSystemHealthy() const;
    
    // PROBLEMATIC METHODS
    void enableMaintenanceTask(bool enable) { maintenanceTaskActive = enable; }
    void enableDiagnostics(bool enable) { diagnosticsRunning = enable; }
    void setUpdateInProgress(bool inProgress) { updateInProgress = inProgress; }
};

#endif // INFOTAINMENT_SYSTEM_H