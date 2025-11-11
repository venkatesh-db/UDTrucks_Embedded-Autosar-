#include "NvM.h"
#include <string.h>

static CalParams cal = {
    .latch_on_delay_ms = 50,
    .unlatch_on_delay_ms = 500,
    .occupancy_debounce_ms = 300,
    .speed_threshold_kph = 10,
    .door_grace_ms = 2000
};

const CalParams* NvM_GetCal(void){ return &cal; }

// Use external Crc16_Calc defined in Crc.c

int NvM_ValidateCal(void){
    // Placeholder: always valid
    uint16_t crc = Crc16_Calc(&cal, sizeof(cal));
    (void)crc; // could compare with stored reference
    return 1;
}
