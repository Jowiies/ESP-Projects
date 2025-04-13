#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "si7021.h"

#define SENSOR_ADDR 0x40

static i2c_master_bus_handle_t master_handler = NULL;

esp_err_t init_master_device()
{
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = GPIO_NUM_22,
        .sda_io_num = GPIO_NUM_21,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &master_handler));

    vTaskDelay(pdMS_TO_TICKS(81));
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SENSOR_ADDR,
        .scl_speed_hz = 100000, 
    };

    i2c_master_dev_handle_t sensor_handler;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(master_handler, &dev_cfg, &sensor_handler));

    si7021_init(sensor_handler);

    return ESP_OK;
}

void si7021_task (void *pvParameter)
{
    float humidity, temperature;
    esp_err_t hum_err, temp_err;

    while (1) {
        hum_err = si7021_read_hum(&humidity);
        temp_err = si7021_read_temp(&temperature);

        if (hum_err == ESP_OK) {
            ESP_LOGI("sensor", "Humidity: %.2f%%", humidity);
        } else {
            ESP_LOGE("sensor", "Failed to read humidity");
        }

        if (temp_err == ESP_OK) {
            ESP_LOGI("sensor", "Temperature: %.2f*C", temperature);
        } else {
            ESP_LOGE("sensor", "Failed to read temperature");
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    init_master_device();
    xTaskCreate(si7021_task, "si7021_task", 2048, NULL, 5, NULL);
}
