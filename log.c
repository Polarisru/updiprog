#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "log.h"

#ifdef __cplusplus
extern "C"
{
#endif

static UPDI_logger global_log_obj = { LOG_LEVEL_ERROR, "glb", NULL, NULL, NULL };

UPDI_logger * UPDI_logger_init(const char * _src, int32_t _level, UPDI_onlog _onlog, UPDI_onlogfree _onfree, void* ud) {
     UPDI_logger * res = (UPDI_logger *)malloc(sizeof(UPDI_logger));
     if (res) {
        memset(res, 0, sizeof(UPDI_logger));
        memcpy(&(res->src_name[0]), _src, LOG_SRCNAME_LEN - 1);
        res->LOG_Level = _level;
        res->onlog = _onlog;
        res->onfree = _onfree;
        res->userdata = ud;
     }
     return NULL;
}

void UPDI_logger_done(UPDI_logger * logger) {
     if (logger) {
        if (logger->onfree)
            logger->onfree(logger->userdata);

        free(logger);
     }
}

UPDI_logger * global_LOG() {
    return (UPDI_logger*)&global_log_obj;
}

/** \brief Print log message according level settings
 *
 * \param [in] level Log level for the message
 * \param [in] msg Message text
 * \param [in] additional parameters (formatters and values)
 * \return Nothing
 *
 */
void LOG_Print(UPDI_logger * logger, uint8_t level, const char *msg, ...)
{
  if (level < logger->LOG_Level)
    return;

  if (logger->onlog) {
    va_list args;

    static const int MAX_LOG_BUFFER = 1024;
    char buffer [MAX_LOG_BUFFER];
    memset(buffer, 0, MAX_LOG_BUFFER);
    va_start(args, msg);
    vsnprintf(buffer, MAX_LOG_BUFFER, msg, args);
    va_end(args);

    logger->onlog(logger->userdata, level, logger->src_name, buffer);
  }
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

#ifdef __cplusplus
}
#endif
