#include <stdio.h>
#include <string.h>
#include "progress.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** \brief Print progress bar with prefix
 *
 * \param [in] iteration Current iteration
 * \param [in] total Total number of iterations
 * \param [in] prefix Prefix text
 * \param [in] fill Char to use for filling the bar
 * \return Noting
 *
 */
void PROGRESS_Print(uint16_t iteration, uint16_t total, char *prefix, char fill)
{
  #ifdef string_logger

  #else
  uint8_t filledLength;
  float percent;
  char bar[PROGRESS_BAR_LENGTH + 1];
  char bar2[PROGRESS_BAR_LENGTH + 1];

  memset(bar, fill, PROGRESS_BAR_LENGTH);
  bar[PROGRESS_BAR_LENGTH] = 0;
  memset(bar2, ' ', PROGRESS_BAR_LENGTH);
  bar2[PROGRESS_BAR_LENGTH] = 0;
  percent = (float)iteration / total * 100;
  filledLength = (uint8_t)(PROGRESS_BAR_LENGTH * iteration / total);

  printf("\r%s [%.*s%.*s] %.1f%%", prefix, filledLength, bar, PROGRESS_BAR_LENGTH - filledLength, bar2, percent);
  fflush(stdout);

  // Print New Line on Complete
  if (iteration >= total)
    printf("\n");
  #endif // string_logger
}

/** \brief Do break in the output
 *
 * \return Nothing
 *
 */
void PROGRESS_Break(void)
{
  #ifdef string_logger

  #else
  printf("\n");
  #endif
}

#ifdef __cplusplus
}
#endif
