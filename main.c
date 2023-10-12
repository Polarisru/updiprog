// TODO (A.K.#1#): Get Linux version working
// TODO (A.K.#1#): Optimize source code
// TODO (A.K.#1#): Add Doxygen comments
// TODO (A.K.#1#): Read device info

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include "devices.h"
#include "link.h"
#include "log.h"
#include "nvm.h"
#include "phy.h"

#define FILENAME_LEN    (64)
#define COMPORT_LEN     (32)
#define FUSES_LEN       (128)

#define SW_VER_NUMBER   "0.7"
#define SW_VER_DATE     "09.03.2022"

typedef struct
{
  bool      erase;
  bool      write;
  bool      read;
  bool      wr_fuses;
  bool      rd_fuses;
  bool      lock;
  bool      unlock;
  bool      show_info;
  uint32_t  baudrate;
  int8_t    device;
  char      port[COMPORT_LEN];
  char      wr_file[FILENAME_LEN];
  char      rd_file[FILENAME_LEN];
  char      fuses[FUSES_LEN];
} tParam;

tParam parameters;

/** \brief Print help screen with list of commands
 *
 * \return Nothing
 *
 */
void help(void)
{
  uint8_t i;

  printf("  -b BAUDRATE - set COM baudrate (default=115200)\n");
  printf("  -d DEVICE   - target device (tinyXXX)\n");
  printf("  -c COM_PORT - COM port to use (Win: COMx | *nix: /dev/ttyX)\n");
  printf("  -e          - erase device\n");
  printf("  -fw X:0xYY  - write fuses (X - fuse number, 0xYY - hex value)\n");
  printf("  -fr         - read all fuses\n");
  printf("  -ls         - lock device\n");
  printf("  -lr         - unlock device\n");
  printf("  -h          - show this help screen\n");
  printf("  -mX         - set logging level (0-all/1-warnings/2-errors)\n");
  printf("  -r FILE.HEX - Hex file to read MCU flash into\n");
  //printf("  -p          - use DTR line to power device\n");
  printf("  -w FILE.HEX - Hex file to write to MCU flash\n");
  printf("\n");
  printf("  List of supported devices:\n    ");
  for (i = 1; i < DEVICES_GetNumber()+1; i++)
  {
    printf("%-14s", DEVICES_GetNameByNumber(i-1));
    if (i % 4 == 0)
    {
      printf("\n    ");
    }
  }
  printf("\n");
}

/** \brief Main application function
 *
 * \param [in] argc Number of command line arguments
 * \param [in] argv Command line arguments
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
  //int ccc;

  printf("################################################################\n");
  printf("#      Simple command line interface for UPDI programming      #\n");
  printf("#                Ver. %3s (%00000010s) by A.K.                 #\n", SW_VER_NUMBER, SW_VER_DATE);
  printf("################################################################\n\n");

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
            else
              printf("Baudrate parameter is wrong!\n");
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
            printf("COM-port name is missing!\n");
          }
          break;
        case 'd':
          /**< set device id */
          if ((i < (argc - 1)) && (argv[i + 1][0] != '-'))
          {
            parameters.device = DEVICES_GetId(argv[i + 1]);
            if (parameters.device < 0)
            {
              printf("Wrong or unsupported device type: %s\n", argv[i + 1]);
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
                printf("Fuses line is too long: %s\n", parameters.fuses);
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
              printf("%s: wrong or unsupported fuses parameter: %s\n", "-fw", parameters.fuses);
              error = true;
            } else
            {
              parameters.wr_fuses = true;
            }
          } else
          {
            printf("%s: wrong or unsupported fuses parameter!\n", argv[i]);
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
            printf("%s: wrong file name for reading!", argv[i]);
            error = true;
          }
          break;
        case 'l':
          /**< lock/unlock device */
          if (argv[i][2] == 's')
          {
            parameters.lock = true;
          } else
          if (argv[i][2] == 'r')
          {
            parameters.unlock = true;
          } else
          {
            printf("%s: wrong or unsupported fuses parameter!\n", argv[i]);
            error = true;
          }
          break;
        case 'm':
          /**< level of messaging */
          if (argv[i][2] >= '0' && argv[i][2] <= '2')
            LOG_SetLevel(global_LOG(), argv[i][2] - '0');
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
            printf("%s: wrong file name for writing!\n", argv[i]);
            error = true;
          }
          break;
        default:
          printf("Unknown parameter: %s\n", argv[i]);
          break;
      }
    }
    if (error == true)
      return -1;
    i++;
  }

  if (parameters.device < 0)
  {
    printf("Device type (-d) is not set!\n");
    return -1;
  }
  if (strlen(parameters.port) == 0)
  {
    printf("COM port name is missing!\n");
    return -1;
  }
  if (!parameters.read && !parameters.write && !parameters.erase && !parameters.rd_fuses &&
      !parameters.wr_fuses && !parameters.unlock)
  {
    printf("Nothing to do, stopping\n");
    return -1;
  }

  UPDI_APP * app = APP_Init();

  if (!app) return -2;

  if (LINK_Init(app, parameters.port, parameters.baudrate, false) == false)
  {
    printf("Can't open port: %s\nPlease check connection and try again.\n", parameters.port);
    APP_Done(app);
    return -1;
  }

  printf("Working with device: %s\n", DEVICES_GetNameByNumber(parameters.device));

  if (parameters.unlock == true)
  {
    printf("Unlocking...   ");
    if (NVM_UnlockDevice(app) == true)
    {
      printf("OK\n");
    }
  }

  if (NVM_EnterProgmode(app) == false)
  {
    printf("Can't enter programming mode, exiting\n");
    return -1;
  }

  /**< process input parameters */
  if (parameters.erase == true)
  {
    printf("Erasing\n");
    NVM_ChipErase(app);
  }
  if (parameters.wr_fuses == true)
  {
    pch = strchr(parameters.fuses, ' ');
    while (pch != NULL)
    {
      pch++;
      if (sscanf(pch, "%hu:0x%02X", &val, &tVal) != 2)
      {
        printf("Wrong fuse settings at: _%.12s...\n", pch);
      } else
      {
        i = (uint8_t)val;
        x = (uint8_t)tVal;
        printf("Writing 0x%02X to fuse Nr. %d\n", x, i);
        NVM_WriteFuse(app, i, x);
      }
      pch = strchr(pch, ' ');
    }
  }
  if (parameters.rd_fuses == true)
  {
    printf("Reading fuses:\n");
    for (i = 0; i < DEVICES_GetFusesNumber(app->DEVICE_Id); i++)
    {
      x = NVM_ReadFuse(app, i);
      printf("  0x%02X: 0x%02X\n", i, x);
    }
  }
  if (parameters.write == true)
  {
    printf("Writing from file: %s\n", parameters.wr_file);
    NVM_LoadIhex(app, parameters.wr_file, DEVICES_GetFlashStart(app->DEVICE_Id), DEVICES_GetFlashLength(app->DEVICE_Id));
  }
  if (parameters.read == true)
  {
    char cwd[PATH_MAX];
    char ch;

    if (getcwd(cwd, sizeof(cwd)) == NULL)
      cwd[0] = 0;
    #ifdef __MINGW32__
    ch = '\\';
    #endif // __MINGW32__
    #if defined(__APPLE__) || defined(__linux)
    ch = '/';
    #endif // __linux
    if (strchr(parameters.rd_file, ch) != NULL)
    {
      cwd[0] = 0;
      ch = 0;
    }
    printf("Reading to file: %s%c%s\n", cwd, ch, parameters.rd_file);
    NVM_SaveIhex(app, parameters.rd_file, DEVICES_GetFlashStart(app->DEVICE_Id), DEVICES_GetFlashLength(app->DEVICE_Id));
  }
  if (parameters.lock == true)
  {
    printf("Locking MCU...   ");
    if (NVM_WriteFuse(app, DEVICE_LOCKBIT_ADDR, 0x00) == true)
    {
      printf("OK\n");
    }
  }

  NVM_LeaveProgmode(app);
  APP_Done(app);

  return 0;
}
