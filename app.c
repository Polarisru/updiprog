#include <stdio.h>
#include <unistd.h>
#include "devices.h"
#include "link.h"
#include "log.h"
#include "sleep.h"
#include "updi.h"

void APP_Reset(bool apply_reset)
{
  //Applies or releases an UPDI reset condition
  if (apply_reset == true)
  {
    LOG_Print(LOG_LEVEL_INFO, "Apply reset");
    LINK_stcs(UPDI_ASI_RESET_REQ, UPDI_RESET_REQ_VALUE);
  } else
  {
    LOG_Print(LOG_LEVEL_INFO, "Release reset");
    LINK_stcs(UPDI_ASI_RESET_REQ, 0x00);
  }
}

bool APP_InProgMode(void)
{
  //Checks whether the NVM PROG flag is up
  if (LINK_ldcs(UPDI_ASI_SYS_STATUS) & (1 << UPDI_ASI_SYS_STATUS_NVMPROG))
    return true;
  return false;
}

bool APP_WaitUnlocked(uint16_t timeout_ms)
{

  //Waits for the device to be unlocked.
  //All devices boot up as locked until proven otherwise

  while (timeout_ms-- > 0)
  {
    msleep(1);
    if (!(LINK_ldcs(UPDI_ASI_SYS_STATUS) & (1 << UPDI_ASI_SYS_STATUS_LOCKSTATUS)))
      return true;
  }

  LOG_Print(LOG_LEVEL_WARNING, "Timeout by waiting for device to unlock");
  return false;
}

bool APP_EnterProgmode(void)
{
  //Enters into NVM programming mode
  uint8_t key_status;

  //First check if NVM is already enabled
  if (APP_InProgMode() == true)
  {
    LOG_Print(LOG_LEVEL_WARNING, "Already in NVM programming mode");
    return true;
  }

  LOG_Print(LOG_LEVEL_WARNING, "Entering NVM programming mode");

  // Put in the key
  LINK_SendKey(UPDI_KEY_NVM, UPDI_KEY_64);

  // Check key status
  key_status = LINK_ldcs(UPDI_ASI_KEY_STATUS);
  LOG_Print(LOG_LEVEL_INFO, "Key status = 0x%02X", key_status);

  if (!(key_status & (1 << UPDI_ASI_KEY_STATUS_NVMPROG)))
  {
    LOG_Print(LOG_LEVEL_WARNING, "Key not accepted");
    return false;
  }

  // Toggle reset
  APP_Reset(true);
  APP_Reset(false);

  // And wait for unlock
  if (!APP_WaitUnlocked(100))
  {
    LOG_Print(LOG_LEVEL_ERROR, "Failed to enter NVM programming mode: device is locked");
    return false;
  }

  // Check for NVMPROG flag
  if (APP_InProgMode() == false)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Failed to enter NVM programming mode");
    return false;
  }

  LOG_Print(LOG_LEVEL_INFO, "Now in NVM programming mode");
  return true;
}

void APP_LeaveProgmode(void)
{
  //Disables UPDI which releases any keys enabled
  LOG_Print(LOG_LEVEL_WARNING, "Leaving NVM programming mode");
  APP_Reset(true);
  APP_Reset(false);
  LINK_stcs(UPDI_CS_CTRLB, (1 << UPDI_CTRLB_UPDIDIS_BIT) | (1 << UPDI_CTRLB_CCDETDIS_BIT));
}

bool APP_Unlock(void)
{
  //Unlock and erase
  uint8_t key_status;

  // Put in the key
  LINK_SendKey(UPDI_KEY_CHIPERASE, UPDI_KEY_64);

  // Check key status
  key_status = LINK_ldcs(UPDI_ASI_KEY_STATUS);
  LOG_Print(LOG_LEVEL_INFO, "Key status = 0x%02X", key_status);

  if (!(key_status & (1 << UPDI_ASI_KEY_STATUS_CHIPERASE)))
  {
    LOG_Print(LOG_LEVEL_WARNING, "Key not accepted");
    return false;
  }

  // Toggle reset
  APP_Reset(true);
  APP_Reset(false);

  // And wait for unlock
  if (!APP_WaitUnlocked(100))
  {
    LOG_Print(LOG_LEVEL_ERROR, "Failed to chip erase using key!");
    return false;
  }

  return true;
}

bool APP_WaitFlashReady(void)
{
  //Waits for the NVM controller to be ready
  uint8_t status;
  uint16_t timeout = 10000; // 10 sec timeout, just to be sure

  LOG_Print(LOG_LEVEL_INFO, "Wait flash ready");
  while (timeout-- > 0)
  {
    msleep(1);
    status = LINK_ld(DEVICES_GetNvmctrlAddress() + UPDI_NVMCTRL_STATUS);
    if (status & (1 << UPDI_NVM_STATUS_WRITE_ERROR))
    {
      LOG_Print(LOG_LEVEL_ERROR, "NVM error");
      return false;
    }

    if (!(status & ((1 << UPDI_NVM_STATUS_EEPROM_BUSY) | (1 << UPDI_NVM_STATUS_FLASH_BUSY))))
      return true;
  }

  LOG_Print(LOG_LEVEL_WARNING, "Waiting for flash ready timed out");

  return false;
}

bool APP_ExecuteNvmCommand(uint8_t command)
{
  //Executes an NVM COMMAND on the NVM CTRL
  //self.logger.info("NVMCMD {:d} executing".format(command))
  LOG_Print(LOG_LEVEL_INFO, "NVMCMD %d executing", command);
  return LINK_st(DEVICES_GetNvmctrlAddress() + UPDI_NVMCTRL_CTRLA, command);
}

bool APP_ChipErase(void)
{
  // Does a chip erase using the NVM controller
  // Note that on locked devices this it not possible
  // and the ERASE KEY has to be used instead
  LOG_Print(LOG_LEVEL_INFO, "Chip erase using NVM CTRL");

  // Wait until NVM CTRL is ready to erase
  if (!APP_WaitFlashReady())
  {
    LOG_Print(LOG_LEVEL_WARNING, "Timeout waiting for flash ready before erase ");
    return false;
  }

  // Erase
  APP_ExecuteNvmCommand(UPDI_NVMCTRL_CTRLA_CHIP_ERASE);

  // And wait for it
  if (!APP_WaitFlashReady())
  {
    LOG_Print(LOG_LEVEL_WARNING, "Timeout by waiting for flash ready after erase");
    return false;
  }

  return true;
}

bool APP_WriteDataWords(uint16_t address, uint8_t *data, uint16_t len)
{
  //Writes a number of words to memory
  uint16_t value;

  // Special-case of 1 word
  if (len == 2)
  {
    value = (uint16_t)data[0] + (data[1] << 8);
    return LINK_st16(address, value);
  }

  // Range check
  if (len > (UPDI_MAX_REPEAT_SIZE + 1) << 1)
  {
    LOG_Print(LOG_LEVEL_WARNING, "Invalid length");
    return false;
  }

  // Store the address
  LINK_st_ptr(address);

  // Fire up the repeat
  LINK_Repeat(len >> 1);
  return LINK_st_ptr_inc16(data, len);
}

bool APP_WriteData(uint16_t address, uint8_t *data, uint16_t len)
{
  //Writes a number of bytes to memory

  // Special case of 1 byte
  if (len == 1)
    return LINK_st(address, data[0]);
  // Special case of 2 byte
  if (len == 2)
  {
    LINK_st(address, data[0]);
    return LINK_st(address + 1, data[1]);
  }

  // Range check
  if (len > (UPDI_MAX_REPEAT_SIZE + 1))
  {
    LOG_Print(LOG_LEVEL_WARNING, "Invalid length");
    return false;
  }

  // Store the address
  LINK_st_ptr(address);

  // Fire up the repeat
  LINK_Repeat(len);
  return LINK_st_ptr_inc(data, len);
}

bool APP_WriteNvm(uint16_t address, uint8_t *data, uint16_t len, bool use_word_access)
{
  //Writes a page of data to NVM.APP_ExecuteNvmCommand
  //By default the PAGE_WRITE command is used, which
  //requires that the page is already erased.
  //By default word access is used (flash)

  // Check that NVM controller is ready
  if (!APP_WaitFlashReady())
  {
    LOG_Print(LOG_LEVEL_WARNING, "Timeout by waiting for flash ready before page buffer clear ");
    return false;
  }

  // Clear the page buffer
  LOG_Print(LOG_LEVEL_INFO, "Clear page buffer");
  APP_ExecuteNvmCommand(UPDI_NVMCTRL_CTRLA_PAGE_BUFFER_CLR);

  // Waif for NVM controller to be ready
  if (!APP_WaitFlashReady())
  {
    LOG_Print(LOG_LEVEL_WARNING, "Timeout by waiting for flash ready after page buffer clear");
    return false;
  }

  // Load the page buffer by writing directly to location
  if (use_word_access == true)
    APP_WriteDataWords(address, data, len);
  else
    APP_WriteData(address, data, len);

  // Write the page to NVM, maybe erase first
  LOG_Print(LOG_LEVEL_INFO, "Committing page");
  APP_ExecuteNvmCommand(UPDI_NVMCTRL_CTRLA_WRITE_PAGE);

  // Wait for NVM controller to be ready again
  if (!APP_WaitFlashReady())
  {
    LOG_Print(LOG_LEVEL_WARNING, "Timeout by waiting for flash ready after page write");
    return false;
  }

  return true;
}

bool APP_ReadData(uint16_t address, uint8_t *data, uint16_t size)
{
  //Reads a number of bytes of data from UPDI
  LOG_Print(LOG_LEVEL_INFO, "Reading %d bytes from 0x%04X", size, address);

  // Range check
  if (size > UPDI_MAX_REPEAT_SIZE + 1)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Cant read that many bytes in one go");
    return false;
  }

  // Store the address
  LINK_st_ptr(address);

  // Fire up the repeat
  if (size > 1)
      LINK_Repeat(size);

  // Do the read(s)
  return LINK_ld_ptr_inc(data, size);
}

bool APP_ReadDataWords(uint16_t address, uint8_t *data, uint16_t words)
{
  //Reads a number of words of data from UPDI
  LOG_Print(LOG_LEVEL_INFO, "Reading %d words from 0x%04X", words, address);

  // Range check
  if (words > (UPDI_MAX_REPEAT_SIZE >> 1) + 1)
  {
    LOG_Print(LOG_LEVEL_ERROR, "Cant read that many words in one go");
    return false;
  }

  // Store the address
  LINK_st_ptr(address);

  // Fire up the repeat
  if (words > 1)
  {
    LINK_Repeat(words);
  }

  // Do the read
  return LINK_ld_ptr_inc16(data, words);
}
