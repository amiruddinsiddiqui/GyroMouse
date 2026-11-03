#include <stdio.h>
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mpu6050.h"

#define I2C_SCL_PIN              19
#define I2C_SDA_PIN              18
#define MPU6050_STARTING_ADDR    0x3B
#define SLAVE_ADDR               0x68

uint8_t raw_data[14];
int16_t process_data[7];
float final_data[7] = {0};

static i2c_master_bus_handle_t i2c_bus_handler;
static i2c_master_dev_handle_t i2c_master_device_handler;

void i2c_init(void) {
    i2c_master_bus_config_t i2c_bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_SCL_PIN,
        .sda_io_num = I2C_SDA_PIN,
    };

    // i2c_master_bus_handle_t i2c_bus_handler;
    // i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_handler);
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_handler));

    i2c_device_config_t i2c_device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SLAVE_ADDR,
        .scl_speed_hz = 100000,
        // .scl_wait_us = 1000,
    };

    // i2c_master_dev_handle_t i2c_master_device_handler;
    // i2c_master_bus_add_device(i2c_bus_handler, &i2c_device_config, &i2c_master_device_handler);
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handler, &i2c_device_config, &i2c_master_device_handler));
}

void mpu_task(void* arg) {
    // i2c_master_receive(i2c_master_device_handler, raw_data, 14, -1);
    uint8_t reg = MPU6050_STARTING_ADDR;

    uint8_t wake_data[2] = {0x6B, 0x00};

    
    while(1) {
        esp_err_t ret = i2c_master_transmit(i2c_master_device_handler, wake_data, sizeof(wake_data), 1000);
    
        if (ret != ESP_OK) {
            ESP_LOGE("MPU6050", "Wake-up failed: %s", esp_err_to_name(ret));
        }

        esp_err_t return_val = i2c_master_transmit_receive(i2c_master_device_handler, &reg, 1, raw_data, 14, 1000);

        if(return_val == ESP_OK) {
            for(uint8_t i = 0; i<7; i++) {
                process_data[i] = (int16_t)(raw_data[i*2]<<8 | raw_data[(i*2)+1]);
            }
        
            final_data[0] = process_data[0] / 16384.0f;
            final_data[1] = process_data[1] / 16384.0f;
            final_data[2] = process_data[2] / 16384.0f;
        
            final_data[3] = (process_data[3] / 333.87f) + 21.0f;
        
            final_data[4] = process_data[4] / 131.0f;
            final_data[5] = process_data[5] / 131.0f;
            final_data[6] = process_data[6] / 131.0f;

            // Print sensor data
            printf("AX: %.2f g | AY: %.2f g | AZ: %.2f g\n", final_data[0], final_data[1], final_data[2]);
            printf("TEMP: %.2f C\n", final_data[3]);
            printf("GX: %.2f | GY: %.2f | GZ: %.2f deg/s\n", final_data[4], final_data[5], final_data[6]);

        } else {
            printf("MPU6050 I2C read failed: %s\n",esp_err_to_name(return_val));
            continue;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

