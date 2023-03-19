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
#include <EEPROM.h>
#include "seatalk_wifi.h"

void dump_eeprom(void)
{
    Serial.print("\r\n");

    if (sensor_data.status.colorize_prettyprint == 1)
        print_attribute(TEXT_ATTRIB_FG_BLUE);
    Serial.print("     0 1 2 3 4 5 6 7 8 9"
                   " A B C D E F\r\n");
    Serial.print("     --------------------------------");
    if (sensor_data.status.colorize_prettyprint == 1)
        print_attribute(TEXT_ATTRIB_NORMAL);
    for (int i = 0; i < EEPROM.length(); i += 16)
    {
        char s[16];
        sprintf(s, "\r\n%04X ", i);
        if (sensor_data.status.colorize_prettyprint == 1)
            print_attribute(TEXT_ATTRIB_FG_BLUE);
        Serial.print(s);
        if (sensor_data.status.colorize_prettyprint == 1)
            print_attribute(TEXT_ATTRIB_NORMAL);
        for (int j = 0; j < 16; j += sizeof(uint8_t))
        {
            sprintf(s, "%02X", EEPROM.read(i + j));
            Serial.print(s);
        }
        Serial.print("  ");
        for (int j = 0; j < 16; j++)
        {
            char ch = EEPROM.read(i + j);
            if ((ch >= ' ') && (ch <= '~'))
            {
                sprintf(s, "%c", ch);
                Serial.print(s);
            }
            else
                Serial.print(".");
        }
    }
    Serial.print("\r\n\r\n");
}

void print_status(int st)
{
    if (st == 0)
    {
        if (sensor_data.status.colorize_prettyprint == 1)
            print_attribute(TEXT_ATTRIB_FG_RED);
        Serial.print("Disabled");
    }
    else    
    {
        if (sensor_data.status.colorize_prettyprint == 1)
            print_attribute(TEXT_ATTRIB_FG_GREEN);
        Serial.print("Enabled");
    }
    if (sensor_data.status.colorize_prettyprint == 1)
        print_attribute(TEXT_ATTRIB_NORMAL);
}

void print_eeprom(void)
{
    char str[32];
    Serial.print("\r\n\r\nEEPROM Contents");
    Serial.print("\r\n-----------------------------------------");
    str[0] = '\0';
    float_to_str(str, 0.5 * sensor_data.status.wifi_power, 1);
    Serial.printf("\r\nWifi Power: %s dBm", str);
    Serial.print("\r\nSerial Logger: ");
    print_status(sensor_data.status.serial_logger);
    Serial.printf("\r\nBaud Rate: %d",
                  baudrates[sensor_data.status.slogger_baudrate]);
    Serial.print("\r\nTelnet Logger: ");
    print_status(sensor_data.status.telnet_logger);
    Serial.printf("\r\nHostname: %s", (char *)sensor_data.hostname);
    Serial.printf("\r\nNMEA Talker: %s", (char *)sensor_data.nmea_talker);
    Serial.printf("\r\nNMEA Port: %d", sensor_data.nmea_port);
    Serial.print("\r\nColorize Prettyprint: ");
    print_status(sensor_data.status.colorize_prettyprint);
    Serial.print("\r\nActivity LED: ");
    print_status(sensor_data.status.activity_led);
    Serial.printf("\r\nOTA Programming: ");
    print_status(sensor_data.status.enable_ota);
    Serial.print("\r\n\r\n");
    Serial.print("Wind Instrument Device Specific Settings\r\n");
    Serial.print("Filter Angle: ");
    print_status(sensor_data.wind_data.filter_angle_enable);
    Serial.print("\r\nFilter Speed: ");
    print_status(sensor_data.wind_data.filter_speed_enable);
    Serial.printf("\r\nFilter Angle Length: %d",
                  sensor_data.wind_data.angle_ma_length);
    Serial.printf("\r\nFilter Speed Length: %d",
                  sensor_data.wind_data.speed_ma_length);
    Serial.print("\r\nSignalK JSON: ");
    print_status(sensor_data.status.signalk_udp_enable);
    Serial.printf("\r\nSignalK Json IP: %d.%d.%d.%d",
                  sensor_data.signalk_udp_ip[0],
                  sensor_data.signalk_udp_ip[1],
                  sensor_data.signalk_udp_ip[2],
                  sensor_data.signalk_udp_ip[3]);
    Serial.printf("\r\nSignalK JSON Port: %d", sensor_data.signalk_udp_port);
    Serial.print("\r\n-----------------------------------------");
    Serial.print("\r\n");
    Serial.print("\r\n");
}

void commit_eeprom(void)
{
    unsigned char *p = (unsigned char *)&sensor_data;
    unsigned char *q = (unsigned char *)&sensor_data_mem;
    for (int i = 0; i < EEPROM_SIZE; i++)
        if (p[i] != q[i])
        {
            EEPROM.put(i, p[i]);
            q[i] = p[i];
        }        
    EEPROM.commit();
    print_eeprom();
}

void erase_eeprom(void)
{
    for (int i = 0; i < EEPROM_SIZE; i++)
        EEPROM.put(i, 0);
    EEPROM.commit();
}
