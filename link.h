#ifndef LINK_H
#define LINK_H

#include <stdint.h>
#include <stdbool.h>

uint8_t LINK_ldcs(uint8_t address);
void LINK_stcs(uint8_t address, uint8_t value);
bool LINK_Init(char *port, uint32_t baudrate, bool onDTR);
bool LINK_SendKey(char *key, uint8_t size);
uint8_t LINK_ld(uint16_t address);
bool LINK_st(uint16_t address, uint8_t value);
bool LINK_st16(uint16_t address, uint16_t value);
void LINK_Repeat(uint16_t repeats);
bool LINK_ld_ptr_inc(uint8_t *data, uint8_t size);
bool LINK_ld_ptr_inc16(uint8_t *data, uint16_t words);
bool LINK_st_ptr(uint16_t address);
bool LINK_st_ptr_inc(uint8_t *data, uint16_t len);
bool LINK_st_ptr_inc16(uint8_t *data, uint16_t len);

#endif
