#ifndef  __SOLID_COLOR_H_
#define  __SOLID_COLOR_H_

#include <framebuffer.h>
#include <rgb.h>

typedef enum {
    SOLID_COLOR_CUSTOM = 0,
    SOLID_COLOR_RED,
    SOLID_COLOR_GREEN,
    SOLID_COLOR_BLUE,
    SOLID_COLOR_WHITE,

    SOLID_COLOR_MAX
} solidColor_t;

typedef struct {
    solidColor_t newColor;
    rgb_t color;
} solidColorParam_t;

esp_err_t led_effect_solid_color_init(framebuffer_t *fb, solidColorParam_t* color);

esp_err_t led_effect_solid_color_done(framebuffer_t *fb);

esp_err_t led_effect_solid_color_set_params(framebuffer_t *fb, solidColorParam_t* color);

esp_err_t led_effect_solid_color_run(framebuffer_t *fb);

#endif // __SOLID_COLOR_H_