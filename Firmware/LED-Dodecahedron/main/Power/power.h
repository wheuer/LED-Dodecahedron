#ifndef __POWER_H_
#define __POWER_H_

#include <stdint.h>
#include <esp_err.h>

#define BOOST_CONVERTER_EN_GPIO     GPIO_NUM_2 // Externally pulled up, active high
#define USB_DETECT_GPIO             GPIO_NUM_0 // Logic level active high

esp_err_t systemPowerInit(void);

esp_err_t boostConverterEnable(void);
esp_err_t boostConverterDisable(void);
int getBoostConverterState(void);

int isUSBPlugged(void);


#endif // __POWER_H_