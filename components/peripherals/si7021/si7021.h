#ifndef SI7021_H
#define SI7021_h

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

void si7021_set_handler(i2c_master_dev_handle_t handler);
esp_err_t si7021_read_hum (float *humidity);
esp_err_t si7021_read_temp (float *temperature);

#endif