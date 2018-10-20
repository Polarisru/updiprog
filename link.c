#include <string.h>
#include "log.h"
#include "phy.h"
#include "updi.h"

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
uint8_t LINK_ldcs(uint8_t address)
{
  //Load data from Control/Status space
  uint8_t response = 0;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_LDCS | (address & 0x0F)};

  LOG_Print(LOG_LEVEL_INFO, "LDCS from 0x%02X", address);
  PHY_Send(buf, sizeof(buf));
  PHY_Receive(&response, 1);
  return response;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void LINK_stcs(uint8_t address, uint8_t value)
{
  //Store a value to Control/Status space
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_STCS | (address & 0x0F), value};

  LOG_Print(LOG_LEVEL_INFO, "STCS to 0x%02X", address);
  PHY_Send(buf, sizeof(buf));
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_Check(void)
{
  //Check UPDI by loading CS STATUSA
  if (LINK_ldcs(UPDI_CS_STATUSA) != 0)
  {
    LOG_Print(LOG_LEVEL_INFO, "UPDI init OK");
    return true;
  }
  LOG_Print(LOG_LEVEL_WARNING, "UPDI not OK - reinitialisation required");
  return false;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void LINK_Start(void)
{
  //Set the inter-byte delay bit and disable collision detection
  LINK_stcs(UPDI_CS_CTRLB, 1 << UPDI_CTRLB_CCDETDIS_BIT);
  LINK_stcs(UPDI_CS_CTRLA, 1 << UPDI_CTRLA_IBDLY_BIT);
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_Init(char *port, uint32_t baudrate, bool onDTR)
{
  uint8_t err = 3;
  uint8_t byte;

  //Create a UPDI physical connection
  if (PHY_Init(port, baudrate, onDTR) == false)
    return false;
  byte = UPDI_BREAK;
  PHY_Send(&byte, sizeof(uint8_t));
  while (err-- > 0)
  {
    LINK_Start();
    //Check answer
    if (LINK_Check() == true)
      return true;
    //Send double break if all is not well, and re-check
    if (PHY_DoBreak(port) == false)
    {
      LOG_Print(LOG_LEVEL_ERROR, "UPDI initialisation failed");
      return false;
    }
    if (PHY_Init(port, baudrate, onDTR) == false)
      return false;
  }
  return false;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
uint8_t LINK_ld(uint16_t address)
{
  //Load a single byte direct from a 16-bit address
  uint8_t response;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_LDS | UPDI_ADDRESS_16 | UPDI_DATA_8, address & 0xFF, (address >> 8) & 0xFF};

  LOG_Print(LOG_LEVEL_INFO, "LD from 0x%04X", address);
  PHY_Send(buf, sizeof(buf));
  PHY_Receive(&response, 1);
  return response;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
uint16_t LINK_ld16(uint16_t address)
{
  //Load a 16-bit word directly from a 16-bit address
  uint16_t response;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_LDS | UPDI_ADDRESS_16 | UPDI_DATA_16, address & 0xFF, (address >> 8) & 0xFF};

  LOG_Print(LOG_LEVEL_INFO, "LD from 0x%04X", address);
  PHY_Send(buf, sizeof(buf));
  PHY_Receive((uint8_t*)&response, 2);
  return response;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_st(uint16_t address, uint8_t value)
{
  //Store a single byte value directly to a 16-bit address
  uint8_t response;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_STS | UPDI_ADDRESS_16 | UPDI_DATA_8, address & 0xFF, (address >> 8) & 0xFF};

  LOG_Print(LOG_LEVEL_INFO, "ST to 0x%04X", address);
  PHY_Send(buf, sizeof(buf));
  PHY_Receive(&response, 1);
  if (response != UPDI_PHY_ACK)
    return false;

  PHY_Send(&value, 1);
  PHY_Receive(&response, 1);
  if (response != UPDI_PHY_ACK)
    return false;

  return true;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_st16(uint16_t address, uint16_t value)
{
  //Store a 16-bit word value directly to a 16-bit address
  uint8_t response;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_STS | UPDI_ADDRESS_16 | UPDI_DATA_16, address & 0xFF, (address >> 8) & 0xFF};

  LOG_Print(LOG_LEVEL_INFO, "ST to 0x%04X", address);
  PHY_Send(buf, sizeof(buf));
  PHY_Receive(&response, 1);
  if (response != UPDI_PHY_ACK)
    return false;

  PHY_Send((uint8_t*)&value, sizeof(uint16_t));
  PHY_Receive(&response, 1);
  if (response != UPDI_PHY_ACK)
    return false;

  return true;
}

/** \brief
 *
 * \param [out] data Data buffer to write received data in
 * \param [in] size Size of received data
 * \return true if succeed
 *
 */
bool LINK_ld_ptr_inc(uint8_t *data, uint8_t size)
{
  //Loads a number of bytes from the pointer location with pointer post-increment
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_LD | UPDI_PTR_INC | UPDI_DATA_8};

  LOG_Print(LOG_LEVEL_INFO, "LD8 from ptr++");
  PHY_Send(buf, sizeof(buf));

  return PHY_Receive(data, size);
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_ld_ptr_inc16(uint8_t *data, uint16_t words)
{
  //Load a 16-bit word value from the pointer location with pointer post-increment
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_LD | UPDI_PTR_INC | UPDI_DATA_16};

  LOG_Print(LOG_LEVEL_INFO, "LD16 from ptr++");
  PHY_Send(buf, sizeof(buf));

  return PHY_Receive(data, words << 1);
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_st_ptr(uint16_t address)
{
  //Set the pointer location
  uint8_t response;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_ST | UPDI_PTR_ADDRESS | UPDI_DATA_16, address & 0xFF, (address >> 8) & 0xFF};

  LOG_Print(LOG_LEVEL_INFO, "ST to ptr");
  PHY_Send(buf, sizeof(buf));
  PHY_Receive(&response, 1);
  if (response != UPDI_PHY_ACK)
    return false;
  return true;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_st_ptr_inc(uint8_t *data, uint16_t len)
{
  //Store data to the pointer location with pointer post-increment
  uint8_t response;
  uint16_t n;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_ST | UPDI_PTR_INC | UPDI_DATA_8, data[0]};

  LOG_Print(LOG_LEVEL_INFO, "ST8 to *ptr++");
  PHY_Send(buf, sizeof(buf));
  PHY_Receive(&response, 1);
  if (response != UPDI_PHY_ACK)
    return false;

  n = 1;
  while (n < len)
  {
    PHY_Send(&data[n], sizeof(uint8_t));
    PHY_Receive(&response, 1);
    if (response != UPDI_PHY_ACK)
      return false;
    n++;
  }

  return true;
}

bool LINK_st_ptr_inc16(uint8_t *data, uint16_t len) // length in words or in bytes??
{
  //Store a 16-bit word value to the pointer location with pointer post-increment
  uint8_t response;
  uint16_t n;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_ST | UPDI_PTR_INC | UPDI_DATA_16, data[0], data[1]};

  LOG_Print(LOG_LEVEL_INFO, "ST16 to *ptr++");
  PHY_Send(buf, sizeof(buf));
  PHY_Receive(&response, 1);
  if (response != UPDI_PHY_ACK)
    return false;

  n = 2;
  while (n < len)
  {
    PHY_Send(&data[n], sizeof(uint16_t));
    PHY_Receive(&response, 1);
    if (response != UPDI_PHY_ACK)
      return false;
    n += 2;
  }
  return true;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void LINK_Repeat(uint16_t repeats)
{
  //Store a value to the repeat counter
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_REPEAT | UPDI_REPEAT_WORD, (repeats - 1) & 0xFF, ((repeats - 1) >> 8) & 0xFF};

  LOG_Print(LOG_LEVEL_INFO, "Repeat %d", repeats);
  PHY_Send(buf, sizeof(buf));
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void LINK_Read_SIB(uint8_t *data)
{
  //Read the SIB
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_KEY | UPDI_KEY_SIB | UPDI_SIB_16BYTES};

  PHY_Send(buf, sizeof(buf));
  PHY_Receive(data, UPDI_SIB_LENGTH);
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_SendKey(char *key, uint8_t size)
{
  //Write a key
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_KEY | UPDI_KEY_KEY | size};
  uint8_t data[8];
  uint8_t n, i;

  LOG_Print(LOG_LEVEL_INFO, "Writing key");
  if (strlen(key) != (8 << size))
  {
    LOG_Print(LOG_LEVEL_ERROR, "Invalid KEY length!");
    return false;
  }
  PHY_Send(buf, sizeof(buf));
  n = strlen(key);
  i = 0;
  while (n > 0)
  {
    data[i++] = key[n - 1];
    n--;
  }
  PHY_Send(data, sizeof(data));

  return true;
}
