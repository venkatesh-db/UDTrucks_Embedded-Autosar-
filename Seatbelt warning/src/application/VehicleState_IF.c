#include "Rte.h"
#include "Dem.h"
#include <stdbool.h>

static uint16_t simSpeed = 0; // km/h
static bool simDoorClosed = true;
static IgnitionState simIgn = IGN_OFF;

void VehicleState_IF_SetSpeed(uint16_t v){ simSpeed = v; }
void VehicleState_IF_SetDoorClosed(bool v){ simDoorClosed = v; }
void VehicleState_IF_SetIgnition(IgnitionState v){ simIgn = v; }

void VehicleState_IF_10ms(void){
    // simplistic staleness check: if speed remains zero with IGN_ON for > 10000 ms maybe stale (example)
    static uint32_t speed_static_ms = 0; static uint16_t lastSpeed = 0;
    if(lastSpeed == simSpeed){ speed_static_ms += 10; } else { speed_static_ms = 0; lastSpeed = simSpeed; }
    if(speed_static_ms == 10000){ Dem_ReportErrorStatus(DTC_VEHICLESTATE_STALE, DEM_EVENT_STATUS_PREFAILED); }
    if(speed_static_ms == 10100){ Dem_ReportErrorStatus(DTC_VEHICLESTATE_STALE, DEM_EVENT_STATUS_PASSED); }

    Rte_Update_VehicleSpeed(simSpeed, VALIDITY_VALID, g_time_ms);
    Rte_Update_IgnitionState(simIgn, VALIDITY_VALID, g_time_ms);
    Rte_Update_DoorClosed(simDoorClosed, VALIDITY_VALID, g_time_ms);
}
