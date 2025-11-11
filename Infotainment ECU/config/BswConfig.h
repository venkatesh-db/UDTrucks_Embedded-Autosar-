/**
 * @file BswConfig.h
 * @brief Basic Software Configuration for Power Management
 * @details Configuration for OS, memory, and low-level drivers
 * @author Battery Drain Case Study
 * @date November 2024
 */

#ifndef BSW_CONFIG_H
#define BSW_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

//=============================================================================
// Operating System Configuration
//=============================================================================

/**
 * @brief OS Task Configuration
 */
#define OS_TASK_COUNT                    8
#define OS_ISR_COUNT                     16
#define OS_ALARM_COUNT                   10
#define OS_COUNTER_COUNT                 4

/**
 * @brief Task IDs and Priorities
 */
typedef enum {
    TASK_POWER_MANAGER = 0,      // Highest priority
    TASK_INFOTAINMENT_MAIN = 1,
    TASK_COM_STACK = 2,
    TASK_DIAGNOSTICS = 3,
    TASK_AUDIO_PROCESSING = 4,
    TASK_DISPLAY_UPDATE = 5,
    TASK_CONNECTIVITY = 6,
    TASK_BACKGROUND = 7          // Lowest priority
} OsTaskId_t;

/**
 * @brief Task Configuration Structure
 */
typedef struct {
    OsTaskId_t taskId;
    uint8_t priority;            // 0 = highest, 7 = lowest
    uint32_t stackSize;          // Stack size in bytes
    uint32_t period_ms;          // Task period in milliseconds
    bool autostart;              // Auto-start task
    bool preemptive;             // Preemptive scheduling
    bool suspendable;            // Can be suspended for power saving
} OsTaskConfig_t;

/**
 * @brief Default task configurations
 */
#define OS_TASK_CONFIGS { \
    {TASK_POWER_MANAGER,    0, 2048, 100,  true,  true,  false}, \
    {TASK_INFOTAINMENT_MAIN,1, 4096, 50,   true,  true,  true},  \
    {TASK_COM_STACK,        2, 2048, 10,   true,  true,  true},  \
    {TASK_DIAGNOSTICS,      3, 2048, 1000, false, false, true},  \
    {TASK_AUDIO_PROCESSING, 4, 8192, 20,   false, false, true},  \
    {TASK_DISPLAY_UPDATE,   5, 4096, 50,   false, false, true},  \
    {TASK_CONNECTIVITY,     6, 3072, 200,  false, false, true},  \
    {TASK_BACKGROUND,       7, 1024, 5000, false, false, true}   \
}

//=============================================================================
// Memory Configuration
//=============================================================================

/**
 * @brief Memory Sections
 */
#define MEM_SECTION_CODE_SIZE           (256 * 1024)    // 256KB for code
#define MEM_SECTION_DATA_SIZE           (64 * 1024)     // 64KB for data
#define MEM_SECTION_STACK_SIZE          (32 * 1024)     // 32KB for stacks
#define MEM_SECTION_HEAP_SIZE           (128 * 1024)    // 128KB for heap

/**
 * @brief Memory Protection Configuration
 */
typedef enum {
    MEM_REGION_OS_CODE = 0,
    MEM_REGION_APP_CODE = 1,
    MEM_REGION_BSW_DATA = 2,
    MEM_REGION_APP_DATA = 3,
    MEM_REGION_SHARED_DATA = 4,
    MEM_REGION_PERIPHERAL = 5
} MemRegionId_t;

//=============================================================================
// Power Management Hardware Abstraction
//=============================================================================

/**
 * @brief Power Management Registers
 */
#define PWR_CTRL_BASE_ADDR              0x40000000
#define PWR_CTRL_POWER_MODE_REG         (PWR_CTRL_BASE_ADDR + 0x00)
#define PWR_CTRL_WAKEUP_SOURCE_REG      (PWR_CTRL_BASE_ADDR + 0x04)
#define PWR_CTRL_CLOCK_GATE_REG         (PWR_CTRL_BASE_ADDR + 0x08)
#define PWR_CTRL_VOLTAGE_REG            (PWR_CTRL_BASE_ADDR + 0x0C)

/**
 * @brief Power Domain Configuration
 */
typedef struct {
    uint8_t domainId;
    bool canBePoweredDown;       // Can this domain be powered down in sleep?
    uint32_t powerDownDelay_us;  // Delay before powering down
    uint32_t powerUpDelay_us;    // Delay after powering up
    bool hasWakeupCapability;    // Can this domain wake up the system?
} PowerDomainConfig_t;

/**
 * @brief Power Domains
 */
typedef enum {
    PWR_DOMAIN_CPU_CORE = 0,     // CPU core (always on)
    PWR_DOMAIN_MEMORY = 1,       // Main memory
    PWR_DOMAIN_AUDIO_DSP = 2,    // Audio DSP (can be powered down)
    PWR_DOMAIN_DISPLAY = 3,      // Display controller
    PWR_DOMAIN_CONNECTIVITY = 4, // WiFi/Bluetooth controllers
    PWR_DOMAIN_GPS = 5,          // GPS receiver
    PWR_DOMAIN_PERIPHERAL = 6    // General peripherals
} PowerDomainId_t;

//=============================================================================
// Clock Configuration
//=============================================================================

/**
 * @brief Clock Sources
 */
typedef enum {
    CLK_SOURCE_HSE = 0,          // High-Speed External (24 MHz)
    CLK_SOURCE_HSI = 1,          // High-Speed Internal (16 MHz)
    CLK_SOURCE_LSE = 2,          // Low-Speed External (32.768 kHz)
    CLK_SOURCE_LSI = 3,          // Low-Speed Internal (32 kHz)
    CLK_SOURCE_PLL = 4           // Phase-Locked Loop (up to 400 MHz)
} ClockSource_t;

/**
 * @brief Clock Configuration for Different Power Modes
 */
typedef struct {
    ClockSource_t cpuClockSource;
    uint32_t cpuFrequency_Hz;
    ClockSource_t peripheralClock;
    uint32_t peripheralFreq_Hz;
    bool enablePLL;
    bool enableHSE;
    bool enableLSE;
} ClockConfig_t;

/**
 * @brief Clock configurations for different power states
 */
#define CLOCK_CONFIG_ACTIVE   {CLK_SOURCE_PLL, 400000000, CLK_SOURCE_HSE, 100000000, true,  true,  true}
#define CLOCK_CONFIG_STANDBY  {CLK_SOURCE_HSE, 24000000,  CLK_SOURCE_HSE, 24000000,  false, true,  true}
#define CLOCK_CONFIG_SLEEP    {CLK_SOURCE_LSI, 32000,     CLK_SOURCE_LSI, 32000,     false, false, true}

//=============================================================================
// Watchdog Configuration
//=============================================================================

/**
 * @brief Watchdog Configuration
 */
typedef struct {
    bool enabled;                // Enable watchdog
    uint32_t timeout_ms;         // Watchdog timeout in milliseconds
    bool enableInSleepMode;      // Keep watchdog active in sleep mode
    bool windowedMode;           // Use windowed watchdog mode
    uint32_t windowStart_ms;     // Window start time
    uint32_t windowEnd_ms;       // Window end time
} WdgConfig_t;

/**
 * @brief Watchdog configurations for different power modes
 */
#define WDG_CONFIG_ACTIVE   {true,  5000,  false, false, 0,    0}     // 5s timeout, disabled in sleep
#define WDG_CONFIG_STANDBY  {true,  10000, false, false, 0,    0}     // 10s timeout
#define WDG_CONFIG_SLEEP    {false, 0,     false, false, 0,    0}     // Disabled in sleep

//=============================================================================
// Interrupt Configuration
//=============================================================================

/**
 * @brief Interrupt Priorities (0 = highest, 15 = lowest)
 */
#define IRQ_PRIORITY_SYSTEM_TIMER       0   // System timer (highest)
#define IRQ_PRIORITY_POWER_MANAGEMENT   1   // Power management events
#define IRQ_PRIORITY_CAN_WAKEUP         2   // CAN wake-up
#define IRQ_PRIORITY_USER_INPUT         3   // User input (buttons, touch)
#define IRQ_PRIORITY_AUDIO_DMA          4   // Audio DMA
#define IRQ_PRIORITY_DISPLAY_VSYNC      5   // Display vertical sync
#define IRQ_PRIORITY_COM_STACK          6   // Communication stack
#define IRQ_PRIORITY_BLUETOOTH          7   // Bluetooth events
#define IRQ_PRIORITY_WIFI               8   // WiFi events
#define IRQ_PRIORITY_GPS                9   // GPS events
#define IRQ_PRIORITY_DIAGNOSTICS        10  // Diagnostic events
#define IRQ_PRIORITY_BACKGROUND         15  // Background tasks (lowest)

/**
 * @brief Wake-up capable interrupts (these can wake from sleep mode)
 */
#define WAKEUP_IRQ_MASK ( \
    (1 << IRQ_PRIORITY_POWER_MANAGEMENT) | \
    (1 << IRQ_PRIORITY_CAN_WAKEUP) | \
    (1 << IRQ_PRIORITY_USER_INPUT) \
)

//=============================================================================
// DMA Configuration
//=============================================================================

/**
 * @brief DMA Channel Configuration
 */
typedef struct {
    uint8_t channelId;
    uint8_t priority;            // 0-3 (3 = highest)
    bool enableInSleepMode;      // Keep DMA active in sleep mode
    uint32_t powerConsumption_uA; // Estimated power consumption
} DmaChannelConfig_t;

/**
 * @brief DMA Channels
 */
typedef enum {
    DMA_CH_AUDIO_TX = 0,         // Audio transmission
    DMA_CH_AUDIO_RX = 1,         // Audio reception
    DMA_CH_DISPLAY = 2,          // Display frame buffer
    DMA_CH_CAN_TX = 3,           // CAN transmission
    DMA_CH_CAN_RX = 4,           // CAN reception
    DMA_CH_SPI_FLASH = 5,        // SPI flash operations
    DMA_CH_MEMORY_COPY = 6,      // Memory operations
    DMA_CH_RESERVED = 7          // Reserved channel
} DmaChannelId_t;

//=============================================================================
// Timer/PWM Configuration
//=============================================================================

/**
 * @brief Timer Configuration
 */
typedef struct {
    uint8_t timerId;
    uint32_t frequency_Hz;       // Timer frequency
    bool enableInSleepMode;      // Keep timer active in sleep
    bool wakeupCapable;          // Can wake up from sleep
    uint32_t powerConsumption_uA; // Power consumption when active
} TimerConfig_t;

/**
 * @brief System Timers
 */
typedef enum {
    TIMER_SYSTEM_TICK = 0,       // OS system tick (1ms)
    TIMER_POWER_MONITOR = 1,     // Power monitoring timer (100ms)
    TIMER_AUDIO_SAMPLE = 2,      // Audio sampling timer
    TIMER_DISPLAY_REFRESH = 3,   // Display refresh timer
    TIMER_CAN_TIMEOUT = 4,       // CAN timeout timer
    TIMER_WAKEUP = 5,            // Wake-up timer
    TIMER_PWM_BACKLIGHT = 6,     // Display backlight PWM
    TIMER_PWM_FAN = 7           // Cooling fan PWM
} TimerId_t;

//=============================================================================
// GPT (General Purpose Timer) Configuration
//=============================================================================

/**
 * @brief GPT Configuration for Power Management
 */
typedef struct {
    bool gptEnabled;
    uint32_t maxTimeout_ms;      // Maximum timeout value
    bool enableWakeup;           // Enable wake-up from timer
    bool enablePredefTimer;      // Enable predefined timer for OS
    uint32_t predefTimerFreq_Hz; // Predefined timer frequency
} GptConfig_t;

//=============================================================================
// Function Declarations
//=============================================================================

/**
 * @brief Initialize BSW modules
 */
void Bsw_Init(void);

/**
 * @brief Deinitialize BSW modules for sleep mode
 */
void Bsw_DeInit(void);

/**
 * @brief Prepare BSW for sleep mode
 */
void Bsw_PrepareSleep(void);

/**
 * @brief Wake up BSW from sleep mode
 */
void Bsw_WakeUp(void);

/**
 * @brief Configure power domains
 */
void Bsw_ConfigurePowerDomains(PowerDomainId_t domain, bool enable);

/**
 * @brief Set clock configuration
 */
void Bsw_SetClockConfig(const ClockConfig_t* config);

/**
 * @brief Configure watchdog for power mode
 */
void Bsw_ConfigureWatchdog(const WdgConfig_t* config);

/**
 * @brief Suspend non-critical tasks
 */
void Bsw_SuspendNonCriticalTasks(void);

/**
 * @brief Resume suspended tasks
 */
void Bsw_ResumeAllTasks(void);

/**
 * @brief Get current power consumption of BSW modules
 */
uint32_t Bsw_GetPowerConsumption(void);

//=============================================================================
// Critical Battery Drain Prevention
//=============================================================================

/**
 * @brief BSW Sleep Mode Checklist
 * These checks must pass before entering sleep mode
 */
#define BSW_SLEEP_CHECKLIST() \
    do { \
        /* Check all non-critical tasks are suspended */ \
        if (Os_GetTaskState(TASK_BACKGROUND) == RUNNING) { \
            /* WARNING: Background task still running! */ \
        } \
        /* Check DMA channels are idle */ \
        if ((DMA->ISR & 0xFF) != 0) { \
            /* WARNING: DMA channels still active! */ \
        } \
        /* Check timers are properly configured */ \
        if (TIM1->CR1 & TIM_CR1_CEN) { \
            /* WARNING: Non-essential timer still running! */ \
        } \
        /* Verify interrupt priorities */ \
        if (NVIC->ISER[0] & ~WAKEUP_IRQ_MASK) { \
            /* WARNING: Non-wakeup interrupts enabled! */ \
        } \
    } while(0)

/**
 * @brief Force BSW to minimal power state
 */
#define BSW_FORCE_LOW_POWER() \
    do { \
        /* Suspend all suspendable tasks */ \
        Os_SuspendAllInterrupts(); \
        /* Disable non-essential DMA channels */ \
        DMA1->CCR1 &= ~DMA_CCR_EN; \
        DMA1->CCR2 &= ~DMA_CCR_EN; \
        /* Set CPU to lowest frequency */ \
        RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI; \
        /* Enable only wake-up interrupts */ \
        NVIC->ICER[0] = ~WAKEUP_IRQ_MASK; \
        Os_ResumeAllInterrupts(); \
    } while(0)

#endif // BSW_CONFIG_H