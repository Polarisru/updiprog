#ifndef COM_H
#define COM_H

#include <stdint.h>
#include <stdbool.h>

extern bool COM_Open(char *port, uint32_t baudrate, bool have_parity, bool two_stopbits);
extern int COM_Write(uint8_t *data, uint16_t len);
extern int COM_Read(uint8_t *data, uint16_t len);
extern uint16_t COM_GetTransTime(uint16_t len);
extern void COM_WaitForTransmit(void);
extern void COM_Close(void);

#endif
