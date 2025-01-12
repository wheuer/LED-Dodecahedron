#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <led_strip.h>
#include <lib8tion.h>
#include <framebuffer.h>
#include <fbanimation.h>

#include "led_panels.h"
#include <effects/noise.h>
#include <effects/plasma_waves.h>
#include <effects/rainbow.h>
#include <effects/waterfall.h>
#include <effects/dna.h>
#include <effects/rays.h>
#include <effects/crazybees.h>
#include <effects/sparkles.h>
#include <effects/matrix.h>
#include <effects/rain.h>
#include <effects/fire.h>

static const char *TAG = "LEDS";

static led_strip_handle_t activeStrip;
static uint8_t currentBrightness = LED_DEFAULT_BRIGHTNESS;
static ledEffect_t current_effect = EFFECT_NONE;
static fb_draw_cb_t effect_done = NULL;

static led_strip_handle_t configureLeds(void)
{
    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO_PIN, // The GPIO that connected to the LED strip's data line
        .max_leds = LED_PANEL_PER_PANEL_COUNT * LED_PANEL_COUNT,      // The number of LEDs in the strip,
        .led_model = LED_MODEL_WS2812,        // LED strip model
        // set the color order of the strip: GRB
        .color_component_format = {
            .format = {
                .r_pos = 1, // red is the second byte in the color data
                .g_pos = 0, // green is the first byte in the color data
                .b_pos = 2, // blue is the third byte in the color data
                .num_components = 3, // total 3 color components
            },
        },
        .flags = {
            .invert_out = false, // don't invert the output signal
        }
    };

    // LED strip backend configuration: SPI
    led_strip_spi_config_t spi_config = {
        .clk_src = SPI_CLK_SRC_DEFAULT, // different clock source can lead to different power consumption
        .spi_bus = SPI2_HOST,           // SPI bus ID
        .flags = {
            .with_dma = true, // Using DMA can improve performance and help drive more LEDs
        }
    };

    // LED Strip object handle
    led_strip_handle_t led_strip;
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with SPI backend");
    return led_strip;
}

static esp_err_t render_frame(framebuffer_t *fb, void *arg)
{
    for (size_t y = 0; y < fb->height; y++)
        for (size_t x = 0; x < fb->width; x++)
        {
            // calculate strip index of pixel
            size_t strip_idx = y * fb->width + (y % 2 ? fb->width - x - 1 : x);
            // find pixel offset in state frame buffer
            rgb_t color = fb->data[FB_OFFSET(fb, x, y)];
            // limit brightness and consuming current
            color = rgb_scale_video(color, currentBrightness);
            led_strip_set_pixel(activeStrip, strip_idx, color.red, color.green, color.blue);
        }
    return led_strip_refresh(activeStrip);
}

static void switch_effect(fb_animation_t *animation)
{
    // stop rendering
    if (current_effect != EFFECT_NONE)
        fb_animation_stop(animation);

    // finish current effect
    if (effect_done)
        effect_done(animation->fb);

    // clear framebuffer
    fb_clear(animation->fb);

    // pick new effect
    if (++current_effect == EFFECT_MAX)
        current_effect = EFFECT_NONE + 1;

    // init new effect
    fb_draw_cb_t effect_func = NULL;
    switch(current_effect)
    {
        case EFFECT_DNA:
            led_effect_dna_init(animation->fb, random8_between(10, 100), random8_between(1, 10), random8_to(2));
            effect_func = led_effect_dna_run;
            effect_done = led_effect_dna_done;
            break;
        case EFFECT_NOISE:
            led_effect_noise_init(animation->fb, random8_between(10, 100), random8_between(1, 50));
            effect_func = led_effect_noise_run;
            effect_done = led_effect_noise_done;
            break;
        case EFFECT_WATERFALL_FIRE:
            led_effect_waterfall_init(animation->fb, random8_between(2, 4), 0, random8_between(20, 120), random8_between(50, 200));
            effect_func = led_effect_waterfall_run;
            effect_done = led_effect_waterfall_done;
            break;
        case EFFECT_WATERFALL:
            led_effect_waterfall_init(animation->fb, random8_to(2), random8_between(1, 255), random8_between(20, 120), random8_between(50, 200));
            effect_func = led_effect_waterfall_run;
            effect_done = led_effect_waterfall_done;
            break;
        case EFFECT_PLASMA_WAVES:
            led_effect_plasma_waves_init(animation->fb, random8_between(50, 255));
            effect_func = led_effect_plasma_waves_run;
            effect_done = led_effect_plasma_waves_done;
            break;
        case EFFECT_RAINBOW:
            led_effect_rainbow_init(animation->fb, random8_to(3), random8_between(10, 50), random8_between(1, 20));
            effect_func = led_effect_rainbow_run;
            effect_done = led_effect_rainbow_done;
            break;
        case EFFECT_RAYS:
            led_effect_rays_init(animation->fb, random8_between(0, 50), random8_between(3, 5), random8_between(5, 10));
            effect_func = led_effect_rays_run;
            effect_done = led_effect_rays_done;
            break;
        case EFFECT_CRAZYBEES:
            led_effect_crazybees_init(animation->fb, random8_between(2, 5));
            effect_func = led_effect_crazybees_run;
            effect_done = led_effect_crazybees_done;
            break;
        case EFFECT_SPARKLES:
            led_effect_sparkles_init(animation->fb, random8_between(1, 20), random8_between(10, 150));
            effect_func = led_effect_sparkles_run;
            effect_done = led_effect_sparkles_done;
            break;
        case EFFECT_MATRIX:
            led_effect_matrix_init(animation->fb, random8_between(10, 250));
            effect_func = led_effect_matrix_run;
            effect_done = led_effect_matrix_done;
            break;
        case EFFECT_RAIN:
            led_effect_rain_init(animation->fb, random8_to(2), random8(), random8_to(100), random8_between(100, 200));
            effect_func = led_effect_rain_run;
            effect_done = led_effect_rain_done;
            break;
        case EFFECT_FIRE:
            led_effect_fire_init(animation->fb, random8_to(3));
            effect_func = led_effect_fire_run;
            effect_done = led_effect_fire_done;
            break;
        default:
            break;
    }

    // start rendering
    fb_animation_play(animation, LED_FPS, effect_func, NULL);
}

void test(void *pvParameters)
{
    // Perform the Espressif led_strip setup
    activeStrip = configureLeds();
    ESP_LOGI(TAG, "LED strip initialized");

    // Setup framebuffer for effects
    framebuffer_t fb;
    fb_init(&fb, LED_EQUIVALENT_WIDTH, LED_EQUIVALENT_HEIGHT, render_frame);

    // Setup animation
    fb_animation_t animation;
    fb_animation_init(&animation, &fb);

    while (1)
    {
        switch_effect(&animation);
        ESP_LOGI(TAG, "Switching to effect: %d", current_effect);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

esp_err_t ledPanelsInit(void)
{
    // Perform the Espressif led_strip setup
    xTaskCreate(test, "LED test", 8192, NULL, 5, NULL);   
    return ESP_OK;
}