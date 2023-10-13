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

#ifdef __cplusplus
extern "C"
{
#endif

/** \brief Read info about current device
 *
 * \return
 *
 */
bool NVM_GetDeviceInfo(UPDI_APP * app)
{
  LOG_Print(app->logger, LOG_LEVEL_INFO, "Reading device info");
  return true;//self.application.device_info()
}

/** \brief Enter programming mode
 *
 * \return true if succeed
 *
 */
bool NVM_EnterProgmode(UPDI_APP * app)
{
  app->NVM_Progmode = APP_EnterProgmode(app);
  return app->NVM_Progmode;
}

/** \brief Leave programming mode
 *
 * \return Nothing
 *
 */
void NVM_LeaveProgmode(UPDI_APP * app)
{
  APP_LeaveProgmode(app);
  app->NVM_Progmode = false;
}

/** \brief Unlock and erase a device
 *
 * \return Nothing
 *
 */
bool NVM_UnlockDevice(UPDI_APP * app)
{
  if (app->NVM_Progmode == true)
  {
    LOG_Print(app->logger, LOG_LEVEL_WARNING, "Device already unlocked");
  } else
  {
    // Unlock after using the NVM key results in prog mode.
    if (APP_Unlock(app) == true)
    {
      app->NVM_Progmode = true;
    } else
    {
      return false;
    }
  }
  return true;
}

/** \brief Erase chip flash memory
 *
 * \return true if succeed
 *
 */
bool NVM_ChipErase(UPDI_APP * app)
{
  if (app->NVM_Progmode == false)
  {
    LOG_Print(app->logger,LOG_LEVEL_ERROR, "Enter progmode first!");
    return false;
  }

  return APP_ChipErase(app);
}

/** \brief Read data from flash memory
 *
 * \param [in] address Starting address
 * \param [out] data Buffer to write data
 * \param [in] size Length of data to read
 * \return true if succeed
 *
 */
bool NVM_ReadFlash(UPDI_APP * app, uint16_t address, uint8_t *data, uint16_t size)
{
  uint16_t i;
  uint16_t pages;
  uint8_t page_size;
  uint8_t err_counter;

  // Must be in prog mode here
  if (app->NVM_Progmode == false)
  {
    LOG_Print(app->logger,LOG_LEVEL_ERROR, "Enter progmode first!");
    return false;
  }

  page_size = DEVICES_GetPageSize(app->DEVICE_Id);

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
    LOG_Print(app->logger,LOG_LEVEL_VERBOSE, "Reading page at 0x%04X", address);
    if (APP_ReadDataWords(app, address, &data[i * page_size], DEVICES_GetPageSize(app->DEVICE_Id) >> 1) == false)
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

/** \brief Write data buffer to flash
 *
 * \param [in] address Address to start writing
 * \param [in] data Data buffer to write
 * \param [in] size Length of data
 * \return true if succeed
 *
 */
bool NVM_WriteFlash(UPDI_APP * app, uint16_t address, uint8_t *data, uint16_t size)
{
  uint8_t page_size;
  uint16_t pages;
  uint16_t i;
  uint8_t err_counter;

  // Must be in prog mode
  if (app->NVM_Progmode == false)
  {
    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Enter progmode first!");
    return false;
  }

  page_size = DEVICES_GetPageSize(app->DEVICE_Id);

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
    LOG_Print(app->logger, LOG_LEVEL_VERBOSE, "Writing page at 0x%04X", address);
    if (APP_WriteNvm(app, address, &data[i * page_size], page_size, true) == false)
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

/** \brief Read fuse value
 *
 * \param [in] fusenum Number of the fuse
 * \return Fuse value as uint8_t
 *
 */
uint8_t NVM_ReadFuse(UPDI_APP * app, uint8_t fusenum)
{
  uint16_t address;

  // Must be in prog mode
  if (app->NVM_Progmode == false)
  {
    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Enter progmode first!");
    return false;
  }

  address = DEVICES_GetFusesAddress(app->DEVICE_Id) + fusenum;

  return LINK_ld(app, address);
}

/** \brief Write fuse value
 *
 * \param [in] fusenum Number of the fuse
 * \param [in] value Fuse value
 * \return true if succeed
 *
 */
bool NVM_WriteFuse(UPDI_APP * app, uint8_t fusenum, uint8_t value)
{
  uint16_t fuse_address;
  uint16_t address;
  uint8_t data;

  // Must be in prog mode
  if (app->NVM_Progmode == false)
  {
    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Enter progmode first!");
    return false;
  }

  if (!APP_WaitFlashReady(app))
  {
    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Flash not ready for fuse setting");
    return false;
  }

  fuse_address = DEVICES_GetFusesAddress(app->DEVICE_Id) + fusenum;

  address = DEVICES_GetNvmctrlAddress(app->DEVICE_Id) + UPDI_NVMCTRL_ADDRL;
  data = (uint8_t)(fuse_address & 0xff);
  APP_WriteData(app, address, &data, sizeof(uint8_t));

  address = DEVICES_GetNvmctrlAddress(app->DEVICE_Id) + UPDI_NVMCTRL_ADDRH;
  data = (uint8_t)(fuse_address >> 8);
  APP_WriteData(app, address, &data, sizeof(uint8_t));

  address = DEVICES_GetNvmctrlAddress(app->DEVICE_Id) + UPDI_NVMCTRL_DATAL;
  APP_WriteData(app, address, &value, sizeof(uint8_t));

  address = DEVICES_GetNvmctrlAddress(app->DEVICE_Id) + UPDI_NVMCTRL_CTRLA;
  data = UPDI_NVMCTRL_CTRLA_WRITE_FUSE;
  APP_WriteData(app, address, &data, sizeof(uint8_t));

  return true;
}

bool NVM_LoadIhexStream(UPDI_APP * app, IHEX_Stream * stream, uint16_t address, uint16_t len) {
  uint8_t *fdata;
  uint8_t errCode;
  uint16_t max_addr, min_addr;
  bool res = false;

  fdata = malloc(len);
  if (!fdata)
  {
    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Unable to allocate %d bytes", (int)len);
    return false;
  }
  max_addr = 0;
  min_addr = 0xFFFF;
  errCode = IHEX_ReadStream(stream, fdata, len, &min_addr, &max_addr);
  switch (errCode)
  {
    case IHEX_ERROR_FILE:
    case IHEX_ERROR_SIZE:
    case IHEX_ERROR_FMT:
    case IHEX_ERROR_CRC:
      LOG_Print(app->logger, LOG_LEVEL_ERROR, "Problem reading Hex file");
      res = false;
      break;
    case IHEX_ERROR_NONE:
      // write data buffer to flash
      if (min_addr < max_addr)
        res = NVM_WriteFlash(app, address + min_addr, &fdata[min_addr], max_addr - min_addr);
      break;
  }

  free(fdata);

  return res;
}

bool NVM_raw_eof(void* ud) {
  NVM_raw_data *src = (NVM_raw_data *)ud;
  return src->pos >= src->len;
}

bool NVM_raw_gets(void* ud, char * dst, int32_t cnt) {
  NVM_raw_data *src = (NVM_raw_data *)ud;
  int len = src->len - src->pos;
  cnt--;
  if (cnt <= 0) return false;

  if (cnt > len)
     cnt = len;
  memcpy(dst, &(src->data[src->pos]), cnt);
  dst[cnt] = 0;
  src->pos+=cnt;

  return true;
}

bool NVM_LoadIhexRaw(UPDI_APP * app, NVM_raw_data *src, uint16_t address, uint16_t len) {
  IHEX_Stream raw_stream;

  raw_stream.ud = src;
  raw_stream.eof = &NVM_raw_eof;
  raw_stream.gets = &NVM_raw_gets;

  bool res = NVM_LoadIhexStream(app, &raw_stream, address, len);

  return res;
}

bool NVM_file_eof(void* ud) {
    return (feof((FILE *) ud));
}

bool NVM_file_gets(void* ud, char * dst, int32_t cnt) {
    return (fgets( dst, cnt, (FILE *) ud ) != NULL);
}

/** \brief Load data from Intel HEX format
 *
 * \param [in] filename Name of the HEX file
 * \param [in] address Chip starting address
 * \param [in] len Length of the data
 * \return true if succeed
 *
 */
bool NVM_LoadIhexFile(UPDI_APP * app, char *filename, uint16_t address, uint16_t len)
{
  FILE *fp;

  if ((fp = fopen(filename, "rt")) == NULL)
  {
    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Unable to open file: %s", filename);
    return false;
  }

  IHEX_Stream file_stream;
  file_stream.ud = (void*)fp;
  file_stream.eof = &NVM_file_eof;
  file_stream.gets = &NVM_file_gets;

  bool res = NVM_LoadIhexStream(app, &file_stream, address, len);
  fclose(fp);

  // Size check: not implemented yet

  return res;
}

bool NVM_SaveIhexStream(UPDI_APP * app, IHEX_Stream * stream, uint16_t address, uint16_t len) {
  uint8_t *fdata;
  bool res = false;

  fdata = malloc(len);
  if (!fdata)
  {
    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Unable to allocate %d bytes", (int)len);
    return false;
  }
  memset(fdata, 0xff, len);

  if (NVM_ReadFlash(app, address, fdata, len) == false)
  {
    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Reading from device failed");
  } else
  {
    if (IHEX_WriteStream(stream, fdata, len) == IHEX_ERROR_NONE)
      res = true;
    else
      LOG_Print(app->logger, LOG_LEVEL_ERROR, "Problem writing Hex file");
  }

  free(fdata);

  return res;
}

bool NVM_raw_write(void* ud, const void *ptr, int32_t len) {
  NVM_raw_data *src = (NVM_raw_data *)ud;
  int cnt = src->len - src->pos;
  if (cnt <= 0) return false;

  if (len > cnt)
     len = cnt;
  memcpy(&(src->data[src->pos]), ptr, len);
  src->pos+=cnt;

  return true;
}

bool NVM_SaveIhexRaw(UPDI_APP * app, NVM_raw_data *dst, uint16_t address, uint16_t len) {
  IHEX_Stream raw_stream;

  raw_stream.ud = dst;
  raw_stream.write = &NVM_raw_write;

  bool res = NVM_LoadIhexStream(app, &raw_stream, address, len);

  return res;
}

bool NVM_file_write(void* ud, const void *ptr, int32_t len) {
  return fwrite(ptr, (size_t)len, 1, (FILE*)ud) > 0;
}

/** \brief Save file to Intel HEX format
 *
 * \param [in] filename Name of the HEX file
 * \param [in] address Chip starting address
 * \param [in] len Length of data
 * \return true if succeed
 *
 */
bool NVM_SaveIhexFile(UPDI_APP * app, char *filename, uint16_t address, uint16_t len)
{
  FILE *fp;
  bool res = false;

  if ((fp = fopen(filename, "w")) == NULL)
  {
    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Unable to open file: %s", filename);
  } else
  {
    IHEX_Stream file_stream;
    file_stream.ud = (void*)fp;
    file_stream.write = &NVM_file_write;

    res = NVM_SaveIhexStream(app, &file_stream, address, len);

    fclose(fp);
  }

  return res;
}

#ifdef __cplusplus
}
#endif
