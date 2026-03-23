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

esp_err_t ssd1363_api_init_smoke_test_display(void)
{
    static const uint8_t unlock[] = {0x12};
    static const uint8_t clock_divider[] = {0x30};
    static const uint8_t multiplex_ratio[] = {SSD1363_MULTIPLEX_RATIO};
    static const uint8_t display_offset[] = {SSD1363_DISPLAY_OFFSET};
    static const uint8_t start_line[] = {SSD1363_DISPLAY_START_LINE};
    static const uint8_t remap[] = {SSD1363_REMAP_BYTE0, SSD1363_REMAP_BYTE1};
    static const uint8_t io_input[] = {0x00};
    static const uint8_t function_select[] = {0x01};
    static const uint8_t vsl[] = {0xA0, 0xFD};
    static const uint8_t contrast[] = {0x7F};
    static const uint8_t master_contrast[] = {0x0F};
    static const uint8_t phase_length[] = {0x74};
    static const uint8_t enhancement[] = {0x82, 0x20};
    static const uint8_t precharge_voltage[] = {0x1F};
    static const uint8_t second_precharge[] = {0x08};
    static const uint8_t vcomh[] = {0x07};

    esp_err_t err = ssd1363_api_write_command_with_data(0xFD, unlock, sizeof(unlock));
    if (err != ESP_OK) return err;

    err = ssd1363_api_command(0xAE);
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xB3, clock_divider, sizeof(clock_divider));
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xCA, multiplex_ratio, sizeof(multiplex_ratio));
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xA2, display_offset, sizeof(display_offset));
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xA1, start_line, sizeof(start_line));
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xA0, remap, sizeof(remap));
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xB5, io_input, sizeof(io_input));
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xAB, function_select, sizeof(function_select));
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xB4, vsl, sizeof(vsl));
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xC1, contrast, sizeof(contrast));
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xC7, master_contrast, sizeof(master_contrast));
    if (err != ESP_OK) return err;

    err = ssd1363_api_command(0xB9);
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xB1, phase_length, sizeof(phase_length));
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xD1, enhancement, sizeof(enhancement));
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xBB, precharge_voltage, sizeof(precharge_voltage));
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xB6, second_precharge, sizeof(second_precharge));
    if (err != ESP_OK) return err;

    err = ssd1363_api_write_command_with_data(0xBE, vcomh, sizeof(vcomh));
    if (err != ESP_OK) return err;

    err = ssd1363_api_command(0xA6);
    if (err != ESP_OK) return err;

    err = ssd1363_api_command(0xA9);
    if (err != ESP_OK) return err;

    ssd1363_interface_delay_ms(10);

    err = ssd1363_api_command(0xAF);
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

    if ((width % 4U) != 0U || width == 0U || height == 0U) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(line_buffer, pattern, width / 2);

    end_column = (uint8_t)((width / 4U) - 1U);

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

esp_err_t ssd1363_api_set_window(uint8_t start_column, uint8_t end_column, uint8_t start_row, uint8_t end_row)
{
    uint8_t column_args[2] = {
        (uint8_t)(SSD1363_COLUMN_OFFSET + start_column),
        (uint8_t)(SSD1363_COLUMN_OFFSET + end_column),
    };
    uint8_t row_args[2] = {start_row, end_row};

    esp_err_t err = ssd1363_api_command(SSD1363_CMD_SET_COLUMN_ADDR);
    if (err != ESP_OK) {
        return err;
    }

    err = ssd1363_api_data_buffer(column_args, sizeof(column_args));
    if (err != ESP_OK) {
        return err;
    }

    err = ssd1363_api_command(SSD1363_CMD_SET_ROW_ADDR);
    if (err != ESP_OK) {
        return err;
    }

    return ssd1363_api_data_buffer(row_args, sizeof(row_args));
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