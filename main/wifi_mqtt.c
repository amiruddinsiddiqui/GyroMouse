#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h" 
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"

#include "wifi_mqtt.h"
#include "mpu6050.h"

#define WIFI_SSID ""
#define WIFI_PSWD ""

#define MQTT_BROKER_URI "mqtt://broker.emqx.io"



// esp_mqtt_client_handle_t mqtt_client_handler;

// // static EventGroupHandle_t wifi_event_grp_handler;
// // #define WIFI_CONNECTED_BIT BIT0

EventGroupHandle_t wifi_event_grp_handler;
const char *TAG = "MPU_PUBLISHER";

esp_mqtt_client_handle_t mqtt_client_handler;

static void mqtt_event_handler(void* handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT client connected");
            esp_mqtt_client_publish(mqtt_client_handler, "esp/status", "sensor_online", 0, 1, 0);
            break;
        
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT client disconnected");
            break;
        
        default:
            break;
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if(event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "WIFI connecting...");
    } else if(event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "WIFI disconnected, trying to reconnect...");
        esp_wifi_connect();
    } else if (event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *ip_event = (ip_event_got_ip_t *) event_data;

        ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&ip_event->ip_info.ip));

        xEventGroupSetBits(wifi_event_grp_handler, WIFI_CONNECTED_BIT);
    }
}

void wifi_init() {
    wifi_event_grp_handler = xEventGroupCreate();
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_init_cfg);

    wifi_config_t wifi_cfg = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PSWD,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);

    esp_wifi_start();
    ESP_LOGI(TAG, "WIFI init finished, waiting to connect...");
}

void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
    };
    mqtt_client_handler = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client_handler, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client_handler);
}


void task_publisher(void *arg) {
    // float final_data[7];
    char json_payload[250];

    while (1) {
        snprintf(json_payload, sizeof(json_payload),
            "{\"device_id\":\"mpu6050\","
            "\"Accel_X\":%.2f,"
            "\"Accel_Y\":%.2f,"
            "\"Accel_Z\":%.2f,"
            "\"Temperature\":%.2f,"
            "\"Gyro_X\":%.2f,"
            "\"Gyro_Y\":%.2f,"
            "\"Gyro_Z\":%.2f}",
            final_data[0], final_data[1], final_data[2],
            final_data[3],
            final_data[4], final_data[5], final_data[6]
        );

        esp_mqtt_client_publish(mqtt_client_handler, "esp/data", json_payload, 0, 0, 0);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}