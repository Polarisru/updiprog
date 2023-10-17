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

DLL_EXPORT uint32_t UPDILIB_cfg_get_baudrate(UPDI_Params * _cfg) {
    if (_cfg) {
        return _cfg->baudrate;
    }
    return 0;
}

DLL_EXPORT const char * UPDILIB_cfg_get_com(UPDI_Params * _cfg) {
    if (_cfg) {
        return _cfg->port;
    }
    return NULL;
}

DLL_EXPORT int8_t UPDILIB_cfg_get_device(UPDI_Params * _cfg) {
    if (_cfg) {
        return _cfg->device;
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

DLL_EXPORT uint8_t UPDILIB_devices_get_fuses_cnt(int8_t _id) {
    if (_id < 0) return 0;
    if (_id >= DEVICES_GetNumber()) return 0;

    return DEVICES_GetFusesNumber(_id);
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

  return app;
}

DLL_HIDDEN UPDI_bool intern_unlink_app(UPDI_APP * app) {

  NVM_LeaveProgmode(app);
  APP_Done(app);

  return 1;
}

DLL_EXPORT UPDI_bool UPDILIB_erase(UPDI_Params * _cfg) {
    UPDI_seq seq [2] = {
        { UPDI_SEQ_ENTER_PM,  NULL, 0 },
        { UPDI_SEQ_ERASE, NULL, 0 },
    };
    return UPDILIB_launch_seq(_cfg, seq, 2);
}

DLL_EXPORT UPDI_bool UPDILIB_write_fuses(UPDI_Params * _cfg, const UPDI_fuse * _fuses, uint8_t _cnt) {
    UPDI_seq seq [2] = {
        { UPDI_SEQ_ENTER_PM,  NULL, 0 },
        { UPDI_SEQ_SET_FUSES, (void*)_fuses, _cnt },
    };
    return UPDILIB_launch_seq(_cfg, seq, 2);
}

DLL_EXPORT UPDI_bool UPDILIB_read_fuses(UPDI_Params * _cfg, UPDI_fuse * _fuses, uint8_t _cnt) {
    UPDI_seq seq [2] = {
        { UPDI_SEQ_ENTER_PM,  NULL, 0 },
        { UPDI_SEQ_GET_FUSES, (void*)_fuses, _cnt },
    };
    return UPDILIB_launch_seq(_cfg, seq, 2);
}

DLL_EXPORT UPDI_bool UPDILIB_write_hex(UPDI_Params * _cfg, const char * _data, int32_t _len) {
    UPDI_seq seq [2] = {
        { UPDI_SEQ_ENTER_PM,  NULL, 0 },
        { UPDI_SEQ_FLASH, (void*)_data, _len },
    };
    return UPDILIB_launch_seq(_cfg, seq, 2);
}

DLL_EXPORT UPDI_bool UPDILIB_read_hex(UPDI_Params * _cfg, char * _data, int32_t * _len) {
    if (!_len) return 0;

    UPDI_seq seq [2] = {
        { UPDI_SEQ_ENTER_PM,  NULL, 0 },
        { UPDI_SEQ_READ, _data, *_len },
    };
    UPDI_bool res = UPDILIB_launch_seq(_cfg, seq, 2);
    if (res)
        *_len = seq[1].data_len;

    return res;
}

DLL_EXPORT UPDI_bool UPDILIB_launch_seq(UPDI_Params * _cfg,
                                        UPDI_seq * _seq,
                                        uint8_t _seq_cnt) {
    if (!_cfg) return 0;
    if (!_seq) return 0;
    if (_seq_cnt == 0) return 0;

    UPDI_APP * app = intern_link_to_app(_cfg);
    if (app) {
        bool break_seq = true;
        int i = 0;
        while ((i < _seq_cnt) && break_seq) {
            switch (_seq[i].seq_type) {
            case UPDI_SEQ_LOCK: {
                LOG_Print(app->logger,  LOG_LEVEL_INFO,  "Locking MCU...   ");
                if (NVM_WriteFuse(app, DEVICE_LOCKBIT_ADDR, 0x00) == true)
                {
                  LOG_Print(app->logger,  LOG_LEVEL_INFO,  "OK");
                } else
                  break_seq = false;

                break;
            }
            case UPDI_SEQ_UNLOCK: {
                LOG_Print(app->logger,  LOG_LEVEL_INFO, "Unlocking...   ");
                if (NVM_UnlockDevice(app) == true)
                {
                    LOG_Print(app->logger,  LOG_LEVEL_INFO,  "OK");
                } else
                  break_seq = false;

                break;
            }
            case UPDI_SEQ_ENTER_PM: {
                if (NVM_EnterProgmode(app) == false)
                {
                    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Can't enter programming mode, exiting");
                    break_seq = false;
                }

                break;
            }
            case UPDI_SEQ_ERASE: {
                LOG_Print(app->logger, LOG_LEVEL_INFO, "Erasing");
                if (!NVM_ChipErase(app))
                    break_seq = false;
                break;
            }
            case UPDI_SEQ_FLASH: {
                if (!_seq[i].data) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, "Passed NULL data pointer to flash");
                   break_seq = false;
                   break;
                }
                if (_seq[i].data_len <= 0) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, "Passed empty data to flash");
                   break_seq = false;
                   break;
                }

                if (break_seq) {
                    LOG_Print(app->logger, LOG_LEVEL_INFO, "Writing from hex data");

                    NVM_raw_data data;
                    data.data = (char*)_seq[i].data;
                    data.len = _seq[i].data_len;
                    data.pos = 0;

                    if (!NVM_LoadIhexRaw(app, &data, DEVICES_GetFlashStart(app->DEVICE_Id), DEVICES_GetFlashLength(app->DEVICE_Id)))
                        break_seq = false;
                }

                break;
            }
            case UPDI_SEQ_READ: {
                if (!_seq[i].data) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, "Passed NULL data pointer");
                   break_seq = false;
                   break;
                }
                if (_seq[i].data_len <= 0) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, "Passed empty data to read flash");
                   break_seq = false;
                   break;
                }

                if (break_seq) {
                    LOG_Print(app->logger, LOG_LEVEL_INFO, "Reading flash to hex data");

                    NVM_raw_data data;
                    data.data = (char*)_seq[i].data;
                    data.len = _seq[i].data_len;
                    data.pos = 0;
                    if (!NVM_SaveIhexRaw(app, &data, DEVICES_GetFlashStart(app->DEVICE_Id), DEVICES_GetFlashLength(app->DEVICE_Id)))
                        break_seq = false;
                    else
                        _seq[i].data_len = data.pos;
                }

                break;
            }
            case UPDI_SEQ_SET_FUSES: {
                if (!_seq[i].data) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, "Passed NULL data pointer as fuses array");
                   break_seq = false;
                   break;
                }
                if (_seq[i].data_len <= 0) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, "Passed empty fuses array");
                   break_seq = false;
                   break;
                }

                if (break_seq) {
                    UPDI_fuse * _fuses = (UPDI_fuse *)(_seq[i].data);
                    int32_t _cnt = _seq[i].data_len;

                    LOG_Print(app->logger, LOG_LEVEL_INFO, "Writing fuses:");

                    int fn = 0;
                    while (fn < _cnt)
                    {
                      LOG_Print(app->logger, LOG_LEVEL_INFO, "Writing 0x%02X to fuse Nr. %d", _fuses[fn].value, _fuses[fn].fuse);
                      NVM_WriteFuse(app, _fuses[fn].fuse, _fuses[fn].value);
                      fn++;
                    }
                }

                break;
            }
            case UPDI_SEQ_GET_FUSES: {
                if (!_seq[i].data) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, "Passed NULL data pointer as fuses array");
                   break_seq = false;
                   break;
                }
                if (_seq[i].data_len <= 0) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, "Passed empty fuses array");
                   break_seq = false;
                   break;
                }

                if (break_seq) {
                    UPDI_fuse * _fuses = (UPDI_fuse *)(_seq[i].data);
                    int32_t _cnt = _seq[i].data_len;

                    LOG_Print(app->logger, LOG_LEVEL_INFO, "Reading fuses:");
                    int max_n =  DEVICES_GetFusesNumber(app->DEVICE_Id);
                    int fn = 0;
                    while (fn < _cnt)
                    {
                      int n = _fuses[fn].fuse;
                      if (n < max_n) {
                          uint8_t x = NVM_ReadFuse(app, n);
                          LOG_Print(app->logger, LOG_LEVEL_INFO, "  0x%02X: 0x%02X", n, x);
                          _fuses[fn].value = x;
                      }
                      fn++;
                    }
                }

                break;
            }


            default:
                LOG_Print(app->logger, LOG_LEVEL_WARNING, "Unknown seq type %d", (int)_seq[i].seq_type);
                break_seq = false;
                break;
            }

            i++;
        }
        intern_unlink_app(app);
        if (break_seq)
            return 1;
        else
            return 0;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif
