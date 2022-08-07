#ifndef __SEATALK_WIFI_H__
#define __SEATALK_WIFI_H__

typedef enum
{
    SM_STATE_SEEK_CMD           = 0,
    SM_STATE_PAYLOAD            = 1,
    SM_STATE_FORMAT             = 2,
    SM_STATE_SEND               = 3,
    SM_STATE_PRETTYPRINT        = 4,
} sm_state_t;

typedef enum
{
    SEATALK_APPARENT_WIND_ANGLE = 0x10,
    SEATALK_APPARENT_WIND_SPEED = 0x11,
    SEATALK_LAMP_INTENSITY      = 0x30,
    SEATALK_UNSUPPORTED         = 0xFFFF,
} seatalk_cmd_t;

struct __cmd
{
    seatalk_cmd_t cmd;  // Command byte.
    uint8_t bytes;      // Number of bytes other than the
                        //  command byte.
    char *cmd_name;     // Command name for pretty print.
};

typedef enum
{
    TEXT_ATTRIB_NORMAL     = 00,
    TEXT_ATTRIB_FG_BLACK   = 30,
    TEXT_ATTRIB_FG_RED     = 31,
    TEXT_ATTRIB_FG_GREEN   = 32,
    TEXT_ATTRIB_FG_YELLOW  = 33,
    TEXT_ATTRIB_FG_BLUE    = 34,
    TEXT_ATTRIB_FG_MAGENTA = 35,
    TEXT_ATTRIB_FG_CYAN    = 36,
    TEXT_ATTRIB_FG_WHITE   = 37,
    TEXT_ATTRIB_BG_BLACK   = 40,
    TEXT_ATTRIB_BG_RED     = 41,
    TEXT_ATTRIB_BG_GREEN   = 42,
    TEXT_ATTRIB_BG_YELLOW  = 43,
    TEXT_ATTRIB_BG_BLUE    = 44,
    TEXT_ATTRIB_BG_MAGENTA = 45,
    TEXT_ATTRIB_BG_CYAN    = 46,
    TEXT_ATTRIB_BG_WHITE   = 47
} text_attribute_t;

#define SEATALK_CMD_COLUMN      20
#define SEATALK_ARG_COLUMN      48

#define SEATALK_DATARATE        4800
#define LOGPORT_DATARATE        115200

#define NMEA_SERVER_DEFAULT_PORT    3030

const int baudrates[] = 
{
    // These values must match those in the SELECT tag in the
    //  HTML code.
    9600, 19200, 38400, 57600, 115200
};

struct __status
{
    volatile uint32_t serial_logger    : 1;
    volatile uint32_t telnet_logger    : 1;
    volatile uint32_t slogger_baudrate : 3;
             uint32_t                  : 27;
};
typedef struct __status status_t;

struct __sensor_data
{
    volatile status_t status;
    volatile uint32_t server_port;
    volatile uint8_t reserved[52];
    volatile uint32_t magic_number;
};
typedef struct __sensor_data sensor_data_t;
#define EEPROM_SIZE                 sizeof(sensor_data_t)
#define MAGIC_NUMBER                0xABBACAFE

#define DEFAULT_HOSTNAME            "wind"

#endif
