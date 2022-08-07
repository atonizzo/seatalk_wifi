void dump_eeprom(void)
{
    // Round up the length to the next 4 words.
    Serial1.println("         0  1  2  3  4  5  6  7  8  9"
                   "  A  B  C  D  E  F");
    Serial1.print("         ----------------------------"
                   "-------------------");
    for (int i = 0; i < EEPROM.length(); i += 16)
    {
        char s[16];
        sprintf(s, "\r\n%04X ", i);
        Serial1.print(s);
        for (int j = 0; j < 16; j += sizeof(uint8_t))
        {
            sprintf(s, "%02X", EEPROM.read(i + j));
            Serial1.print(s);
        }
        Serial1.print("  ");
        for (int j = 0; j < 16; j++)
        {
            char ch = EEPROM.read(i + j);
            if ((ch >= ' ') && (ch <= '~'))
            {
                sprintf(s, "%c", ch);
                Serial1.print(s);
            }
            else
                Serial1.print(".");
        }
    }
    Serial1.print("\r\n\r\n");
}

void print_eeprom(void)
{
    Serial1.println("EEPROM Contents");
    Serial1.println("-----------------------------------------");
    Serial1.print("Serial Logger: ");
    if (sensor_data.status.serial_logger == 0)
        Serial1.println("Disabled");
    else    
        Serial1.println("Enabled");
    Serial1.print("Telnet Logger: ");
    if (sensor_data.status.telnet_logger == 0)
        Serial1.println("Disabled");
    else    
        Serial1.println("Enabled");
    Serial1.print("Serial Logger Baud Rate: ");
    Serial1.println(baudrates[sensor_data.status.slogger_baudrate]);
    Serial1.print("Server Port: ");
    Serial1.println(sensor_data.server_port);
    Serial1.print("\r\n");
}

void commit_eeprom(void)
{
    unsigned char *p = (unsigned char *)&sensor_data;
    for (int i = 0; i < EEPROM_SIZE; i++)
        EEPROM.put(i, p[i]);
    EEPROM.commit();
    print_eeprom();
}
