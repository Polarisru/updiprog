#ifndef PROGRESS_H
#define PROGRESS_H

#include <stdint.h>
#include <stdbool.h>

#define PROGRESS_BAR_LENGTH   (20)

#ifdef __cplusplus
extern "C"
{
#endif

void PROGRESS_Print(uint16_t iteration, uint16_t total, char *prefix, char fill);
void PROGRESS_Break(void);

#ifdef __cplusplus
}
#endif

#endif // PROGRESS_H
