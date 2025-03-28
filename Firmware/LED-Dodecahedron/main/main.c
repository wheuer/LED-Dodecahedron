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
#include "effects/solid_color.h"
#include <rgb.h>
#include "Network/network.h"
#include "Network/http_server.h"

#define MONITORING_TASK_STACK_SIZE  4096
#define MONITORING_TASK_PRIORITY    10      // WiFi manager is priority 5, not sure about WiFi drivers (likely interrupt driven?)

#define BATTERY_CHECK_INTERVAL              (3000 / portTICK_PERIOD_MS) // ms
#define BATTERY_MINIMUM_PERCENTAGE_FALLING  25
#define BATTERY_MINIMUM_PERCENTAGE_RISING   40
#define BATTERY_CURRENT_DRAW_MAXIMUM        4 // A little under 2C discharge

#define LED_DEFAULT_EFFECT EFFECT_NOISE

typedef enum {
    NORMAL_OPERATION = 0,
    WAITING_FOR_CHARGE,
    CHARGING,
    SYSTEM_ERROR,
    STATE_MAX
} systemState;

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
static TaskHandle_t monitorTaskHandle;

static systemState systemStatus = NORMAL_OPERATION;

static void monitorTask(void* param)
{
    // Turn on the LEDs and start a default effect
    ledUpdate_t incomingEffect;
    incomingEffect.newBrightness = LED_DEFAULT_BRIGHTNESS;
    incomingEffect.newEffect = LED_DEFAULT_EFFECT;
    boostConverterEnable();
    switch_effect(&incomingEffect);

    while (1)
    {
        // The things we care about are incoming requests to change the effect and the state of the battery
        // If there is a new effect request we will handle it
        // If the battery is too low, we will move into a low power state waiting to charge
        // In either case these events are not time sensitive so just wait on the http server's effect queue with a timeout

        // Start every cycle by checking the state of the battery and moving to the proper state
        float batStateOfCharge = readStateOfCharge(); 
        float batCurrentDraw = readBatteryCurrentDraw();
        uint8_t chargingStatus = isUSBPlugged();

        // Check for overcurrent
        if (batCurrentDraw > BATTERY_CURRENT_DRAW_MAXIMUM)
        {
            incomingEffect.newBrightness = 0;
            incomingEffect.newEffect = EFFECT_NONE;
            switch_effect(&incomingEffect);
            boostConverterDisable();
            systemStatus = SYSTEM_ERROR;
        }

        switch (systemStatus)
        {
            case NORMAL_OPERATION:
                ESP_LOGI(TAG, "NORMAL OPERATION");
                if (batStateOfCharge < BATTERY_MINIMUM_PERCENTAGE_FALLING)
                {
                    incomingEffect.newBrightness = 0;
                    incomingEffect.newEffect = EFFECT_NONE;
                    switch_effect(&incomingEffect);
                    boostConverterDisable();
                    systemStatus = WAITING_FOR_CHARGE;
                }
                else if (chargingStatus)
                {
                    incomingEffect.newBrightness = LED_DEFAULT_BRIGHTNESS;
                    incomingEffect.newEffect = EFFECT_CHARGING;
                    switch_effect(&incomingEffect);
                    boostConverterDisable();
                    systemStatus = CHARGING;
                }
                break;
            case WAITING_FOR_CHARGE:
                ESP_LOGI(TAG, "WAITING FOR CHARGE");
                if (chargingStatus)
                {
                    incomingEffect.newBrightness = LED_DEFAULT_BRIGHTNESS;
                    incomingEffect.newEffect = EFFECT_CHARGING;
                    switch_effect(&incomingEffect);
                    systemStatus = CHARGING;
                }
                else if (batStateOfCharge > BATTERY_MINIMUM_PERCENTAGE_RISING)
                {
                    incomingEffect.newBrightness = LED_DEFAULT_BRIGHTNESS;
                    incomingEffect.newEffect = LED_DEFAULT_EFFECT;
                    switch_effect(&incomingEffect);
                    boostConverterEnable();
                    systemStatus = NORMAL_OPERATION;
                }
                break;
            case CHARGING:
                ESP_LOGI(TAG, "CHARGING");
                if (!chargingStatus)
                {
                    if (batStateOfCharge > BATTERY_MINIMUM_PERCENTAGE_RISING)
                    {
                        incomingEffect.newBrightness = LED_DEFAULT_BRIGHTNESS;
                        incomingEffect.newEffect = LED_DEFAULT_EFFECT;
                        switch_effect(&incomingEffect);
                        boostConverterEnable();
                        systemStatus = NORMAL_OPERATION;
                    }
                    else // Not charged enough
                    {
                        incomingEffect.newBrightness = 0;
                        incomingEffect.newEffect = EFFECT_NONE;
                        switch_effect(&incomingEffect);
                        systemStatus = WAITING_FOR_CHARGE;
                    }
                }
                break;
            case SYSTEM_ERROR:
                ESP_LOGE(TAG, "SYSTEM ERROR: LIKELY BATTERY OVERCURRENT");
                break;
            default:
                break;
        }

        // If we are in normal operation, wait on new effect request, otherwise just wait until next cycle
        if (systemStatus == NORMAL_OPERATION)
        {
            if (xQueueReceive(newEffectQueueHandle, &incomingEffect, BATTERY_CHECK_INTERVAL))
            {
                if (incomingEffect.newEffect != EFFECT_INVALID)
                {
                    switch_effect(&incomingEffect);
                }
            }
        }
        else
        {
            vTaskDelay(BATTERY_CHECK_INTERVAL);
        }

    }
}

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

    // The call to launch the network will block until we are connected so do it after all init is complete
    networkLaunch();

    // Create main monitoring/update task
    xTaskCreate(monitorTask, "MAIN-MONITOR", MONITORING_TASK_STACK_SIZE, NULL, MONITORING_TASK_PRIORITY, &monitorTaskHandle);

}
