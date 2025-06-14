#define MODE_UART           0
#define MODE_SINGLE_UART    0x3A
#define MODE_OCD            0xC5

#define CMD_RESET           0
#define CMD_VERIFY          0x13
#define CMD_ERASE           0x22
#define CMD_BLANK_CHECK     0x32
#define CMD_PROG            0x40
#define CMD_SET_BAUDE_RATE  0x9A
#define CMD_SECURITY_SET    0xA0
#define CMD_SECURITY_GET    0xA1
#define CMD_SECURITY_RLS    0xA2
#define CMD_CHECKSUM        0xB0
#define CMD_SIGNATURE       0xC0

#define STS_CMD_NUMBER_ERR  0x04
#define STS_PARAM_ERR       0x05
#define STS_ACK             0x06
#define STS_CHECKSUM_ERR    0x07
#define STS_VERIFY_ERR      0x0F
#define STS_PROTECT_ERR     0x10
#define STS_NACK            0x15
#define STS_ERASE_ERR       0x1A
#define STS_INTERNAL_ERR    0x1B
#define STS_WRITE_ERR       0x1C

#define OCD_CMD_CONNECT     0x91
#define OCD_CMD_READ        0x92
#define OCD_CMD_WRITE       0x93
#define OCD_CMD_EXEC        0x94

#define OCD_STS_UNLOCKED    0xF0
#define OCD_STS_LOCKED      0xF1
#define OCD_STS_UNLOCK_OK   0xF2
#define OCD_STS_UNLOCK_ERR  0xF3
#define OCD_STS_UNLOCK_NG   0xF4
