#include <stdio.h>

#include "temperature_sensor.h"

const static char* TAG = "[TEMP]";

static i2c_master_bus_handle_t activeBusHandle;
static i2c_master_dev_handle_t tempDevHandle;

static i2c_device_config_t tempDevCfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = TEMP_SENSE_I2C_ADDR,
    .scl_speed_hz = 100000,
};

float readTemperature(void)
{
    uint8_t registerAddress = REGISTER_TEMP_ADDR;
    uint8_t response[2];

    // Read the battery charge
    esp_err_t err = i2c_master_transmit_receive(tempDevHandle, &registerAddress, 1, response, 2, TEMP_SENSE_I2C_TIMEOUT);
    if (err == ESP_OK) 
    {
        // Returned as a 12-bit twos complement number with the four LSBs being garbage (zeros but not apart of temp)
        // LSB is 0.0625 degrees C
        int16_t rawResult = ((response[0] << 8) | response[1]) >> 4;
        
        // Here we will need to sign extend the 12 bit to a 16 bit
        if (response[0] & 0x80)
        {
            // Was negative so we need to sign extend with a one
            rawResult |= 0xF000;
        }
        
        return rawResult * 0.0625;
    }
    else 
    {
        ESP_LOGE(TAG, "Read temperature FAIL");
        return -1;
    }
}

esp_err_t tempSenseInit(i2c_master_bus_handle_t busHandle)
{
    activeBusHandle = busHandle;
    
    esp_err_t err = i2c_master_bus_add_device(busHandle, &tempDevCfg, &tempDevHandle);
    if (err != ESP_OK) ESP_LOGE(TAG, "i2c_master_bus_add_device FAIL");

    // No initilization is neccesary, the temp sensor will power on in continous conversion mode

    return ESP_OK;
}