#include <stdio.h>
#include <stdarg.h>
#include "log.h"

#ifdef string_logger

#include <stdlib.h>
#include <string.h>

str_log * str_log_init() {
    str_log * res = (str_log *)malloc(sizeof(str_log));
    if (res)
        memset(res, 0, sizeof(str_log));
    return res;
}

void str_log_push(str_log * logger, const char * msg) {
    str_log_entry * res = (str_log_entry *)malloc(sizeof(str_log_entry));
    if (res) {
        memset(res, 0, sizeof(str_log_entry));
        if (logger->last)
            logger->last->next = res;
        else
            logger->first = res;
        logger->last = res;
        int len = strlen(msg);
        res->value = (char *)malloc(len + 1);
        if (res->value) {
            memcpy(res->value, msg, len);
            res->value[len] = 0;
        }
    }
}

str_log_entry * str_log_pop(str_log * logger) {
    if (logger->cnt > 0) {
        str_log_entry * res = logger->first;
        if (res == logger->last) {
          logger->first = NULL;
          logger->last = NULL;
        } else
          logger->first = res->next;
        logger->cnt--;
        return res;
    } else {
        return NULL;
    }
}

void str_log_entry_done(str_log_entry * entry) {
    if (entry->value) free(entry->value);
    free(entry);
}

void str_log_done(str_log * logger) {
    str_log_entry * entry = logger->first;
    while (entry) {
        str_log_entry * e = entry->next;
        str_log_entry_done(entry);
        entry = e;
    }
    free(logger);
}

static str_log global_log_obj = { NULL, NULL, 0 };

#else

static UPDI_logger global_log_obj = { LOG_LEVEL_ERROR };

#endif

UPDI_logger * global_LOG() {
    return (UPDI_logger*)&global_log_obj;
}

void LOG_Print_GLOBAL(uint8_t level, char *msg, ...)
{
  va_list args;

  va_start(args, msg);
  LOG_Print(global_LOG(), level, msg, args);
  va_end(args);
}

/** \brief Print log message according level settings
 *
 * \param [in] level Log level for the message
 * \param [in] msg Message text
 * \param [in] additional parameters (formatters and values)
 * \return Nothing
 *
 */
void LOG_Print(UPDI_logger * logger, uint8_t level, char *msg, ...)
{
  va_list args;

  if (level < logger->LOG_Level)
    return;

#ifdef string_logger
  static const int MAX_LOG_BUFFER = 1024;
  char buffer [MAX_LOG_BUFFER];
  memset(buffer, 0, MAX_LOG_BUFFER);
  va_start(args, msg);
  vsnprintf(buffer, MAX_LOG_BUFFER, msg, args);
  va_end(args);
#else
  switch (level)
  {
    case LOG_LEVEL_INFO:
      printf("INFO: ");
      break;
    case LOG_LEVEL_WARNING:
      printf("WARNING: ");
      break;
    case LOG_LEVEL_ERROR:
      printf("ERROR: ");
      break;
  }
  va_start(args, msg);
  vprintf(msg, args);
  va_end(args);
  printf("\n");
#endif
}

/** \brief Set log level (INFO/WARNING/ERROR)
 *
 * \param [in] level New level for the log
 * \return Nothing
 *
 */
void LOG_SetLevel(UPDI_logger * logger, uint8_t level)
{
  if (level >= LOG_LEVEL_LAST)
    return;
  logger->LOG_Level = level;
}
