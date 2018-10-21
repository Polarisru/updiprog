#ifndef PHY_H
#define PHY_H

#include <stdint.h>
#include <stdbool.h>

#define PHY_BAUDRATE      (115200)

bool PHY_Init(char *port, uint32_t baudrate, bool onDTR);
bool PHY_DoBreak(char *port);
bool PHY_Send(uint8_t *data, uint8_t len);
bool PHY_Receive(uint8_t *data, uint16_t len);
void PHY_Close(void);

#endif
