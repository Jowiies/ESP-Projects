#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#define CMD_MESURE_HUM_NOHOLDMODE   0xf5
#define CMD_MESURE_TEMP_AFTERHUM    0xe0
#define CMD_MESURE_TEMP_NOHOLDMODE  0xf3
#define SENSOR_ADDR                 0x40

static const char *TAG = "SI7021";

static i2c_master_bus_handle_t master_handler = NULL;
static i2c_master_dev_handle_t sensor_handler = NULL;

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

    ESP_ERROR_CHECK(i2c_master_bus_add_device(master_handler, &dev_cfg, &sensor_handler));

    return ESP_OK;
}

esp_err_t read_raw_data(uint16_t *raw_data, uint8_t cmd)
{
    uint8_t data[3] = {0};
    esp_err_t err = i2c_master_transmit(sensor_handler, &cmd, sizeof(cmd), 100);
    
    if (err != ESP_OK) {
        printf("I2C Transmit Failed: %s\n", esp_err_to_name(err));
        return err;
    }

    vTaskDelay(pdMS_TO_TICKS(50));
    err = i2c_master_receive(sensor_handler, data, sizeof(data), 100);
    vTaskDelay(pdMS_TO_TICKS(1));
    
    if (err != ESP_OK) {
        printf("I2C Receive Failed: %s\n", esp_err_to_name(err));
        return err;
    }
    
    *raw_data = ((uint16_t)data[0] << 8) | data[1];

    return ESP_OK;
}

esp_err_t read_hum (float *humidity)
{
    uint8_t cmd = CMD_MESURE_HUM_NOHOLDMODE;
    uint16_t raw_hum;
    
    esp_err_t err = read_raw_data(&raw_hum, cmd);
    if (err != ESP_OK) return err;
    
    *humidity = (125.0 * raw_hum / 65536.0 ) - 6.0;
    
    return ESP_OK;
}

esp_err_t read_temp (float *temperature)
{
    uint8_t cmd = CMD_MESURE_TEMP_NOHOLDMODE;
    uint16_t raw_temp;
    
    esp_err_t err = read_raw_data(&raw_temp, cmd);
    if (err != ESP_OK) return err;
   
    *temperature = (175.72 * raw_temp / 65536.0) - 46.85;

    return ESP_OK;
}


void si7021_task(void *pvParameter)
{
    float humidity, temperature;
    esp_err_t hum_err, temp_err;

    while (1) {
        hum_err = read_hum(&humidity);
        temp_err = read_temp(&temperature);

        if (hum_err == ESP_OK) {
            ESP_LOGI(TAG, "Humidity: %.2f%%", humidity);
        } else {
            ESP_LOGE(TAG, "Failed to read humidity");
        }

        if (temp_err == ESP_OK) {
            ESP_LOGI(TAG, "Temperature: %.2f*C", temperature);
        } else {
            ESP_LOGE(TAG, "Failed to read temperature");
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    init_master_device();
    xTaskCreate(si7021_task, "si7021_task", 2048, NULL, 5, NULL);
}
