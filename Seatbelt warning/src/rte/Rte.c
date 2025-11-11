#include "Rte.h"

uint32_t g_time_ms = 0;

static RteBoolSignal seatbeltLatchFiltered;
static RteOccupancySignal occupancyFiltered;
static RteSpeedSignal vehicleSpeed;
static RteIgnitionSignal ignitionState;
static RteBoolSignal doorClosed;
static uint8_t warningRequest = 0;

void Rte_Write_SBW_WarningRequest(uint8_t level){
    warningRequest = level;
}

uint8_t Rte_Get_WarningRequest(void){
    return warningRequest;
}

RteBoolSignal Rte_Read_SeatbeltLatchFiltered(void){ return seatbeltLatchFiltered; }
RteOccupancySignal Rte_Read_OccupancyFiltered(void){ return occupancyFiltered; }
RteSpeedSignal Rte_Read_VehicleSpeed(void){ return vehicleSpeed; }
RteIgnitionSignal Rte_Read_IgnitionState(void){ return ignitionState; }
RteBoolSignal Rte_Read_DoorClosed(void){ return doorClosed; }

void Rte_Update_SeatbeltLatchFiltered(bool v, RteValidity val, uint32_t ts){
    seatbeltLatchFiltered.value = v; seatbeltLatchFiltered.validity = val; seatbeltLatchFiltered.timestamp_ms = ts;
}
void Rte_Update_OccupancyFiltered(OccupancyState v, RteValidity val, uint32_t ts){
    occupancyFiltered.value = v; occupancyFiltered.validity = val; occupancyFiltered.timestamp_ms = ts;
}
void Rte_Update_VehicleSpeed(uint16_t v, RteValidity val, uint32_t ts){
    vehicleSpeed.value = v; vehicleSpeed.validity = val; vehicleSpeed.timestamp_ms = ts;
}
void Rte_Update_IgnitionState(IgnitionState v, RteValidity val, uint32_t ts){
    ignitionState.value = v; ignitionState.validity = val; ignitionState.timestamp_ms = ts;
}
void Rte_Update_DoorClosed(bool v, RteValidity val, uint32_t ts){
    doorClosed.value = v; doorClosed.validity = val; doorClosed.timestamp_ms = ts;
}
