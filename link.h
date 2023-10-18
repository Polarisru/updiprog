#ifndef LINK_H
#define LINK_H

#include <stdint.h>
#include <stdbool.h>

#include "app.h"

#ifdef __cplusplus
extern "C"
{
#endif

uint8_t LINK_ldcs(UPDI_APP*, uint8_t address);
void LINK_stcs(UPDI_APP*,uint8_t address, uint8_t value);
bool LINK_Init(UPDI_APP*,char *port, uint32_t baudrate, bool onDTR);
bool LINK_SendKey(UPDI_APP*,char *key, uint8_t size);
uint8_t LINK_ld(UPDI_APP*,uint16_t address);
uint8_t LINK_ld16(UPDI_APP*,uint16_t address);
bool LINK_st(UPDI_APP*,uint16_t address, uint8_t value);
bool LINK_st16(UPDI_APP*,uint16_t address, uint16_t value);
void LINK_Repeat(UPDI_APP*,uint16_t repeats);
bool LINK_ld_ptr_inc(UPDI_APP*,uint8_t *data, uint8_t size);
bool LINK_ld_ptr_inc16(UPDI_APP*,uint8_t *data, uint16_t words);
bool LINK_st_ptr(UPDI_APP*,uint16_t address);
bool LINK_st_ptr_inc(UPDI_APP*,uint8_t *data, uint16_t len);
bool LINK_st_ptr_inc16(UPDI_APP*,uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
