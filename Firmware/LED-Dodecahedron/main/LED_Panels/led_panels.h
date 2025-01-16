#ifndef  __LED_PANELS_H_
#define  __LED_PANELS_H_

#include <stdio.h>
#include <stdint.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_mac.h>

typedef enum {
    EFFECT_INVALID = 0,
    
    EFFECT_NONE,
    EFFECT_CHARGING,           // We should not discharge more than the charge current (or really at all) so have dedicated effect
    EFFECT_WAIT_FOR_CHARGE,    // Battery is too low to perform an arbitrary effect
    EFFECT_SOLID_COLOR,        // Right now this is just solid white
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

#define EFFECT_MAXIMUM_PARAM_LENGTH 16
typedef struct {
    ledEffect_t newEffect;
    uint8_t newBrightness;
    char param[EFFECT_MAXIMUM_PARAM_LENGTH];
} ledUpdate_t;

#define LED_STRIP_GPIO_PIN          3
#define LED_PANEL_PER_PANEL_COUNT   36
#define LED_PANEL_COUNT             12

// For now we will treat all the LEDs as one massive panel
// In future it would be worth doing 12 different panels?
#define LED_EQUIVALENT_HEIGHT   LED_PANEL_PER_PANEL_COUNT
#define LED_EQUIVALENT_WIDTH    LED_PANEL_COUNT

#define LED_MAX_BRIGHTNESS      8 // It likes to crash at higher values, not sure why, the hardware should be able to handle it, could be the step current draw
#define LED_DEFAULT_BRIGHTNESS  5
#define LED_DEFAULT_EFFECT      EFFECT_DNA

#define LED_FPS 60

esp_err_t ledPanelsInit(void);
void switch_effect(ledUpdate_t* newEffect);
uint8_t getCurrentBrightness(void);
ledEffect_t getCurrentEffect(void);

#endif //  __LED_PANELS_H_