#include "Rte.h"
#include "Dem.h"
#include "NvM.h"
#include <stdint.h>
#include <stdbool.h>

// Raw input simulation variable (would come from IoHwAb)
static bool rawLatch = true; // true = latched
static uint16_t latch_timer_ms = 0;
static uint16_t unlatch_timer_ms = 0;
static bool filteredLatch = true;

void Seatbelt_Sensor_IF_SetRaw(bool v){ rawLatch = v; }

void Seatbelt_Sensor_IF_10ms(void){
    const CalParams* cal = NvM_GetCal();
    if(rawLatch){
        latch_timer_ms += 10; unlatch_timer_ms = 0;
        if(latch_timer_ms >= cal->latch_on_delay_ms){ filteredLatch = true; }
    } else {
        unlatch_timer_ms += 10; latch_timer_ms = 0;
        if(unlatch_timer_ms >= cal->unlatch_on_delay_ms){ filteredLatch = false; }
    }

    // Stuck detection simplistic: if always same state > 5000 ms report prefailed
    static uint32_t same_state_ms = 0;
    static bool last_raw = true;
    if(last_raw == rawLatch){ same_state_ms += 10; } else { same_state_ms = 0; last_raw = rawLatch; }
    if(same_state_ms == 5000){ Dem_ReportErrorStatus(DTC_SEATBELT_STUCK, DEM_EVENT_STATUS_PREFAILED); }
    if(same_state_ms == 5100){ Dem_ReportErrorStatus(DTC_SEATBELT_STUCK, DEM_EVENT_STATUS_PASSED); }

    Rte_Update_SeatbeltLatchFiltered(filteredLatch, VALIDITY_VALID, g_time_ms);
}
