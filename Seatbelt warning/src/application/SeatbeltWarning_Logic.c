#include "Rte.h"
#include "NvM.h"
#include <stdio.h>

static uint32_t doorGraceRemaining = 0;

void SeatbeltWarning_Logic_Init(void){
    const CalParams* cal = NvM_GetCal();
    doorGraceRemaining = cal->door_grace_ms; // start grace on power-up (simulating door just closed)
}

void SeatbeltWarning_Logic_10ms(void){
    const CalParams* cal = NvM_GetCal();
    RteBoolSignal latch = Rte_Read_SeatbeltLatchFiltered();
    RteOccupancySignal occ = Rte_Read_OccupancyFiltered();
    RteSpeedSignal spd = Rte_Read_VehicleSpeed();
    RteIgnitionSignal ign = Rte_Read_IgnitionState();
    RteBoolSignal door = Rte_Read_DoorClosed();

    if(doorGraceRemaining > 0){
        if(door.value && door.validity == VALIDITY_VALID){
            if(doorGraceRemaining >= 10) doorGraceRemaining -= 10; else doorGraceRemaining = 0;
        } else {
            // door opened resets grace
            doorGraceRemaining = cal->door_grace_ms;
        }
    }

    uint8_t output = 0; // Off
    // Gating conditions
    bool gate = (ign.value == IGN_ON) && (spd.value >= cal->speed_threshold_kph) && (doorGraceRemaining == 0);

    if(gate){
        // Validity checks (simplified): require valid occupancy & latch
        bool inputsValid = (occ.validity == VALIDITY_VALID) && (latch.validity == VALIDITY_VALID);
        if(inputsValid){
            if(occ.value == OCC_OCCUPIED && latch.value == false){
                output = 2; // AudioVisual warning
            } else {
                output = 0; // No warning
            }
        } else {
            output = 0; // inhibit
        }
    }

    Rte_Write_SBW_WarningRequest(output);

    // Logging demo
    if(g_time_ms % 100 == 0){
        printf("[TIME %5u] IGN=%d SPD=%u OCC=%d LATCH=%d DOOR=%d GRACE=%u WARN=%u\n", g_time_ms, ign.value, spd.value, occ.value, latch.value, door.value, doorGraceRemaining, output);
    }
}
