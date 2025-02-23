
#pragma once


#include "hardware/i2c.h"

// Define the EEPROM page size (check the EEPROM's datasheet)
#define EEPROM_PAGE_SIZE 64

#define EEPROM_SDA_PIN 16
#define EEPROM_SCL_PIN 17
#define EEPROM_I2C_BUS i2c0
#define EEPROM_I2C_ADDR 0x50

// The magic string at the top of our binary
#define MAGIC_WORD "HOP!"
#define MAGIC_WORD_SIZE 4

void eeprom_setup_i2c();
void eeprom_read(i2c_inst_t *i2c, uint8_t eeprom_addr, uint16_t mem_addr, uint8_t *data, size_t len);
void read_eeprom_and_configure();
int parse_eeprom_data(const uint8_t *data, size_t len);
int extract_string(const uint8_t *data, size_t len, size_t *offset,
                   char *buffer, size_t bufsize, const char *fieldName);

