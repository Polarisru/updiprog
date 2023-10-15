#include "updilib.h"
#include "stdlib.h"
#include "string.h"
#include "../app.h"
#include "../devices.h"
#include "../link.h"
#include "../log.h"
#include "../nvm.h"
#include "../phy.h"

#ifdef __cplusplus
extern "C"
{
#endif

DLL_EXPORT UPDI_logger * UPDILIB_logger_init(const char * _src, int32_t _level, UPDI_onlog _onlog, UPDI_onlogfree _onfree, void * _ud) {
     return UPDI_logger_init(_src, _level, _onlog, _onfree, _ud);
}

DLL_EXPORT void UPDILIB_logger_done(UPDI_logger * logger) {
     UPDI_logger_done(logger);
}

DLL_EXPORT UPDI_Params * UPDILIB_cfg_init() {
    UPDI_Params * res = (UPDI_Params *)malloc(sizeof(UPDI_Params));
    if (res) {
        res->baudrate = PHY_BAUDRATE;
        res->device = DEVICE_UNKNOWN_ID;
        res->port[0] = 0;
        return res;
    }
    return NULL;
}

DLL_EXPORT void UPDILIB_cfg_done(UPDI_Params * cfg) {
    if (cfg) free(cfg);
}

DLL_EXPORT void UPDILIB_set_glb_logger_onlog(UPDI_onlog _onlog, void * _ud) {
    global_LOG()->onlog = _onlog;
    global_LOG()->userdata = _ud;
}

DLL_EXPORT void UPDILIB_set_glb_logger_level(int32_t _level) {
    global_LOG()->LOG_Level = _level;
}

DLL_EXPORT UPDI_bool UPDILIB_cfg_set_buadrate(UPDI_Params * cfg, uint32_t val) {
    if (cfg) {
        cfg->baudrate = val;
        return 1;
    }
    return 0;
}

DLL_EXPORT UPDI_bool UPDILIB_cfg_set_com(UPDI_Params * cfg, const char * val) {
    if (cfg) {
        int len = strlen(val);
        if ((len <= 1) || (len >= COMPORT_LEN)) return 0;

        memcpy(&(cfg->port[0]), val, len);
        cfg->port[len] = 0;

        return 1;
    }
    return 0;
}

DLL_EXPORT UPDI_bool UPDILIB_cfg_set_logger(UPDI_Params * _cfg, UPDI_logger * _logger) {
    if (_cfg) {
        _cfg->logger = _logger;
        return 1;
    }
    return 0;
}

DLL_EXPORT UPDI_bool UPDILIB_cfg_set_device(UPDI_Params * _cfg, const char * _name) {
    if (_cfg) {
        int8_t d = DEVICES_GetId(_name);
        if (d < 0) return 0;

        _cfg->device = d;

        return 1;
    }
    return 0;
}

DLL_EXPORT int32_t UPDILIB_devices_get_count() {
    return DEVICES_GetNumber();
}

DLL_EXPORT UPDI_bool UPDILIB_devices_get_name(int8_t _id, char * _name, int32_t * _len) {
    if (!_name) return 0;
    if (!_len) return 0;
    if (*_len < 1) return 0;
    if (_id < 0) return 0;
    if (_id >= DEVICES_GetNumber()) return 0;

    char * res = DEVICES_GetNameByNumber(_id);

    int32_t rlen = strlen(res);
    if (rlen > *_len) rlen = *_len;
    memcpy(_name, res, rlen);
    _name[rlen] = 0;
    *_len = rlen;

    return 1;
}

DLL_HIDDEN UPDI_APP * intern_link_to_app(UPDI_Params * _cfg) {
  UPDI_APP * app = APP_Init(_cfg->logger);

  if (!app) return NULL;

  app->DEVICE_Id = _cfg->device;

  if (LINK_Init(app, _cfg->port, _cfg->baudrate, false) == false)
  {
    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Can't open port: %s", _cfg->port);
    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Please check connection and try again.");
    APP_Done(app);
    return NULL;
  }

  LOG_Print(app->logger, LOG_LEVEL_INFO, "Working with device: %s", DEVICES_GetNameByNumber(_cfg->device));

  /*
  if (parameters.unlock == true)
  {
    LOG_Print(app->logger,  LOG_LEVEL_INFO, "Unlocking...   ");
    if (NVM_UnlockDevice(app) == true)
    {
      LOG_Print(app->logger,  LOG_LEVEL_INFO,  "OK");
    }
  }
  */

  if (NVM_EnterProgmode(app) == false)
  {
    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Can't enter programming mode, exiting");
    APP_Done(app);
    return NULL;
  }

  return app;
}

DLL_HIDDEN UPDI_bool intern_unlink_app(UPDI_APP * app) {

  /*
  if (parameters.lock == true)
  {
    LOG_Print(app->logger,  LOG_LEVEL_INFO,  "Locking MCU...   ");
    if (NVM_WriteFuse(app, DEVICE_LOCKBIT_ADDR, 0x00) == true)
    {
      LOG_Print(app->logger,  LOG_LEVEL_INFO,  "OK");
    }
  }
  */

  NVM_LeaveProgmode(app);
  APP_Done(app);

  return 1;
}

DLL_EXPORT UPDI_bool UPDILIB_erase(UPDI_Params * _cfg) {
  UPDI_APP * app = intern_link_to_app(_cfg);
  if (app) {
      LOG_Print(app->logger, LOG_LEVEL_INFO, "Erasing");
      NVM_ChipErase(app);
      intern_unlink_app(app);
      return 1;
  }
  return 0;
}

DLL_EXPORT UPDI_bool UPDILIB_write_fuses(UPDI_Params * _cfg, const UPDI_fuse * _fuses, int32_t _cnt) {
  if (!_cfg) return 0;
  if (!_fuses) return 0;
  if (_cnt <= 0) return 0;

  UPDI_APP * app = intern_link_to_app(_cfg);
  if (app) {
      int i = 0;
      while (i < _cnt)
      {
        LOG_Print(app->logger, LOG_LEVEL_INFO, "Writing 0x%02X to fuse Nr. %d", _fuses[i].value, _fuses[i].fuse);
        NVM_WriteFuse(app, _fuses[i].fuse, _fuses[i].value);
        i++;
      }
      intern_unlink_app(app);
      return 1;
  }
  return 0;
}

DLL_EXPORT UPDI_bool UPDILIB_read_fuses(UPDI_Params * _cfg, UPDI_fuse * _fuses, int32_t * _cnt) {
  if (!_cfg) return 0;
  if (!_fuses) return 0;
  if (!_cnt) return 0;
  if (*_cnt <= 0) return 0;

  UPDI_APP * app = intern_link_to_app(_cfg);
  if (app) {
      LOG_Print(app->logger, LOG_LEVEL_INFO, "Reading fuses:");
      int max_n =  DEVICES_GetFusesNumber(app->DEVICE_Id);
      for (int i = 0; i < *_cnt; i++)
      {
        int n = _fuses[i].fuse;
        if (n < max_n) {
            uint8_t x = NVM_ReadFuse(app, n);
            LOG_Print(app->logger, LOG_LEVEL_INFO, "  0x%02X: 0x%02X", n, x);
            _fuses[i].value = x;
        }
      }
      intern_unlink_app(app);
      return 1;
  }
  return 0;
}

DLL_EXPORT UPDI_bool UPDILIB_write_hex(UPDI_Params * _cfg, const char * _data, int32_t _len) {
  if (!_cfg) return 0;
  if (!_data) return 0;
  if (_len <= 0) return 0;

  UPDI_APP * app = intern_link_to_app(_cfg);
  if (app) {
    LOG_Print(app->logger, LOG_LEVEL_INFO, "Writing from hex data");

    NVM_raw_data data;
    data.data = (char *)_data;
    data.len = _len;
    data.pos = 0;

    NVM_LoadIhexRaw(app, &data, DEVICES_GetFlashStart(app->DEVICE_Id), DEVICES_GetFlashLength(app->DEVICE_Id));
    intern_unlink_app(app);
    return 1;
  }
  return 0;
}

DLL_EXPORT UPDI_bool UPDILIB_read_hex(UPDI_Params * _cfg, char * _data, int32_t * _len) {
  if (!_cfg) return 0;
  if (!_data) return 0;
  if (!_len) return 0;
  if (*_len <= 0) return 0;

  UPDI_APP * app = intern_link_to_app(_cfg);
  if (app) {
    LOG_Print(app->logger, LOG_LEVEL_INFO, "Reading flash to hex data");

    NVM_raw_data data;
    data.data = _data;
    data.len = *_len;
    data.pos = 0;
    NVM_SaveIhexRaw(app, &data, DEVICES_GetFlashStart(app->DEVICE_Id), DEVICES_GetFlashLength(app->DEVICE_Id));
    *_len = data.pos;
    intern_unlink_app(app);
    return 1;
  }
  return 0;
}

#ifdef __cplusplus
}
#endif
