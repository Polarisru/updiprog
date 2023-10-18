#ifndef UPDILIB_H
#define UPDILIB_H

#include <stdint.h>
#include <stdbool.h>

#include "../com.h"
#include "../log.h"
#include "../progress.h"

#ifdef BUILD_DLL
    #ifdef WIN64
    #define DLL_EXPORT __declspec(dllexport)
    #define DLL_HIDDEN
    #else
    #define DLL_EXPORT __attribute__((visibility("default")))
    #define DLL_HIDDEN __attribute__((visibility("hidden")))
    #endif
#else
    #ifdef WIN64
    #define DLL_EXPORT __declspec(dllimport)
    #else
    #define DLL_EXPORT
    #endif
    #define DLL_HIDDEN
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef int8_t UPDI_bool;

typedef struct
{
  uint32_t       baudrate;
  int8_t         device;
  char           port[COMPORT_LEN];
  UPDI_logger*   logger;
  UPDI_progress* progress;
} UPDI_Params;

typedef struct
{
  uint8_t fuse;
  uint8_t value;
} UPDI_fuse;

typedef struct
{
  uint8_t seq_type;
  void * data;
  int32_t data_len;
} UPDI_seq;

#define UPDI_SEQ_UNLOCK      1
#define UPDI_SEQ_ENTER_PM    2
#define UPDI_SEQ_ERASE       3
#define UPDI_SEQ_FLASH       4
#define UPDI_SEQ_READ        5
#define UPDI_SEQ_SET_FUSES   6
#define UPDI_SEQ_GET_FUSES   7
#define UPDI_SEQ_LOCK        8
#define UPDI_SEQ_GET_SIG_ROW 9

DLL_EXPORT UPDI_logger * UPDILIB_logger_init(const char *, int32_t, UPDI_onlog, UPDI_onlogfree, void *);
DLL_EXPORT void UPDILIB_logger_done(UPDI_logger *);

DLL_EXPORT UPDI_progress * UPDILIB_progress_init(UPDI_onprgsstart, UPDI_onprgs, UPDI_onprgsfinish, void *);
DLL_EXPORT void UPDILIB_progress_done(UPDI_progress *);

DLL_EXPORT void UPDILIB_set_glb_logger_onlog(UPDI_onlog, void *);
DLL_EXPORT void UPDILIB_set_glb_logger_level(int32_t);

DLL_EXPORT UPDI_Params * UPDILIB_cfg_init();
DLL_EXPORT void UPDILIB_cfg_done(UPDI_Params *);

DLL_EXPORT UPDI_bool UPDILIB_cfg_set_logger(UPDI_Params *, UPDI_logger *);
DLL_EXPORT UPDI_bool UPDILIB_cfg_set_progress(UPDI_Params *, UPDI_progress *);
DLL_EXPORT UPDI_bool UPDILIB_cfg_set_buadrate(UPDI_Params *, uint32_t);
DLL_EXPORT UPDI_bool UPDILIB_cfg_set_com(UPDI_Params *, const char *);
DLL_EXPORT UPDI_bool UPDILIB_cfg_set_device(UPDI_Params *, const char *);

DLL_EXPORT uint32_t     UPDILIB_cfg_get_baudrate(UPDI_Params *);
DLL_EXPORT const char * UPDILIB_cfg_get_com(UPDI_Params *);
DLL_EXPORT int8_t       UPDILIB_cfg_get_device(UPDI_Params *);

DLL_EXPORT int32_t   UPDILIB_devices_get_count();
DLL_EXPORT UPDI_bool UPDILIB_devices_get_name(int8_t, char *, int32_t *);
DLL_EXPORT uint8_t   UPDILIB_devices_get_fuses_cnt(int8_t);

/* Programming */

DLL_EXPORT UPDI_bool UPDILIB_launch_seq(UPDI_Params *, UPDI_seq *, uint8_t);

DLL_EXPORT UPDI_bool UPDILIB_read_dev_info(UPDI_Params *, uint8_t *);
DLL_EXPORT UPDI_bool UPDILIB_erase(UPDI_Params *);
DLL_EXPORT UPDI_bool UPDILIB_lock(UPDI_Params *);
DLL_EXPORT UPDI_bool UPDILIB_unlock(UPDI_Params *);

DLL_EXPORT UPDI_bool UPDILIB_write_fuses(UPDI_Params *, const UPDI_fuse *, uint8_t);
DLL_EXPORT UPDI_bool UPDILIB_read_fuses(UPDI_Params *, UPDI_fuse *, uint8_t);

DLL_EXPORT UPDI_bool UPDILIB_write_hex(UPDI_Params *, const char *, int32_t);
DLL_EXPORT UPDI_bool UPDILIB_read_hex(UPDI_Params *, char *, int32_t *);

#ifdef __cplusplus
}
#endif



#endif // UPDILIB_H
