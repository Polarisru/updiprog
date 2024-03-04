#include <string.h>
#include "devices.h"

tDevice DEVICES_List[] =
{
  {
    "mega480x",
    0x4000,
    48 * 1024,
    128,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "mega320x",
    0x4000,
    32 * 1024,
    128,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "mega160x",
    0x4000,
    16 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "mega80x",
    0x4000,
    8 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "AVR32DAxx",
    0x8000,
    32 * 1024,
    512,
    0x0F00,
    0x1000,
    0x1100,
    0x1050,
    0x1080,
    9
  },
  {
    "AVR32DBxx",
    0x8000,
    32 * 1024,
    512,
    0x0F00,
    0x1000,
    0x1100,
    0x1050,
    0x1080,
    9
  },
  {
    "AVR16DDxx",
    0x8000,
    16 * 1024,
    512,
    0x0F00,
    0x1000,
    0x1100,
    0x1050,
    0x1080,
    9
  },
  {
    "AVR32DDxx",
    0x8000,
    32 * 1024,
    512,
    0x0F00,
    0x1000,
    0x1100,
    0x1050,
    0x1080,
    9
  },
  {
    "tiny321x",
    0x8000,
    32 * 1024,
    128,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "tiny160x",
    0x8000,
    16 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "tiny161x",
    0x8000,
    16 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "tiny162x",
    0x8000,
    16 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "tiny80x",
    0x8000,
    8 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "tiny81x",
    0x8000,
    8 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "tiny82x",
    0x8000,
    8 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "tiny40x",
    0x8000,
    4 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "tiny41x",
    0x8000,
    4 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "tiny42x",
    0x8000,
    4 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "tiny20x",
    0x8000,
    2 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "tiny21x",
    0x8000,
    2 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  },
  {
    "tiny22x",
    0x8000,
    2 * 1024,
    64,
    0x0F00,
    0x1000,
    0x1100,
    0x1280,
    0x1300,
    9
  }
};

static int8_t DEVICE_Id = DEVICE_UNKNOWN_ID;

/** \brief Get device ID from name string
 *
 * \param [in] name Name to find as string
 * \return index of the found device or -1 as error
 *
 */
int8_t DEVICES_GetId(char *name)
{
  uint8_t i;

  for (i = 0; i < sizeof(DEVICES_List) / sizeof(tDevice); i++)
  {
    if (strcmp(name, DEVICES_List[i].name) == 0)
    {
      DEVICE_Id = i;
      return i;
    }
  }

  DEVICE_Id = DEVICE_UNKNOWN_ID;
  return -1;
}

/** \brief Get flash memory length for selected device
 *
 * \return Size of the flash memory as uint16_t
 *
 */
uint16_t DEVICES_GetFlashLength(void)
{
  if (DEVICE_Id == DEVICE_UNKNOWN_ID)
    return 0;
  else
    return DEVICES_List[DEVICE_Id].flash_size;
}

/** \brief Get flash start address for selected device
 *
 * \return Address of the flash area as uint16_t
 *
 */
uint16_t DEVICES_GetFlashStart(void)
{
  if (DEVICE_Id == DEVICE_UNKNOWN_ID)
    return 0;
  else
    return DEVICES_List[DEVICE_Id].flash_start;
}

/** \brief Get flash page size for selected device
 *
 * \return Flash page size as uint16_t
 *
 */
uint16_t DEVICES_GetPageSize(void)
{
  if (DEVICE_Id == DEVICE_UNKNOWN_ID)
    return 0;
  else
    return DEVICES_List[DEVICE_Id].flash_pagesize;
}

/** \brief Get NVM control registers address for selected device
 *
 * \return NVM registers adress as uint16_t
 *
 */
uint16_t DEVICES_GetNvmctrlAddress(void)
{
  if (DEVICE_Id == DEVICE_UNKNOWN_ID)
    return 0;
  else
    return DEVICES_List[DEVICE_Id].nvmctrl_address;
}

/** \brief Get fuses address for selected device
 *
 * \return Fuses address as uint16_t
 *
 */
uint16_t DEVICES_GetFusesAddress(void)
{
  if (DEVICE_Id == DEVICE_UNKNOWN_ID)
    return 0;
  else
    return DEVICES_List[DEVICE_Id].fuses_address;
}

/** \brief Get number of the fuses for selected device
 *
 * \return Number of the fuses as uint8_t
 *
 */
uint8_t DEVICES_GetFusesNumber(void)
{
  if (DEVICE_Id == DEVICE_UNKNOWN_ID)
    return 0;
  else
    return DEVICES_List[DEVICE_Id].number_of_fuses;
}

/** \brief Get number of devices in the list
 *
 * \return Number of the devices as uint8_t
 *
 */
uint8_t DEVICES_GetNumber(void)
{
  return (uint8_t)(sizeof(DEVICES_List) / sizeof(tDevice));
}

/** \brief Get string name of the device by ID
 *
 * \return Device name as string
 *
 */
char *DEVICES_GetNameByNumber(uint8_t number)
{
  if (number >= sizeof(DEVICES_List) / sizeof(tDevice))
    number = 0;
  return DEVICES_List[number].name;
}
