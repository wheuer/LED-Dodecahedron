#include <stdio.h>

#include "driver/gpio.h"

#include "power.h"

static gpio_config_t boostOnPinConfig = {
    .pin_bit_mask = (1 << BOOST_CONVERTER_EN_GPIO),
    .mode = GPIO_MODE_INPUT_OUTPUT_OD,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

static gpio_config_t usbDetectConfig = {
    .pin_bit_mask = (1 << USB_DETECT_GPIO),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_ANYEDGE
};

esp_err_t boostConverterEnable(void)
{
    return gpio_set_level(BOOST_CONVERTER_EN_GPIO, 1);
}

esp_err_t boostConverterDisable(void)
{
    return gpio_set_level(BOOST_CONVERTER_EN_GPIO, 0);
}

int getBoostConverterState(void)
{
    return gpio_get_level(BOOST_CONVERTER_EN_GPIO);
}

int isUSBPlugged(void)
{
    return gpio_get_level(USB_DETECT_GPIO);
}

static void usbDetectHandler(void* arg)
{
    // This intterupt is set to trigger on either a falling or rising edge
    // In either case, read the value of the pin and alert the main system loop
}

esp_err_t systemPowerInit(void)
{
    // Configure the USB Detect and Boot EN Pins
    esp_err_t err = gpio_config(&boostOnPinConfig);
    err += gpio_config(&usbDetectConfig);

    // Enable the GPIO interrupt for the USB sense pin
    // err += gpio_install_isr_service(NULL);
    // err += gpio_isr_handler_add(USB_DETECT_GPIO, &usbDetectHandler, NULL);

    // IMPORTANT: After initilization the default state of the GPIO will be low -> i.e. The boost converter will be disabled

    return err;
}