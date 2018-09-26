#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <stdbool.h>

extern bool APP_EnterProgmode(void);
extern void APP_LeaveProgmode(void);
extern bool APP_WaitFlashReady(void);
extern bool APP_Unlock(void);
extern bool APP_ChipErase(void);
extern bool APP_ReadDataWords(uint16_t address, uint8_t *data, uint16_t words);
extern bool APP_WriteData(uint16_t address, uint8_t *data, uint16_t len);
extern bool APP_WriteNvm(uint16_t address, uint8_t *data, uint16_t len, bool use_word_access);

#endif
