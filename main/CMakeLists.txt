idf_component_register(
    SRCS
        "main.c"
        "wifi_mqtt.c"
        "mpu6050.c"
        "mouse_buttons.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        mqtt
        esp_wifi
        esp_event
        esp_netif
        nvs_flash
        esp_driver_i2c
        esp_driver_gpio
)