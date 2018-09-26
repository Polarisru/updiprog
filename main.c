// TODO (AleksejKiselev#1#): Parse and write fuses
// TODO (AleksejKiselev#1#): Add baudrate settings for Linux
// TODO (AleksejKiselev#1#): Optimize source code
// TODO (AleksejKiselev#1#): Add Doxygen comments
// TODO (AleksejKiselev#1#): Read device info

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "devices.h"
#include "link.h"
#include "log.h"
#include "nvm.h"
#include "phy.h"

#define FILENAME_LEN    (64)
#define COMPORT_LEN     (32)
#define FUSES_LEN       (128)

#define SW_VER_NUMBER   "0.1"
#define SW_VER_DATE     "13.09.2018"

typedef struct
{
  bool      erase;
  bool      write;
  bool      read;
  bool      wr_fuses;
  bool      rd_fuses;
  bool      show_info;
  uint32_t  baudrate;
  int8_t    device;
  char      port[COMPORT_LEN];
  char      wr_file[FILENAME_LEN];
  char      rd_file[FILENAME_LEN];
  char      fuses[FUSES_LEN];
} tParam;

tParam parameters;

void help(void)
{
  uint8_t i;

  printf("  -b set COM baudrate (default=115200)\n");
  printf("  -d DEVICE - target device (tinyXXX)\n");
  printf("  -c COM_PORT - COM port to use(Windows: COMx | *nix: /dev/ttyX)\n");
  printf("  -e - erase device\n");
  printf("  -fw - write fuses\n");
  printf("  -fr - read all fuses\n");
  printf("  -h - show this help screen\n");
  printf("  -mX - set level of messaging (0-all/1-warnings/2-errors)\n");
  printf("  -r FILE_TO_READ.HEX - Hex file to read flash in\n");
  printf("  -p - use DTR line to power device\n");
  printf("  -w FILE_TO_WRITE.HEX - Hex file to write to flash\n");
  printf("\n");
  printf("  List of supported devices:\n    ");
  for (i = 0; i < DEVICES_GetNumber(); i++)
  {
    printf(DEVICES_GetNameByNumber(i));
    printf("  ");
  }
}

/** \brief Main application function
 *
 * \param [in] argc
 * \param [in] argv
 * \return exit code for OS
 *
 */
int main(int argc, char* argv[])
{
  uint8_t i;
  uint8_t x;
  bool error;
  uint32_t tVal;
  char *pch;
  uint16_t val;
  int ccc;

  printf("---------------------------------------------------------------\n");
  printf("|      Simple command line interface for UPDI programming     |\n");
  printf("|                   Ver. %3s (%00000010s)                     |\n", SW_VER_NUMBER, SW_VER_DATE);
  printf("---------------------------------------------------------------\n\n");

  COM_Open("/dev/ttyUSB0", 9600, false, false);
  i = 0;
  while (i < 100)
  {
    char buf[16];
    COM_Write("Test\n", 5);
    strcpy(buf, "");
    ccc = COM_Read(buf, 5);
    buf[5] = 0;
    printf("R: %s (%d)\n", buf, strlen(buf));
    if (ccc != 5)
      printf("ERROR!\n");
  }
  return 0;

  if (argc < 2)
  {
    help();
    return 0;
  }

  memset(&parameters, 0, sizeof(tParam));
  parameters.baudrate = PHY_BAUDRATE;
  parameters.device = -1;

  i = 1;
  error = false;
  while (i < argc)
  {
    if (argv[i][0] == '-')
    {
      switch (argv[i][1])
      {
        case 'b':
          /**< set communication baudrate */
          if ((i < (argc - 1)) && (argv[i + 1][0] != '-'))
          {
            if (sscanf(argv[i + 1], "%u", &tVal) == 1)
              parameters.baudrate = tVal;
          }
          break;
        case 'c':
          /**< set COM-port */
          if ((i < (argc - 1)) && (argv[i + 1][0] != '-'))
          {
            strncpy(parameters.port, argv[i + 1], COMPORT_LEN);
            parameters.port[COMPORT_LEN - 1] = 0;
          } else
          {
            printf("COM-port name is missing!");
          }
          break;
        case 'd':
          /**< set device id */
          if ((i < (argc - 1)) && (argv[i + 1][0] != '-'))
          {
            parameters.device = DEVICES_GetId(argv[i + 1]);
            if (parameters.device < 0)
            {
              printf("Wrong or unsupported device type: %s", argv[i + 1]);
              error = true;
            }
          }
          break;
        case 'e':
          /**< erase memory */
          parameters.erase = true;
          break;
        case 'f':
          /**< fuses: read or write */
          if (argv[i][2] == 'r')
          {
            parameters.rd_fuses = true;
          } else
          if (argv[i][2] == 'w')
          {
            parameters.fuses[0] = 0;
            while ((i < (argc - 1)) && (argv[i + 1][0] != '-'))
            {
              if (strlen(parameters.fuses) + strlen(argv[i + 1]) + 1 >= FUSES_LEN - 1)
              {
                printf("Fuses line is too long: %s", parameters.fuses);
                error = true;
                break;
              }
              strcat(parameters.fuses, " ");
              strcat(parameters.fuses, argv[i + 1]);
              i++;
              if ( i >= argc)
                break;
            }
            if (strlen(parameters.fuses) <= 1)
            {
              printf("Wrong or unsupported fuses parameter: -fw %s", parameters.fuses);
              error = true;
            } else
            {
              parameters.wr_fuses = true;
            }
          } else
          {
            printf("Wrong or unsupported fuses parameter: %s", argv[i]);
            error = true;
          }
          break;
        case 'h':
          /**< print help screen */
          help();
          return 0;
        case 'i':
          /**< show device info */
          parameters.show_info = true;
          break;
        case 'r':
          /**< read from flash to HEX file */
          if ((i < (argc - 1)) && (argv[i + 1][0] != '-'))
          {
            strncpy(parameters.rd_file, argv[i + 1], FILENAME_LEN);
            parameters.rd_file[FILENAME_LEN - 1] = 0;
            parameters.read = true;
            i++;
          } else
          {
            printf("Wrong file name for reading!");
            error = true;
          }
          break;
        case 'm':
          /**< level of messaging */
          if (argv[i][2] >= '0' && argv[i][2] <= '2')
            LOG_SetLevel(argv[i][2] - '0');
          break;
        case 'w':
          /**< write to flash from HEX file */
          if ((i < (argc - 1)) && (argv[i + 1][0] != '-'))
          {
            strncpy(parameters.wr_file, argv[i + 1], FILENAME_LEN);
            parameters.wr_file[FILENAME_LEN - 1] = 0;
            parameters.write = true;
            i++;
          } else
          {
            printf("Wrong file name for writing!");
            error = true;
          }
          break;
      }
    }
    if (error == true)
      return -1;
    i++;
  }

  if (parameters.device < 0)
  {
    printf("Device type (-d) is not set!");
    return -1;
  }
  if (strlen(parameters.port) == 0)
  {
    printf("COM port name is missing!");
    return -1;
  }
  if (!parameters.read && !parameters.write && !parameters.erase && !parameters.rd_fuses && !parameters.wr_fuses)
  {
    printf("Nothing to do, stopping");
    return -1;
  }

  if (LINK_Init(parameters.port, parameters.baudrate, false) == false)
  {
    printf("Can't open port: %s\nPlease check connection and try again.", parameters.port);
    return -1;
  }

  if (NVM_EnterProgmode() == false)
  {
    printf("Can't enter programming mode, exiting");
    return -1;
  }
  //print("Device is locked. Performing unlock with chip erase.")
  //nvm.unlock_device()


  printf("Working with device: %s\n", DEVICES_GetNameByNumber(parameters.device));
  /**< process input parameters */
  if (parameters.show_info == true)
  {
    //nvm.get_device_info()
  }
  if (parameters.erase == true)
  {
    printf("Erasing\n");
    NVM_ChipErase();
  }
  if (parameters.wr_fuses == true)
  {
    pch = strchr(parameters.fuses, ' ');
    while (pch != NULL)
    {
      pch++;
      if (sscanf(pch, "%hu:0x%02X", &val, &tVal) != 2)
      {
        printf("Wrong fuse settings at: _%.12s...", pch);
      } else
      {
        i = (uint8_t)val;
        x = (uint8_t)tVal;
        printf("Writing 0x%02X to fuse Nr. %d\n", x, i);
        NVM_WriteFuse(i, x);
      }
      pch = strchr(pch, ' ');
    }
  }
  if (parameters.rd_fuses == true)
  {
    printf("Reading fuses:\n");
    for (i = 0; i < DEVICES_GetFusesNumber(); i++)
    {
      x = NVM_ReadFuse(i);
      printf("  0x%02X: 0x%02X\n", i, x);
    }
  }
  if (parameters.write == true)
  {
    printf("Writing from file: %s\n", parameters.wr_file);
    NVM_LoadIhex(parameters.wr_file, DEVICES_GetFlashStart(), DEVICES_GetFlashLength());
  }
  if (parameters.read == true)
  {
    printf("Reading to file: %s\n", parameters.rd_file);
    NVM_SaveIhex(parameters.rd_file, DEVICES_GetFlashStart(), DEVICES_GetFlashLength());
  }

  NVM_LeaveProgmode();
  PHY_Close();

  return 0;
}
