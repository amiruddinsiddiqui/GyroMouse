// #ifndef WIFI_MQTT_H
// #define WIFI_MQTT_H

// #include "esp_event.h"

// static EventGroupHandle_t wifi_event_grp_handler;
// #define WIFI_CONNECTED_BIT BIT0
// static const char* TAG = "MPU_PUBLISHER";

// static void mqtt_event_handler(void* handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
// static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
// static void wifi_init();
// static void mqtt_app_start(void);
// void task_publisher(void* arg);

// #endif

#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define WIFI_CONNECTED_BIT BIT0

extern EventGroupHandle_t wifi_event_grp_handler;
extern const char *TAG;

void wifi_init(void);
void mqtt_app_start(void);
void task_publisher(void *arg);

#endif