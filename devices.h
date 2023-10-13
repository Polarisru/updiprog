#ifndef DEVICES_H
#define DEVICES_H

#include <stdint.h>

#define DEVICES_NAME_LEN    (16)

#define DEVICE_UNKNOWN_ID   (-1)

#define DEVICE_LOCKBIT_ADDR (0x0A)

typedef struct
{
  char     name[DEVICES_NAME_LEN];
  uint16_t flash_start;
  uint16_t flash_size;
  uint16_t flash_pagesize;
  uint16_t syscfg_address;
  uint16_t nvmctrl_address;
  uint16_t sigrow_address;
  uint16_t fuses_address;
  uint16_t userrow_address;
  uint8_t  number_of_fuses;
} tDevice;

extern tDevice DEVICES_List[];

int8_t DEVICES_GetId(const char *name);
uint16_t DEVICES_GetFlashLength(int8_t id);
uint16_t DEVICES_GetFlashStart(int8_t id);
uint16_t DEVICES_GetPageSize(int8_t id);
uint16_t DEVICES_GetNvmctrlAddress(int8_t id);
uint16_t DEVICES_GetFusesAddress(int8_t id);
uint8_t DEVICES_GetFusesNumber(int8_t id);
uint8_t DEVICES_GetNumber(void);
char *DEVICES_GetNameByNumber(uint8_t number);

#endif // DEVICES_H
