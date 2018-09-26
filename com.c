#ifdef __MINGW32__
#include <windows.h>
#include <winbase.h>
#endif
#ifdef __linux
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>
#endif
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __MINGW32__
static HANDLE hSerial;
#endif
#ifdef __linux
static int fd;
#endif

static uint32_t COM_Baudrate = 115200;
static uint16_t COM_Bytes;

/** \brief Open COM port with settings
 *
 * \param [in] port
 * \param [in] baudrate
 * \param [in] have_parity
 * \param [in] two_stopbits
 * \return true if succeed
 *
 */
bool COM_Open(char *port, uint32_t baudrate, bool have_parity, bool two_stopbits)
{
  char str[64];

  printf("Opening %s at %u baud\n", port, baudrate);
  #ifdef __MINGW32__
  sprintf(str, "\\\\.\\%s", port);
  hSerial = CreateFile(str, GENERIC_READ | GENERIC_WRITE, 0,
                              NULL, OPEN_EXISTING, 0, NULL);
  if (hSerial == INVALID_HANDLE_VALUE)
    return false;
  DCB dcbSerialParams = { 0 }; // Initializing DCB structure
  dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
  GetCommState(hSerial, &dcbSerialParams);
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
  SetCommState(hSerial, &dcbSerialParams);
  COMMTIMEOUTS timeouts;
  timeouts.ReadIntervalTimeout = 0xFFFFFFFF;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.ReadTotalTimeoutConstant = 50;
  timeouts.WriteTotalTimeoutMultiplier = 1;
  timeouts.WriteTotalTimeoutConstant = 1;
  SetCommTimeouts(hSerial, &timeouts);
  COM_Baudrate = baudrate;
  COM_Bytes = 0;
  #endif

  #ifdef __linux
  fd = open(port, O_RDWR | O_NOCTTY );
  if (fd <0)
    return false;
  struct termios SerialPortSettings;
  tcgetattr(fd, &SerialPortSettings);	/* Get the current attributes of the Serial port */
	/* Setting the Baud rate */
	cfsetispeed(&SerialPortSettings, B9600); /* Set Read  Speed as 9600                       */
	cfsetospeed(&SerialPortSettings, B9600); /* Set Write Speed as 9600                       */
  if (have_parity == true)
    SerialPortSettings.c_cflag |= PARENB;   /* Enables the Parity Enable bit(PARENB) */
  else
    SerialPortSettings.c_cflag &= ~PARENB;  /* Disables the Parity Enable bit(PARENB),So No Parity   */
  if (two_stopbits == true)
    SerialPortSettings.c_cflag |= CSTOPB;   /* CSTOPB = 2 Stop bits */
  else
	  SerialPortSettings.c_cflag &= ~CSTOPB;  /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
	SerialPortSettings.c_cflag &= ~CSIZE;	    /* Clears the mask for setting the data size             */
	SerialPortSettings.c_cflag |=  CS8;       /* Set the data bits = 8                                 */
  SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
  SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */
  SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
  SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode                            */
  tcsetattr(fd, TCSANOW, &SerialPortSettings);  /* Set the attributes to the termios structure*/
  tcflush(fd, TCIFLUSH);
  #endif

  return true;
}

/** \brief Write data to COM port
 *
 * \param [in] data
 * \param [in] len
 * \return 0 if everything Ok
 *
 */
int COM_Write(uint8_t *data, uint16_t len)
{
  #ifdef __MINGW32__
  DWORD dwBytesWritten = 0;
  DWORD signal;
  //OVERLAPPED ov = { 0 };
  int res;
  //ov.hEvent = CreateEvent(NULL, true, true, NULL);

  if (!WriteFile(hSerial, data, len, &dwBytesWritten, NULL))
    return -1;
  //COM_Bytes += dwBytesWritten;
//  WriteFile(hSerial, data, len, &dwBytesWritten, &ov);
//  signal = WaitForSingleObject(ov.hEvent, INFINITE);
//  if ((signal == WAIT_OBJECT_0) && (GetOverlappedResult(hSerial, &ov, &dwBytesWritten, true)))
//    res = 0;
//  else
//    res = -1;
//  CloseHandle(ov.hEvent);
//  return res;
  #endif
  #ifdef __linux
  int iOut = write(fd, data, len);
  if (iOut < 0)
    return -1;
  #endif

  return 0;
}

/** \brief Read data from COM port
 *
 * \param [out] data
 * \param [in] len
 * \return number of received bytes as int
 *
 */
int COM_Read(uint8_t *data, uint16_t len)
{
  #ifdef __MINGW32__
  //OVERLAPPED ov = { 0 };
  COMSTAT status;
  DWORD errors;
  DWORD mask, btr, temp, signal;
  DWORD dwBytesRead = 0;
//  ClearCommError(hSerial, &errors, &status);
//  if (!ReadFile(hSerial, data, len, &dwBytesRead, &ov))
//    return -1;

//  btr = 0;
//  while (btr < len)
//  {
//    SetCommMask(hSerial, EV_RXCHAR);
//    WaitCommEvent(hSerial, &mask, NULL);
//    if (mask & EV_ERR)
//      break;
//    ClearCommError(hSerial, &temp, &status);
//    btr = status.cbInQue;
//    if (btr >= len)
//    {
//      ReadFile(hSerial, data, len, &dwBytesRead, NULL);
//    }
//  }
  ReadFile(hSerial, data, len, &dwBytesRead, NULL);
  #endif
  #ifdef __linux
  int iOut = read(fd, data, len);
  if (iOut < 0)
    return -1;
  #endif

  return dwBytesRead;
}

/** \brief Calculate time for transmission with current baudrate
 *
 * \param
 * \return time in milliseconds as uint16_t
 *
 */
uint16_t COM_GetTransTime(uint16_t len)
{
  return (uint16_t)(len * 1000 * 11 / COM_Baudrate + 1);
}

void COM_WaitForTransmit(void)
{
  COMSTAT rStat;
  DWORD nErr;
  do {
    ClearCommError(hSerial, &nErr, &rStat);
  } while (rStat.cbOutQue > 0);
}

/** \brief Close current COM port
 *
 * \return
 *
 */
void COM_Close(void)
{
  printf("Closing COM port\n");
  #ifdef __MINGW32__
  CloseHandle(hSerial);
  #endif
  #ifdef __linux
  close(fd);
  #endif
}
