#ifndef LOG_H
#define LOG_H

#include <stdint.h>

#define LOG_SRCNAME_LEN (8)

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*UPDI_onlog) (void*, int32_t, const char *, const char *);
typedef void (*UPDI_onlogfree) (void*);

enum {
  LOG_LEVEL_INFO = 0,
  LOG_LEVEL_WARNING = 1,
  LOG_LEVEL_ERROR = 2,
  LOG_LEVEL_LAST = 3
};

typedef struct {
  uint8_t LOG_Level;
  char src_name[LOG_SRCNAME_LEN];
  UPDI_onlog  onlog;
  UPDI_onlogfree onfree;
  void * userdata;
} UPDI_logger;

void LOG_Print(UPDI_logger * logger, uint8_t level, const char *msg, ...);
void LOG_SetLevel(UPDI_logger * logger, uint8_t level);
UPDI_logger * global_LOG();

UPDI_logger * UPDI_logger_init(const char *, int32_t, UPDI_onlog, UPDI_onlogfree, void*);
void UPDI_logger_done(UPDI_logger *);

#define LOG_Print_GLOBAL(...) LOG_Print(global_LOG(), __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
