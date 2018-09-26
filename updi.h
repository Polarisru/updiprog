#ifndef UPDI_H
#define UPDI_H

// UPDI commands and control definitions
#define UPDI_BREAK        0x00

#define UPDI_LDS          0x00
#define UPDI_STS          0x40
#define UPDI_LD           0x20
#define UPDI_ST           0x60
#define UPDI_LDCS         0x80
#define UPDI_STCS         0xC0
#define UPDI_REPEAT       0xA0
#define UPDI_KEY          0xE0

#define UPDI_PTR          0x00
#define UPDI_PTR_INC      0x04
#define UPDI_PTR_ADDRESS  0x08

#define UPDI_ADDRESS_8    0x00
#define UPDI_ADDRESS_16   0x04

#define UPDI_DATA_8       0x00
#define UPDI_DATA_16      0x01

#define UPDI_KEY_SIB      0x04
#define UPDI_KEY_KEY      0x00

#define UPDI_KEY_64       0x00
#define UPDI_KEY_128      0x01

#define UPDI_SIB_8BYTES   UPDI_KEY_64
#define UPDI_SIB_16BYTES  UPDI_KEY_128

#define UPDI_REPEAT_BYTE  0x00
#define UPDI_REPEAT_WORD  0x01

#define UPDI_PHY_SYNC     0x55
#define UPDI_PHY_ACK      0x40

#define UPDI_MAX_REPEAT_SIZE  0xFF

// CS and ASI Register Address map
#define UPDI_CS_STATUSA       0x00
#define UPDI_CS_STATUSB       0x01
#define UPDI_CS_CTRLA         0x02
#define UPDI_CS_CTRLB         0x03
#define UPDI_ASI_KEY_STATUS   0x07
#define UPDI_ASI_RESET_REQ    0x08
#define UPDI_ASI_CTRLA        0x09
#define UPDI_ASI_SYS_CTRLA    0x0A
#define UPDI_ASI_SYS_STATUS   0x0B
#define UPDI_ASI_CRC_STATUS   0x0C

#define UPDI_CTRLA_IBDLY_BIT      7
#define UPDI_CTRLB_CCDETDIS_BIT   3
#define UPDI_CTRLB_UPDIDIS_BIT    2

#define UPDI_KEY_NVM              "NVMProg "
#define UPDI_KEY_CHIPERASE        "NVMErase"

#define UPDI_ASI_STATUSA_REVID    4
#define UPDI_ASI_STATUSB_PESIG    0

#define UPDI_ASI_KEY_STATUS_CHIPERASE   3
#define UPDI_ASI_KEY_STATUS_NVMPROG     4
#define UPDI_ASI_KEY_STATUS_UROWWRITE   5

#define UPDI_ASI_SYS_STATUS_RSTSYS      5
#define UPDI_ASI_SYS_STATUS_INSLEEP     4
#define UPDI_ASI_SYS_STATUS_NVMPROG     3
#define UPDI_ASI_SYS_STATUS_UROWPROG    2
#define UPDI_ASI_SYS_STATUS_LOCKSTATUS  0

#define UPDI_RESET_REQ_VALUE    0x59

// FLASH CONTROLLER
#define UPDI_NVMCTRL_CTRLA      0x00
#define UPDI_NVMCTRL_CTRLB      0x01
#define UPDI_NVMCTRL_STATUS     0x02
#define UPDI_NVMCTRL_INTCTRL    0x03
#define UPDI_NVMCTRL_INTFLAGS   0x04
#define UPDI_NVMCTRL_DATAL      0x06
#define UPDI_NVMCTRL_DATAH      0x07
#define UPDI_NVMCTRL_ADDRL      0x08
#define UPDI_NVMCTRL_ADDRH      0x09

// CTRLA
#define UPDI_NVMCTRL_CTRLA_NOP                0x00
#define UPDI_NVMCTRL_CTRLA_WRITE_PAGE         0x01
#define UPDI_NVMCTRL_CTRLA_ERASE_PAGE         0x02
#define UPDI_NVMCTRL_CTRLA_ERASE_WRITE_PAGE   0x03
#define UPDI_NVMCTRL_CTRLA_PAGE_BUFFER_CLR    0x04
#define UPDI_NVMCTRL_CTRLA_CHIP_ERASE         0x05
#define UPDI_NVMCTRL_CTRLA_ERASE_EEPROM       0x06
#define UPDI_NVMCTRL_CTRLA_WRITE_FUSE         0x07

#define UPDI_NVM_STATUS_WRITE_ERROR   2
#define UPDI_NVM_STATUS_EEPROM_BUSY   1
#define UPDI_NVM_STATUS_FLASH_BUSY    0

#define UPDI_SIB_LENGTH               16

#endif
