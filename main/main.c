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

#include "mpu6050.h"
#include "wifi_mqtt.h"
#include "mouse_buttons.h"



void app_main(void) {
    i2c_init();

    wifi_init();

    xEventGroupWaitBits(wifi_event_grp_handler, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI(TAG, "WIFI connection established, Starting MQTT client.");

    mqtt_app_start();

    xTaskCreate(button_task, "btn_state", 2048, NULL, 10, NULL);
    xTaskCreate(mpu_task, "mpu_task", 2048, NULL, 5, NULL);
    xTaskCreate(task_publisher, "mqtt_task", 2048, NULL, 5, NULL);
}
