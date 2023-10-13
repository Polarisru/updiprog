#ifndef COM_H
#define COM_H

#ifdef __MINGW32__
#include <windows.h>
#include <winbase.h>
#endif
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__linux)
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define COMPORT_LEN     (32)

typedef struct {
#ifdef __MINGW32__
 HANDLE hSerial;
#endif
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__linux)
 int fd;
#endif
 uint32_t COM_Baudrate;
 char port [COMPORT_LEN];
} UPDI_COM_port;

UPDI_COM_port* COM_Open(char *port, uint32_t baudrate, bool have_parity, bool two_stopbits);
int COM_Write(UPDI_COM_port*,uint8_t *data, uint16_t len);
int COM_Read(UPDI_COM_port*,uint8_t *data, uint16_t len);
uint16_t COM_GetTransTime(UPDI_COM_port*,uint16_t len);
void COM_WaitForTransmit(UPDI_COM_port*);
void COM_Close(UPDI_COM_port**);

#ifdef __cplusplus
}
#endif

#endif
