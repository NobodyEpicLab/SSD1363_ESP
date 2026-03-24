#ifndef SSD1363_API_H
#define SSD1363_API_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "ssd1363_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SSD1363_CMD_SET_COLUMN_ADDR           0x15
#define SSD1363_CMD_WRITE_RAM                 0x5C
#define SSD1363_CMD_SET_ROW_ADDR              0x75
#define SSD1363_CMD_SET_REMAP                 0xA0
#define SSD1363_CMD_SET_START_LINE            0xA1
#define SSD1363_CMD_SET_DISPLAY_OFFSET        0xA2
#define SSD1363_CMD_SET_DISPLAY_MODE_NORMAL   0xA6
#define SSD1363_CMD_DISPLAY_OFF               0xAE
#define SSD1363_CMD_DISPLAY_ON                0xAF
#define SSD1363_CMD_FUNCTION_SELECTION        0xAB
#define SSD1363_CMD_SET_PHASE_LENGTH          0xB1
#define SSD1363_CMD_SET_CLOCK_DIVIDER         0xB3
#define SSD1363_CMD_SET_DISPLAY_ENHANCEMENT   0xD1
#define SSD1363_CMD_SET_GPIO                  0xB5
#define SSD1363_CMD_SET_SECOND_PRECHARGE      0xB6
#define SSD1363_CMD_USE_DEFAULT_GRAY_TABLE    0xB9
#define SSD1363_CMD_SET_PRECHARGE_VOLTAGE     0xBB
#define SSD1363_CMD_SET_VCOMH                 0xBE
#define SSD1363_CMD_SET_CONTRAST              0xC1
#define SSD1363_CMD_SET_MASTER_CONTRAST       0xC7
#define SSD1363_CMD_SET_MULTIPLEX_RATIO       0xCA
#define SSD1363_CMD_COMMAND_LOCK              0xFD

esp_err_t ssd1363_api_init(ssd1363_bus_t bus);
esp_err_t ssd1363_api_panel_init(void);
esp_err_t ssd1363_api_unlock_command_interface(uint8_t unlock_code);
esp_err_t ssd1363_api_display_off(void);
esp_err_t ssd1363_api_display_on(void);
esp_err_t ssd1363_api_set_multiplex_ratio(uint8_t ratio);
esp_err_t ssd1363_api_set_display_offset(uint8_t offset);
esp_err_t ssd1363_api_set_start_line(uint8_t line);
esp_err_t ssd1363_api_set_remap(uint8_t remap_byte0, uint8_t remap_byte1);
esp_err_t ssd1363_api_set_contrast(uint8_t contrast);
esp_err_t ssd1363_api_set_master_contrast(uint8_t contrast);
esp_err_t ssd1363_api_command(uint8_t command);
esp_err_t ssd1363_api_data(uint8_t data);
esp_err_t ssd1363_api_data_buffer(const uint8_t *data, size_t len);
esp_err_t ssd1363_api_set_column_address(uint8_t start_column, uint8_t end_column);
esp_err_t ssd1363_api_set_row_address(uint8_t start_row, uint8_t end_row);
esp_err_t ssd1363_api_set_window(uint8_t start_column, uint8_t end_column, uint8_t start_row, uint8_t end_row);
esp_err_t ssd1363_api_send_buffer(const uint8_t *buffer, size_t len);
esp_err_t ssd1363_api_fill_active_area(uint8_t pattern, uint16_t width, uint16_t height);

#ifdef __cplusplus
}
#endif

#endif