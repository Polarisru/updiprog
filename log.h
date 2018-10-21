#ifndef LOG_H
#define LOG_H

#include <stdint.h>

enum {
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_LAST
};

void LOG_Print(uint8_t level, char *msg, ...);
void LOG_SetLevel(uint8_t level);

#endif
