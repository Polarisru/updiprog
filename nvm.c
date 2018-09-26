#include <stdlib.h>
#include <string.h>
#include "app.h"
#include "devices.h"
#include "ihex.h"
#include "link.h"
#include "log.h"
#include "nvm.h"
#include "progress.h"
#include "updi.h"

static bool NVM_Progmode = false;

bool NVM_GetDeviceInfo(void)
{
  //Reads device info
  LOG_Print(LOG_LEVEL_INFO, "Reading device info");
  return true;//self.application.device_info()
}

bool NVM_EnterProgmode(void)
{
  //Enter programming mode
  LOG_Print(LOG_LEVEL_INFO, "Entering NVM programming mode");
  NVM_Progmode = APP_EnterProgmode();
  return NVM_Progmode;
}

void NVM_LeaveProgmode(void)
{
  //Leave programming mode
  LOG_Print(LOG_LEVEL_INFO, "Leaving NVM programming mode");
  APP_LeaveProgmode();
  NVM_Progmode = false;
}

void NVM_UnlockDevice(void)
{
  //Unlock and erase a device

  if (NVM_Progmode == true)
  {
    LOG_Print(LOG_LEVEL_WARNING, "Device already unlocked");
    return;
  }

  // Unlock after using the NVM key results in prog mode.
  if (APP_Unlock() == true)
    NVM_Progmode = true;
}

bool NVM_ChipErase(void)
{
  //Erase (unlocked) device

  if (NVM_Progmode == false)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Enter progmode first!");
    return false;
  }

  return APP_ChipErase();
}

bool NVM_ReadFlash(uint16_t address, uint8_t *data, uint16_t size)
{
  //Reads from flash
  uint16_t i;
  uint16_t pages;
  uint8_t page_size;
  uint8_t err_counter;

  // Must be in prog mode here
  if (NVM_Progmode == false)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Enter progmode first!");
    return false;
  }

  page_size = DEVICES_GetPageSize();

  // Find the number of pages
  pages = size / page_size;
  if (size % page_size != 0)
    pages++;

  PROGRESS_Print(0, pages, "Reading: ", '#');
  i = 0;

  err_counter = 0;
  // Read out page-wise for convenience
  while (i < pages)
  {
    LOG_Print(LOG_LEVEL_INFO, "Reading page at 0x%04X", address);
    if (APP_ReadDataWords(address, &data[i * page_size], DEVICES_GetPageSize() >> 1) == false)
    {
      // error occurred, try once more
      err_counter++;
      if (err_counter > NVM_MAX_ERRORS)
      {
        PROGRESS_Break();
        return false;
      }
      continue;
    } else
    {
      err_counter = 0;
    }
    i++;
    // show progress bar
    PROGRESS_Print(i, pages, "Reading: ", '#');
    address += page_size;
  }

  return true;
}

bool NVM_WriteFlash(uint16_t address, uint8_t *data, uint16_t size)
{
  //Writes to flash
  uint8_t page_size;
  uint16_t pages;
  uint16_t i;
  uint8_t err_counter;

  // Must be in prog mode
  if (NVM_Progmode == false)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Enter progmode first!");
    return false;
  }

  page_size = DEVICES_GetPageSize();

  // Pad to full page
  //data = self.pad_data(data, self.device.flash_pagesize)

  // Divide up into pages
  pages = size / page_size;
  if (size % page_size != 0)
    pages++;

  PROGRESS_Print(0, pages, "Writing: ", '#');
  i = 0;

  err_counter = 0;
  // Program each page
  while (i < pages)
  {
    LOG_Print(LOG_LEVEL_INFO, "Writing page at 0x%04X", address);
    if (APP_WriteNvm(address, &data[i * page_size], page_size, true) == false)
    {
      err_counter++;
      if (err_counter > NVM_MAX_ERRORS)
      {
        PROGRESS_Break();
        return false;
      }
      continue;
    } else
    {
      err_counter = 0;
    }
    i++;
    // show progress bar
    PROGRESS_Print(i, pages, "Writing: ", '#');
    address += page_size;
  }

  return true;
}

uint8_t NVM_ReadFuse(uint8_t fusenum)
{
  //Reads one fuse value
  uint16_t address;

  // Must be in prog mode
  if (NVM_Progmode == false)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Enter progmode first!");
    return false;
  }

  address = DEVICES_GetFusesAddress() + fusenum;

  return LINK_ld(address);
}

bool NVM_WriteFuse(uint8_t fusenum, uint8_t value)
{
  //Writes one fuse value
  uint16_t fuse_address;
  uint16_t address;
  uint8_t data;

  // Must be in prog mode
  if (NVM_Progmode == false)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Enter progmode first!");
    return false;
  }


  if (!APP_WaitFlashReady())
  {
    LOG_Print(LOG_LEVEL_ERROR, "Flash not ready for fuse setting");
    return false;
  }

  fuse_address = DEVICES_GetFusesAddress() + fusenum;

  address = DEVICES_GetNvmctrlAddress() + UPDI_NVMCTRL_ADDRL;
  data = (uint8_t)(fuse_address & 0xff);
  APP_WriteData(address, &data, sizeof(uint8_t));

  address = DEVICES_GetNvmctrlAddress() + UPDI_NVMCTRL_ADDRH;
  data = (uint8_t)(fuse_address >> 8);
  APP_WriteData(address, &data, sizeof(uint8_t));

  address = DEVICES_GetNvmctrlAddress() + UPDI_NVMCTRL_DATAL;
  APP_WriteData(address, &value, sizeof(uint8_t));

  address = DEVICES_GetNvmctrlAddress() + UPDI_NVMCTRL_CTRLA;
  data = UPDI_NVMCTRL_CTRLA_WRITE_FUSE;
  APP_WriteData(address, &data, sizeof(uint8_t));

  return true;
}

bool NVM_LoadIhex(char *filename, uint16_t address, uint16_t len)
{
  //Load from intel hex format
  uint8_t *fdata;
  uint8_t errCode;
  uint16_t max_addr, min_addr;
  FILE *fp;
  bool res;

  fdata = malloc(len);
  if (!fdata)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Unable to allocate %d bytes\n", (int)len);
    return false;
  }
  if ((fp = fopen(filename, "rt")) == NULL)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Unable to open file: %s", filename);
    free(fdata);
    return false;
  }
  max_addr = 0;
  min_addr = 0;
  errCode = IHEX_ReadFile(fp, fdata, len, &max_addr);
  switch (errCode)
  {
    case IHEX_ERROR_FILE:
    case IHEX_ERROR_SIZE:
    case IHEX_ERROR_FMT:
    case IHEX_ERROR_CRC:
      LOG_Print(LOG_LEVEL_ERROR, "Problem reading Hex file");
      res = false;
      break;
    case IHEX_ERROR_NONE:
      // write data buffer to flash
      res = NVM_WriteFlash(address, fdata, max_addr - min_addr);
      break;
  }
  free(fdata);
  fclose(fp);

  //self.logger.warning("Loaded {0:d} bytes from ihex starting at address 0x{1:04X}".format(len(data), start_address))

  // Size check
  //if len(data) > self.device.flash_size:
  //    raise Exception("ihex too large for flash")

  // Offset to actual flash start
  //if start_address < self.device.flash_start:
  //    self.logger.info("Adjusting flash offset to address 0x{:04X}".format(self.device.flash_start))
  //    start_address += self.device.flash_start

  return res;
}

bool NVM_SaveIhex(char *filename, uint16_t address, uint16_t len)
{
  //Save file to intel hex format
  uint8_t *fdata;
  FILE *fp;
  bool res = false;

  fdata = malloc(len);
  if (!fdata)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Unable to allocate %d bytes", (int)len);
    return false;
  }
  memset(fdata, 0xff, len);
  if ((fp = fopen(filename, "w")) == NULL)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Unable to open file: %s", filename);
  } else
  {
    if (NVM_ReadFlash(address, fdata, len) == false)
    {
      LOG_Print(LOG_LEVEL_ERROR, "Reading from device failed");
    } else
    {
      if (IHEX_WriteFile(fp, fdata, len) == IHEX_ERROR_NONE)
        res = true;
      else
        LOG_Print(LOG_LEVEL_ERROR, "Problem writing Hex file");
    }
    fclose(fp);
  }
  free(fdata);

  return res;
}
