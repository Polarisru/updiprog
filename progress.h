#ifndef PROGRESS_H
#define PROGRESS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef UPDI_CLI_mode

#define PROGRESS_BAR_LENGTH   (20)

#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*UPDI_onprgs) (void*, uint16_t, uint16_t);
typedef void (*UPDI_onprgsstart) (void*, uint16_t);
typedef void (*UPDI_onprgsfinish) (void*, uint16_t, uint16_t);

typedef struct {
  void * ud;
  uint16_t iteration;
  uint16_t total;
  UPDI_onprgs       onprogress;
  UPDI_onprgsstart  onstart;
  UPDI_onprgsfinish onfinish;
} UPDI_progress;

UPDI_progress * PROGRESS_Init(void *, UPDI_onprgsstart, UPDI_onprgs, UPDI_onprgs);
void PROGRESS_SetUserData(UPDI_progress*, void*);
void PROGRESS_Start(UPDI_progress*,uint16_t total);
void PROGRESS_Step(UPDI_progress*,uint16_t iteration);
void PROGRESS_Break(UPDI_progress *);
void PROGRESS_Done(UPDI_progress *);

#ifdef UPDI_CLI_mode

typedef struct {
  const char * prompt;
  char   sym;
} UPDI_cli_ud;

void PROGRESS_CLI_step(void*, uint16_t, uint16_t);
void PROGRESS_CLI_start(void*, uint16_t);
void PROGRESS_CLI_stop(void*, uint16_t, uint16_t);

#endif

#ifdef __cplusplus
}
#endif

#endif // PROGRESS_H
