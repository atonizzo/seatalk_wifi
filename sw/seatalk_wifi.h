// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: Anthony Tonizzo - 2022

#ifndef __SEATALK_WIFI_H__
#define __SEATALK_WIFI_H__

typedef enum
{
    // The SM_STATE_SEEK_CMD looks for a byte from
    //  the input data stream that comes with the
    //  parity bit set.
    SM_STATE_SEEK_CMD           = 0,

    // The SM_STATE_PAYLOAD collects the payload
    //  bytes for the specific command just received
    //  during the SM_STATE_SEEK_CMD state. The number
    //  of bytes to receive is held in the
    //  seatalk_commands[] structure.
    SM_STATE_PAYLOAD            = 1,

    // The SM_STATE_SANITY_CHECK is meant to check if
    //  the stream of payload bytes that was received
    //  is correct.
    SM_STATE_SANITY_CHECK       = 2,
    
    // The SM_STATE_FORMAT state extract any relevant
    //  information from the payload bytes.
    SM_STATE_FORMAT             = 3,

    // The SM_STATE_SEND state creates the NMEA sentence
    //  and sends it out via the nmea_server.
    SM_STATE_SEND               = 4,

    // The SM_STATE_PRETTYPRINT state prints out a message
    //  to log the SeaTalk sentence that was just processed.
    SM_STATE_PRETTYPRINT        = 5,
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

#define SEATALK_CMD_COLUMN          32
#define SEATALK_ARG_COLUMN          60

#define SEATALK_DATARATE            4800
#define LOGPORT_DATARATE            115200

#define NMEA_SERVER_DEFAULT_PORT    3030
#define NMEA_DEFAULT_TALKER         "LR"

struct __status
{
    volatile uint32_t serial_logger         : 1;
    volatile uint32_t telnet_logger         : 1;
    volatile uint32_t slogger_baudrate      : 3;
    volatile uint32_t colorize_prettyprint  : 1;
    volatile uint32_t activity_led          : 1;
    volatile uint32_t wifi_power            : 8;
             uint32_t                       : 17;
};
typedef struct __status status_t;

struct __wind_data
{
    volatile uint32_t filter_angle_enable  : 1;
    volatile uint32_t filter_speed_enable  : 1;
    volatile uint32_t angle_ma_length      : 8;
    volatile uint32_t speed_ma_length      : 8;
             uint32_t                      : 14;
};
typedef struct __wind_data wind_data_t;

struct __sensor_data
{
    volatile status_t status;
    volatile uint32_t server_port;
    volatile uint8_t hostname[32];
    volatile uint8_t nmea_talker[3];
    wind_data_t wind_data;
    volatile uint8_t reserved[9];
    volatile uint32_t magic_number;
};
typedef struct __sensor_data sensor_data_t;
extern sensor_data_t sensor_data, sensor_data_mem;

#define EEPROM_SIZE                 sizeof(sensor_data_t)
#define MAGIC_NUMBER                0xABBACAFE
#define DEFAULT_HOSTNAME            "wind"

#define WIND_ANGLE_FILTER_TAPS      16
extern unsigned int wind_angle_history[WIND_ANGLE_FILTER_TAPS];
extern unsigned int last_wind_angle;

#define WIND_SPEED_FILTER_TAPS      16
extern unsigned int wind_speed_history[WIND_SPEED_FILTER_TAPS];
extern unsigned int last_wind_speed;

#define ACTIVITY_LED                12

extern const int baudrates[];
void print_attribute(text_attribute_t);
unsigned long float_to_str(char *, float, int);

extern const char homepage[];

#endif
