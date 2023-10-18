#include "updilib.h"
#include "stdlib.h"
#include "string.h"
#include "../app.h"
#include "../devices.h"
#include "../link.h"
#include "../log.h"
#include "../progress.h"
#include "../nvm.h"
#include "../phy.h"
#include "libmsgs.h"

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

DLL_EXPORT UPDI_progress * UPDILIB_progress_init(UPDI_onprgsstart _onstart, UPDI_onprgs _onstep, UPDI_onprgsfinish _onfinish, void * _ud) {
     return PROGRESS_Init(_ud, _onstart, _onstep, _onfinish);

}
DLL_EXPORT void UPDILIB_progress_done(UPDI_progress * _po) {
    PROGRESS_Done(_po);
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

DLL_EXPORT UPDI_bool UPDILIB_cfg_set_progress(UPDI_Params * _cfg, UPDI_progress * _po) {
    if (_cfg) {
        _cfg->progress = _po;
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
  UPDI_APP * app = APP_Init(_cfg->logger, _cfg->progress);

  if (!app) return NULL;

  app->DEVICE_Id = _cfg->device;

  if (LINK_Init(app, _cfg->port, _cfg->baudrate, false) == false)
  {
    LOG_Print(app->logger, LOG_LEVEL_ERROR, MSG_CANT_OPEN_PORT, _cfg->port);
    LOG_Print(app->logger, LOG_LEVEL_ERROR, MSG_CHECK_CONNECTION);
    APP_Done(app);
    return NULL;
  }

  LOG_Print(app->logger, LOG_LEVEL_INFO, MSG_WORKING_WITH_DEVICE, DEVICES_GetNameByNumber(_cfg->device));

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

DLL_EXPORT UPDI_bool UPDILIB_read_dev_info(UPDI_Params * _cfg, uint8_t * _data) {
    UPDI_seq seq [2] = {
        { UPDI_SEQ_ENTER_PM,  NULL, 0 },
        { UPDI_SEQ_GET_SIG_ROW, (void*)_data, 0 },
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
                LOG_Print(app->logger,  LOG_LEVEL_INFO,  MSG_LOCKING);
                if (NVM_WriteFuse(app, DEVICE_LOCKBIT_ADDR, 0x00) == true)
                  LOG_Print(app->logger,  LOG_LEVEL_INFO,  MSG_OK);
                else
                  break_seq = false;

                break;
            }
            case UPDI_SEQ_UNLOCK: {
                LOG_Print(app->logger,  LOG_LEVEL_INFO, MSG_UNLOCKING);
                if (NVM_UnlockDevice(app) == true)
                  LOG_Print(app->logger,  LOG_LEVEL_INFO,  MSG_OK);
                else
                  break_seq = false;

                break;
            }
            case UPDI_SEQ_ENTER_PM: {
                if (NVM_EnterProgmode(app) == false)
                {
                    LOG_Print(app->logger, LOG_LEVEL_ERROR, MSG_CANT_ENTER_PROG_MODE);
                    break_seq = false;
                }

                break;
            }
            case UPDI_SEQ_ERASE: {
                LOG_Print(app->logger, LOG_LEVEL_INFO, MSG_ERASING);
                if (NVM_ChipErase(app) == true)
                    LOG_Print(app->logger,  LOG_LEVEL_INFO,  MSG_OK);
                else
                    break_seq = false;
                break;
            }
            case UPDI_SEQ_FLASH: {
                if (!_seq[i].data) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, MSG_WRONG_DATA);
                   break_seq = false;
                   break;
                }
                if (_seq[i].data_len <= 0) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, MSG_PASSED_EMPTY_DATA);
                   break_seq = false;
                   break;
                }

                if (break_seq) {
                    LOG_Print(app->logger, LOG_LEVEL_INFO, MSG_WRITING_FLASH);

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
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, MSG_WRONG_DATA);
                   break_seq = false;
                   break;
                }
                if (_seq[i].data_len <= 0) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, MSG_PASSED_EMPTY_DATA);
                   break_seq = false;
                   break;
                }

                if (break_seq) {
                    LOG_Print(app->logger, LOG_LEVEL_INFO, MSG_READING_FLASH);

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
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, MSG_WRONG_DATA);
                   break_seq = false;
                   break;
                }
                if (_seq[i].data_len <= 0) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, MSG_PASSED_EMPTY_DATA);
                   break_seq = false;
                   break;
                }

                if (break_seq) {
                    UPDI_fuse * _fuses = (UPDI_fuse *)(_seq[i].data);
                    int32_t _cnt = _seq[i].data_len;

                    LOG_Print(app->logger, LOG_LEVEL_INFO, MSG_WRITING_FUSES);

                    int fn = 0;
                    while (fn < _cnt)
                    {
                      LOG_Print(app->logger, LOG_LEVEL_INFO, MSG_WRITE_FUSE, _fuses[fn].value, _fuses[fn].fuse);
                      NVM_WriteFuse(app, _fuses[fn].fuse, _fuses[fn].value);
                      fn++;
                    }
                }

                break;
            }
            case UPDI_SEQ_GET_FUSES: {
                if (!_seq[i].data) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, MSG_WRONG_DATA);
                   break_seq = false;
                   break;
                }
                if (_seq[i].data_len <= 0) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, MSG_PASSED_EMPTY_DATA);
                   break_seq = false;
                   break;
                }

                if (break_seq) {
                    UPDI_fuse * _fuses = (UPDI_fuse *)(_seq[i].data);
                    int32_t _cnt = _seq[i].data_len;

                    LOG_Print(app->logger, LOG_LEVEL_INFO, MSG_READING_FUSES);
                    int max_n =  DEVICES_GetFusesNumber(app->DEVICE_Id);
                    int fn = 0;
                    while (fn < _cnt)
                    {
                      int n = _fuses[fn].fuse;
                      if (n < max_n) {
                          uint8_t x = NVM_ReadFuse(app, n);
                          LOG_Print(app->logger, LOG_LEVEL_INFO, MSG_FUSE, n, x);
                          _fuses[fn].value = x;
                      }
                      fn++;
                    }
                }

                break;
            }
            case UPDI_SEQ_GET_SIG_ROW: {
                if (!_seq[i].data) {
                   LOG_Print(app->logger, LOG_LEVEL_ERROR, MSG_WRONG_DATA);
                   break_seq = false;
                   break;
                }

                if (break_seq) {
                    if (NVM_GetDeviceInfo(app))
                      memcpy(_seq[i].data, &app->DEVICE_sigrow[0], DEV_INFO_LEN);
                    else {
                      memset(_seq[i].data, 0, DEV_INFO_LEN);
                      break_seq = false;
                    }
                }

                break;
            }

            default:
                LOG_Print(app->logger, LOG_LEVEL_WARNING, MSG_WRONG_SEQ, (int)_seq[i].seq_type);
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
