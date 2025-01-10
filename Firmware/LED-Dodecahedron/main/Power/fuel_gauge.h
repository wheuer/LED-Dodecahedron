#ifndef __FUEL_GAUGE_H_
#define __FUEL_GAUGE_H_

#include <stdint.h>
#include <esp_err.h>
#include <esp_log.h>
#include "driver/i2c_master.h"

#define POWER_METER_I2C_ADDR    0x36
#define FUEL_GAUGE_I2C_TIMEOUT  100 // ms

// Data Registers
#define REGISTER_VCELL_ADDR     0x02
#define REGISTER_SOC_ADDR       0x04
#define REGISTER_VERSION_ADDR   0x08
#define REGISTER_CRATE_ADDR     0x16

// Chip Config Registers
#define REGISTER_MODE_ADDR      0x06
#define REGISTER_HIBRT_ADDR     0x0A
#define REGISTER_CONFIG_ADDR    0x0C
#define REGISTER_VALRT_ADDR     0x14
#define REGISTER_VRESET_ADDR    0x18
#define REGISTER_ID_ADDR        0x18
#define REGISTER_STATUS_ADDR    0x1A
#define REGISTER_CMD_ADDR       0xFE

// Battery Parameter Configuration Registers
#define REGISTER_TABLE_START_ADDR 0x40
#define REGISTER_TABLE_END_ADDR 0x7F

typedef struct FuelStatus {
    uint8_t resetFlag;              // This bit is set when the device powers up
    uint8_t voltageHighWaterFlag;   // VCELL > ALRT.VALRTMAX
    uint8_t voltageLowWaterFlag;    // VCELL < ALRT.VALRTMIN
    uint8_t voltageResetFlag;       // Set after device reset if EnVr is set
    uint8_t hdLowFlag;              // SOC crosses the value in CONFIG.ATHD
    uint8_t socChangeFlag;          // SOC changed by at least 1% if CONFIG.ALSC is set
} FuelStatus;

esp_err_t fuelGaugeInit(i2c_master_bus_handle_t busHandle);
float readStateOfCharge(void);
float readRawBatteryVoltage(void);
float readCRate(void);
void updateBatteryTemperature(float temperature);

#endif // __FUEL_GAUGE_H_