#include <stdio.h>

#include "power_meter.h"

const static char* TAG = "[POWER-METER]";

static i2c_master_bus_handle_t activeBusHandle;
static i2c_master_dev_handle_t meterDevHandle;

static i2c_device_config_t fuelDevCfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = POWER_METER_I2C_ADDR,
    .scl_speed_hz = 100000,
};

// Returned in mV
float readBusVoltage(void)
{
    uint8_t registerAddress = REGISTER_BUS_VOLTAGE_ADDR;
    uint8_t response[2];

    // Read the battery charge
    esp_err_t err = i2c_master_transmit_receive(meterDevHandle, &registerAddress, 1, response, 2, POWER_METER_I2C_TIMEOUT);
    if (err == ESP_OK) 
    {
        // Returned as two bytes in twos complement format (MSB is always zero as the bus voltage is always positive, at least in theory...)
        // The returned unit is in terms of a modified LSB of 1.6 mV
        int16_t convertedResponse = (response[0] << 8) | response[1];
        // printf("BUS VOLTAGE Got bytes: %x %x\n", response[0], response[1]);
        return convertedResponse * 1.6; // mV
    }
    else 
    {
        ESP_LOGE(TAG, "Read Bus Voltage FAIL");
        return -1;
    }
}

// Returned in mV
float readBatteryShuntVoltage(void)
{
    uint8_t registerAddress = REGISTER_SHUNT_VOLTAGE_ADDR;
    uint8_t response[2];

    // Read the battery charge
    esp_err_t err = i2c_master_transmit_receive(meterDevHandle, &registerAddress, 1, response, 2, POWER_METER_I2C_TIMEOUT);
    if (err == ESP_OK) 
    {
        // Returned as two bytes in twos complement format
        // The returned unit is in terms of the IC's ADC result which as configured has a resolution of 2.5uV
        int16_t convertedResponse = (response[0] << 8) | response[1];
        // printf("SHUNT VOLTAGE Got bytes: %x %x\n", response[0], response[1]);
        return convertedResponse * 0.0025; // mV
    }
    else 
    {
        ESP_LOGE(TAG, "Read Shunt Voltage FAIL");
        return -1;
    }
}

// Returned in A
float readBatteryCurrentDraw(void)
{
    uint8_t registerAddress = REGISTER_CURRENT_ADDR;
    uint8_t response[2];

    // Read the battery charge
    esp_err_t err = i2c_master_transmit_receive(meterDevHandle, &registerAddress, 1, response, 2, POWER_METER_I2C_TIMEOUT);
    if (err == ESP_OK) 
    {
        // Returned as two bytes in twos complement format
        // The LSB is based on what we selected for the CURRENT_LSB in the calibration value calculation
        int16_t convertedResponse = (response[0] << 8) | response[1];
        return (float) (convertedResponse * POWER_METER_MINIMUM_CURRENT_LSB);
    }
    else 
    {
        ESP_LOGE(TAG, "Read Current Draw FAIL");
        return -1;
    }
}

esp_err_t powerMeterInit(i2c_master_bus_handle_t busHandle)
{
    activeBusHandle = busHandle;

    esp_err_t err = i2c_master_bus_add_device(busHandle, &fuelDevCfg, &meterDevHandle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "i2c_master_bus_add_device FAIL");
        return ESP_FAIL;
    }

    // In terms of configuring the IC itself, we only need to set the calibration value for our shunt resistor
    // The configuration register will be left in it's default state meaning:
    //      +/- 81.92 mV ADC Range (required)
    //      1 ADC Reading per result (could increase to filter noisy readings)
    //      1100 us VBUS ADC Conversion Time
    //      1100 us Shunt Voltage ADC Conversion Time
    //      Continuous conversions for both shunt and bus voltages

    // Set the calibration register
    uint8_t registerAddress = REGISTER_CALIBRATION_ADDR;
    uint16_t calibrationValue = POWER_METER_CALIBRATION_VALUE;
    uint8_t message[3] = {registerAddress, calibrationValue >> 8, calibrationValue & 0xFF};
    err = i2c_master_transmit(meterDevHandle, message, 3, POWER_METER_I2C_TIMEOUT);

    // Read the set value back to make sure it was set
    uint8_t response[2];
    err = i2c_master_transmit_receive(meterDevHandle, &registerAddress, 1, response, 2, POWER_METER_I2C_TIMEOUT);
    if (response[0] != (calibrationValue >> 8) || response[1] != (calibrationValue & 0xFF) || err != ESP_OK)
    {
        printf("SHUNT VOLTAGE Got bytes: %x %x\n", response[0], response[1]);
        ESP_LOGE(TAG, "Failed to set calibration register; Expected %x %x Got %x %x", message[1], message[2], response[0], response[1]);
        return ESP_FAIL;
    }

    return ESP_OK;
}