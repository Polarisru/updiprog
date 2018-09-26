#include <string.h>
#include "devices.h"

tDevice DEVICES_List[] =
{
  {
    "tiny1617",
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
    "tiny817",
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
    "tiny417",
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
    "tiny214",
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
 * \param [in] name
 * \return
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
 * \return
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
 * \return
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
 * \return
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
 * \return
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
 * \return
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
 * \return
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
 * \return
 *
 */
uint8_t DEVICES_GetNumber(void)
{
  return (uint8_t)(sizeof(DEVICES_List) / sizeof(tDevice));
}

/** \brief Get string name of the device by ID
 *
 * \return
 *
 */
char *DEVICES_GetNameByNumber(uint8_t number)
{
  if (number >= sizeof(DEVICES_List) / sizeof(tDevice))
    number = 0;
  return DEVICES_List[number].name;
}
