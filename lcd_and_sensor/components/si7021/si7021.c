#include "si7021.h"

#define CMD_MESURE_HUM_NOHOLDMODE   0xf5
#define CMD_MESURE_TEMP_AFTERHUM    0xe0
#define CMD_MESURE_TEMP_NOHOLDMODE  0xf3

static i2c_master_dev_handle_t si7021_handler = NULL;

void si7021_set_handler(i2c_master_dev_handle_t handler)
{
    si7021_handler = handler;
}

void si7021_init(i2c_master_dev_handle_t handler)
{
    vTaskDelay(pdMS_TO_TICKS(81)); // wait to reach > 1.9 v
    si7021_handler = handler;
}

static esp_err_t read_raw_data(uint16_t *raw_data, uint8_t cmd)
{
    uint8_t data[3] = {0};
    esp_err_t err = i2c_master_transmit(si7021_handler, &cmd, sizeof(cmd), 100);
    
    if (err != ESP_OK) {
        printf("I2C Transmit Failed: %s\n", esp_err_to_name(err));
        return err;
    }

    vTaskDelay(pdMS_TO_TICKS(50));
    err = i2c_master_receive(si7021_handler, data, sizeof(data), 100);
    vTaskDelay(pdMS_TO_TICKS(1));
    
    if (err != ESP_OK) {
        printf("I2C Receive Failed: %s\n", esp_err_to_name(err));
        return err;
    }
    
    *raw_data = ((uint16_t)data[0] << 8) | data[1];

    return ESP_OK;
}

esp_err_t si7021_read_hum (float *humidity)
{
    uint8_t cmd = CMD_MESURE_HUM_NOHOLDMODE;
    uint16_t raw_hum;
    
    esp_err_t err = read_raw_data(&raw_hum, cmd);
    if (err != ESP_OK) return err;
    
    *humidity = (125.0 * raw_hum / 65536.0 ) - 6.0;

    *humidity = (*humidity > 100) ? 100 : *humidity;
    
    return ESP_OK;
}

esp_err_t si7021_read_temp (float *temperature)
{
    uint8_t cmd = CMD_MESURE_TEMP_NOHOLDMODE;
    uint16_t raw_temp;
    
    esp_err_t err = read_raw_data(&raw_temp, cmd);
    if (err != ESP_OK) return err;
   
    *temperature = (175.72 * raw_temp / 65536.0) - 46.85;

    return ESP_OK;
}