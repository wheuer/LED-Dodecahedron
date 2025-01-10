#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include <esp_log.h>
#include "driver/gpio.h"
#include "driver/i2c_master.h"

#include "Power/fuel_gauge.h"
#include "Temperature/temperature_sensor.h"

const static char* TAG = "[MAIN]";

i2c_master_bus_config_t i2c_mst_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = -1, // -1 for auto select, esp32c3 should only have one I2C port
    .scl_io_num = GPIO_NUM_18,
    .sda_io_num = GPIO_NUM_19,
    .glitch_ignore_cnt = 7, // 7 is the typical value
    .flags.enable_internal_pullup = false
};

static i2c_master_bus_handle_t bus_handle;

void app_main(void)
{
    printf("APP START\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Give some time for the fuel guage and power meter to power up

    esp_err_t err = i2c_new_master_bus(&i2c_mst_config, &bus_handle);
    if (err != ESP_OK) ESP_LOGE(TAG, "I2C Bus Creation: %u\n", err);

    fuelGaugeInit(bus_handle);
    tempSenseInit(bus_handle);
    
    while (1)
    {   
        ESP_LOGI(TAG, "Battery SOC: %.2f %%", readStateOfCharge());
        ESP_LOGI(TAG, "Raw Battery Voltage: %.2f mV", readRawBatteryVoltage());
        ESP_LOGI(TAG, "Temperature: %.2f C", readTemperature());
        vTaskDelay(2500 / portTICK_PERIOD_MS);
    }

}