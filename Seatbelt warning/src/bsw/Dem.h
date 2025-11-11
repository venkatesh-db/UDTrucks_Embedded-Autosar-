#ifndef DEM_H
#define DEM_H
#include <stdint.h>

#define DEM_EVENT_STATUS_PASSED 0
#define DEM_EVENT_STATUS_PREFAILED 1

void Dem_ReportErrorStatus(uint16_t dtc, uint8_t status);

// Example DTC IDs
#define DTC_SEATBELT_STUCK  0x1001
#define DTC_OCCUPANCY_INVALID 0x1002
#define DTC_PLAUSIBILITY_CONFLICT 0x1003
#define DTC_VEHICLESTATE_STALE 0x1004

#endif // DEM_H
