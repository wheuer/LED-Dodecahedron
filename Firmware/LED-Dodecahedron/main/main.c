#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include <esp_log.h>
#include "driver/gpio.h"
#include "driver/i2c_master.h"

#include "Power/power.h"
#include "Power/fuel_gauge.h"
#include "Power/power_meter.h"
#include "Temperature/temperature_sensor.h"
#include "LED_Panels/led_panels.h"

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

    systemPowerInit();
    fuelGaugeInit(bus_handle);
    tempSenseInit(bus_handle);
    powerMeterInit(bus_handle);
    ledPanelsInit();

    boostConverterEnable();
  
    // ESP_LOGI(TAG, "BC This should be 0: %u", getBoostConverterState());
    // boostConverterDisable();
    // ESP_LOGI(TAG, "BC This should be 0: %u", getBoostConverterState());
    // boostConverterEnable();
    // ESP_LOGI(TAG, "BC This should be 1: %u", getBoostConverterState());

    // ESP_LOGI(TAG, "USBS This should be 1: %u", isUSBPlugged());

    while (1)
    {   
        ESP_LOGI(TAG, "--------------------");

        // Fuel Gauge
        float cRate = readCRate();
        ESP_LOGI(TAG, "Approx C-Rate: %.2f, ~%.2f mA", cRate, cRate * (2200.0/3600)); 
        ESP_LOGI(TAG, "Battery SOC: %.2f %%", readStateOfCharge());
        ESP_LOGI(TAG, "Raw Battery Voltage: %.2f mV", readRawBatteryVoltage());
        
        // Temp Sensor
        ESP_LOGI(TAG, "Temperature: %.2f C", readTemperature());

        // Power Meter
        ESP_LOGI(TAG, "Bus Voltage: %.2f mV", readBusVoltage());
        ESP_LOGI(TAG, "Shunt Voltage: %.2f mV", readBatteryShuntVoltage());
        ESP_LOGI(TAG, "Battery Current: %.2f mA", readBatteryCurrentDraw() * 1000);

        vTaskDelay(2500 / portTICK_PERIOD_MS);
    }

}