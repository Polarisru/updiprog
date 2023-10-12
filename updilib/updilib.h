#ifndef UPDILIB_H
#define UPDILIB_H

#include <stdint.h>
#include <stdbool.h>

#define FILENAME_LEN    (64)
#define COMPORT_LEN     (32)
#define FUSES_LEN       (128)

typedef UPDI_bool int8_t;

typedef struct
{
  UPDI_bool erase;
  UPDI_bool write;
  UPDI_bool read;
  UPDI_bool wr_fuses;
  UPDI_bool rd_fuses;
  UPDI_bool lock;
  UPDI_bool unlock;
  UPDI_bool show_info;
  uint32_t  baudrate;
  int8_t    device;
  char      port[COMPORT_LEN];
} UPDI_Params;

typedef struct
{
  uint8_t fuse;
  uint8_t value;
} UPDI_fuse;

UPDI_Params * UPDILIB_cfg_init();
UPDI_bool UPDILIB_cfg_set_com(UPDI_Params *, const char *);
UPDI_bool UPDILIB_cfg_set_device(UPDI_Params *, const char *);

UPDI_bool UPDILIB_device_get_name(UPDI_Params *, char *, int32_t);

UPDI_bool UPDILIB_write_fuses(UPDI_Params *, const UPDI_fuse *, int32_t);
UPDI_bool UPDILIB_read_fuses(UPDI_Params *, UPDI_fuse *, int32_t *);

UPDI_bool UPDILIB_write_hex(UPDI_Params *, const char *, int32_t);
UPDI_bool UPDILIB_read_hex(UPDI_Params *, char *, int32_t *);


#endif // UPDILIB_H
