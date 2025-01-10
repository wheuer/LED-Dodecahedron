#ifndef __TEMP_SENSE_H_
#define __TEMP_SENSE_H_

#include <stdint.h>
#include <esp_err.h>
#include <esp_log.h>
#include "driver/i2c_master.h"

#define TEMP_SENSE_I2C_ADDR     0x48
#define TEMP_SENSE_I2C_TIMEOUT  100 // ms

// Data Registers
#define REGISTER_TEMP_ADDR      0x00

// Chip Config Registers
#define REGISTER_CFGR_ADDR          0x01
#define REGISTER_LOW_LIMIT_ADDR     0x02
#define REGISTER_HIGH_LIMIT_ADDR    0x03

esp_err_t tempSenseInit(i2c_master_bus_handle_t busHandle);
float readTemperature(void);

#endif // __TEMP_SENSE_H_