#include <unistd.h>
#include "com.h"
#include "log.h"
#include "phy.h"
#include "updi.h"
#include "sleep.h"

/** \brief Initialize physical interface
 *
 * \param [in] port Port name as string
 * \param [in] baudrate Transmission baudrate
 * \param [in] onDTR True if using DTR for power
 * \return true if success
 *
 */
bool PHY_Init(char *port, uint32_t baudrate, bool onDTR)
{
  return COM_Open(port, baudrate, true, true);
}

/** \brief Sends a double break to reset the UPDI port
 *         BREAK is actually just a slower zero frame
 *         A double break is guaranteed to push the UPDI state
 *         machine into a known state, albeit rather brutally
 *
 * \param [in] port Port name as string
 * \return true if success
 *
 */
bool PHY_DoBreak(char *port)
{
  uint8_t buf[] = {UPDI_BREAK, UPDI_BREAK};

  LOG_Print(LOG_LEVEL_INFO, "Sending double break");
  COM_Close();
  // Re-init at a lower baudrate
  // At 300 bauds, the break character will pull the line low for 30ms
  // Which is slightly above the recommended 24.6ms
  // no parity, one stop bit
  if (COM_Open(port, 300, false, false) != true)
    return false;
  // Send two break characters, with 1 stop bit in between
  COM_Write(buf, sizeof(buf));
  // Wait for the double break end
  msleep(1000);  // wait for 1 second
  if (COM_Read(buf, 2) != 2)
    LOG_Print(LOG_LEVEL_WARNING, "No answer received");

  COM_Close();

  return true;
}

/** \brief Send data to physical interface
 *
 * \param [in] data Buffer with data
 * \param [in] len Length of data buffer
 * \return true if success
 *
 */
bool PHY_Send(uint8_t *data, uint8_t len)
{
  //uint8_t i;

  /*for (i = 0; i < len; i++)
  {
    COM_Write(&data[i], 1);
  }*/
  COM_Write(data, len);
  // read echo
  //usleep(10);
  //msleep(COM_GetTransTime(len));
  //Sleep(10);

  COM_Read(data, len);

  return true;
}

/** \brief Receive data from physical interface to data buffer
 *
 * \param [out] data Data buffer to write data in
 * \param [in] len Length of data to be received
 * \return true if success
 *
 */
bool PHY_Receive(uint8_t *data, uint16_t len)
{
  int val = COM_Read(data, len);
  if ((val < 0) || (val != len))
    return false;
  return true;
}

/** \brief Close physical interface
 *
 * \return Nothing
 *
 */
void PHY_Close(void)
{
  COM_Close();
}
