#include "effects/solid_color.h"

#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)

static uint8_t updatedFlag = false;

esp_err_t led_effect_solid_color_init(framebuffer_t *fb, solidColorParam_t* color)
{
    CHECK_ARG(fb && color);

    // Allocate internal storage for color
    fb->internal = calloc(1, sizeof(solidColorParam_t));
    if (!fb->internal)
        return ESP_ERR_NO_MEM;

    return led_effect_solid_color_set_params(fb, color);
}

esp_err_t led_effect_solid_color_done(framebuffer_t *fb)
{
    CHECK_ARG(fb && fb->internal);

    // free internal storage
    if (fb->internal)
        free(fb->internal);

    updatedFlag = false;

    return ESP_OK;
}

esp_err_t led_effect_solid_color_set_params(framebuffer_t *fb, solidColorParam_t* color)
{
    CHECK_ARG(fb && fb->internal && color);

    solidColorParam_t* params = (solidColorParam_t*) fb->internal;
    params->newColor = color->newColor;
    updatedFlag = false;

    // Manually set the RGB values so the original caller can just use the color enum 
    if (color->newColor != SOLID_COLOR_CUSTOM)
    {
        switch(color->newColor)
        {
            case SOLID_COLOR_RED:
                params->color = rgb_from_values(0xFF, 0x00, 0x00);
                break;
            case SOLID_COLOR_GREEN:
                params->color = rgb_from_values(0x00, 0xFF, 0x00);
                break;
            case SOLID_COLOR_BLUE:
                params->color = rgb_from_values(0x00, 0x00, 0xFF);
                break;
            case SOLID_COLOR_WHITE:
                params->color = rgb_from_values(0xFF, 0xFF, 0xFF);
                break;
            default:
                params->color = rgb_from_values(0x00, 0x00, 0x00);
                break;
        }
    }
    else
    {
        params->color = color->color;
    }

    return ESP_OK;
}

esp_err_t led_effect_solid_color_run(framebuffer_t *fb)
{
    // We just want to set the whole framebuffer to the single color and we only need to set it once
    solidColorParam_t* params = (solidColorParam_t*) fb->internal;
    if (!updatedFlag)
    {
        for (size_t x = 0; x < fb->width; x++)
            for (size_t y = 0; y < fb->height; y++)
                fb_set_pixel_rgb(fb, x, y, params->color);
        updatedFlag = true;
    }

    return ESP_OK;
}