#include "Rte.h"
#include "NvM.h"
#include <stdio.h>
#include <unistd.h>
#include "Dem.h"
#include <string.h>

// Forward declarations of application runnables
void Seatbelt_Sensor_IF_10ms(void);
void Occupancy_Sensor_IF_10ms(void);
void VehicleState_IF_10ms(void);
void SeatbeltWarning_Logic_10ms(void);
void SeatbeltWarning_Logic_Init(void);

// Simulation scenario helpers (simulate raw inputs)
void Seatbelt_Sensor_IF_SetRaw(bool v);
void Occupancy_Sensor_IF_SetRaw(OccupancyState v);
void VehicleState_IF_SetSpeed(uint16_t v);
void VehicleState_IF_SetDoorClosed(bool v);
void VehicleState_IF_SetIgnition(IgnitionState v);

static void scenario_step(uint32_t t){
    // Realistic scenario timeline for false trigger prevention demonstration
    // 0 ms: Ignition ON, door closes (grace starts), seat occupied, belt latched
    if(t==0){ VehicleState_IF_SetIgnition(IGN_ON); VehicleState_IF_SetDoorClosed(true); Occupancy_Sensor_IF_SetRaw(OCC_OCCUPIED); Seatbelt_Sensor_IF_SetRaw(true); VehicleState_IF_SetSpeed(0); }
    // 500 ms: Vehicle starts moving
    if(t==500){ VehicleState_IF_SetSpeed(12); }
    // 1500 ms: Grace ends; conditions gated
    // 2000 ms: User briefly wiggles belt causing 40 ms unlatch chatter (should NOT trigger)
    if(t==2000){ Seatbelt_Sensor_IF_SetRaw(false); }
    if(t==2040){ Seatbelt_Sensor_IF_SetRaw(true); }
    // 4000 ms: Driver unbuckles (sustained unlatch) -> should trigger warning after debounce (500 ms)
    if(t==4000){ Seatbelt_Sensor_IF_SetRaw(false); }
    // 4700 ms: Debounce completes -> warning active
    // 6000 ms: Driver re-buckles -> latch stable after 50 ms
    if(t==6000){ Seatbelt_Sensor_IF_SetRaw(true); }
    // 8000 ms: Seat becomes empty (occupant leaves) -> occupancy debounce 300 ms then warning suppressed
    if(t==8000){ Occupancy_Sensor_IF_SetRaw(OCC_EMPTY); }
    // 9000 ms: Speed drops to 0 (stop) -> gating removes warning
    if(t==9000){ VehicleState_IF_SetSpeed(0); }
}

int main(int argc, char** argv){
    // Easter egg / quick check: print a smile and exit
    for(int i=1;i<argc;i++){
        if(strcmp(argv[i], "--smile") == 0 || strcmp(argv[i], "smile") == 0){
            printf("\n  ^_^   Seatbelt Warning System\n");
            printf(" (o_o)  Drive safe and buckle up!\n");
            printf("  \\/   \n\n");
            return 0;
        }
    }
    if(!NvM_ValidateCal()){ printf("Calibration invalid; exiting\n"); return 1; }
    SeatbeltWarning_Logic_Init();

    const uint32_t SIM_DURATION_MS = 10000; // 10 s
    for(g_time_ms=0; g_time_ms<=SIM_DURATION_MS; g_time_ms+=10){
        scenario_step(g_time_ms);
        // Call runnables (order: inputs then logic)
        Seatbelt_Sensor_IF_10ms();
        Occupancy_Sensor_IF_10ms();
        VehicleState_IF_10ms();
        SeatbeltWarning_Logic_10ms();
        // Sleep 1ms to slow output (optional)
        usleep(1000);
    }
    printf("Simulation complete. Final warning=%u\n", Rte_Get_WarningRequest());
    return 0;
}
