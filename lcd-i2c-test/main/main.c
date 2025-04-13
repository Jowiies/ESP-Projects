#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_err.h"

#define I2C_MASTER_SCL_IO          GPIO_NUM_22  
#define I2C_MASTER_SDA_IO          GPIO_NUM_21
#define I2C_MASTER_NUM             I2C_NUM_0  
#define I2C_MASTER_CLK_SPEED       100000    

#include "i2c_lcd1602a.h"
#define LCD_I2C_ADDRESS 0x3F

static i2c_master_bus_handle_t i2c_bus = NULL;   // I2C Bus Handle
static i2c_master_dev_handle_t lcd_handle = NULL; // LCD Device Handle

void i2c_master_init() 
{
    // Configure the I2C master bus
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &i2c_bus));

    // Add LCD device to I2C bus
    i2c_device_config_t lcd_device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = LCD_I2C_ADDRESS,
        .scl_speed_hz = I2C_MASTER_CLK_SPEED
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &lcd_device_config, &lcd_handle));

    ESP_LOGI("I2C", "I2C initialized successfully");
}

void lcd_send(uint8_t byte, uint8_t mode) 
{
    //printf("Data: %2X, Mode: %2X", byte, mode);
    uint8_t data_h = (byte & 0xF0) | LCD_BACKLIGHT | mode;
    uint8_t data_l = ((byte << 4) & 0xF0) | LCD_BACKLIGHT | mode;
    uint8_t buffer[4] = {
        data_h | LCD_ENABLE,
        data_h,
        data_l | LCD_ENABLE,
        data_l,    
    };

    ESP_ERROR_CHECK(i2c_master_transmit(lcd_handle, buffer, sizeof(buffer), 10000));
    //ESP_LOGI("I2C", "Buffer: %02X %02X %02X %02X", buffer[0], buffer[1], buffer[2], buffer[3]);
}

void lcd_init()
{
    vTaskDelay(pdMS_TO_TICKS(50)); // Wait for LCD to power on

    // Force LCD into 4-bit mode
    lcd_send(LCD_FUNCTIONSET | LCD_8BITMODE, LCD_RS_COMMAND); 
    vTaskDelay(pdMS_TO_TICKS(5));

    lcd_send(LCD_FUNCTIONSET | LCD_8BITMODE, LCD_RS_COMMAND);
    vTaskDelay(pdMS_TO_TICKS(5));

    lcd_send(LCD_FUNCTIONSET | LCD_8BITMODE, LCD_RS_COMMAND);
    esp_rom_delay_us(150);

    lcd_send(LCD_FUNCTIONSET | LCD_4BITMODE, LCD_RS_COMMAND);
    esp_rom_delay_us(150);

    // Configure LCD
    lcd_send(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS, LCD_RS_COMMAND);
    esp_rom_delay_us(40);
    lcd_send(LCD_DISPLAYCONTROL | LCD_DISPLAYOFF, LCD_RS_COMMAND); 
    esp_rom_delay_us(40);
    lcd_send(LCD_CLEARDISPLAY, LCD_RS_COMMAND); 
    vTaskDelay(pdMS_TO_TICKS(2));
    lcd_send(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT , LCD_RS_COMMAND); 
    esp_rom_delay_us(40);
    lcd_send(LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_RS_COMMAND);
    esp_rom_delay_us(40);
}
void lcd_set_cursor(int row, int col)
{
    col |= (row == 0) ? 0x80 : 0xC0;
    lcd_send(col, LCD_RS_COMMAND);
    esp_rom_delay_us(40);
}

void lcd_print(const char *str)
{
    while (*str) lcd_send(*str++, LCD_RS_DATA);
}

void lcd_clear(void)
{
    lcd_send(LCD_CLEARDISPLAY, LCD_RS_COMMAND); 
    vTaskDelay(pdMS_TO_TICKS(2));
}

void app_main(void) 
{
    i2c_master_init();
    lcd_init();
    lcd_clear();
    lcd_set_cursor(0,0);
    lcd_print("Hello World!");
    lcd_set_cursor(1,0);
    lcd_print("From ESP32!");
}
