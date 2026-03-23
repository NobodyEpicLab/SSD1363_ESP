#ifndef SSD1363_API_H
#define SSD1363_API_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "ssd1363_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SSD1363_CMD_SET_COLUMN_ADDR    0x15
#define SSD1363_CMD_WRITE_RAM          0x5C
#define SSD1363_CMD_SET_ROW_ADDR       0x75
#define SSD1363_CMD_COMMAND_LOCK       0xFD

esp_err_t ssd1363_api_init(ssd1363_bus_t bus);
esp_err_t ssd1363_api_init_smoke_test_display(void);
esp_err_t ssd1363_api_command(uint8_t command);
esp_err_t ssd1363_api_data(uint8_t data);
esp_err_t ssd1363_api_data_buffer(const uint8_t *data, size_t len);
esp_err_t ssd1363_api_set_window(uint8_t start_column, uint8_t end_column, uint8_t start_row, uint8_t end_row);
esp_err_t ssd1363_api_send_buffer(const uint8_t *buffer, size_t len);
esp_err_t ssd1363_api_fill_active_area(uint8_t pattern, uint16_t width, uint16_t height);

#ifdef __cplusplus
}
#endif

#endif