#include <stdio.h>
#include <string.h>
#include "progress.h"

void PROGRESS_Print(uint16_t iteration, uint16_t total, char *prefix, char fill)
{
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
}

void PROGRESS_Break(void)
{
  printf("\n");
}
