#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/apps/sntp.h"

#include "esp_log.h"

#include "esp_websocket_client.h"
 
#define WIFI_TAG "WiFi"
#define WEBSOCKET_TAG "WEBSOCKET"

#define WIFI_SSID "June"
#define WIFI_PASS "wifipassword"

 

// FreeRTOS event group to signal when we are connected & ready to make a request
static EventGroupHandle_t inet_event_group;
static esp_websocket_client_handle_t websocket_client;
  

#define DEVELOP
#ifdef DEVELOP
#define _LOGI(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#else
#define _LOGI(tag, format, ...)
#endif
#define _LOGE(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#define _LOGW(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)

 
static const char *WEBSOCKET_ECHO_ENDPOINT =  "ws://192.168.1.100";

static const int CONNECTED_BIT = BIT0; // status of AP connection

/**
 * @brief  	WiFi event handler
 * @return  esp_err_t
*/
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    _LOGI(WIFI_TAG, "wifi_event_handler id=%d(0x%x)", event->event_id, event->event_id);
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(inet_event_group, CONNECTED_BIT);
        _LOGI(WIFI_TAG, "Wifi connected successfull.");
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        // This is a workaround as ESP32 WiFi libs don't currently
        // auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(inet_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

/**
 * @brief  	Init wifi 
   @return	esp_err_t
*/
static esp_err_t wifi_initialize(void)
{
	_LOGI(WIFI_TAG, "Initializing WiFi station");

    tcpip_adapter_init();
    inet_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

	return ESP_OK;
} 
 
/**
 * @brief  Connect to WiFi
 */
static esp_err_t wifi_connect(const char *ssid, const char *password) {
	wifi_config_t wifi_config = {};
	strcpy((char*)wifi_config.sta.ssid, ssid);
	strcpy((char*)wifi_config.sta.password, password);
	
    _LOGI(WIFI_TAG, "Connecting to SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );

	return ESP_OK;
}
   

/**
 * @brief  Wait for WIFI connection
*/
void wait_to_connect()
{
	xEventGroupWaitBits(inet_event_group, CONNECTED_BIT,
					false, true, portMAX_DELAY);
}

/**
 * @brief  Websocket callback
*/
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    // esp_websocket_client_handle_t client = (esp_websocket_client_handle_t)handler_args;
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            _LOGI(WEBSOCKET_TAG, "WEBSOCKET_EVENT_CONNECTED");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            _LOGI(WEBSOCKET_TAG, "WEBSOCKET_EVENT_DISCONNECTED");
            break;
        case WEBSOCKET_EVENT_DATA:
            _LOGI(WEBSOCKET_TAG, "WEBSOCKET_EVENT_DATA");
            _LOGW(WEBSOCKET_TAG, "Received=%.*s\r\n", data->data_len, (char*)data->data_ptr);
            break;
        case WEBSOCKET_EVENT_ERROR:
            _LOGI(WEBSOCKET_TAG, "WEBSOCKET_EVENT_ERROR");
            break;
    }
}

/**
 * @brief  Connect to websocket server
*/
static void websocket_app_start(void)
{
    _LOGI(WEBSOCKET_TAG, "Connecting to %s...", WEBSOCKET_ECHO_ENDPOINT);

    const esp_websocket_client_config_t websocket_cfg = {
        .uri = WEBSOCKET_ECHO_ENDPOINT,  
		.port = 8080 //9090
	};

    websocket_client = esp_websocket_client_init(&websocket_cfg);
	
    esp_websocket_register_events(websocket_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)websocket_client);
    esp_websocket_client_start(websocket_client);
}
  

/**
 * @brief  Main entry point
*/
void app_main() {
	
	esp_log_level_set("*", ESP_LOG_VERBOSE);
	 
	 // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

	_LOGI(WIFI_TAG, "Free memory: %d bytes", esp_get_free_heap_size());
	_LOGI(WIFI_TAG, "IDF version: %s", esp_get_idf_version());

	// /* Init Wi-Fi */
    ESP_ERROR_CHECK(wifi_initialize());

 	/* Start by connecting to WiFi */
    wifi_connect(WIFI_SSID, WIFI_PASS);

	/* Wait for WiFi connection */
	wait_to_connect(); 

	/* connect to websocket server */
	websocket_app_start();
 
}
