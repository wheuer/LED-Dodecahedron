#include <stdio.h>

#include "fuel_gauge.h"

const static char* TAG = "[POWER-FUEL]";

static i2c_master_bus_handle_t activeBusHandle;
static i2c_master_dev_handle_t fuelDevHandle;

static i2c_device_config_t fuelDevCfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = FUEL_GAUGE_I2C_ADDR,
    .scl_speed_hz = 100000,
};

// Updated every 250 ms (as an average of 4 readings) in active mode and 45s in hibernation
// Directly measures between VCELL and GND so it could be skewed by the system load
// Returned in mV
float readRawBatteryVoltage(void)
{
    uint8_t registerAddress = REGISTER_VCELL_ADDR;
    uint8_t response[2];

    // Read the battery charge
    esp_err_t err = i2c_master_transmit_receive(fuelDevHandle, &registerAddress, 1, response, 2, FUEL_GAUGE_I2C_TIMEOUT);
    if (err == ESP_OK) 
    {
        // Returned as two bytes with a 78.125 uV LSB
        //printf("Voltage Got bytes: %x %x\n", response[0], response[1]);
        return ((uint16_t) ((response[0] << 8) | response[1])) * (0.078125); // mV
    }
    else 
    {
        ESP_LOGE(TAG, "Read State of Charge FAIL");
        return -1;
    }
}

// Only avaliable (otherwise garbage POR value) 1 second after power on reset
// Since the IC is powered by the battery it is likely that we will only every be reading valid results
float readStateOfCharge(void)
{
    uint8_t registerAddress = REGISTER_SOC_ADDR;
    uint8_t response[2];

    // Read the battery charge
    esp_err_t err = i2c_master_transmit_receive(fuelDevHandle, &registerAddress, 1, response, 2, FUEL_GAUGE_I2C_TIMEOUT);
    if (err == ESP_OK) 
    {
        // Returned as two bytes with a 1%/256 LSB (upper byte is rounded down whole number percentage)
        //printf("SOC Got bytes: %x %x\n", response[0], response[1]);
        return ((uint16_t) ((response[0] << 8) | response[1])) / 256.0;
    }
    else 
    {
        ESP_LOGE(TAG, "Read State of Charge FAIL");
        return -1;
    }
}

// Note that although the C-Rate should indicate a current draw value, the datasheet says it shouldn't be converted to amperes
// Returned value is in percent/hour
// Since the battery has a nominal capacity of 2200mAh the theoretical conversion to ampere (mA) would be 2200/3600  
float readCRate(void)
{
    uint8_t registerAddress = REGISTER_CRATE_ADDR;
    uint8_t response[2];

    // Read the battery charge
    esp_err_t err = i2c_master_transmit_receive(fuelDevHandle, &registerAddress, 1, response, 2, FUEL_GAUGE_I2C_TIMEOUT);
    if (err == ESP_OK) 
    {
        // Returned as two bytes with a 0.208%/hour LSB
        //printf("SOC Got bytes: %x %x\n", response[0], response[1]);
        return ((uint16_t) ((response[0] << 8) | response[1])) * 0.208;
    }
    else 
    {
        ESP_LOGE(TAG, "Read C-Rate FAIL");
        return -1;
    }
}

// Recommended to update at least every minute for optimal performance
// Changed by setting the CONFIG.RCOMP register, 0x97 is the reset value and is equivalent to 20C = 68F
void updateBatteryTemperature(float temperature)
{
    
}

esp_err_t fuelGaugeInit(i2c_master_bus_handle_t busHandle)
{
    activeBusHandle = busHandle;

    esp_err_t err = i2c_master_bus_add_device(busHandle, &fuelDevCfg, &fuelDevHandle);
    if (err != ESP_OK) ESP_LOGE(TAG, "i2c_master_bus_add_device FAIL");

    return ESP_OK;
}