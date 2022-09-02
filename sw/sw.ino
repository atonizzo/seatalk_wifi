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

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
// This sketch uses Lennart Henning's ESPTelnet library.
// https://github.com/LennartHennigs/ESPTelnet
#include "ESPTelnet.h"   
#include "seatalk_wifi.h"

sensor_data_t sensor_data, sensor_data_mem;

#include "wifi_credentials.h"   // Define WIFI_SSID and WIFI_PASS here.
#import "web_pages.h"

char cmd_payload[16];   // Holds the payload of the SeaTalk sentence.
char nmea_message[96];  // Maximum NMEA sentence length is 82 bytes.
int current_state = SM_STATE_SEEK_CMD;

unsigned int wind_angle_history[WIND_ANGLE_FILTER_TAPS];
unsigned int last_wind_angle;
unsigned int wind_angle_sum;

unsigned int wind_speed_history[WIND_SPEED_FILTER_TAPS];
unsigned int last_wind_speed;
unsigned int wind_speed_sum;

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

static struct __cmd seatalk_commands[] =
{
    {SEATALK_APPARENT_WIND_ANGLE, 4, "APPARENT_WIND_ANGLE"},
    {SEATALK_APPARENT_WIND_SPEED, 4, "APPARENT_WIND_SPEED"},
    {SEATALK_LAMP_INTENSITY,      3, "LAMP_INTENSITY"},
    // Add new entries to this table above this line.
    {SEATALK_UNSUPPORTED,         0, "UNSUPPORTED"}, 
};

ESP8266WebServer http_server(80);

// This is the server that pushes the NMEA 0183 sentances to
//  requesting clients.
ESPTelnet nmea_server_pri, nmea_server_sec;
bool nmea_server_pri_connected = false;
bool nmea_server_sec_connected = false;

ESPTelnet telnet_logger;
bool telnet_logger_connected = false;

static void telnet_logger_connect(String ip)
{
    telnet_logger_connected = true;
    telnet_logger.print("\r\nTimestamp  Raw Data");
    int cursor_position = strlen("Timestamp  Raw Data");
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

static void nmea_server_pri_connect(String ip)
{
    nmea_server_pri_connected = true;
}

static void nmea_server_pri_disconnect(String ip)
{
    nmea_server_pri_connected = false;
}

static void nmea_server_sec_connect(String ip)
{
    nmea_server_sec_connected = true;
}

static void nmea_server_sec_disconnect(String ip)
{
    nmea_server_sec_connected = false;
}

int append_blanks(char *p, unsigned int to, unsigned int from)
{
    if (from >= to)
        return 0;
    int added = to - from;
    while (from < to)
    {
        strcat(p, " ");
        from += 1;
    }
    return added;
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

    // Round the float number to the precision requested.
    precision = min(10, precision);
    num += 5. / pow(10, precision + 1);
    character_count += sprintf(p + strlen(p), "%d", (long)num);
    if (precision == 0)
        return character_count;
    num -= (long)num;

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
    unsigned int chksum = 0, i = 0;
    while (buf[i] != 0)
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
        Serial.print(p);
    }
}

// From Hacker's Delight.
bool calculate_parity(unsigned int a)
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
    //  somewhat educated assumption that when Serial.hasRxError()
    //  returns an error it will be a parity violation.
    int has_rx_error = Serial.hasRxError();
    return should_parity ^ has_rx_error;
}

void reset_wind_data(void)
{
    // Wind direction filter initializaton.
    memset(wind_angle_history,
           0,
           sizeof(wind_angle_history[WIND_ANGLE_FILTER_TAPS]));
    last_wind_angle = 0;
    wind_angle_sum = 0;

    // Wind speed filter initializaton.
    memset(wind_speed_history,
           0,
           sizeof(wind_speed_history[WIND_SPEED_FILTER_TAPS]));
    last_wind_speed = 0;
    wind_speed_sum = 0;
}

static void prettyprint_command(char *p, unsigned int cursor_position)
{
    p[0] = '\0';
    cursor_position = sprintf(p, "[%08d] ", millis());
    append_text_attribute(p, TEXT_ATTRIB_FG_BLUE);
    cursor_position += sprintf(p + strlen(p), "%02X ", cmd_payload[0]);
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
    Serial1.print("\r\nTimestamp  Raw Data");
    cursor_position = strlen("Timestamp  Raw Data");
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

void handler_push_settings(void)
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
    sensor_data.status.activity_led = http_server.arg("led").toInt();
    sensor_data.nmea_talker[0] = toupper(http_server.arg("nmea_talker").c_str()[0]);
    sensor_data.nmea_talker[1] = toupper(http_server.arg("nmea_talker").c_str()[1]);
    sensor_data.nmea_talker[3] = '\0';
    http_server.send(200, "text/plain", "\r\n");
    commit_eeprom();
}

void handler_push_wind_settings(void)
{
    String value = http_server.arg("filter_angle_enable");
    sensor_data.wind_data.filter_angle_enable = value.toInt();
    value = http_server.arg("filter_speed_enable");
    sensor_data.wind_data.filter_speed_enable = value.toInt();
    value = http_server.arg("angle_length");
    sensor_data.wind_data.angle_ma_length = value.toInt();
    value = http_server.arg("speed_length");
    sensor_data.wind_data.speed_ma_length = value.toInt();
    http_server.send(200, "text/plain", "\r\n");
    commit_eeprom();
    reset_wind_data();
}

void handler_pull_settings(void)
{
    char s[128];
    sprintf(s,
            "ipaddr=%s&slogger=%d&tlogger=%d&"
            "slogger_baudrate=%d&server_port=%d&"
            "hostname=%s&colorize=%d&led=%d&"
            "nmea_talker=%s",
            WiFi.localIP().toString().c_str(),
            sensor_data.status.serial_logger,
            sensor_data.status.telnet_logger,
            sensor_data.status.slogger_baudrate,
            sensor_data.server_port,
            sensor_data.hostname,
            sensor_data.status.colorize_prettyprint,
            sensor_data.status.activity_led,
            sensor_data.nmea_talker);
    http_server.send(200, "text/plain", s);
}

void handler_pull_wind_settings(void)
{
    char s[256];
    sprintf(s,
            "filter_angle_enable=%d&filter_speed_enable=%d"
            "&filter_angle_max=%d&filter_speed_max=%d"
            "&filter_angle_len=%d&filter_speed_len=%d",
            sensor_data.wind_data.filter_angle_enable,
            sensor_data.wind_data.filter_speed_enable,
            WIND_ANGLE_FILTER_TAPS,
            WIND_SPEED_FILTER_TAPS,
            sensor_data.wind_data.angle_ma_length,
            sensor_data.wind_data.speed_ma_length);
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

unsigned int print_error_header(char *p)
{
    // Packet is compromised.
    str_logger[0] = '\0';
    cursor_position = sprintf(str_logger + strlen(str_logger),
                              "[%08d] ",
                              millis());
    append_text_attribute(str_logger, TEXT_ATTRIB_FG_BLUE);
    if (packet_bytes_received == 1)
        append_text_attribute(str_logger, TEXT_ATTRIB_FG_RED);
    cursor_position += sprintf(str_logger + strlen(str_logger),
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
    cursor_position = SEATALK_CMD_COLUMN;
    append_text_attribute(str_logger, TEXT_ATTRIB_FG_RED);
    strcat(str_logger, p);
    cursor_position += strlen(p);
    append_text_attribute(str_logger, TEXT_ATTRIB_NORMAL);
    return cursor_position;
}

void print_framing_error(unsigned int expected)
{
    // Packet is compromised.
    cursor_position = print_error_header("FRAMING ERROR");
    append_blanks(str_logger, SEATALK_ARG_COLUMN, cursor_position);
    sprintf(str_logger + strlen(str_logger), "Expected 0x%02X", expected);
    sendout_strlogger();
}

void print_parity_error(void)
{
    // Packet is compromised.
    print_error_header("PARITY ERROR");
    sendout_strlogger();
}

float ma_wind_angle(unsigned int angle_measurement)
{
    last_wind_angle = 
      (last_wind_angle == sensor_data.wind_data.angle_ma_length - 1) ?
         0 : last_wind_angle + 1;
    wind_angle_sum += angle_measurement - 
                            wind_angle_history[last_wind_angle];
    wind_angle_history[last_wind_angle] = angle_measurement;

    // Simple moving average.
    return 1.0 * wind_angle_sum / sensor_data.wind_data.angle_ma_length;
}

float ma_wind_speed(unsigned int speed_measurement)
{
    last_wind_speed = 
       (last_wind_speed == sensor_data.wind_data.speed_ma_length - 1) ?
        0 : last_wind_speed + 1;
    wind_speed_sum += speed_measurement - 
                            wind_speed_history[last_wind_speed];
    wind_speed_history[last_wind_speed] = speed_measurement;

    // Simple moving average.
    return 1.0 * wind_speed_sum / sensor_data.wind_data.speed_ma_length;
}

void setup()
{
    // This is the serial used to communicate with the SeaTalk bus.
    Serial.begin(SEATALK_DATARATE, SERIAL_8E1);
    while (!Serial);

    //Init EEPROM
    EEPROM.begin(EEPROM_SIZE);

    unsigned char *p = (unsigned char *)&sensor_data;
    unsigned char *q = (unsigned char *)&sensor_data_mem;
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        p[i] = EEPROM.read(i);
        q[i] = p[i];
    }
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
        strncpy((char *)sensor_data.nmea_talker, NMEA_DEFAULT_TALKER, 2);        
        sensor_data.wind_data.filter_angle_enable = 1;
        sensor_data.wind_data.filter_speed_enable = 1;
        sensor_data.wind_data.angle_ma_length = 10;
        sensor_data.wind_data.speed_ma_length = 6;
        sensor_data.magic_number = MAGIC_NUMBER;
        commit_eeprom();
    }

    // This is the serial used for logging messages.
    Serial1.begin(baudrates[sensor_data.status.slogger_baudrate]);
    while (!Serial1);

    Serial.print("\e[2J\r\n");    
    Serial1.print("\e[2J\r\n");    
    dump_eeprom();
    print_eeprom();

    int seconds = 0;

    Serial.print("Connecting to WiFi: ");
    WiFi.mode(WIFI_STA);
    WiFi.hostname((char *)sensor_data.hostname);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        seconds += 1;
        if (seconds == 15)
            break;
        Serial.print(seconds);
        Serial.print(" ");
    }

    Serial.print("\r\n");
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("MAC Address: " + WiFi.macAddress());
        Serial.println("WiFi IP: " + WiFi.localIP().toString());
        Serial.println("Hostname: " + WiFi.hostname());
        // WiFi RF power output ranges from 0dBm to 20.5dBm.
        WiFi.setOutputPower(10);

        // Start the NMEA servers.
        nmea_server_pri.onConnect(nmea_server_pri_connect);
        nmea_server_pri.onReconnect(nmea_server_pri_connect);
        nmea_server_pri.onDisconnect(nmea_server_pri_disconnect);
        Serial.print("Primary NMEA Server Status: ");
        if (nmea_server_pri.begin(sensor_data.server_port))
        {
            print_attribute(TEXT_ATTRIB_FG_GREEN);
            Serial.println("running.");
            print_attribute(TEXT_ATTRIB_NORMAL);
            Serial.print("NMEA Server Port: ");
            Serial.println(sensor_data.server_port);
        }
        else
        {
            print_attribute(TEXT_ATTRIB_FG_RED);
            Serial.println("not connected.");
            print_attribute(TEXT_ATTRIB_NORMAL);
        }

        nmea_server_sec.onConnect(nmea_server_sec_connect);
        nmea_server_sec.onReconnect(nmea_server_sec_connect);
        nmea_server_sec.onDisconnect(nmea_server_sec_disconnect);
        Serial.print("Secondary NMEA Server Status: ");
        if (nmea_server_sec.begin(sensor_data.server_port + 1))
        {
            print_attribute(TEXT_ATTRIB_FG_GREEN);
            Serial.println("running.");
            print_attribute(TEXT_ATTRIB_NORMAL);
            Serial.print("NMEA Server Port: ");
            Serial.println(sensor_data.server_port + 1);
        }
        else
        {
            print_attribute(TEXT_ATTRIB_FG_RED);
            Serial.println("not connected.");
            print_attribute(TEXT_ATTRIB_NORMAL);
        }

        telnet_logger.onConnect(telnet_logger_connect);
        telnet_logger.onReconnect(telnet_logger_connect);
        telnet_logger.onDisconnect(telnet_logger_disconnect);
        Serial.print("Telnet Server Status: ");
        if (telnet_logger.begin(23))
        {
            print_attribute(TEXT_ATTRIB_FG_GREEN);
            Serial.println("running");
        }
        else
        {
            print_attribute(TEXT_ATTRIB_FG_RED);
            Serial.println("not connected.");
        }
        print_attribute(TEXT_ATTRIB_NORMAL);

        // HTTP Server
        http_server.on("/", handleRoot);
        http_server.on("/push_settings", handler_push_settings);
        http_server.on("/push_wind_settings", handler_push_wind_settings);
        http_server.on("/pull_settings", handler_pull_settings);
        http_server.on("/pull_wind_settings", handler_pull_wind_settings);
        http_server.onNotFound(handler_page_not_found);
        http_server.begin();
    }
    else    
        Serial.println("Can't connect to WiFi Network");

    if (sensor_data.status.serial_logger == 1)
        print_slogger_banner();

    pinMode(ACTIVITY_LED, OUTPUT);
    digitalWrite(ACTIVITY_LED, HIGH);
    
    ms_timer = 0;
    reset_wind_data();
}

void loop()
{
    int parity;
    unsigned int payload_byte;
    unsigned int seatalk_cmd;
    WiFiClient cl;

    if (millis() > ms_timer)
        digitalWrite(16, HIGH);
        
    nmea_server_pri.loop();
    nmea_server_sec.loop();
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
                    current_state = SM_STATE_PAYLOAD;
                    break;
                }
                table_item += 1;
            }
            while (seatalk_commands[table_item].cmd != 
                                                SEATALK_UNSUPPORTED);
            if (seatalk_commands[table_item].cmd == SEATALK_UNSUPPORTED)
                current_state = SM_STATE_PRETTYPRINT;
            cmd_payload[0] = seatalk_cmd;
            packet_bytes_received = 1;
        }
        break;
    case SM_STATE_PAYLOAD:
        if (Serial.available() == 0)
            break;
        payload_byte = Serial.read();
        cmd_payload[packet_bytes_received] = payload_byte;
        packet_bytes_received += 1;
        parity = parity_bit(payload_byte);
        if (parity == 1)
        {
            // Payload bytes are never supposed to have the parity
            //  bit set. We'll toss the entire frame.
            print_parity_error();
            current_state = SM_STATE_SEEK_CMD;
        }
        else
        {
            // Perform due diligence on the data received.
            switch (seatalk_commands[table_item].cmd)
            {
            case SEATALK_APPARENT_WIND_ANGLE:
            case SEATALK_APPARENT_WIND_SPEED:
                // First payload byte must be 0x01.
                if (cmd_payload[1] != 0x01)
                {
                    print_framing_error(0x01);
                    current_state = SM_STATE_SEEK_CMD;
                }
                break;
            case SEATALK_LAMP_INTENSITY:
                if (cmd_payload[1] != 0x00)
                {
                    print_framing_error(0x00);
                    current_state = SM_STATE_SEEK_CMD;
                }
                break;
            default:
                current_state = SM_STATE_FORMAT;
                break;
            }
            if ((current_state == SM_STATE_PAYLOAD) &&
                            (packet_bytes_received == 
                                seatalk_commands[table_item].bytes))
                current_state = SM_STATE_FORMAT;
        }
        break;
    case SM_STATE_FORMAT:
        switch (seatalk_commands[table_item].cmd)
        {
        case SEATALK_APPARENT_WIND_ANGLE:
            if (sensor_data.wind_data.filter_angle_enable == 1)
                // The moving average for the speed is done with
                //  values that are 2 times the ones reported by the
                //  instrument. This is done to avoid rounding issues.
                apparent_wind_angle = 0.5 *
                       ma_wind_angle((cmd_payload[2] << 8) +
                                         cmd_payload[3]);
            else                                    
                apparent_wind_angle = 0.5 * ((cmd_payload[2] << 8) +
                                                 cmd_payload[3]);
            current_state = SM_STATE_SEND;
            break;
        case SEATALK_APPARENT_WIND_SPEED:
            if (sensor_data.wind_data.filter_speed_enable == 1)
                // The moving average for the speed is done with
                //  values that are 10 times the ones reported by the
                //  instrument. This is done to avoid rounding issues.
                apparent_wind_speed = 0.1 *
                       ma_wind_speed((cmd_payload[2] & 0x7F) * 10 +
                                     (cmd_payload[3] & 0x0F));
            else
                apparent_wind_speed = 1.0 * (cmd_payload[2] & 0x7F) + 
                                0.1 * (cmd_payload[3] & 0x0F);
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
            if ((nmea_server_pri_connected == true) ||
                (nmea_server_sec_connected == true))
            {
                // Speed is always in knots.
                sprintf(nmea_message,
                        "$%sMWV,%3.1f,R,%2.1f,N,A*",
                        sensor_data.nmea_talker,
                        apparent_wind_angle,
                        apparent_wind_speed);
                sprintf(nmea_message + strlen(nmea_message),
                        "%02X\r\n",
                        nmea_compute_checksum(nmea_message));
            if (nmea_server_pri_connected == true)
                nmea_server_pri.print(nmea_message);
            if (nmea_server_sec_connected == true)
                nmea_server_sec.print(nmea_message);
            }
            break;
        default:
            break;
        }
        current_state = SM_STATE_PRETTYPRINT;
        if (sensor_data.status.activity_led == 1)
        {
            digitalWrite(ACTIVITY_LED, LOW);
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
                print_error_header(seatalk_commands[table_item].cmd_name);
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
