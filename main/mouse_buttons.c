
#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mouse_buttons.h"

#define LEFT_BTN_PIN    GPIO_NUM_34
#define RIGHT_BTN_PIN   GPIO_NUM_35

volatile int left_btn_state = 1;
volatile int right_btn_state = 1;

void button_task(void *arg) {

    gpio_config_t btn_config = {
        .pin_bit_mask = (1ULL << LEFT_BTN_PIN) | (1ULL << RIGHT_BTN_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&btn_config));

    int previous_left = 1;
    int previous_right = 1;

    while (1) {

        int current_left = gpio_get_level(LEFT_BTN_PIN);
        int current_right = gpio_get_level(RIGHT_BTN_PIN);

        left_btn_state = current_left;
        right_btn_state = current_right;

        if (current_left == 0 && previous_left == 1) {
            ESP_LOGI("BUTTON", "LEFT CLICK");
        }

        if (current_right == 0 && previous_right == 1) {
            ESP_LOGI("BUTTON", "RIGHT CLICK");
        }

        previous_left = current_left;
        previous_right = current_right;

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}


