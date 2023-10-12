#include <string.h>
#include "app.h"
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
uint8_t LINK_ldcs(UPDI_APP* app,uint8_t address)
{
  //Load data from Control/Status space
  uint8_t response = 0;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_LDCS | (address & 0x0F)};

  LOG_Print(app->logger, LOG_LEVEL_INFO, "LDCS from 0x%02X", address);
  PHY_Send(app->port, buf, sizeof(buf));
  PHY_Receive(app->port, &response, 1);
  return response;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void LINK_stcs(UPDI_APP* app,uint8_t address, uint8_t value)
{
  //Store a value to Control/Status space
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_STCS | (address & 0x0F), value};

  LOG_Print(app->logger, LOG_LEVEL_INFO, "STCS to 0x%02X", address);
  PHY_Send(app->port, buf, sizeof(buf));
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_Check(UPDI_APP* app)
{
  //Check UPDI by loading CS STATUSA
  if (LINK_ldcs(app, UPDI_CS_STATUSA) != 0)
  {
    LOG_Print(app->logger, LOG_LEVEL_INFO, "UPDI init OK");
    return true;
  }
  LOG_Print(app->logger, LOG_LEVEL_WARNING, "UPDI not OK - reinitialisation required");
  return false;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void LINK_Start(UPDI_APP* app)
{
  //Set the inter-byte delay bit and disable collision detection
  LINK_stcs(app, UPDI_CS_CTRLB, 1 << UPDI_CTRLB_CCDETDIS_BIT);
  LINK_stcs(app, UPDI_CS_CTRLA, 1 << UPDI_CTRLA_IBDLY_BIT);
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_Init(UPDI_APP* app, char *port, uint32_t baudrate, bool onDTR)
{
  uint8_t err = 3;
  uint8_t byte;

  //Create a UPDI physical connection
  if ((app->port = PHY_Init(port, baudrate, onDTR)) == NULL)
    return false;
  byte = UPDI_BREAK;
  PHY_Send(app->port, &byte, sizeof(uint8_t));
  while (err-- > 0)
  {
    LINK_Start(app);
    //Check answer
    if (LINK_Check(app) == true)
      return true;
    //Send double break if all is not well, and re-check
    if (PHY_DoBreak(&(app->port)) == false)
    {
      LOG_Print(app->logger, LOG_LEVEL_ERROR, "UPDI initialization failed");
      return false;
    }
    if ((app->port = PHY_Init(port, baudrate, onDTR)) == NULL)
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
uint8_t LINK_ld(UPDI_APP* app, uint16_t address)
{
  //Load a single byte direct from a 16-bit address
  uint8_t response;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_LDS | UPDI_ADDRESS_16 | UPDI_DATA_8, address & 0xFF, (address >> 8) & 0xFF};

  LOG_Print(app->logger, LOG_LEVEL_INFO, "LD from 0x%04X", address);
  PHY_Send(app->port, buf, sizeof(buf));
  PHY_Receive(app->port, &response, 1);
  return response;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
uint16_t LINK_ld16(UPDI_APP* app, uint16_t address)
{
  //Load a 16-bit word directly from a 16-bit address
  uint16_t response;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_LDS | UPDI_ADDRESS_16 | UPDI_DATA_16, address & 0xFF, (address >> 8) & 0xFF};

  LOG_Print(app->logger, LOG_LEVEL_INFO, "LD from 0x%04X", address);
  PHY_Send(app->port, buf, sizeof(buf));
  PHY_Receive(app->port, (uint8_t*)&response, 2);
  return response;
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_st(UPDI_APP* app, uint16_t address, uint8_t value)
{
  //Store a single byte value directly to a 16-bit address
  uint8_t response;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_STS | UPDI_ADDRESS_16 | UPDI_DATA_8, address & 0xFF, (address >> 8) & 0xFF};

  LOG_Print(app->logger, LOG_LEVEL_INFO, "ST to 0x%04X", address);
  PHY_Send(app->port, buf, sizeof(buf));
  PHY_Receive(app->port, &response, 1);
  if (response != UPDI_PHY_ACK)
    return false;

  PHY_Send(app->port, &value, 1);
  PHY_Receive(app->port, &response, 1);
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
bool LINK_st16(UPDI_APP* app, uint16_t address, uint16_t value)
{
  //Store a 16-bit word value directly to a 16-bit address
  uint8_t response;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_STS | UPDI_ADDRESS_16 | UPDI_DATA_16, address & 0xFF, (address >> 8) & 0xFF};

  LOG_Print(app->logger, LOG_LEVEL_INFO, "ST to 0x%04X", address);
  PHY_Send(app->port, buf, sizeof(buf));
  PHY_Receive(app->port, &response, 1);
  if (response != UPDI_PHY_ACK)
    return false;

  PHY_Send(app->port, (uint8_t*)&value, sizeof(uint16_t));
  PHY_Receive(app->port, &response, 1);
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
bool LINK_ld_ptr_inc(UPDI_APP* app, uint8_t *data, uint8_t size)
{
  //Loads a number of bytes from the pointer location with pointer post-increment
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_LD | UPDI_PTR_INC | UPDI_DATA_8};

  LOG_Print(app->logger, LOG_LEVEL_INFO, "LD8 from ptr++");
  PHY_Send(app->port, buf, sizeof(buf));

  return PHY_Receive(app->port, data, size);
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_ld_ptr_inc16(UPDI_APP* app, uint8_t *data, uint16_t words)
{
  //Load a 16-bit word value from the pointer location with pointer post-increment
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_LD | UPDI_PTR_INC | UPDI_DATA_16};

  LOG_Print(app->logger, LOG_LEVEL_INFO, "LD16 from ptr++");
  PHY_Send(app->port, buf, sizeof(buf));

  return PHY_Receive(app->port, data, words << 1);
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_st_ptr(UPDI_APP* app, uint16_t address)
{
  //Set the pointer location
  uint8_t response;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_ST | UPDI_PTR_ADDRESS | UPDI_DATA_16, address & 0xFF, (address >> 8) & 0xFF};

  LOG_Print(app->logger, LOG_LEVEL_INFO, "ST to ptr");
  PHY_Send(app->port, buf, sizeof(buf));
  PHY_Receive(app->port, &response, 1);
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
bool LINK_st_ptr_inc(UPDI_APP* app, uint8_t *data, uint16_t len)
{
  //Store data to the pointer location with pointer post-increment
  uint8_t response;
  uint16_t n;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_ST | UPDI_PTR_INC | UPDI_DATA_8, data[0]};

  LOG_Print(app->logger, LOG_LEVEL_INFO, "ST8 to *ptr++");
  PHY_Send(app->port, buf, sizeof(buf));
  PHY_Receive(app->port, &response, 1);
  if (response != UPDI_PHY_ACK)
    return false;

  n = 1;
  while (n < len)
  {
    PHY_Send(app->port, &data[n], sizeof(uint8_t));
    PHY_Receive(app->port, &response, 1);
    if (response != UPDI_PHY_ACK)
      return false;
    n++;
  }

  return true;
}

bool LINK_st_ptr_inc16(UPDI_APP* app, uint8_t *data, uint16_t len) // length in words or in bytes??
{
  //Store a 16-bit word value to the pointer location with pointer post-increment
  uint8_t response;
  uint16_t n;
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_ST | UPDI_PTR_INC | UPDI_DATA_16, data[0], data[1]};

  LOG_Print(app->logger, LOG_LEVEL_INFO, "ST16 to *ptr++");
  PHY_Send(app->port, buf, sizeof(buf));
  PHY_Receive(app->port, &response, 1);
  if (response != UPDI_PHY_ACK)
    return false;

  n = 2;
  while (n < len)
  {
    PHY_Send(app->port, &data[n], sizeof(uint16_t));
    PHY_Receive(app->port, &response, 1);
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
void LINK_Repeat(UPDI_APP* app, uint16_t repeats)
{
  //Store a value to the repeat counter
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_REPEAT | UPDI_REPEAT_WORD, (repeats - 1) & 0xFF, ((repeats - 1) >> 8) & 0xFF};

  LOG_Print(app->logger, LOG_LEVEL_INFO, "Repeat %d", repeats);
  PHY_Send(app->port, buf, sizeof(buf));
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
void LINK_Read_SIB(UPDI_APP* app, uint8_t *data)
{
  //Read the SIB
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_KEY | UPDI_KEY_SIB | UPDI_SIB_16BYTES};

  PHY_Send(app->port, buf, sizeof(buf));
  PHY_Receive(app->port, data, UPDI_SIB_LENGTH);
}

/** \brief
 *
 * \param
 * \param
 * \return
 *
 */
bool LINK_SendKey(UPDI_APP* app, char *key, uint8_t size)
{
  //Write a key
  uint8_t buf[] = {UPDI_PHY_SYNC, UPDI_KEY | UPDI_KEY_KEY | size};
  uint8_t data[8];
  uint8_t n, i;

  LOG_Print(app->logger, LOG_LEVEL_INFO, "Writing key");
  if (strlen(key) != (8 << size))
  {
    LOG_Print(app->logger, LOG_LEVEL_ERROR, "Invalid KEY length!");
    return false;
  }
  PHY_Send(app->port, buf, sizeof(buf));
  n = strlen(key);
  i = 0;
  while (n > 0)
  {
    data[i++] = key[n - 1];
    n--;
  }
  PHY_Send(app->port, data, sizeof(data));

  return true;
}
