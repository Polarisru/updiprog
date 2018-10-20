#include <stdio.h>
#include <string.h>
#include "ihex.h"

static uint8_t crc;

/** \brief Convert byte to string with HEX representation
 *
 * \param [in] byte One byte of data
 * \return HEX representation as string
 *
 */
static char* IHEX_AddByte(uint8_t byte)
{
  static char res[3];

  crc += byte;
  uint8_t n = (byte & 0xF0U) >> 4; // high nybble
  res[0] = IHEX_DIGIT(n);
  n = byte & 0x0FU; // low nybble
  res[1] = IHEX_DIGIT(n);
  res[2] = 0;

  return res;
}

/** \brief Write Intel HEX ending to a file
 *
 * \param [in] fp File handle
 * \return true if succeed
 *
 */
bool IHEX_WriteEnd(FILE *fp)
{
  fwrite(IHEX_ENDFILE, strlen(IHEX_ENDFILE), 1, fp);
  fwrite(IHEX_NEWLINE, strlen(IHEX_NEWLINE), 1, fp);
  return true;
}

/** \brief Write data buffer to HEX file
 *
 * \param [in] fp File handle
 * \param [in] data Data buffer to write
 * \param [in] len Length of data buffer
 * \return error code as uint8_t
 *
 */
uint8_t IHEX_WriteFile(FILE *fp, uint8_t *data, uint16_t len)
{
  uint16_t i;
  uint8_t x;
  uint8_t width;
  char str[128];

  crc = 0;
  for (i = 0; i < len; i += IHEX_LINE_LENGTH)
  {
    strcpy(str, IHEX_START);
    // write length
    if (len - i >= IHEX_LINE_LENGTH)
      width = IHEX_LINE_LENGTH;
    else
      width = (uint8_t)(len - i);
    strcat(str, IHEX_AddByte(width));
    // write address
    strcat(str, IHEX_AddByte((uint8_t)(i >> 8)));
    strcat(str, IHEX_AddByte((uint8_t)i));
    // write type (data)
    strcat(str, IHEX_AddByte(IHEX_DATA_RECORD));
    for (x = 0; x < width; x++)
    {
      strcat(str, IHEX_AddByte(*data++));
    }
    crc = (uint8_t)(0x100 - crc);
    strcat(str, IHEX_AddByte(crc));
    strcat(str, IHEX_NEWLINE);
    fwrite(str, strlen(str), 1, fp);
    crc = 0;
  }
  IHEX_WriteEnd(fp);

  return IHEX_ERROR_NONE;
}

/** \brief Get one nibble from char
 *
 * \param [in] c Char value
 * \return data nibble as uint8_t
 *
 */
uint8_t IHEX_GetNibble(char c)
{
  if (c >= '0' && c <= '9')
    return (uint8_t)(c - '0');
  else if (c >= 'A' && c <= 'F')
    return (uint8_t)(c - 'A' + 10);
  else if (c >= 'a' && c <= 'f')
    return (uint8_t)(c - 'a' + 10);
  else
    return 0;
}

/** \brief Get full byte from two chars
 *
 * \param [in] data Two chars as HEX representation
 * \return byte value as uint8_t
 *
 */
uint8_t IHEX_GetByte(char *data)
{
  uint8_t res = IHEX_GetNibble(*data++) << 4;
  res += IHEX_GetNibble(*data);
  return res;
}

/** \brief Read Intel HEX file to a binary memory buffer
 *
 * \param [in] fp File handler
 * \param [out] data Data buffer to read data into
 * \param [in] maxlen Maximal data length
 * \param [out] max_addr Maximal address with non-empty data
 * \return error code as uint8_t
 *
 */
uint8_t IHEX_ReadFile(FILE *fp, uint8_t *data, uint16_t maxlen, uint16_t *max_addr)
{
  uint16_t addr;
  uint8_t len;
  uint8_t type;
  uint16_t segment;
  uint8_t i;
  uint8_t byte;
  char str[128];

  addr = 0;
  segment = 0;
  while (!feof(fp))
  {
    if (fgets(str, sizeof(str), fp) == NULL)
      return IHEX_ERROR_FILE;
    if (strlen(str) < IHEX_MIN_STRING)
      return IHEX_ERROR_FMT;
    len = IHEX_GetByte(&str[IHEX_OFFS_LEN]);
    addr = (IHEX_GetByte(&str[IHEX_OFFS_ADDR]) << 8) + IHEX_GetByte(&str[IHEX_OFFS_ADDR + 2]);
    if (addr + segment >= maxlen)
      return IHEX_ERROR_SIZE;
    type = IHEX_GetByte(&str[IHEX_OFFS_TYPE]);
    if (len * 2 + IHEX_MIN_STRING != strlen(str))
      return IHEX_ERROR_FMT;
    switch (type)
    {
      case IHEX_DATA_RECORD:
        //if (addr + segment + len >= *address)
        //  *address = addr + segment + len;
        for (i = 0; i < len; i++)
        {
          byte = IHEX_GetByte(&str[IHEX_OFFS_DATA + i * 2]);
          if (byte != 0xFF)
            *max_addr = addr + segment + i + 1;
          data[addr + segment + i] = byte;
        }
        break;
      case IHEX_END_OF_FILE_RECORD:
        return IHEX_ERROR_NONE;
      case IHEX_EXTENDED_SEGMENT_ADDRESS_RECORD:
        segment = ((IHEX_GetByte(&str[IHEX_OFFS_DATA]) << 8) + IHEX_GetByte(&str[IHEX_OFFS_DATA + 2])) << 4;
        break;
      case IHEX_START_SEGMENT_ADDRESS_RECORD:
        break;
      case IHEX_EXTENDED_LINEAR_ADDRESS_RECORD:
        break;
      case IHEX_START_LINEAR_ADDRESS_RECORD:
        break;
      default:
        return IHEX_ERROR_FMT;
    }
  }

  return IHEX_ERROR_NONE;
}
