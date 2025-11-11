#ifndef NVM_H
#define NVM_H
#include <stdint.h>

typedef struct {
    uint16_t latch_on_delay_ms;    // 50 ms
    uint16_t unlatch_on_delay_ms;  // 500 ms
    uint16_t occupancy_debounce_ms;// 300 ms
    uint16_t speed_threshold_kph;  // 10 km/h
    uint16_t door_grace_ms;        // 2000 ms
} CalParams;

const CalParams* NvM_GetCal(void);
uint16_t Crc16_Calc(const void* data, uint32_t len);
int NvM_ValidateCal(void);

#endif // NVM_H
