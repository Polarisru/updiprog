#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <stdbool.h>

bool APP_EnterProgmode(void);
void APP_LeaveProgmode(void);
bool APP_WaitFlashReady(void);
bool APP_Unlock(void);
bool APP_ChipErase(void);
bool APP_ReadDataWords(uint16_t address, uint8_t *data, uint16_t words);
bool APP_WriteData(uint16_t address, uint8_t *data, uint16_t len);
bool APP_WriteNvm(uint16_t address, uint8_t *data, uint16_t len, bool use_word_access);

#endif
