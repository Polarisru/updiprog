#ifndef LOG_H
#define LOG_H

#include <stdint.h>

enum {
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_LAST
};

typedef struct {
  uint8_t LOG_Level;
} UPDI_logger;

void LOG_Print(UPDI_logger * logger, uint8_t level, char *msg, ...);
void LOG_SetLevel(UPDI_logger * logger, uint8_t level);
void LOG_Print_GLOBAL(uint8_t level, char *msg, ...);
UPDI_logger * global_LOG();

#ifdef string_logger

typedef struct str_log_entry_t {
    char * value;
    struct str_log_entry_t * next;
} str_log_entry;

typedef struct str_log_t {
    UPDI_logger * logger;
    str_log_entry * first;
    str_log_entry * last;
    int cnt;
} str_log;

str_log * str_log_init();
void str_log_push(str_log * logger, const char * msg);
str_log_entry * str_log_pop(str_log * logger);
void str_log_entry_done(str_log_entry * entry);
void str_log_done(str_log * logger);

#endif

#endif
