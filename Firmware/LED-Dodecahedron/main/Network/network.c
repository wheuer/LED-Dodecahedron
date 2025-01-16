#include "esp_event.h"
#include <esp_log.h>
#include <nvs_flash.h>
#include "esp_netif.h"
#include "esp_check.h"
#include "esp_wifi.h"

#include "wifi_manager.h"
#include "http_app.h"

#include "mdns.h"

#include "Network/network.h"
#include "Network/http_server.h"

#define WIFI_CONNECTED_BIT  BIT0

static const char *TAG = "[NETWORK]";

static EventGroupHandle_t wifiEventGroup; 

void connectionSuccessCallback(void *pvParameter){
    // Passed parameter will contain the newly received IP
	ip_event_got_ip_t* param = (ip_event_got_ip_t*)pvParameter;

    // Log the new station IP address to the console
	char str_ip[16];
	esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);
	ESP_LOGI(TAG, "Connected to WiFi. Station IP is %s", str_ip);

    // Setup mDNS service for IP advertising
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set("Dodecahedron")); // Access through Dodecahedron.local
    ESP_ERROR_CHECK(mdns_instance_name_set("Slotech LED Dodecahedron")); // General friendly name for the device
    
    // Add mDNS service to advertise our HTTP server
    // As we are only interested in IP resolution, the only property of the service that will be set is the name of the device
    mdns_txt_item_t serviceTxtData = {
        .key = "name", 
        .value = "Slotech LED Dodecahedron"
    };
    ESP_ERROR_CHECK(mdns_service_add("Dodecahedron-WebServer", "_http", "_tcp", 80, &serviceTxtData, 1));

    // Signal that we are connected to the WiFi
    xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
}

void networkLaunch(void)
{
    ESP_LOGI(TAG, "Initializing for WiFi connection");
    // Create event group to wait on WiFi connection status
    wifiEventGroup = xEventGroupCreate();

    // Perform any initilization for app's HTTP server 
    appServerInit();

    // Start WiFi manager, this will begin trying to connect to the WiFi
    // If no/invalid credentials are saved in flash it will also start the soft AP for SSID/pwd setup
    wifi_manager_start();

    // Set wifi manager callbacks to notify when we are successfully connected to WiFi
    wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &connectionSuccessCallback);

    // Setup custom callbacks for the WiFi manager's HTTP server (implemented in http_server.c)
    http_app_set_handler_hook(HTTP_GET, &appGETHandler);
    http_app_set_handler_hook(HTTP_POST, &appPOSTHandler);

    // Wait until the wifi manager is able to connect to the wifi
    ESP_LOGI(TAG, "Waiting for WiFi connection");
    xEventGroupWaitBits(wifiEventGroup,
            WIFI_CONNECTED_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    // At this point the network should be connected to the WiFi and the HTTP server running
}


