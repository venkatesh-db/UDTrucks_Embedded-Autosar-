#include "Dem.h"
#include <stdio.h>

void Dem_ReportErrorStatus(uint16_t dtc, uint8_t status){
    const char* s = status==DEM_EVENT_STATUS_PASSED?"PASSED":"PREFAILED";
    printf("[DEM] DTC 0x%04X status %s\n", dtc, s);
}
