/*!
 *  \file      updilib.h
 *  \brief     Library and API for programming AVR devices over
 *             the UPD interface using a standard TTL serial port
 *  \details   Main header for API. UPDIlib is a dynamically linking
 *             library and API for working with the UPDInterface
 *  \author    Ilya Medvedkov
 *  \version   1.0
 *  \date      2023
 *  \remark
 *  \copyright
 */

#ifndef UPDILIB_H
#define UPDILIB_H

#include <stdint.h>
#include <stdbool.h>

#include "../com.h"
#include "../log.h"
#include "../progress.h"

#ifdef BUILD_DLL
    #ifdef WIN64
    #define DLL_EXPORT __declspec(dllexport)
    #define DLL_HIDDEN
    #else
    #define DLL_EXPORT __attribute__((visibility("default")))
    #define DLL_HIDDEN __attribute__((visibility("hidden")))
    #endif
#else
    #ifdef WIN64
    #define DLL_EXPORT __declspec(dllimport)
    #else
    #define DLL_EXPORT
    #endif
    #define DLL_HIDDEN
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*! \defgroup MAIN_DECLS Declaring basic API types and constants
 *
 * @{
 */

typedef int8_t  UPDI_bool;           /*!< \brief UPDIlib bool type. */

/*! @brief Configuration of UPDI connection with AVR device */
typedef struct
{
  uint32_t       baudrate;           /*!< \brief Baudrate for COM port */
  int8_t         device;             /*!< \brief The AVR device ID */
  char           port[COMPORT_LEN];  /*!< \brief The symbolic name of the COM port */
  UPDI_logger*   logger;             /*!< \brief Pointer to the logger object */
  UPDI_progress* progress;           /*!< \brief Pointer to the progress indicator object */
} UPDI_Params;

/*! @brief AVR fuse */
typedef struct
{
  uint8_t fuse;                      /*!< \brief Fuse ID */
  uint8_t value;                     /*!< \brief Fuse value */
} UPDI_fuse;

/*! @brief Description of the API request element  */
typedef struct
{
  uint8_t seq_type;                  /*!< \brief The type of the request element */
  void * data;                       /*!< \brief Pointer to the data of the element */
  int32_t data_len;                  /*!< \brief The length of the data */
} UPDI_seq;

/*! @brief Possible types of request element */
enum {
  /*! \brief Request to unlock the device */
  UPDI_SEQ_UNLOCK         = 1,
  /*! \brief Request to enter to programming mode */
  UPDI_SEQ_ENTER_PM       = 2,
  /*! \brief Request to erase the device */
  UPDI_SEQ_ERASE          = 3,
  /*! \brief Request to flash the device
   *
   * \a UPDI_seq.data - the contents of the flash mem to write <br>
   * \a UPDI_seq.data_len - the length of the contents
   */
  UPDI_SEQ_FLASH          = 4,
  /*! \brief Request to read flash memory from the device
   *
   * As a result of the request, the contents of the device's
   * flash memory will be saved to the memory block. The actual
   * value of the number of bytes copied will be assigned to the
   * field \a UPDI_seq.data_len <br>
   * \a UPDI_seq.data - pointer to the memory block to save data <br>
   * \a UPDI_seq.data_len - available memory block size
   */
  UPDI_SEQ_READ           = 5,
  /*! \brief Request to write fuses values to the device
   *
   * As a result of the request, the fuse values will be
   * recorded to the \ref UPDI_fuse.value field of each element
   * of the array <br>
   * \a UPDI_seq.data - pointer to the \ref UPDI_fuse array <br>
   * \a UPDI_seq.data_len - number of array elements
   */
  UPDI_SEQ_SET_FUSES      = 6,
  /*! \brief Request to read fuses values from the device
   *
   * \a UPDI_seq.data - pointer to the \ref UPDI_fuse array <br>
   * \a UPDI_seq.data_len - number of array elements
   */
  UPDI_SEQ_GET_FUSES      = 7,
  /*! \brief Request to lock the device */
  UPDI_SEQ_LOCK           = 8,
  /*! \brief Request to read the sigrow
   *
   * As a result of the request, the contents of the device's
   * sigrow will be saved to the memory block. The actual
   * value of the number of bytes copied will be assigned to the
   * field \a UPDI_seq.data_len <br>
   * \a UPDI_seq.data - pointer to the memory block to save data <br>
   * \a UPDI_seq.data_len - available memory block size
   */
  UPDI_SEQ_GET_SIG_ROW    = 9
};

/** @} */


/*! \defgroup LOGGER_SEC Functions for working with logs
 *
 * @{
 */

/*! @brief Init the new logger object
 *
 *  @param name      label for the logger object
 *  @param level     level of verbosity for the logger object
 *  @param on_log    callback to react to a new log message
 *  @param on_free   callback to react to logger destruction
 *  @param user_data user data to pass to callbacks
 *  @return Pointer to the logger object on success, NULL on error
 */
DLL_EXPORT UPDI_logger * UPDILIB_logger_init(const char * name,
                                             int32_t level,
                                             UPDI_onlog on_log,
                                             UPDI_onlogfree on_free,
                                             void * user_data);
/*! @brief Destroy the logger object
 *
 *  @param logger    pointer to the logger object
 */
DLL_EXPORT void UPDILIB_logger_done(UPDI_logger * logger);

/*! @brief Set callback to react to a new log message passed in
 *         the global logger object
 *
 *  @param on_log    callback to react to a new log message
 *  @param user_data user data to pass to callbacks
 */
DLL_EXPORT void UPDILIB_set_glb_logger_onlog(UPDI_onlog on_log,
                                             void * user_data);
/*! @brief Set level of verbosity for the global logger object
 *
 *  @param level     level of verbosity for the logger object
 */
DLL_EXPORT void UPDILIB_set_glb_logger_level(int32_t level);

/** @} */


/*! \defgroup LOGGER_SEC Functions for working with progress indicator
 *
 * @{
 */

/*! @brief Init the new progress indicator object (PIO)
 *
 *  @param on_start  callback to react to progress starts
 *  @param on_step   callback to react to progress step
 *  @param on_stop   callback to react to progress finished
 *  @param user_data user data to pass to callbacks
 *  @return Pointer to the PIO on success, NULL on error
 */
DLL_EXPORT UPDI_progress * UPDILIB_progress_init(UPDI_onprgsstart on_start,
                                                 UPDI_onprgs on_step,
                                                 UPDI_onprgsfinish on_stop,
                                                 void * user_data);
/*! @brief Destroy the progress indicator object (PIO)
 *
 *  @param pio       pointer to the PIO
 */
DLL_EXPORT void UPDILIB_progress_done(UPDI_progress * pio);

/** @} */

/*! \defgroup CONFIG_SEC Functions for working with the API config
 *
 * @{
 */

/*! @brief Init the new API config
 *
 *  @return Pointer to the API config on success, NULL on error
 */
DLL_EXPORT UPDI_Params * UPDILIB_cfg_init();
/*! @brief Destroy the progress indicator object (PIO)
 *
 *  @param cfg       pointer to the API config
 */
DLL_EXPORT void UPDILIB_cfg_done(UPDI_Params * cfg);

/*! @brief Assign the logger object with the API config
 *
 *  @param cfg       pointer to the API config
 *  @param logger    pointer to the logger object
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_cfg_set_logger(UPDI_Params * cfg,
                                            UPDI_logger * logger);
/*! @brief Assign the progress indicator object (PIO) with the API config
 *
 *  @param cfg       pointer to the API config
 *  @param pio       pointer to the PIO
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_cfg_set_progress(UPDI_Params * cfg,
                                              UPDI_progress * pio);
/*! @brief Set the COM port baudrate value for the API config
 *
 *  @param cfg       pointer to the API config
 *  @param rate      baudrate value
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_cfg_set_buadrate(UPDI_Params * cfg,
                                              uint32_t rate);
/*! @brief Set the symbolic name of the COM port for the API config
 *
 *  @param cfg       pointer to the API config
 *  @param port      NULL-terminated string - symbolic name of
 *                   the COM port
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_cfg_set_com(UPDI_Params * cfg,
                                         const char * port);
/*! @brief Set the symbolic name of the device for the API config
 *
 *  @param cfg       pointer to the API config
 *  @param port      NULL-terminated string - symbolic name of
 *                   the device \ref UPDILIB_devices_get_name
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_cfg_set_device(UPDI_Params *, const char *);

/*! @brief Get the COM port baudrate value for the API config
 *
 *  @param cfg       pointer to the API config
 *  @return Baudrate value on success, zero on error
 */
DLL_EXPORT uint32_t     UPDILIB_cfg_get_baudrate(UPDI_Params * cfg);
/*! @brief Get the symbolic name of the COM port for the API config
 *
 *  @param cfg       pointer to the API config
 *  @return NULL-terminated string on success, NULL on error
 */
DLL_EXPORT const char * UPDILIB_cfg_get_com(UPDI_Params * cfg);
/*! @brief Get the device ID for the API config
 *
 *  @param cfg       pointer to the API config
 *  @return The device ID on success, zero on error
 */
DLL_EXPORT int8_t       UPDILIB_cfg_get_device(UPDI_Params * cfg);

/** @} */


/*! \defgroup DEVICES_SEC Functions for working with the devices list
 *
 * @{
 */

/*! @brief Get number of devices in the devices list
 *
 *  @return Number of devices on success, zero on error
 */
DLL_EXPORT int32_t   UPDILIB_devices_get_count();
/*! @brief Get the device name with specified ID
 *
 *  @param id       device ID
 *  @param value    pointer to string to save device name
 *  @param len      pointer to the string length with device name.
 *                  On input contains the available size of the memory block.
 *                  On output contains the length of the string in bytes.
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_devices_get_name(int8_t id,
                                              char * value,
                                              int32_t * len);
/*! @brief Get number of device fuses for the specified device ID
 *
 *  @param id       device ID
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT uint8_t   UPDILIB_devices_get_fuses_cnt(int8_t id);

/** @} */

/*! \defgroup PROG_SEC Functions for launching programming sequences
 *
 * @{
 */

/*! @brief Launch programming sequences
 *
 *  @param cfg       pointer to the API config
 *  @param seq       pointer to array with the programming sequence \ref UPDI_seq
 *  @param cnt       number of elements in the sequence
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_launch_seq(UPDI_Params * cfg,
                                        UPDI_seq * seq,
                                        uint8_t cnt);

DLL_EXPORT UPDI_bool UPDILIB_read_dev_info(UPDI_Params *, uint8_t *);

/*! \brief Erase the device
 *
 *  @param cfg       pointer to the API config
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_erase(UPDI_Params * cfg);
/*! \brief Lock the device
 *
 *  @param cfg       pointer to the API config
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_lock(UPDI_Params * cfg);
/*! \brief Unlock the device
 *
 *  @param cfg       pointer to the API config
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_unlock(UPDI_Params * cfg);

/*! \brief Write fuse values to the device
 *
 *  @param cfg       pointer to the API config
 *  @param fuses     pointer to the \ref UPDI_fuse array
 *  @param cnt       number of array elements
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_write_fuses(UPDI_Params * cfg,
                                         const UPDI_fuse * fuses,
                                         uint8_t cnt);
/*! \brief Read fuse values from the device
 *
 *  As a result of the sequence, the fuse values will be
 *  recorded to the \ref UPDI_fuse.value field of each element
 *  of the \a fuses array
 *  @param cfg       pointer to the API config
 *  @param fuses     pointer to the \ref UPDI_fuse array
 *  @param cnt       number of array elements
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_read_fuses(UPDI_Params * cfg,
                                        UPDI_fuse * fuses,
                                        uint8_t cnt);

/*! \brief Flash the device
 *
 *  @param cfg       pointer to the API config
 *  @param data      the contents of the flash mem to write
 *  @param data_len  available memory block size
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_write_hex(UPDI_Params * cfg,
                                       const char * data,
                                       int32_t data_len);


/*! \brief Read flash memory from the device
 *
 *  As a result of the sequence, the contents of the device's
 *  flash memory will be saved to the memory block. The actual
 *  value of the number of bytes copied will be assigned to the
 *  param \a data_len
 *  @param cfg       pointer to the API config
 *  @param data      pointer to the memory block to save data
 *  @param data_len  available memory block size
 *  @return Positive value on success, zero on error
 */
DLL_EXPORT UPDI_bool UPDILIB_read_hex(UPDI_Params * cfg,
                                      char * data,
                                      int32_t * data_len);

/** @} */

#ifdef __cplusplus
}
#endif



#endif // UPDILIB_H
