#include "ssd1363_api.h"

#include "ssd1363_config.h"

#include <string.h>

static esp_err_t ssd1363_api_write_command_with_data(uint8_t command, const uint8_t *data, size_t len)
{
    esp_err_t err = ssd1363_api_command(command);
    if (err != ESP_OK) {
        return err;
    }

    if (len == 0 || data == NULL) {
        return ESP_OK;
    }

    return ssd1363_api_data_buffer(data, len);
}

static esp_err_t ssd1363_api_write_pair(uint8_t command, uint8_t value0, uint8_t value1)
{
    const uint8_t args[2] = {value0, value1};

    return ssd1363_api_write_command_with_data(command, args, sizeof(args));
}

esp_err_t ssd1363_api_init(ssd1363_bus_t bus)
{
    esp_err_t err = ssd1363_interface_init(bus);
    if (err != ESP_OK) {
        return err;
    }

    err = ssd1363_interface_reset(false);
    if (err != ESP_OK) {
        return err;
    }
    ssd1363_interface_delay_ms(1);

    err = ssd1363_interface_reset(true);
    if (err != ESP_OK) {
        return err;
    }
    ssd1363_interface_delay_ms(10);

    return ESP_OK;
}

esp_err_t ssd1363_api_unlock_command_interface(uint8_t unlock_code)
{
    return ssd1363_api_write_command_with_data(SSD1363_CMD_COMMAND_LOCK, &unlock_code, 1);
}

esp_err_t ssd1363_api_display_off(void)
{
    return ssd1363_api_command(SSD1363_CMD_DISPLAY_OFF);
}

esp_err_t ssd1363_api_display_on(void)
{
    return ssd1363_api_command(SSD1363_CMD_DISPLAY_ON);
}

esp_err_t ssd1363_api_exit_partial_display(void)
{
    return ssd1363_api_command(SSD1363_CMD_EXIT_PARTIAL_DISPLAY);
}

esp_err_t ssd1363_api_set_display_mode_normal(void)
{
    return ssd1363_api_command(SSD1363_CMD_SET_DISPLAY_MODE_NORMAL);
}

esp_err_t ssd1363_api_set_display_clock(uint8_t clock_setting)
{
    return ssd1363_api_write_command_with_data(SSD1363_CMD_SET_CLOCK_DIVIDER, &clock_setting, 1);
}

esp_err_t ssd1363_api_set_function_selection(uint8_t function_selection)
{
    return ssd1363_api_write_command_with_data(SSD1363_CMD_FUNCTION_SELECTION, &function_selection, 1);
}

esp_err_t ssd1363_api_set_gpio(uint8_t gpio_config)
{
    return ssd1363_api_write_command_with_data(SSD1363_CMD_SET_GPIO, &gpio_config, 1);
}

esp_err_t ssd1363_api_set_multiplex_ratio(uint8_t ratio)
{
    return ssd1363_api_write_command_with_data(SSD1363_CMD_SET_MULTIPLEX_RATIO, &ratio, 1);
}

esp_err_t ssd1363_api_set_display_offset(uint8_t offset)
{
    return ssd1363_api_write_command_with_data(SSD1363_CMD_SET_DISPLAY_OFFSET, &offset, 1);
}

esp_err_t ssd1363_api_set_start_line(uint8_t line)
{
    return ssd1363_api_write_command_with_data(SSD1363_CMD_SET_START_LINE, &line, 1);
}

esp_err_t ssd1363_api_set_remap(uint8_t remap_byte0, uint8_t remap_byte1)
{
    return ssd1363_api_write_pair(SSD1363_CMD_SET_REMAP, remap_byte0, remap_byte1);
}

esp_err_t ssd1363_api_set_segment_low_voltage(uint8_t level0, uint8_t level1)
{
    return ssd1363_api_write_pair(SSD1363_CMD_SET_SEGMENT_LOW_VOLTAGE, level0, level1);
}

esp_err_t ssd1363_api_set_contrast(uint8_t contrast)
{
    return ssd1363_api_write_command_with_data(SSD1363_CMD_SET_CONTRAST, &contrast, 1);
}

esp_err_t ssd1363_api_set_master_contrast(uint8_t contrast)
{
    return ssd1363_api_write_command_with_data(SSD1363_CMD_SET_MASTER_CONTRAST, &contrast, 1);
}

esp_err_t ssd1363_api_use_default_gray_table(void)
{
    return ssd1363_api_command(SSD1363_CMD_USE_DEFAULT_GRAY_TABLE);
}

esp_err_t ssd1363_api_set_phase_length(uint8_t phase_length)
{
    return ssd1363_api_write_command_with_data(SSD1363_CMD_SET_PHASE_LENGTH, &phase_length, 1);
}

esp_err_t ssd1363_api_set_display_enhancement(uint8_t value0, uint8_t value1)
{
    return ssd1363_api_write_pair(SSD1363_CMD_SET_DISPLAY_ENHANCEMENT, value0, value1);
}

esp_err_t ssd1363_api_set_precharge_voltage(uint8_t voltage)
{
    return ssd1363_api_write_command_with_data(SSD1363_CMD_SET_PRECHARGE_VOLTAGE, &voltage, 1);
}

esp_err_t ssd1363_api_set_second_precharge(uint8_t period)
{
    return ssd1363_api_write_command_with_data(SSD1363_CMD_SET_SECOND_PRECHARGE, &period, 1);
}

esp_err_t ssd1363_api_set_vcomh(uint8_t level)
{
    return ssd1363_api_write_command_with_data(SSD1363_CMD_SET_VCOMH, &level, 1);
}

esp_err_t ssd1363_api_panel_init(void)
{
    esp_err_t err = ssd1363_api_unlock_command_interface(SSD1363_PANEL_INIT_UNLOCK_CODE);
    if (err != ESP_OK) return err;

    err = ssd1363_api_display_off();
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_display_clock(SSD1363_PANEL_INIT_DISPLAY_CLOCK);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_multiplex_ratio(SSD1363_MULTIPLEX_RATIO);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_display_offset(SSD1363_DISPLAY_OFFSET);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_start_line(SSD1363_DISPLAY_START_LINE);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_remap(SSD1363_REMAP_BYTE0, SSD1363_REMAP_BYTE1);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_gpio(SSD1363_PANEL_INIT_GPIO_CONFIG);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_function_selection(SSD1363_PANEL_INIT_FUNCTION_SELECTION);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_segment_low_voltage(SSD1363_PANEL_INIT_VSL0, SSD1363_PANEL_INIT_VSL1);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_contrast(SSD1363_PANEL_INIT_CONTRAST);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_master_contrast(SSD1363_PANEL_INIT_MASTER_CONTRAST);
    if (err != ESP_OK) return err;

    err = ssd1363_api_use_default_gray_table();
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_phase_length(SSD1363_PANEL_INIT_PHASE_LENGTH);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_display_enhancement(SSD1363_PANEL_INIT_ENHANCEMENT0, SSD1363_PANEL_INIT_ENHANCEMENT1);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_precharge_voltage(SSD1363_PANEL_INIT_PRECHARGE_VOLTAGE);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_second_precharge(SSD1363_PANEL_INIT_SECOND_PRECHARGE);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_vcomh(SSD1363_PANEL_INIT_VCOMH);
    if (err != ESP_OK) return err;

    err = ssd1363_api_set_display_mode_normal();
    if (err != ESP_OK) return err;

    err = ssd1363_api_exit_partial_display();
    if (err != ESP_OK) return err;

    ssd1363_interface_delay_ms(10);

    err = ssd1363_api_display_on();
    if (err != ESP_OK) return err;

    ssd1363_interface_delay_ms(50);
    return ESP_OK;
}

esp_err_t ssd1363_api_fill_active_area(uint8_t pattern, uint16_t width, uint16_t height)
{
    uint8_t line_buffer[SSD1363_PANEL_WIDTH / 2];
    uint8_t end_column;

    if (width > SSD1363_PANEL_WIDTH || height > SSD1363_PANEL_HEIGHT) {
        return ESP_ERR_INVALID_ARG;
    }

    if ((width % SSD1363_PANEL_COLUMN_ADDR_UNIT_PIXELS) != 0U || width == 0U || height == 0U) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(line_buffer, pattern, width / 2);

    end_column = (uint8_t)((width / SSD1363_PANEL_COLUMN_ADDR_UNIT_PIXELS) - 1U);

    esp_err_t err = ssd1363_api_set_window(0, end_column, 0, height - 1);
    if (err != ESP_OK) {
        return err;
    }

    err = ssd1363_api_command(SSD1363_CMD_WRITE_RAM);
    if (err != ESP_OK) {
        return err;
    }

    for (uint16_t row = 0; row < height; ++row) {
        if (ssd1363_interface_get_bus() == SSD1363_BUS_I2C) {
            size_t offset = 0;
            while (offset < (width / 2)) {
                size_t chunk_size = (width / 2) - offset;
                if (chunk_size > SSD1363_I2C_DATA_CHUNK_SIZE) {
                    chunk_size = SSD1363_I2C_DATA_CHUNK_SIZE;
                }

                err = ssd1363_api_data_buffer(line_buffer + offset, chunk_size);
                if (err != ESP_OK) {
                    return err;
                }

                offset += chunk_size;
            }
        } else {
            err = ssd1363_api_data_buffer(line_buffer, width / 2);
            if (err != ESP_OK) {
                return err;
            }
        }
    }

    return ESP_OK;
}

esp_err_t ssd1363_api_command(uint8_t command)
{
    return ssd1363_interface_write_command(&command, 1);
}

esp_err_t ssd1363_api_data(uint8_t data)
{
    return ssd1363_interface_write_data(&data, 1);
}

esp_err_t ssd1363_api_data_buffer(const uint8_t *data, size_t len)
{
    return ssd1363_interface_write_data(data, len);
}

esp_err_t ssd1363_api_set_column_address(uint8_t start_column, uint8_t end_column)
{
    uint8_t column_args[2] = {
        (uint8_t)(SSD1363_PANEL_COLUMN_OFFSET + start_column),
        (uint8_t)(SSD1363_PANEL_COLUMN_OFFSET + end_column),
    };

    return ssd1363_api_write_command_with_data(SSD1363_CMD_SET_COLUMN_ADDR, column_args, sizeof(column_args));
}

esp_err_t ssd1363_api_set_row_address(uint8_t start_row, uint8_t end_row)
{
    uint8_t row_args[2] = {start_row, end_row};

    return ssd1363_api_write_command_with_data(SSD1363_CMD_SET_ROW_ADDR, row_args, sizeof(row_args));
}

esp_err_t ssd1363_api_set_window(uint8_t start_column, uint8_t end_column, uint8_t start_row, uint8_t end_row)
{
    esp_err_t err = ssd1363_api_set_column_address(start_column, end_column);
    if (err != ESP_OK) {
        return err;
    }

    return ssd1363_api_set_row_address(start_row, end_row);
}

esp_err_t ssd1363_api_send_buffer(const uint8_t *buffer, size_t len)
{
    esp_err_t err = ssd1363_api_command(SSD1363_CMD_WRITE_RAM);
    if (err != ESP_OK) {
        return err;
    }

    if (ssd1363_interface_get_bus() != SSD1363_BUS_I2C) {
        return ssd1363_api_data_buffer(buffer, len);
    }

    size_t offset = 0;
    while (offset < len) {
        size_t chunk_size = len - offset;
        if (chunk_size > SSD1363_I2C_DATA_CHUNK_SIZE) {
            chunk_size = SSD1363_I2C_DATA_CHUNK_SIZE;
        }

        err = ssd1363_api_data_buffer(buffer + offset, chunk_size);
        if (err != ESP_OK) {
            return err;
        }

        offset += chunk_size;
    }

    return ESP_OK;
}