#ifndef PHY_H
#define PHY_H

#include <stdint.h>
#include <stdbool.h>

#include "com.h"

#define PHY_BAUDRATE      (115200)

UPDI_COM_port * PHY_Init(char *port, uint32_t baudrate, bool onDTR);
bool PHY_DoBreak(UPDI_COM_port **);
bool PHY_Send(UPDI_COM_port *, uint8_t *data, uint8_t len);
bool PHY_Receive(UPDI_COM_port *, uint8_t *data, uint16_t len);
void PHY_Close(UPDI_COM_port **);

#endif
