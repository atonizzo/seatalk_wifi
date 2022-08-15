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

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
// This sketch uses Lennart Henning's ESPTelnet library.
// https://github.com/LennartHennigs/ESPTelnet
#include "ESPTelnet.h"   
#include "seatalk_wifi.h"

sensor_data_t sensor_data;

#include "eeprom_helpers.h"
#import "web_pages.h"

#define WIFI_SSID                   "LindaRae"
#define WIFI_PASS                   "goodlife"

char cmd_payload[16];   // Holds the payload of the SeaTalk sentence.
char nmea_message[96];  // Maximum NMEA sentence length is 82 bytes.
const char *nmea_talker = "LR";
int current_state = SM_STATE_SEEK_CMD;

// Text buffer for the serial and/or telnet loggers.
char str_logger[256];

unsigned long ms_timer;

const int baudrates[] = 
{
    // These values must match those in the SELECT tag in the
    //  HTML code.
    9600, 19200, 38400, 57600, 115200
};

// Number of the entry in the command array for the sentence just
//  received.
unsigned int table_item;
unsigned int packet_bytes_received = 0;
unsigned int cursor_position;
float apparent_wind_angle = 0., apparent_wind_speed;

ESP8266WebServer http_server(80);

// This is the server that pushes the NMEA 0183 sentances to
//  requesting clients.
ESPTelnet nmea_server;
bool nmea_server_connected = false;

ESPTelnet telnet_logger;
bool telnet_logger_connected = false;

static void telnet_logger_connect(String ip)
{
    telnet_logger_connected = true;
    telnet_logger.print("\r\nRaw Data");
    int cursor_position = strlen("Raw Data");
    while (cursor_position < SEATALK_CMD_COLUMN)
    {
        telnet_logger.print(" ");
        cursor_position += 1;
    }
    telnet_logger.print("Command");
    cursor_position = SEATALK_CMD_COLUMN + strlen("Command");
    while (cursor_position < SEATALK_ARG_COLUMN)
    {
        telnet_logger.print(" ");
        cursor_position += 1;
    }
    telnet_logger.println("Arguments\r\n"
                   "------------------------------------"
                   "------------------------------------");
}

static void telnet_logger_disconnect(String ip)
{
    telnet_logger_connected = false;
}

static void nmea_server_connect(String ip)
{
    nmea_server_connected = true;
}

static void nmea_server_disconnect(String ip)
{
    nmea_server_connected = false;
}

static struct __cmd seatalk_commands[] =
{
    {SEATALK_APPARENT_WIND_ANGLE, 4, "APPARENT_WIND_ANGLE"},
    {SEATALK_APPARENT_WIND_SPEED, 4, "APPARENT_WIND_SPEED"},
    {SEATALK_LAMP_INTENSITY,      3, "LAMP_INTENSITY"},
    // Add new entries to this table above this line.
    {SEATALK_UNSUPPORTED,         0, "UNSUPPORTED"}, 
};

void append_blanks(char *p, unsigned int to, unsigned int from)
{
    if (from >= to)
        return;
    while (from < to)
    {
        strcat(p, " ");
        from += 1;
    }
}

unsigned long float_to_str(char *p, float num, int precision)
{
    unsigned long character_count = 0;
    if (num < 0)
    {
        sprintf(p + strlen(p), "-");
        character_count = 1;
        num = -num;
    }    

    character_count += sprintf(p + strlen(p), "%d", (long)num);
    if (precision == 0)
        return character_count;
    num -= (long)num;
    precision = min(10, precision);

    strcat(p, ".");
    character_count += 1;
    while (precision > 0)
    {
        num *= 10;
        sprintf(p + strlen(p), "%d", (int)num);
        num -= (int)num;
        character_count += 1;
        precision -= 1;
    }
    return character_count;
}

int nmea_compute_checksum(char *buf)
{
    int chksum = 0, i = 0;
    while (buf[i] !=0)
    {
        switch(buf[i])
        {
            case '$':
                i += 1;
                break;
            case '*':
                return chksum;
                continue;
            default:
                // Is this the first value for the checksum?
                if (chksum == 0)
                    chksum = buf[i];
                else
                    // No. XOR the checksum with this character's value
                    chksum ^= buf[i];
                i += 1;
                break;
        }
    }

    // Should not be here.
    return chksum;
}

void append_text_attribute(char *p, text_attribute_t c)
{
    if (sensor_data.status.colorize_prettyprint == 1)
    {
        strcat(p, "\e[");
        sprintf(p + strlen(p), "%dm", c);
    }
}

void print_attribute(text_attribute_t c)
{
    if (sensor_data.status.colorize_prettyprint == 1)
    {
        char p[8];
        p[0] = '\0';
        append_text_attribute(p, c);
        Serial1.print(p);
    }
}

// From Hacker's Delight.
bool calculate_parity(int a)
{
    int y = a ^ (a >> 1);
    y = y ^ (y >> 2);
    y = y ^ (y >> 4);
    return y & 1;
}

bool parity_bit(int c)
{
    int should_parity = calculate_parity(c);

    // In the ESP8266 Serial.hasRxError() returns 1 when at least one
    //  of 3 different errors are present: Framing, parity and FIFO
    //  timout. Without madifying the standard library it is not
    //  possible to isolate the parity bit and thus this code makes the
    //  somewhat educated assumption that when an error is present it
    //  will be a parity violation.
    int has_rx_error = Serial.hasRxError();
    return should_parity ^ has_rx_error;
}

static void prettyprint_command(char *p, unsigned int cursor_position)
{
    p[0] = '\0';
    append_text_attribute(p, TEXT_ATTRIB_FG_BLUE);
    cursor_position = sprintf(p + strlen(p), "%02X ", cmd_payload[0]);
    append_text_attribute(p, TEXT_ATTRIB_NORMAL);

    for (int i = 1; i < packet_bytes_received; i++)
        cursor_position += sprintf(p + strlen(p),
                                   "%02X ",
                                   cmd_payload[i]);
    append_blanks(p, SEATALK_CMD_COLUMN, cursor_position);
    append_text_attribute(p, TEXT_ATTRIB_FG_GREEN);
    strcat(p, seatalk_commands[table_item].cmd_name);
    append_text_attribute(p, TEXT_ATTRIB_NORMAL);
    cursor_position = SEATALK_CMD_COLUMN +
          strlen(seatalk_commands[table_item].cmd_name);
    append_blanks(p, SEATALK_ARG_COLUMN, cursor_position);
}

static void print_slogger_banner(void)
{
    Serial1.print("\r\nRaw Data");
    cursor_position = strlen("Raw Data");
    while (cursor_position++ < SEATALK_CMD_COLUMN)
        Serial1.print(" ");
    Serial1.print("Command");
    cursor_position = SEATALK_CMD_COLUMN + strlen("Command");
    while (cursor_position++ < SEATALK_ARG_COLUMN)
        Serial1.print(" ");
    Serial1.print("Arguments\r\n"
                   "------------------------------------"
                   "------------------------------------\r\n");
}

void handleRoot()
{
    String hp(homepage);
    http_server.send(200, "text/html", hp.c_str());
}

void handler_updater(void)
{
    String value = http_server.arg("slogger");
    if ((sensor_data.status.serial_logger == 0) &&
                                    (value.toInt() == 1))
        print_slogger_banner();
    sensor_data.status.serial_logger = value.toInt();
    value = http_server.arg("tlogger");
    sensor_data.status.telnet_logger = value.toInt();
    value = http_server.arg("slogger_baud");
    sensor_data.status.slogger_baudrate = value.toInt();
    if (http_server.arg("server_port").toInt() < 65536)
        sensor_data.server_port =
                        http_server.arg("server_port").toInt();
    sensor_data.hostname[0] = '\0';
    strncat((char *)sensor_data.hostname,
            http_server.arg("hostname").c_str(),
            31);        
    value = http_server.arg("colorize");
    sensor_data.status.colorize_prettyprint = value.toInt();
    value = http_server.arg("led");
    sensor_data.status.activity_led = value.toInt();
    http_server.send(200, "text/plain", "\r\n");
    commit_eeprom();
}

void handler_get_status(void)
{
    char s[128];
    sprintf(s,
            "ipaddr=%s&slogger=%d&tlogger=%d&"
            "slogger_baudrate=%d&server_port=%d&"
            "hostname=%s&colorize=%d&led=%d",
            WiFi.localIP().toString().c_str(),
            sensor_data.status.serial_logger,
            sensor_data.status.telnet_logger,
            sensor_data.status.slogger_baudrate,
            sensor_data.server_port,
            sensor_data.hostname,
            sensor_data.status.colorize_prettyprint,
            sensor_data.status.activity_led);
    http_server.send(200, "text/plain", s);
}

void handler_page_not_found(void)
{
    http_server.send(404, "text/plain", "404: Not found");
}

void sendout_strlogger(void)
{
    strcat(str_logger, "\r\n");
    if (sensor_data.status.serial_logger == 1)
        Serial1.print(str_logger);

    if (sensor_data.status.telnet_logger == 1)
        if (telnet_logger_connected == true)
            telnet_logger.print(str_logger);
}

void print_framing_error(unsigned int payload_byte,
                         unsigned int expected)
{
    // Packet is compromised.
    cursor_position += sprintf(str_logger + strlen(str_logger),
                               "%02X ",
                               payload_byte);
    append_blanks(str_logger, SEATALK_CMD_COLUMN, cursor_position);
    append_text_attribute(str_logger, TEXT_ATTRIB_FG_RED);
    sprintf(str_logger + strlen(str_logger), "FRAMING ERROR\r\n");
    append_text_attribute(str_logger, TEXT_ATTRIB_NORMAL);
    append_blanks(str_logger, SEATALK_ARG_COLUMN, cursor_position);
    sprintf(str_logger + strlen(str_logger), "Expected 0x%02X", expected);
    sendout_strlogger();
}

void print_parity_error(unsigned int payload_byte)
{
    // Packet is compromised.
    str_logger[0] = '\0';
    append_text_attribute(str_logger, TEXT_ATTRIB_FG_BLUE);
    if (packet_bytes_received == 1)
        append_text_attribute(str_logger, TEXT_ATTRIB_FG_RED);
    cursor_position = sprintf(str_logger + strlen(str_logger),
                              "%02X ",
                              cmd_payload[0]);
    append_text_attribute(str_logger, TEXT_ATTRIB_NORMAL);

    for (int i = 1; i < packet_bytes_received; i++)
    {
        if (i == packet_bytes_received - 1)
            append_text_attribute(str_logger, TEXT_ATTRIB_FG_RED);
        cursor_position += sprintf(str_logger + strlen(str_logger),
                                   "%02X ",
                                   cmd_payload[i]);
        append_text_attribute(str_logger, TEXT_ATTRIB_NORMAL);
    }
    append_blanks(str_logger, SEATALK_CMD_COLUMN, cursor_position);
    append_text_attribute(str_logger, TEXT_ATTRIB_FG_RED);
    sprintf(str_logger + strlen(str_logger), "PARITY ERROR");
    append_text_attribute(str_logger, TEXT_ATTRIB_NORMAL);
    sendout_strlogger();
}

void setup()
{
    // This is the serial used to communicate with the SeaTalk bus.
    Serial.begin(SEATALK_DATARATE, SERIAL_8E1);
    while (!Serial);

    //Init EEPROM
    EEPROM.begin(EEPROM_SIZE);

    unsigned char *p = (unsigned char *)&sensor_data;
    for (int i = 0; i < EEPROM_SIZE; i++)
        p[i] = EEPROM.read(i);
    if (sensor_data.magic_number != MAGIC_NUMBER)
    {
        memset(&sensor_data, '\0', EEPROM_SIZE);
        // Set default values for the sensor_data structure.
        sensor_data.status.serial_logger = 1;
        sensor_data.status.telnet_logger = 1;
        sensor_data.status.slogger_baudrate = 4;
        sensor_data.status.colorize_prettyprint = 0;
        sensor_data.status.activity_led = 1;
        sensor_data.server_port = NMEA_SERVER_DEFAULT_PORT;
        sensor_data.hostname[0] = '\0';
        strncat((char *)sensor_data.hostname, DEFAULT_HOSTNAME, 31);        
        sensor_data.magic_number = MAGIC_NUMBER;
        commit_eeprom();
    }

    // This is the serial used for logging messages.
    Serial1.begin(baudrates[sensor_data.status.slogger_baudrate]);
    while (!Serial1);

    Serial1.print("\e[2J\r\n");    
    dump_eeprom();
    print_eeprom();

    int seconds = 0;

    Serial1.print("Connecting to WiFi: ");
    WiFi.mode(WIFI_STA);
    WiFi.hostname((char *)sensor_data.hostname);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        seconds += 1;
        if (seconds == 15)
            break;
        Serial1.print(seconds);
        Serial1.print(" ");
    }

    Serial1.print("\r\n");
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial1.println("WiFi IP: " + WiFi.localIP().toString());
        Serial1.println("Hostname: " + WiFi.hostname());
        // WiFi RF power output ranges from 0dBm to 20.5dBm.
        WiFi.setOutputPower(10);

        // Start the NMEA bridge.
        nmea_server.onConnect(nmea_server_connect);
        nmea_server.onReconnect(nmea_server_connect);
        nmea_server.onDisconnect(nmea_server_disconnect);
        Serial1.print("NMEA Server Status: ");
        if (nmea_server.begin(sensor_data.server_port))
        {
            print_attribute(TEXT_ATTRIB_FG_GREEN);
            Serial1.println("running.");
            print_attribute(TEXT_ATTRIB_NORMAL);
            Serial1.print("NMEA Server Port: ");
            Serial1.println(sensor_data.server_port);
        }
        else
        {
            print_attribute(TEXT_ATTRIB_FG_RED);
            Serial1.println("not connected.");
            print_attribute(TEXT_ATTRIB_NORMAL);
        }

        telnet_logger.onConnect(telnet_logger_connect);
        telnet_logger.onReconnect(telnet_logger_connect);
        telnet_logger.onDisconnect(telnet_logger_disconnect);
        Serial1.print("Telnet Server Status: ");
        if (telnet_logger.begin(23))
        {
            print_attribute(TEXT_ATTRIB_FG_GREEN);
            Serial1.println("running");
        }
        else
        {
            print_attribute(TEXT_ATTRIB_FG_RED);
            Serial1.println("not connected.");
        }
        print_attribute(TEXT_ATTRIB_NORMAL);

        // HTTP Server
        http_server.on("/", handleRoot);
        http_server.on("/updater", handler_updater);
        http_server.on("/get_status", handler_get_status);
        http_server.onNotFound(handler_page_not_found);
        http_server.begin();
    }
    else    
        Serial1.println("Can't connect to WiFi Network");

    if (sensor_data.status.serial_logger == 1)
        print_slogger_banner();

    pinMode(16, OUTPUT);
    digitalWrite(16, HIGH);
}

void loop()
{
    int parity;
    unsigned int payload_byte;
    unsigned int seatalk_cmd;
    WiFiClient cl;

    if (millis() > ms_timer)
        digitalWrite(16, HIGH);
        
    nmea_server.loop();
    if (sensor_data.status.telnet_logger == 1)
        telnet_logger.loop();
    http_server.handleClient();
    
    switch (current_state)
    {
    case SM_STATE_SEEK_CMD:
        if (Serial.available() == 0)
            break;
        seatalk_cmd = Serial.read();
        parity = parity_bit(seatalk_cmd);
        if (parity == 1)
        {
            // Make sure that the command received is one of those
            //  currently supported, which are held in the
            //  seatalk_commands[] array.
            table_item = 0;
            do
            {
                if (seatalk_commands[table_item].cmd == seatalk_cmd)
                {
                    cmd_payload[0] = seatalk_cmd;
                    packet_bytes_received = 1;
                    current_state = SM_STATE_PAYLOAD;
                    break;
                }
                table_item += 1;
            }
            while (seatalk_commands[table_item].cmd != 
                                                SEATALK_UNSUPPORTED);
            if (seatalk_commands[table_item].cmd == SEATALK_UNSUPPORTED)
                current_state = SM_STATE_PRETTYPRINT;
        }
        break;
    case SM_STATE_PAYLOAD:
        if (Serial.available() == 0)
            break;
        payload_byte = Serial.read();
        parity = parity_bit(payload_byte);
        if (parity == 1)
        {
            // Payload bytes are never supposed to have the parity
            //  bit set. We'll toss the entire frame.
            print_parity_error(payload_byte);
            current_state = SM_STATE_SEEK_CMD;
        }
        else
        {
            cmd_payload[packet_bytes_received] = payload_byte;
            packet_bytes_received += 1;
            if (packet_bytes_received == 
                                seatalk_commands[table_item].bytes)
                current_state = SM_STATE_SANITY_CHECK;
        }
        break;
    case SM_STATE_SANITY_CHECK:
        // Perform due diligence on the data received.
        switch (seatalk_commands[table_item].cmd)
        {
        case SEATALK_APPARENT_WIND_ANGLE:
        case SEATALK_APPARENT_WIND_SPEED:
            // First payload byte must be 0x01.
            if (cmd_payload[1] != 0x01)
            {
                print_framing_error(cmd_payload[1], 0x01);
                current_state = SM_STATE_SEEK_CMD;
            }
            else
                current_state = SM_STATE_FORMAT;
            break;
        case SEATALK_LAMP_INTENSITY:
            if (cmd_payload[1] != 0x00)
            {
                print_framing_error(cmd_payload[1], 0x00);
                current_state = SM_STATE_SEEK_CMD;
            }
            else
                current_state = SM_STATE_FORMAT;
            break;
        default:
            current_state = SM_STATE_FORMAT;
            break;
        }
        break;
    case SM_STATE_FORMAT:
        switch (seatalk_commands[table_item].cmd)
        {
        case SEATALK_APPARENT_WIND_ANGLE:
            apparent_wind_angle = 0.5 * (((cmd_payload[2] << 8) +
                                                cmd_payload[3]) % 360);
            current_state = SM_STATE_SEND;
            break;
        case SEATALK_APPARENT_WIND_SPEED:
            apparent_wind_speed = 1.0 * (cmd_payload[2] & 0x7F) +
                                 (1.0 * (cmd_payload[3] & 0x0F)) / 10.;
            if ((cmd_payload[2] & 0x80) != 0)
                // Value for speed is in m/s. We'll convert to knots.
                apparent_wind_speed *= 1.944;
            current_state = SM_STATE_SEND;
            break;
        case SEATALK_LAMP_INTENSITY:
            current_state = SM_STATE_PRETTYPRINT;
            break;
        default:
            current_state = SM_STATE_SEEK_CMD;
            break;
        }
        break;
    case SM_STATE_SEND:
        switch (seatalk_commands[table_item].cmd)
        {
        case SEATALK_APPARENT_WIND_SPEED:
        case SEATALK_APPARENT_WIND_ANGLE:
            if (nmea_server_connected == true)
            {
                // Speed is always in knots.
                sprintf(nmea_message,
                        "$%sMWV,%3.1f,R,%2.1f,N,A*",
                        nmea_talker,
                        apparent_wind_angle,
                        apparent_wind_speed);
                sprintf(nmea_message + strlen(nmea_message),
                        "%02X\r\n",
                        nmea_compute_checksum(nmea_message));
                nmea_server.print(nmea_message);
            }
            break;
        default:
            break;
        }
        current_state = SM_STATE_PRETTYPRINT;
        if (sensor_data.status.activity_led == 1)
        {
            digitalWrite(16, LOW);
            ms_timer = millis() + 100;
        }
        break;
    case SM_STATE_PRETTYPRINT:
        if ((sensor_data.status.serial_logger == 1) ||
                (sensor_data.status.telnet_logger == 1))
        {        
            switch (seatalk_commands[table_item].cmd)
            {
            case SEATALK_APPARENT_WIND_ANGLE:
                prettyprint_command(str_logger, cursor_position);
                append_text_attribute(str_logger, TEXT_ATTRIB_FG_BLUE);
                float_to_str(str_logger, apparent_wind_angle, 1);
                strcat(str_logger, " deg");
                append_text_attribute(str_logger, TEXT_ATTRIB_NORMAL);
                break;
            case SEATALK_APPARENT_WIND_SPEED:
                prettyprint_command(str_logger, cursor_position);
                append_text_attribute(str_logger, TEXT_ATTRIB_FG_BLUE);
                float_to_str(str_logger, apparent_wind_speed, 1);
                strcat(str_logger, " knots");
                append_text_attribute(str_logger, TEXT_ATTRIB_NORMAL);
                break;
            case SEATALK_LAMP_INTENSITY:
                prettyprint_command(str_logger, cursor_position);
                append_text_attribute(str_logger, TEXT_ATTRIB_FG_BLUE);
                sprintf(str_logger + strlen(str_logger),
                        "%d",
                        (cmd_payload[2] & 0x0C) >> 2);
                append_text_attribute(str_logger, TEXT_ATTRIB_NORMAL);
                break;
            default:
                // Unsupported SeaTalk sentence.
                str_logger[0] = '\0';
                append_text_attribute(str_logger, TEXT_ATTRIB_FG_RED);
                cursor_position = sprintf(str_logger + strlen(str_logger),
                                          "%02X ",
                                          cmd_payload[0]);
                append_blanks(str_logger, SEATALK_CMD_COLUMN, cursor_position);
                strcat(str_logger, seatalk_commands[table_item].cmd_name);
                append_text_attribute(str_logger, TEXT_ATTRIB_NORMAL);
                break;
            }
        }

        sendout_strlogger();
        current_state = SM_STATE_SEEK_CMD;
        break;    
    default:
        break;
    }
}
