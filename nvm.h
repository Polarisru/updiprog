#ifndef NVM_H
#define NVM_H

#include <stdint.h>
#include <stdbool.h>

#define NVM_MAX_ERRORS    (3)

extern bool NVM_EnterProgmode(void);
extern void NVM_LeaveProgmode(void);
extern bool NVM_ChipErase(void);
extern uint8_t NVM_ReadFuse(uint8_t fusenum);
extern bool NVM_WriteFuse(uint8_t fusenum, uint8_t value);
extern bool NVM_LoadIhex(char *filename, uint16_t address, uint16_t len);
extern bool NVM_SaveIhex(char *filename, uint16_t address, uint16_t len);

#endif
