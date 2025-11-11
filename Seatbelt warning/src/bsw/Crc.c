#include "Crc.h"
uint16_t Crc16_Calc(const void* data, uint32_t len){
    const uint8_t* p = (const uint8_t*)data;
    uint16_t crc=0xFFFF;
    for(uint32_t i=0;i<len;i++){
        crc ^= p[i];
        for(int j=0;j<8;j++){
            if(crc & 1) crc = (crc>>1) ^ 0xA001; else crc >>=1;
        }
    }
    return crc;
}
