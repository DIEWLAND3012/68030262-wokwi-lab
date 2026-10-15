#include <stdint.h>
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#define I2C_SCL_IO GPIO_NUM_22
#define I2C_SDA_IO GPIO_NUM_21
#define I2C_MASTER_FREQ_HZ 100000
#define LCD_SLAVE_ADDR 0x27

static const char *TAG = "I2C_PROJECT";

void app_main(void)
{
    const i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_SCL_IO,
        .sda_io_num = I2C_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    const i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = LCD_SLAVE_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    i2c_master_dev_handle_t device_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(
        bus_handle, &device_config, &device_handle));

    ESP_LOGI(TAG, "Probing LCD at I2C address 0x%02X...", LCD_SLAVE_ADDR);
    const uint8_t probe[] = { 0x08 };
    const esp_err_t err = i2c_master_transmit(
        device_handle, probe, sizeof(probe), 1000);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "I2C peripheral acknowledged the transfer.");
    } else {
        ESP_LOGE(TAG, "I2C transfer failed: %s", esp_err_to_name(err));
    }
}
