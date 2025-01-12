#ifndef  __LED_PANELS_H_
#define  __LED_PANELS_H_

#include <stdio.h>
#include <stdint.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_mac.h>

#define LED_STRIP_GPIO_PIN          3
#define LED_PANEL_PER_PANEL_COUNT   36
#define LED_PANEL_COUNT             1  //12

// For now we will treat all the LEDs as one massive panel
// In future it would be worth doing 12 different panels?
#define LED_EQUIVALENT_HEIGHT   LED_PANEL_PER_PANEL_COUNT
#define LED_EQUIVALENT_WIDTH    LED_PANEL_COUNT

#define LED_MAX_BRIGHTNESS      5 // TBD, 5 should be fine for now
#define LED_DEFAULT_BRIGHTNESS  5

#define LED_FPS 60

typedef enum {
    EFFECT_NONE = 0,

    EFFECT_DNA,
    EFFECT_NOISE,
    EFFECT_WATERFALL_FIRE,
    EFFECT_WATERFALL,
    EFFECT_PLASMA_WAVES,
    EFFECT_RAINBOW,
    EFFECT_RAYS,
    EFFECT_CRAZYBEES,
    EFFECT_SPARKLES,
    EFFECT_MATRIX,
    EFFECT_RAIN,
    EFFECT_FIRE,

    EFFECT_MAX
} ledEffect_t;

esp_err_t ledPanelsInit(void);

#endif //  __LED_PANELS_H_