#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "com.h"
#include "log.h"
#include "msgs.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** \brief Open COM port with settings
 *
 * \param [in] port Port name as string
 * \param [in] baudrate Port baudrate
 * \param [in] have_parity true if parity should be switched on
 * \param [in] two_stopbits true if 2 stop bits used
 * \return true if succeed
 *
 */
UPDI_COM_port* COM_Open(char *port, uint32_t baudrate, bool have_parity, bool two_stopbits)
{
  int port_len = strlen(port);
  if ((port_len <= 1) || (port_len >= COMPORT_LEN)) return NULL;

  UPDI_COM_port * res = (UPDI_COM_port *)malloc(sizeof(UPDI_COM_port));

  if (!res) return NULL;

  LOG_Print_GLOBAL(LOG_LEVEL_INFO, MSG_OPEN_COM, port, baudrate);

  memcpy(&(res->port[0]), port, port_len);
  res->port[port_len] = 0;

  res->COM_Baudrate = baudrate;
  #ifdef __MINGW32__
  char str[64];
  uint8_t multiplier;

  sprintf(str, "\\\\.\\%s", port);
  res->hSerial = CreateFile(str, GENERIC_READ | GENERIC_WRITE, 0,
                              NULL, OPEN_EXISTING, 0, NULL);
  if (res->hSerial == INVALID_HANDLE_VALUE) {
    free(res);
    return NULL;
  }
  DCB dcbSerialParams = { 0 }; // Initializing DCB structure
  dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
  GetCommState(res->hSerial, &dcbSerialParams);
  dcbSerialParams.BaudRate = baudrate;  // Setting BaudRate
  dcbSerialParams.ByteSize = 8;         // Setting ByteSize = 8
  if (two_stopbits == true)
    dcbSerialParams.StopBits = TWOSTOPBITS;
  else
    dcbSerialParams.StopBits = ONESTOPBIT;
  if (have_parity == true)
    dcbSerialParams.Parity   = EVENPARITY;  // Setting Parity = None
  else
    dcbSerialParams.Parity   = NOPARITY;
  dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
  SetCommState(res->hSerial, &dcbSerialParams);
  COMMTIMEOUTS timeouts;
  multiplier = (uint8_t)ceil((float)100000 / baudrate);
  timeouts.ReadIntervalTimeout = 20 * multiplier;
  timeouts.ReadTotalTimeoutMultiplier = 1 * multiplier;
  timeouts.ReadTotalTimeoutConstant = 100 * multiplier;
  timeouts.WriteTotalTimeoutMultiplier = 1;
  timeouts.WriteTotalTimeoutConstant = 1;
  SetCommTimeouts(res->hSerial, &timeouts);
  //COM_Bytes = 0;
  #endif

  #if defined(__APPLE__) || defined(__FreeBSD__) || defined(__linux)
  res->fd = open(port, O_RDWR | O_NOCTTY | O_SYNC );
  if (res->fd <0) {
    free(res);
    return false;
  }
  struct termios SerialPortSettings;
  tcgetattr(res->fd, &SerialPortSettings);	/* Get the current attributes of the Serial port */
  /* Setting the Baud rate */
  switch (baudrate)
  {
    case 300:
      cfsetispeed(&SerialPortSettings, B300);
      cfsetospeed(&SerialPortSettings, B300);
      break;
    case 9600:
      cfsetispeed(&SerialPortSettings, B9600);
      cfsetospeed(&SerialPortSettings, B9600);
      break;
    case 38400:
      cfsetispeed(&SerialPortSettings, B38400);
      cfsetospeed(&SerialPortSettings, B38400);
      break;
    case 57600:
      cfsetispeed(&SerialPortSettings, B57600);
      cfsetospeed(&SerialPortSettings, B57600);
      break;
    default:
    case 115200:
      cfsetispeed(&SerialPortSettings, B115200);
      cfsetospeed(&SerialPortSettings, B115200);
      break;
    case 230400:
      cfsetispeed(&SerialPortSettings, B230400);
      cfsetospeed(&SerialPortSettings, B230400);
      break;
  }
  cfmakeraw(&SerialPortSettings);           /* Set raw mode (special processing disabled) */
  if (have_parity == true)
    SerialPortSettings.c_cflag |= PARENB;   /* Enables the Parity Enable bit(PARENB) */
  else
    SerialPortSettings.c_cflag &= ~PARENB;  /* Disables the Parity Enable bit(PARENB), so No Parity */
  if (two_stopbits == true)
    SerialPortSettings.c_cflag |= CSTOPB;   /* CSTOPB = 2 Stop bits */
  else
    SerialPortSettings.c_cflag &= ~CSTOPB;  /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
  SerialPortSettings.c_cflag |= (CREAD | CLOCAL); /* Enable receiver,Ignore Modem Control lines       */
  SerialPortSettings.c_cflag &= ~ICANON;
  SerialPortSettings.c_cc[VMIN]  = 0;            // read doesn't block
  SerialPortSettings.c_cc[VTIME] = 50;            // 5 seconds read timeout
  tcsetattr(res->fd, TCSANOW, &SerialPortSettings);  /* Set the attributes to the termios structure*/
  tcflush(res->fd, TCIFLUSH);
  #endif

  return res;
}

/** \brief Write data to COM port
 *
 * \param [in] data Data buffer for writing
 * \param [in] len Length of data buffer
 * \return 0 if everything Ok
 *
 */
int COM_Write(UPDI_COM_port* port, uint8_t *data, uint16_t len)
{
  if (!port) return -1;

  #ifdef __MINGW32__
  DWORD dwBytesWritten = 0;
  //DWORD signal;
  //OVERLAPPED ov = { 0 };
  //int res;
  //ov.hEvent = CreateEvent(NULL, true, true, NULL);

  if (!WriteFile(port->hSerial, data, len, &dwBytesWritten, NULL))
    return -1;
  //COM_Bytes += dwBytesWritten;
//  WriteFile(port->hSerial, data, len, &dwBytesWritten, &ov);
//  signal = WaitForSingleObject(ov.hEvent, INFINITE);
//  if ((signal == WAIT_OBJECT_0) && (GetOverlappedResult(port->hSerial, &ov, &dwBytesWritten, true)))
//    res = 0;
//  else
//    res = -1;
//  CloseHandle(ov.hEvent);
//  return res;
  #endif
  #if defined(__APPLE__) || defined(__FreeBSD__) || defined(__linux)
  int iOut = write(port->fd, data, len);
  if (iOut < 0)
    return -1;
  #endif

  return 0;
}

/** \brief Read data from COM port
 *
 * \param [out] data Data buffer to read data in
 * \param [in] len Length of data to read
 * \return number of received bytes as int
 *
 */
int COM_Read(UPDI_COM_port* port, uint8_t *data, uint16_t len)
{
  if (!port) return -1;

  #ifdef __MINGW32__
  //OVERLAPPED ov = { 0 };
  //COMSTAT status;
  //DWORD errors;
  //DWORD mask, btr, temp, signal;
  DWORD dwBytesRead = 0;
//  ClearCommError(port->hSerial, &errors, &status);
//  if (!ReadFile(port->hSerial, data, len, &dwBytesRead, &ov))
//    return -1;

//  btr = 0;
//  while (btr < len)
//  {
//    SetCommMask(port->hSerial, EV_RXCHAR);
//    WaitCommEvent(port->hSerial, &mask, NULL);
//    if (mask & EV_ERR)
//      break;
//    ClearCommError(port->hSerial, &temp, &status);
//    btr = status.cbInQue;
//    if (btr >= len)
//    {
//      ReadFile(port->hSerial, data, len, &dwBytesRead, NULL);
//    }
//  }
  ReadFile(port->hSerial, data, len, &dwBytesRead, NULL);
  #endif
  #if defined(__APPLE__) || defined(__FreeBSD__) || defined(__linux)
  int dwBytesRead = 0;

  while (len > 0) {
      int c = read(port->fd, data, len);
      if (c < 0)  // error
        return -1;
      if (c == 0) // timeout
        break;
      data += c;
      dwBytesRead += c;
      len -= c;
  }
  #endif

  return dwBytesRead;
}

/** \brief Calculate time for transmission with current baudrate
 *
 * \param [in] len Length of transmitted data
 * \return time in milliseconds as uint16_t
 *
 */
uint16_t COM_GetTransTime(UPDI_COM_port* port, uint16_t len)
{
  return (uint16_t)(len * 1000 * 11 / port->COM_Baudrate + 1);
}

#ifdef __MINGW32__
void COM_WaitForTransmit(void)
{
  COMSTAT rStat;
  DWORD nErr;
  do {
    ClearCommError(hSerial, &nErr, &rStat);
  } while (rStat.cbOutQue > 0);
}
#endif // __MINGW32__

/** \brief Close current COM port
 *
 * \return Nothing
 *
 */
void COM_Close(UPDI_COM_port** port)
{
  if (!port) return;
  if (!(*port)) return;

  LOG_Print_GLOBAL(LOG_LEVEL_INFO, MSG_CLOSE_COM, (*port)->port);
  #ifdef __MINGW32__
  CloseHandle(port->hSerial);
  #endif
  #if defined(__APPLE__) || defined(__FreeBSD__) || defined(__linux)
  close((*port)->fd);
  #endif

  free(*port);
  *port = NULL;
}

#ifdef __cplusplus
}
#endif
