#ifndef NVM_H
#define NVM_H

#include <stdint.h>
#include <stdbool.h>

#define NVM_MAX_ERRORS    (3)

bool NVM_EnterProgmode(void);
void NVM_LeaveProgmode(void);
bool NVM_UnlockDevice(void);
bool NVM_ChipErase(void);
uint8_t NVM_ReadFuse(uint8_t fusenum);
bool NVM_WriteFuse(uint8_t fusenum, uint8_t value);
bool NVM_LoadIhex(char *filename, uint16_t address, uint16_t len);
bool NVM_SaveIhex(char *filename, uint16_t address, uint16_t len);

#endif
