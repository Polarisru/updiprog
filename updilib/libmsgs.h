#ifndef LIB_MSGS_H
#define LIB_MSGS_H


#define MSG_CANT_ENTER_PROG_MODE    "Can't enter programming mode, exiting"
#define MSG_CANT_OPEN_PORT          "Can't open port: %s"
#define MSG_CHECK_CONNECTION        "Please check connection and try again."
#define MSG_WORKING_WITH_DEVICE     "Working with device: %s"
#define MSG_FUSE                    "  0x%02X: 0x%02X"
#define MSG_WRITE_FUSE              "Writing 0x%02X to fuse Nr. %d"

#define MSG_LOCKING                 "Locking MCU..."
#define MSG_UNLOCKING               "Unlocking MCU..."
#define MSG_ERASING                 "Erasing..."
#define MSG_WRITING_FLASH           "Writing from hex data..."
#define MSG_READING_FLASH           "Reading flash to hex data..."
#define MSG_WRITING_FUSES           "Writing fuses:"
#define MSG_READING_FUSES           "Reading fuses:"

#define MSG_WRONG_DATA              "Passed NULL data pointer"
#define MSG_PASSED_EMPTY_DATA       "Passed empty data"
#define MSG_WRONG_SEQ               "Unknown seq type %d"

#define MSG_OK                      "OK"


#endif // LIB_MSGS_H
