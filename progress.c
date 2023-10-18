#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "progress.h"

#ifdef __cplusplus
extern "C"
{
#endif

UPDI_progress * PROGRESS_Init(void * _ud, UPDI_onprgsstart _onstart,
                                          UPDI_onprgs _onstep,
                                          UPDI_onprgs _onfinish) {
   UPDI_progress * res = (UPDI_progress *)malloc(sizeof(UPDI_progress));
   if (res) {
      res->ud = _ud;
      res->iteration = 0;
      res->total = 0;
      res->onfinish = _onfinish;
      res->onstart = _onstart;
      res->onprogress = _onstep;
   }
   return res;
}

void PROGRESS_SetUserData(UPDI_progress* _po, void* _ud) {
   if (_po) {
      _po->ud = _ud;
   }
}

void PROGRESS_Start(UPDI_progress* _po, uint16_t _total) {
   if (_po) {
       _po->total = _total;
       if (_po->onstart)
         _po->onstart(_po->ud, _total);
   }
}

void PROGRESS_Step(UPDI_progress* _po, uint16_t _iteration) {
   if (_po) {
       _po->iteration = _iteration;
       if (_po->onprogress)
         _po->onprogress(_po->ud, _po->iteration, _po->total);
   }
}

void PROGRESS_Break(UPDI_progress * _po) {
    if (_po) {
       if (_po->onfinish)
         _po->onfinish(_po->ud, _po->iteration, _po->total);
   }
}

void PROGRESS_Done(UPDI_progress * _po) {
    if (_po)
        free(_po);
}


#ifdef UPDI_CLI_mode

void PROGRESS_CLI_step(void*_ud, uint16_t _iteration, uint16_t _total) {
  UPDI_cli_ud * cliud = (UPDI_cli_ud *)_ud;
  if (!cliud) return;

  uint8_t filledLength;
  float percent;
  char bar[PROGRESS_BAR_LENGTH + 1];
  char bar2[PROGRESS_BAR_LENGTH + 1];

  memset(bar, cliud->sym, PROGRESS_BAR_LENGTH);
  bar[PROGRESS_BAR_LENGTH] = 0;
  memset(bar2, ' ', PROGRESS_BAR_LENGTH);
  bar2[PROGRESS_BAR_LENGTH] = 0;
  percent = (float)_iteration / _total * 100;
  filledLength = (uint8_t)(PROGRESS_BAR_LENGTH * _iteration / _total);

  printf("\r%s [%.*s%.*s] %.1f%%", cliud->prompt, filledLength, bar, PROGRESS_BAR_LENGTH - filledLength, bar2, percent);
  fflush(stdout);
}

void PROGRESS_CLI_start(void*_ud, uint16_t _total) {
  // do nothing
}

void PROGRESS_CLI_stop(void*_ud, uint16_t _iteration, uint16_t _total) {
  // Print New Line on Complete
  printf("\n");
}

#endif  // string_logger

#ifdef __cplusplus
}
#endif
