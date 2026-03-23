#include "ssd1363_interface.h"

#include "ssd1363_config.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static ssd1363_bus_t g_active_bus = SSD1363_BUS_I2C;
static bool g_i2c_initialized = false;
static bool g_spi_initialized = false;
static spi_device_handle_t g_spi_handle = NULL;

static esp_err_t ssd1363_interface_config_output_pin(int pin)
{
    if (pin < 0) {
        return ESP_OK;
    }

    gpio_config_t config = {
        .pin_bit_mask = 1ULL << pin,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    return gpio_config(&config);
}

static esp_err_t ssd1363_interface_init_i2c(void)
{
    if (g_i2c_initialized) {
        return ESP_OK;
    }

    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SSD1363_I2C_SDA_PIN,
        .scl_io_num = SSD1363_I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = SSD1363_I2C_CLOCK_HZ,
    };

    esp_err_t err = i2c_param_config((i2c_port_t)SSD1363_I2C_PORT, &config);
    if (err != ESP_OK) {
        return err;
    }

    err = i2c_driver_install((i2c_port_t)SSD1363_I2C_PORT, config.mode, 0, 0, 0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    g_i2c_initialized = true;
    return ESP_OK;
}

static esp_err_t ssd1363_interface_deinit_i2c(void)
{
    if (!g_i2c_initialized) {
        return ESP_OK;
    }

    esp_err_t err = i2c_driver_delete((i2c_port_t)SSD1363_I2C_PORT);
    if (err == ESP_OK) {
        g_i2c_initialized = false;
    }

    return err;
}

static esp_err_t ssd1363_interface_init_spi(void)
{
    if (g_spi_initialized) {
        return ESP_OK;
    }

    spi_bus_config_t bus_config = {
        .mosi_io_num = SSD1363_SPI_MOSI_PIN,
        .miso_io_num = -1,
        .sclk_io_num = SSD1363_SPI_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };

    spi_device_interface_config_t device_config = {
        .clock_speed_hz = SSD1363_SPI_CLOCK_HZ,
        .mode = 3,
        .spics_io_num = SSD1363_SPI_CS_PIN,
        .queue_size = SSD1363_SPI_QUEUE_SIZE,
    };

    esp_err_t err = spi_bus_initialize((spi_host_device_t)SSD1363_SPI_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    err = spi_bus_add_device((spi_host_device_t)SSD1363_SPI_HOST, &device_config, &g_spi_handle);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    if (SSD1363_SPI_DC_PIN >= 0) {
        err = ssd1363_interface_config_output_pin(SSD1363_SPI_DC_PIN);
        if (err != ESP_OK) {
            return err;
        }
    }

    if (SSD1363_RESET_PIN >= 0) {
        err = ssd1363_interface_config_output_pin(SSD1363_RESET_PIN);
        if (err != ESP_OK) {
            return err;
        }
    }

    g_spi_initialized = true;
    return ESP_OK;
}

static esp_err_t ssd1363_interface_deinit_spi(void)
{
    if (!g_spi_initialized) {
        return ESP_OK;
    }

    if (g_spi_handle != NULL) {
        spi_bus_remove_device(g_spi_handle);
        g_spi_handle = NULL;
    }

    esp_err_t err = spi_bus_free((spi_host_device_t)SSD1363_SPI_HOST);
    if (err == ESP_OK) {
        g_spi_initialized = false;
    }

    return err;
}

static esp_err_t ssd1363_interface_i2c_write(uint8_t control, const uint8_t *data, size_t len)
{
    if (!g_i2c_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (cmd == NULL) {
        return ESP_ERR_NO_MEM;
    }

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SSD1363_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, control, true);

    if (len > 0 && data != NULL) {
        i2c_master_write(cmd, (uint8_t *)data, len, true);
    }

    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(
        (i2c_port_t)SSD1363_I2C_PORT,
        cmd,
        pdMS_TO_TICKS(SSD1363_I2C_TIMEOUT_MS)
    );

    i2c_cmd_link_delete(cmd);
    return err;
}

static esp_err_t ssd1363_interface_spi_write(bool is_data, const uint8_t *data, size_t len)
{
    if (!g_spi_initialized || g_spi_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    if (SSD1363_SPI_DC_PIN >= 0) {
        esp_err_t err = gpio_set_level((gpio_num_t)SSD1363_SPI_DC_PIN, is_data ? 1 : 0);
        if (err != ESP_OK) {
            return err;
        }
    }

    if (len == 0 || data == NULL) {
        return ESP_OK;
    }

    spi_transaction_t transaction = {
        .length = len * 8,
        .tx_buffer = data,
    };

    return spi_device_transmit(g_spi_handle, &transaction);
}

esp_err_t ssd1363_interface_init(ssd1363_bus_t bus)
{
    g_active_bus = bus;

    if (SSD1363_RESET_PIN >= 0) {
        esp_err_t err = ssd1363_interface_config_output_pin(SSD1363_RESET_PIN);
        if (err != ESP_OK) {
            return err;
        }
    }

    if (bus == SSD1363_BUS_I2C) {
        return ssd1363_interface_init_i2c();
    }

    return ssd1363_interface_init_spi();
}

esp_err_t ssd1363_interface_deinit(void)
{
    if (g_active_bus == SSD1363_BUS_I2C) {
        return ssd1363_interface_deinit_i2c();
    }

    return ssd1363_interface_deinit_spi();
}

ssd1363_bus_t ssd1363_interface_get_bus(void)
{
    return g_active_bus;
}

esp_err_t ssd1363_interface_write_command(const uint8_t *data, size_t len)
{
    if (g_active_bus == SSD1363_BUS_I2C) {
        return ssd1363_interface_i2c_write(SSD1363_I2C_CONTROL_COMMAND, data, len);
    }

    return ssd1363_interface_spi_write(false, data, len);
}

esp_err_t ssd1363_interface_write_data(const uint8_t *data, size_t len)
{
    if (g_active_bus == SSD1363_BUS_I2C) {
        return ssd1363_interface_i2c_write(SSD1363_I2C_CONTROL_DATA, data, len);
    }

    return ssd1363_interface_spi_write(true, data, len);
}

esp_err_t ssd1363_interface_reset(bool high)
{
    if (SSD1363_RESET_PIN < 0) {
        return ESP_OK;
    }

    return gpio_set_level((gpio_num_t)SSD1363_RESET_PIN, high ? 1 : 0);
}

void ssd1363_interface_delay_ms(uint32_t delay_ms)
{
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
}