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

#include "seatalk_wifi.h"

void dump_eeprom(void)
{
    Serial.print("\r\n");

    print_attribute(TEXT_ATTRIB_FG_BLUE);
    Serial.print("     0 1 2 3 4 5 6 7 8 9"
                   " A B C D E F\r\n");
    Serial.print("     --------------------------------");
    print_attribute(TEXT_ATTRIB_NORMAL);
    for (int i = 0; i < EEPROM.length(); i += 16)
    {
        char s[16];
        sprintf(s, "\r\n%04X ", i);
        print_attribute(TEXT_ATTRIB_FG_BLUE);
        Serial.print(s);
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

void print_eeprom(void)
{
    Serial.print("EEPROM Contents");
    Serial.print("\r\n-----------------------------------------");
    Serial.print("\r\nSerial Logger: ");
    if (sensor_data.status.serial_logger == 0)
    {
        print_attribute(TEXT_ATTRIB_FG_RED);
        Serial.print("Disabled");
    }
    else    
    {
        print_attribute(TEXT_ATTRIB_FG_GREEN);
        Serial.print("Enabled");
    }
    print_attribute(TEXT_ATTRIB_NORMAL);
    Serial.print("\r\nBaud Rate: ");
    Serial.print(baudrates[sensor_data.status.slogger_baudrate]);
    Serial.print("\r\nTelnet Logger: ");
    if (sensor_data.status.telnet_logger == 0)
    {
        print_attribute(TEXT_ATTRIB_FG_RED);
        Serial.print("Disabled");
    }
    else    
    {
        print_attribute(TEXT_ATTRIB_FG_GREEN);
        Serial.print("Enabled");
    }
    print_attribute(TEXT_ATTRIB_NORMAL);
    Serial.print("\r\nHostname: ");
    Serial.print((char *)sensor_data.hostname);
    Serial.print("\r\nServer Port: ");
    Serial.print(sensor_data.server_port);
    Serial.print("\r\nColorize Prettyprint: ");
    if (sensor_data.status.colorize_prettyprint == 0)
    {
        Serial.print("Disabled");
    }
    else    
    {
        print_attribute(TEXT_ATTRIB_FG_GREEN);
        Serial.print("Enabled");
        print_attribute(TEXT_ATTRIB_NORMAL);
    }
    Serial.print("\r\nActivity LED: ");
    if (sensor_data.status.activity_led == 0)
    {
        Serial.print("Disabled");
    }
    else    
    {
        print_attribute(TEXT_ATTRIB_FG_GREEN);
        Serial.print("Enabled");
        print_attribute(TEXT_ATTRIB_NORMAL);
    }
    Serial.print("\r\n");
}

void commit_eeprom(void)
{
    unsigned char *p = (unsigned char *)&sensor_data;
    for (int i = 0; i < EEPROM_SIZE; i++)
        EEPROM.put(i, p[i]);
    EEPROM.commit();
}

void erase_eeprom(void)
{
    for (int i = 0; i < EEPROM_SIZE; i++)
        EEPROM.put(i, 0);
    EEPROM.commit();
}
