#include <assert.h>
#include <stdio.h>
#include "Rte.h"
#include "NvM.h"

void Seatbelt_Sensor_IF_SetRaw(bool v);
void Seatbelt_Sensor_IF_10ms(void);
void Occupancy_Sensor_IF_SetRaw(OccupancyState v);
void Occupancy_Sensor_IF_10ms(void);

static void advance_ms(uint32_t ms){
    for(uint32_t t=0;t<ms;t+=10){
        g_time_ms += 10;
        Seatbelt_Sensor_IF_10ms();
        Occupancy_Sensor_IF_10ms();
    }
}

int main(){
    // Test: 40 ms unlatch chatter should not change filtered latch (requires 500 ms off-delay)
    g_time_ms=0;
    Seatbelt_Sensor_IF_SetRaw(true);
    Occupancy_Sensor_IF_SetRaw(OCC_OCCUPIED);
    advance_ms(100);
    Seatbelt_Sensor_IF_SetRaw(false);
    advance_ms(40);
    Seatbelt_Sensor_IF_SetRaw(true);
    advance_ms(60);
    assert(Rte_Read_SeatbeltLatchFiltered().value == true);

    // Test: 500 ms sustained unlatch should change filtered to false
    Seatbelt_Sensor_IF_SetRaw(false);
    advance_ms(500);
    assert(Rte_Read_SeatbeltLatchFiltered().value == false);

    // Test: Occupancy debounce 300 ms
    Occupancy_Sensor_IF_SetRaw(OCC_OCCUPIED);
    advance_ms(100);
    Occupancy_Sensor_IF_SetRaw(OCC_EMPTY);
    advance_ms(200); // not enough
    assert(Rte_Read_OccupancyFiltered().value == OCC_OCCUPIED);
    advance_ms(100); // now 300 ms total
    assert(Rte_Read_OccupancyFiltered().value == OCC_EMPTY);

    printf("All debounce tests passed\n");
    return 0;
}
