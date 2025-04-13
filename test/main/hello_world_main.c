/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

esp_err_t init_led()
{
    printf("Initializing GPIO_2 (LED)...\n");
    return gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
}

void blink_task(void *pvParameter)
{
    esp_err_t err;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    for ( ;; ) {
        err = gpio_set_level(GPIO_NUM_2, 1);
        if (err != ESP_OK) {
            vTaskDelete(NULL);
            return;
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
        
        err = gpio_set_level(GPIO_NUM_2, 0);
        if (err != ESP_OK) {
            vTaskDelete(NULL);
            return;
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    if (init_led() != ESP_OK) {
        printf("ERROR: Couldn't initialize GPIO_2\n");
        return;
    }

    printf("Creating task...\n");
    BaseType_t tsk = xTaskCreate(
        blink_task, 
        "blink_task", 
        1024, 
        NULL, 
        5, 
        NULL
    );

    if (tsk != pdPASS) {
        printf("Coudn't succesfully complete the task...\n");
    }

}
