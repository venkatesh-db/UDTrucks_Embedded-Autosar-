/**
 * @file simulation_main.c
 * @brief PC Simulation of AUTOSAR ABS Malfunction Detection System
 * @author Generated for demonstration purposes
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

// Simplified data types for PC simulation
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef float float32;
typedef uint8 boolean;

#define TRUE 1
#define FALSE 0
#define E_OK 0
#define E_NOT_OK 1

// Simplified speed sensor types
typedef enum {
    WHEEL_FRONT_LEFT = 0,
    WHEEL_FRONT_RIGHT = 1,
    WHEEL_REAR_LEFT = 2,
    WHEEL_REAR_RIGHT = 3,
    WHEEL_MAX = 4
} WheelPosition_t;

typedef struct {
    float32 wheelSpeed;
    float32 wheelSpeedRaw;
    float32 accelerationX;
    boolean speedValid;
    uint8 qualityFactor;
} SpeedData_t;

typedef struct {
    float32 correctionFactor;
    float32 offsetValue;
    boolean calibrationValid;
} SpeedSensorCalibration_t;

typedef enum {
    ABS_MALFUNCTION_NONE = 0,
    ABS_MALFUNCTION_SPEED_SENSOR_MISCALIBRATION = 1,
    ABS_MALFUNCTION_SPEED_DIFFERENCE_EXCESSIVE = 2,
    ABS_MALFUNCTION_ACCELERATION_IMPLAUSIBLE = 3
} ABS_MalfunctionType_t;

typedef struct {
    ABS_MalfunctionType_t malfunctionType;
    boolean isActive;
    WheelPosition_t affectedWheel;
    float32 deviationValue;
    boolean confirmedMalfunction;
} ABS_MalfunctionStatus_t;

// Global simulation data
static SpeedData_t g_WheelSpeeds[WHEEL_MAX];
static SpeedSensorCalibration_t g_Calibrations[WHEEL_MAX];
static ABS_MalfunctionStatus_t g_MalfunctionStatus[WHEEL_MAX];
static float32 g_VehicleSpeed = 60.0f; // km/h
static uint32 g_SimulationStep = 0;

// Simulation functions
void InitializeSimulation(void);
void SimulateSpeedSensors(void);
void DetectMalfunctions(void);
void PrintStatus(void);
void SimulateMiscalibration(void);

int main(void)
{
    printf("🚗 AUTOSAR ABS Malfunction Detection Simulation\n");
    printf("===============================================\n\n");
    
    InitializeSimulation();
    
    printf("Starting simulation...\n");
    printf("Vehicle speed: %.1f km/h\n", g_VehicleSpeed);
    printf("Monitoring for malfunctions...\n\n");
    
    // Simulation loop
    for (int i = 0; i < 200; i++) {
        g_SimulationStep = i;
        
        // Simulate normal operation for first 50 steps
        if (i == 50) {
            printf("\n🔧 Introducing miscalibration in Front Left sensor...\n\n");
            SimulateMiscalibration();
        }
        
        // Simulate speed sensor readings
        SimulateSpeedSensors();
        
        // Run malfunction detection
        DetectMalfunctions();
        
        // Print status every 10 steps
        if (i % 10 == 0) {
            PrintStatus();
        }
        
        // Sleep for visualization
        usleep(100000); // 100ms
    }
    
    printf("\n✅ Simulation completed!\n");
    return 0;
}

void InitializeSimulation(void)
{
    srand(time(NULL));
    
    // Initialize calibrations
    for (int i = 0; i < WHEEL_MAX; i++) {
        g_Calibrations[i].correctionFactor = 1.0f;
        g_Calibrations[i].offsetValue = 0.0f;
        g_Calibrations[i].calibrationValid = TRUE;
        
        // Initialize wheel speeds
        g_WheelSpeeds[i].wheelSpeed = g_VehicleSpeed;
        g_WheelSpeeds[i].wheelSpeedRaw = g_VehicleSpeed;
        g_WheelSpeeds[i].accelerationX = 0.0f;
        g_WheelSpeeds[i].speedValid = TRUE;
        g_WheelSpeeds[i].qualityFactor = 100;
        
        // Initialize malfunction status
        g_MalfunctionStatus[i].malfunctionType = ABS_MALFUNCTION_NONE;
        g_MalfunctionStatus[i].isActive = FALSE;
        g_MalfunctionStatus[i].affectedWheel = i;
        g_MalfunctionStatus[i].deviationValue = 0.0f;
        g_MalfunctionStatus[i].confirmedMalfunction = FALSE;
    }
}

void SimulateSpeedSensors(void)
{
    // Add some realistic noise and variation
    for (int i = 0; i < WHEEL_MAX; i++) {
        // Add random noise (±2 km/h)
        float32 noise = ((float32)rand() / RAND_MAX - 0.5f) * 4.0f;
        
        // Simulate raw speed with noise
        g_WheelSpeeds[i].wheelSpeedRaw = g_VehicleSpeed + noise;
        
        // Apply calibration
        g_WheelSpeeds[i].wheelSpeed = g_WheelSpeeds[i].wheelSpeedRaw * 
                                      g_Calibrations[i].correctionFactor + 
                                      g_Calibrations[i].offsetValue;
        
        // Simulate acceleration (derivative of speed)
        static float32 lastSpeeds[WHEEL_MAX] = {0};
        g_WheelSpeeds[i].accelerationX = (g_WheelSpeeds[i].wheelSpeed - lastSpeeds[i]) * 10.0f; // Assuming 100ms cycle
        lastSpeeds[i] = g_WheelSpeeds[i].wheelSpeed;
        
        // Validate speed data
        g_WheelSpeeds[i].speedValid = (g_WheelSpeeds[i].wheelSpeed >= 0.0f) && 
                                      (g_WheelSpeeds[i].wheelSpeed <= 300.0f);
        
        // Calculate quality factor based on calibration
        float32 calibrationError = fabsf(g_Calibrations[i].correctionFactor - 1.0f);
        g_WheelSpeeds[i].qualityFactor = (uint8)(100.0f * (1.0f - calibrationError));
        if (g_WheelSpeeds[i].qualityFactor > 100) g_WheelSpeeds[i].qualityFactor = 100;
    }
}

void DetectMalfunctions(void)
{
    // Calculate median speed for reference
    float32 speeds[WHEEL_MAX];
    for (int i = 0; i < WHEEL_MAX; i++) {
        speeds[i] = g_WheelSpeeds[i].wheelSpeed;
    }
    
    // Simple bubble sort for median calculation
    for (int i = 0; i < WHEEL_MAX - 1; i++) {
        for (int j = 0; j < WHEEL_MAX - i - 1; j++) {
            if (speeds[j] > speeds[j + 1]) {
                float32 temp = speeds[j];
                speeds[j] = speeds[j + 1];
                speeds[j + 1] = temp;
            }
        }
    }
    float32 medianSpeed = (speeds[1] + speeds[2]) / 2.0f;
    
    // Check each wheel for malfunctions
    for (int i = 0; i < WHEEL_MAX; i++) {
        // Reset malfunction status
        g_MalfunctionStatus[i].malfunctionType = ABS_MALFUNCTION_NONE;
        g_MalfunctionStatus[i].isActive = FALSE;
        g_MalfunctionStatus[i].deviationValue = 0.0f;
        
        // Check calibration drift
        float32 calibrationDrift = fabsf(g_Calibrations[i].correctionFactor - 1.0f) * 100.0f;
        if (calibrationDrift > 10.0f) { // 10% threshold
            g_MalfunctionStatus[i].malfunctionType = ABS_MALFUNCTION_SPEED_SENSOR_MISCALIBRATION;
            g_MalfunctionStatus[i].isActive = TRUE;
            g_MalfunctionStatus[i].deviationValue = calibrationDrift;
            g_MalfunctionStatus[i].confirmedMalfunction = (g_SimulationStep > 55); // Confirm after 5 cycles
        }
        
        // Check speed difference
        float32 speedDifference = fabsf(g_WheelSpeeds[i].wheelSpeed - medianSpeed);
        if (speedDifference > 20.0f) { // 20 km/h threshold
            if (g_MalfunctionStatus[i].malfunctionType == ABS_MALFUNCTION_NONE) {
                g_MalfunctionStatus[i].malfunctionType = ABS_MALFUNCTION_SPEED_DIFFERENCE_EXCESSIVE;
                g_MalfunctionStatus[i].isActive = TRUE;
                g_MalfunctionStatus[i].deviationValue = speedDifference;
                g_MalfunctionStatus[i].confirmedMalfunction = TRUE;
            }
        }
        
        // Check acceleration plausibility
        if (fabsf(g_WheelSpeeds[i].accelerationX) > 15.0f) { // 15 m/s² threshold
            if (g_MalfunctionStatus[i].malfunctionType == ABS_MALFUNCTION_NONE) {
                g_MalfunctionStatus[i].malfunctionType = ABS_MALFUNCTION_ACCELERATION_IMPLAUSIBLE;
                g_MalfunctionStatus[i].isActive = TRUE;
                g_MalfunctionStatus[i].deviationValue = fabsf(g_WheelSpeeds[i].accelerationX);
                g_MalfunctionStatus[i].confirmedMalfunction = TRUE;
            }
        }
    }
}

void PrintStatus(void)
{
    const char* wheelNames[] = {"FL", "FR", "RL", "RR"};
    const char* malfunctionNames[] = {
        "None",
        "Miscalibration",
        "Speed Difference",
        "Acceleration Error"
    };
    
    printf("Step %03d | ", g_SimulationStep);
    
    // Print wheel speeds
    for (int i = 0; i < WHEEL_MAX; i++) {
        printf("%s:%.1f ", wheelNames[i], g_WheelSpeeds[i].wheelSpeed);
    }
    
    printf("| Malfunctions: ");
    
    // Print active malfunctions
    boolean anyMalfunction = FALSE;
    for (int i = 0; i < WHEEL_MAX; i++) {
        if (g_MalfunctionStatus[i].confirmedMalfunction) {
            printf("[%s:%s(%.1f)] ", 
                   wheelNames[i], 
                   malfunctionNames[g_MalfunctionStatus[i].malfunctionType],
                   g_MalfunctionStatus[i].deviationValue);
            anyMalfunction = TRUE;
        }
    }
    
    if (!anyMalfunction) {
        printf("None");
    }
    
    printf("\n");
}

void SimulateMiscalibration(void)
{
    // Introduce miscalibration in front left sensor
    g_Calibrations[WHEEL_FRONT_LEFT].correctionFactor = 1.15f; // 15% error
    g_Calibrations[WHEEL_FRONT_LEFT].calibrationValid = FALSE;
}