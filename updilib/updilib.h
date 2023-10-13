#ifndef UPDILIB_H
#define UPDILIB_H

#include <stdint.h>
#include <stdbool.h>

#include "../com.h"
#include "../log.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef int8_t UPDI_bool;

typedef struct
{
  uint32_t     baudrate;
  int8_t       device;
  char         port[COMPORT_LEN];
  UPDI_logger* logger;
} UPDI_Params;

typedef struct
{
  uint8_t fuse;
  uint8_t value;
} UPDI_fuse;

UPDI_logger * UPDILIB_logger_init(const char *, int32_t, UPDI_onlog, UPDI_onlogfree, void *);
void UPDILIB_logger_done(UPDI_logger *);

UPDI_Params * UPDILIB_cfg_init();
void UPDILIB_cfg_done(UPDI_Params *);

UPDI_bool UPDILIB_cfg_set_logger(UPDI_Params *, UPDI_logger *);
UPDI_bool UPDILIB_cfg_set_buadrate(UPDI_Params *, uint32_t);
UPDI_bool UPDILIB_cfg_set_com(UPDI_Params *, const char *);
UPDI_bool UPDILIB_cfg_set_device(UPDI_Params *, const char *);

int32_t   UPDILIB_devices_get_count();
UPDI_bool UPDILIB_devices_get_name(int8_t, char *, int32_t *);

UPDI_bool UPDILIB_erase(UPDI_Params *);

UPDI_bool UPDILIB_write_fuses(UPDI_Params *, const UPDI_fuse *, int32_t);
UPDI_bool UPDILIB_read_fuses(UPDI_Params *, UPDI_fuse *, int32_t *);

UPDI_bool UPDILIB_write_hex(UPDI_Params *, const char *, int32_t);
UPDI_bool UPDILIB_read_hex(UPDI_Params *, char *, int32_t *);

#ifdef __cplusplus
}
#endif



#endif // UPDILIB_H
