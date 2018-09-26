#ifndef LINK_H
#define LINK_H

#include <stdint.h>
#include <stdbool.h>

extern uint8_t LINK_ldcs(uint8_t address);
extern void LINK_stcs(uint8_t address, uint8_t value);
extern bool LINK_Init(char *port, uint32_t baudrate, bool onDTR);
extern bool LINK_SendKey(char *key, uint8_t size);
extern uint8_t LINK_ld(uint16_t address);
extern bool LINK_st(uint16_t address, uint8_t value);
extern bool LINK_st16(uint16_t address, uint16_t value);
extern void LINK_Repeat(uint16_t repeats);
extern bool LINK_ld_ptr_inc(uint8_t *data, uint8_t size);
extern bool LINK_ld_ptr_inc16(uint8_t *data, uint16_t words);
extern bool LINK_st_ptr(uint16_t address);
extern bool LINK_st_ptr_inc(uint8_t *data, uint16_t len);
extern bool LINK_st_ptr_inc16(uint8_t *data, uint16_t len);

#endif
