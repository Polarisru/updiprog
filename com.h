#ifndef COM_H
#define COM_H

#include <stdint.h>
#include <stdbool.h>

bool COM_Open(char *port, uint32_t baudrate, bool have_parity, bool two_stopbits);
int COM_Write(uint8_t *data, uint16_t len);
int COM_Read(uint8_t *data, uint16_t len);
uint16_t COM_GetTransTime(uint16_t len);
void COM_WaitForTransmit(void);
void COM_Close(void);

#endif
