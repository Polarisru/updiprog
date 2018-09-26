#ifndef DEVICES_H
#define DEVICES_H

#include <stdint.h>

#define DEVICES_NAME_LEN    (16)

#define DEVICE_UNKNOWN_ID   (-1)

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

extern int8_t DEVICES_GetId(char *name);
extern uint16_t DEVICES_GetFlashLength(void);
extern uint16_t DEVICES_GetFlashStart(void);
extern uint16_t DEVICES_GetPageSize(void);
extern uint16_t DEVICES_GetNvmctrlAddress(void);
extern uint16_t DEVICES_GetFusesAddress(void);
extern uint8_t DEVICES_GetFusesNumber(void);
extern uint8_t DEVICES_GetNumber(void);
extern char *DEVICES_GetNameByNumber(uint8_t number);

#endif // DEVICES_H
