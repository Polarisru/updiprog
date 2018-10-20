#include <stdio.h>
#include <stdarg.h>
#include "log.h"

static uint8_t LOG_Level = LOG_LEVEL_ERROR;

/** \brief Print log message according level settings
 *
 * \param [in] level Log level for the message
 * \param [in] msg Message text
 * \param [in] additional parameters (formatters and values)
 * \return Nothing
 *
 */
void LOG_Print(uint8_t level, char *msg, ...)
{
  va_list args;

  if (level < LOG_Level)
    return;

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
}

/** \brief Set log level (INFO/WARNING/ERROR)
 *
 * \param [in] level New level for the log
 * \return Nothing
 *
 */
void LOG_SetLevel(uint8_t level)
{
  if (level >= LOG_LEVEL_LAST)
    return;
  LOG_Level = level;
}
