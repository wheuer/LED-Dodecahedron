#ifndef __POWER_METER_H_
#define __POWER_METER_H_

#include <stdint.h>
#include <esp_err.h>
#include <esp_log.h>
#include "driver/i2c_master.h"

#define POWER_METER_I2C_ADDR     0x40
#define POWER_METER_I2C_TIMEOUT  100 // ms

#define POWER_METER_SHUNT_RESISTANCE        (20/1000)
#define POWER_METER_MINIMUM_CURRENT_LSB     0.00016 // Based on a theoretical maximum current of ~5A, this is overkill for the battery we are using but shouldn't hurt resolution that much
#define POWER_METER_CALIBRATION_VALUE       1600 // (0.00512 / (POWER_METER_SHUNT_RESISTANCE * POWER_METER_MINIMUM_CURRENT_LSB))

#define REGISTER_PM_CONFIG_ADDR         0x00
#define REGISTER_SHUNT_VOLTAGE_ADDR     0x01 // Voltage across shunt resistor
#define REGISTER_BUS_VOLTAGE_ADDR       0x02 // Voltage on the V- side to ground (i.e. the voltage after the shunt resistor)
#define REGISTER_POWER_ADDR             0x03
#define REGISTER_CURRENT_ADDR           0x04
#define REGISTER_CALIBRATION_ADDR       0x05 // This is where we need to set the shunt resistance
#define REGISTER_MASK_ADDR              0x06
#define REGISTER_ALERT_LIMIT_ADDR       0x07
#define REGISTER_MANUFACTURER_ID_ADDR   0x3E
#define REGISTER_DEVICE_ID_ADDR         0x3F

esp_err_t powerMeterInit(i2c_master_bus_handle_t busHandle);
float readBusVoltage(void);
float readBatteryShuntVoltage(void);
float readBatteryCurrentDraw(void);

#endif // __POWER_METER_H_