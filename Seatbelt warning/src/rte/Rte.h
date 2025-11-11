#ifndef RTE_H
#define RTE_H
#include <stdint.h>
#include <stdbool.h>

typedef enum { VALIDITY_VALID, VALIDITY_INVALID, VALIDITY_UNKNOWN } RteValidity;

typedef struct {
    bool value;
    RteValidity validity;
    uint32_t timestamp_ms;
} RteBoolSignal;

typedef enum { OCC_EMPTY=0, OCC_OCCUPIED=1, OCC_UNKNOWN=2 } OccupancyState;

typedef struct {
    OccupancyState value;
    RteValidity validity;
    uint32_t timestamp_ms;
} RteOccupancySignal;

typedef struct {
    uint16_t value; // km/h scaled
    RteValidity validity;
    uint32_t timestamp_ms;
} RteSpeedSignal;

typedef enum { IGN_OFF=0, IGN_ON=1 } IgnitionState;

typedef struct {
    IgnitionState value;
    RteValidity validity;
    uint32_t timestamp_ms;
} RteIgnitionSignal;

// Write APIs for logic output
void Rte_Write_SBW_WarningRequest(uint8_t level); // 0=Off,1=Visual,2=AudioVisual
uint8_t Rte_Get_WarningRequest(void);
// Read APIs for input signals (filtered interface components will populate these)
RteBoolSignal Rte_Read_SeatbeltLatchFiltered(void);
RteOccupancySignal Rte_Read_OccupancyFiltered(void);
RteSpeedSignal Rte_Read_VehicleSpeed(void);
RteIgnitionSignal Rte_Read_IgnitionState(void);
RteBoolSignal Rte_Read_DoorClosed(void);

// Update (internal use by interface SWCs)
void Rte_Update_SeatbeltLatchFiltered(bool v, RteValidity val, uint32_t ts);
void Rte_Update_OccupancyFiltered(OccupancyState v, RteValidity val, uint32_t ts);
void Rte_Update_VehicleSpeed(uint16_t v, RteValidity val, uint32_t ts);
void Rte_Update_IgnitionState(IgnitionState v, RteValidity val, uint32_t ts);
void Rte_Update_DoorClosed(bool v, RteValidity val, uint32_t ts);

// Time base
extern uint32_t g_time_ms;

#endif // RTE_H
