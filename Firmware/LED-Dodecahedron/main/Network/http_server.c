#include <esp_http_server.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "http_app.h"
#include "http_server.h"
#include "LED_Panels/led_panels.h"
#include "Power/fuel_gauge.h"
#include "Power/power.h"
#include "Power/power_meter.h"
#include "Temperature/temperature_sensor.h"

static const char* TAG = "[NETWORK-HTTP_SERVER]";

QueueHandle_t newEffectQueueHandle;
static uint8_t newEffectQueueBuffer[sizeof(ledUpdate_t) * EFFECT_QUEUE_MAX_ITEMS];
static StaticQueue_t newEffectQueue;
static ledUpdate_t newEffectMessage;

extern const uint8_t app_index_html_start[] asm("_binary_app_index_html_start");
extern const uint8_t app_index_html_end[] asm("_binary_app_index_html_end");

esp_err_t appGETHandler(httpd_req_t* request)
{
    char buffer[64];
    if(strcmp(request->uri, "/") == 0)
    {
        httpd_resp_set_status(request, http_200_hdr);
        httpd_resp_set_type(request, http_content_type_html);
        httpd_resp_send(request, (char*)app_index_html_start, app_index_html_end - app_index_html_start);
    }
    else if (strcmp(request->uri, "/check-in") == 0) 
    {
        // Send back seven things:
        //      - Active effect
        //      - Active brightness
        //      - Raw battery voltage
        //      - Battery state of charge
        //      - Board temperature
        //      - Battery current
        //      - Charging status   
        ledEffect_t currentEffect = getCurrentEffect(); 
        uint8_t currentBrightness = getCurrentBrightness();
        float batteryVoltage = readRawBatteryVoltage() / 1000.0;
        float stateOfCharge = readStateOfCharge();
        float boardTemp = readTemperature();
        int batteryCurrent = readBatteryCurrentDraw() * 1000.0; // Returned in amps, convert to mA to send
        int chargeStatus = isUSBPlugged();

        snprintf(buffer, 64, "effect=%i&bv=%.2f&bl=%i&cs=%i&bt=%.2f&bc=%.2f&bcu=%i", currentEffect, batteryVoltage, currentBrightness, 
                                                                                    chargeStatus, boardTemp, stateOfCharge, batteryCurrent);

        httpd_resp_set_status(request, http_200_hdr);
        httpd_resp_set_type(request, http_content_type_text);
        httpd_resp_send(request, buffer, HTTPD_RESP_USE_STRLEN);
    }
    else
    {
        httpd_resp_set_status(request, http_404_hdr);
        httpd_resp_send(request, NULL, 0);
    }
    //ESP_LOGI(TAG, "GET REQUEST ON %s", request->uri);
    return ESP_OK;
}

esp_err_t appPOSTHandler(httpd_req_t* request)
{
    char buf[128];
    int ret;
    int remaining = request->content_len;

    while (remaining > 0) 
    {
        // Read the data for the request, ret should be the length of data that was read
        ret = httpd_req_recv(request, buf, MIN(remaining, sizeof(buf)));
        if (ret <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) continue;
            else return ESP_FAIL;
        }

        // Update how much is left to read
        remaining -= ret;

        // Log the data that was received
        // ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        // ESP_LOGI(TAG, "%.*s", ret, buf);
        // ESP_LOGI(TAG, "====================================");

        // Try and decode the request
        if(strcmp(request->uri, "/light-effect") == 0)
        {
            // Looking for a "theme", "brightness", and "param" value to set as the active LED theme
            char response[EFFECT_MAXIMUM_PARAM_LENGTH];
            uint8_t newBrightness = LED_DEFAULT_BRIGHTNESS;
            ledEffect_t incomingEffect = EFFECT_NONE;
        
            if (httpd_query_key_value(buf, "theme", response, EFFECT_MAXIMUM_PARAM_LENGTH) == ESP_OK)
            {
                // atoi returns 0 (INVALID effect) if unable to resolve a number
                incomingEffect = (ledEffect_t) atoi(response);
                if (incomingEffect >= EFFECT_MAX) incomingEffect = 0; // Main application will check for validity before trying to update
            }

            if (httpd_query_key_value(buf, "brightness", response, EFFECT_MAXIMUM_PARAM_LENGTH) == ESP_OK)
            {
                newBrightness = atoi(response);
                if (newBrightness > LED_MAX_BRIGHTNESS) newBrightness = LED_MAX_BRIGHTNESS;
            }

            if (httpd_query_key_value(buf, "param", response, EFFECT_MAXIMUM_PARAM_LENGTH) == ESP_OK)
            {
                memcpy(newEffectMessage.param, response, EFFECT_MAXIMUM_PARAM_LENGTH);
            }
        
            // Set effect values and send to effect queue
            // We will ignore if the queue is full
            newEffectMessage.newBrightness = newBrightness;
            newEffectMessage.newEffect = incomingEffect;
            xQueueSend(newEffectQueueHandle, &newEffectMessage, (TickType_t) 0);
            ESP_LOGI(TAG, "New effect received (effect=%d) (brightness=%i) (param=%.*s)", incomingEffect, newBrightness, EFFECT_MAXIMUM_PARAM_LENGTH, newEffectMessage.param);

            // Message should be <128 bytes (smaller than any intermediate buffers) so this shouldn't trigger prematurely 
            // Break to avoid sending multiple queue items
            break;
        }
    }

    // Need to respond. Currently always just send user back to index.
    httpd_resp_send(request, (char*)app_index_html_start, app_index_html_end - app_index_html_start);

    ESP_LOGI(TAG, "POST REQUEST ON %s", request->uri);
    return ESP_OK;
}

void appServerInit(void)
{
    // Create queue for incoming effect change requests
    newEffectQueueHandle = xQueueCreateStatic(EFFECT_QUEUE_MAX_ITEMS, 
                                              sizeof(ledEffectMessage), 
                                              newEffectQueueBuffer,
                                              &newEffectQueue);
}
