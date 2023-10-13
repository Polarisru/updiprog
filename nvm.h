#ifndef NVM_H
#define NVM_H

#include <stdint.h>
#include <stdbool.h>

#include "app.h"

#define NVM_MAX_ERRORS    (3)

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct NVM_raw_data_t {
    char * data;
    int32_t len;
    int32_t pos;
} NVM_raw_data;

bool NVM_EnterProgmode(UPDI_APP *);
void NVM_LeaveProgmode(UPDI_APP *);
bool NVM_UnlockDevice(UPDI_APP *);
bool NVM_ChipErase(UPDI_APP *);
uint8_t NVM_ReadFuse(UPDI_APP *,uint8_t fusenum);
bool NVM_WriteFuse(UPDI_APP *,uint8_t fusenum, uint8_t value);
bool NVM_LoadIhexFile(UPDI_APP *,char *filename, uint16_t address, uint16_t len);
bool NVM_LoadIhexRaw(UPDI_APP *, NVM_raw_data *src, uint16_t address, uint16_t len);
bool NVM_SaveIhexFile(UPDI_APP *,char *filename, uint16_t address, uint16_t len);
bool NVM_SaveIhexRaw(UPDI_APP *,NVM_raw_data *dst, uint16_t address, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
