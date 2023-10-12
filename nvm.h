#ifndef NVM_H
#define NVM_H

#include <stdint.h>
#include <stdbool.h>

#include "app.h"

#define NVM_MAX_ERRORS    (3)

bool NVM_EnterProgmode(UPDI_APP *);
void NVM_LeaveProgmode(UPDI_APP *);
bool NVM_UnlockDevice(UPDI_APP *);
bool NVM_ChipErase(UPDI_APP *);
uint8_t NVM_ReadFuse(UPDI_APP *,uint8_t fusenum);
bool NVM_WriteFuse(UPDI_APP *,uint8_t fusenum, uint8_t value);
bool NVM_LoadIhex(UPDI_APP *,char *filename, uint16_t address, uint16_t len);
bool NVM_SaveIhex(UPDI_APP *,char *filename, uint16_t address, uint16_t len);

#endif
