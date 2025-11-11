#include "Rte.h"
#include "NvM.h"
#include <stdbool.h>

static OccupancyState rawOcc = OCC_EMPTY;
static uint16_t occ_timer_ms = 0;
static OccupancyState filteredOcc = OCC_EMPTY;

void Occupancy_Sensor_IF_SetRaw(OccupancyState v){ rawOcc = v; }

void Occupancy_Sensor_IF_10ms(void){
    const CalParams* cal = NvM_GetCal();
    if(rawOcc == filteredOcc){
        occ_timer_ms = 0; // stable
    } else {
        occ_timer_ms += 10;
        if(occ_timer_ms >= cal->occupancy_debounce_ms){ filteredOcc = rawOcc; occ_timer_ms = 0; }
    }
    Rte_Update_OccupancyFiltered(filteredOcc, VALIDITY_VALID, g_time_ms);
}
