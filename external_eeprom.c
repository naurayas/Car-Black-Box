
#include <xc.h>
#include "external_eeprom.h"
#include "i2c.h"

void write_external_eeprom(unsigned char address, unsigned char data)
{
    i2c_start();
    i2c_write(SLAVE_WRITE_E);
    i2c_write(address);
    i2c_write(data);
    i2c_stop();
    for (unsigned int wait = 3000;wait--;);
}

unsigned char read_external_eeprom(unsigned char address)
{
    unsigned char data;

    i2c_start();
    i2c_write(SLAVE_WRITE_E);
    i2c_write(address);
    i2c_rep_start();
    i2c_write(SLAVE_READ_E);
    data = i2c_read(1);
    i2c_stop();

    return data;
}

void str_write_ext_eeprom(unsigned char addr, char *data)
{
    while (*data != 0)
    {
        write_external_eeprom(addr, *data);
        data++;
        addr++;
    }
}

