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
#define SSD1363_CMD_EXIT_PARTIAL_DISPLAY      0xA9
#define SSD1363_CMD_DISPLAY_OFF               0xAE
#define SSD1363_CMD_DISPLAY_ON                0xAF
#define SSD1363_CMD_FUNCTION_SELECTION        0xAB
#define SSD1363_CMD_SET_PHASE_LENGTH          0xB1
#define SSD1363_CMD_SET_CLOCK_DIVIDER         0xB3
#define SSD1363_CMD_SET_SEGMENT_LOW_VOLTAGE   0xB4
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

/* -------------------------------------------------------------------------- */
/* Initialization                                                              */
/* -------------------------------------------------------------------------- */

/*
 * Initialize the selected SSD1363 transport backend.
 *
 * Arguments:
 * - bus: transport backend to use
 *   - valid values: SSD1363_BUS_I2C or SSD1363_BUS_SPI
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if transport initialization fails
 */
esp_err_t ssd1363_api_init(ssd1363_bus_t bus);

/*
 * Run the SSD1363 panel initialization sequence currently used by this project.
 *
 * Arguments:
 * - none
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if any initialization command fails
 */
esp_err_t ssd1363_api_panel_init(void);

/*
 * Unlock the SSD1363 command interface.
 *
 * Arguments:
 * - unlock_code: command lock value accepted by the controller
 *   - valid range: 0x00..0xFF
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_api_unlock_command_interface(uint8_t unlock_code);

/* -------------------------------------------------------------------------- */
/* Display Mode / Power                                                        */
/* -------------------------------------------------------------------------- */

/* Turn panel output off. */
esp_err_t ssd1363_api_display_off(void);

/* Turn panel output on. */
esp_err_t ssd1363_api_display_on(void);

/* Exit partial display mode. */
esp_err_t ssd1363_api_exit_partial_display(void);

/* Restore normal display mode. */
esp_err_t ssd1363_api_set_display_mode_normal(void);

/* -------------------------------------------------------------------------- */
/* Timing / Mapping / Basic Configuration                                      */
/* -------------------------------------------------------------------------- */

/*
 * Set the display clock divider / oscillator register.
 *
 * Arguments:
 * - clock_setting: raw SSD1363 register value
 *   - valid range: 0x00..0xFF
 */
esp_err_t ssd1363_api_set_display_clock(uint8_t clock_setting);

/*
 * Set the function selection register.
 *
 * Arguments:
 * - function_selection: raw SSD1363 register value
 *   - valid range: 0x00..0xFF
 */
esp_err_t ssd1363_api_set_function_selection(uint8_t function_selection);

/*
 * Set SSD1363 GPIO configuration.
 *
 * Arguments:
 * - gpio_config: raw SSD1363 register value
 *   - valid range: 0x00..0xFF
 */
esp_err_t ssd1363_api_set_gpio(uint8_t gpio_config);

/*
 * Set multiplex ratio.
 *
 * Arguments:
 * - ratio: COM multiplex ratio register value
 *   - valid range for a 128-row panel: 0x00..0x7F
 */
esp_err_t ssd1363_api_set_multiplex_ratio(uint8_t ratio);

/*
 * Set display offset.
 *
 * Arguments:
 * - offset: COM display offset register value
 *   - valid range for a 128-row panel: 0x00..0x7F
 */
esp_err_t ssd1363_api_set_display_offset(uint8_t offset);

/*
 * Set start line.
 *
 * Arguments:
 * - line: start line register value
 *   - valid range for a 128-row panel: 0x00..0x7F
 */
esp_err_t ssd1363_api_set_start_line(uint8_t line);

/*
 * Set both SSD1363 remap bytes.
 *
 * Arguments:
 * - remap_byte0: first remap byte, raw register value, 0x00..0xFF
 * - remap_byte1: second remap byte, raw register value, 0x00..0xFF
 */
esp_err_t ssd1363_api_set_remap(uint8_t remap_byte0, uint8_t remap_byte1);

/*
 * Set segment low voltage values.
 *
 * Arguments:
 * - level0: first raw register byte, 0x00..0xFF
 * - level1: second raw register byte, 0x00..0xFF
 */
esp_err_t ssd1363_api_set_segment_low_voltage(uint8_t level0, uint8_t level1);

/* -------------------------------------------------------------------------- */
/* Grayscale / Analog Tuning                                                   */
/* -------------------------------------------------------------------------- */

/*
 * Set contrast current using command C1h.
 *
 * Arguments:
 * - contrast: raw contrast value
 *   - valid range: 0x00..0xFF
 */
esp_err_t ssd1363_api_set_contrast(uint8_t contrast);

/*
 * Set master contrast using command C7h.
 *
 * Arguments:
 * - contrast: raw master contrast value
 *   - valid range: 0x00..0x0F on SSD1363
 */
esp_err_t ssd1363_api_set_master_contrast(uint8_t contrast);

/* Select the controller's default grayscale table. */
esp_err_t ssd1363_api_use_default_gray_table(void);

/*
 * Set phase length register.
 *
 * Arguments:
 * - phase_length: raw register value
 *   - valid range: 0x00..0xFF
 */
esp_err_t ssd1363_api_set_phase_length(uint8_t phase_length);

/*
 * Set display enhancement register pair.
 *
 * Arguments:
 * - value0: first raw register byte, 0x00..0xFF
 * - value1: second raw register byte, 0x00..0xFF
 */
esp_err_t ssd1363_api_set_display_enhancement(uint8_t value0, uint8_t value1);

/*
 * Set precharge voltage.
 *
 * Arguments:
 * - voltage: raw register value
 *   - valid range: 0x00..0xFF
 */
esp_err_t ssd1363_api_set_precharge_voltage(uint8_t voltage);

/*
 * Set second precharge period.
 *
 * Arguments:
 * - period: raw register value
 *   - valid range: 0x00..0xFF
 */
esp_err_t ssd1363_api_set_second_precharge(uint8_t period);

/*
 * Set VCOMH level.
 *
 * Arguments:
 * - level: raw register value
 *   - valid range: 0x00..0xFF
 */
esp_err_t ssd1363_api_set_vcomh(uint8_t level);

/* -------------------------------------------------------------------------- */
/* Raw Command / Data Access                                                   */
/* -------------------------------------------------------------------------- */

/*
 * Send a single SSD1363 command byte.
 *
 * Arguments:
 * - command: raw command value
 *   - valid range: 0x00..0xFF
 */
esp_err_t ssd1363_api_command(uint8_t command);

/*
 * Send a single SSD1363 data byte.
 *
 * Arguments:
 * - data: raw data value
 *   - valid range: 0x00..0xFF
 */
esp_err_t ssd1363_api_data(uint8_t data);

/*
 * Send a data buffer.
 *
 * Arguments:
 * - data: pointer to data bytes, must not be NULL when len > 0
 * - len: number of bytes to send, valid range: 0..SIZE_MAX
 */
esp_err_t ssd1363_api_data_buffer(const uint8_t *data, size_t len);

/* -------------------------------------------------------------------------- */
/* Addressing / Buffer Writes                                                  */
/* -------------------------------------------------------------------------- */

/*
 * Set column address window.
 *
 * Arguments:
 * - start_column: SSD1363 column address in controller units, 0x00..0xFF
 * - end_column: SSD1363 column address in controller units, 0x00..0xFF
 */
esp_err_t ssd1363_api_set_column_address(uint8_t start_column, uint8_t end_column);

/*
 * Set row address window.
 *
 * Arguments:
 * - start_row: row address, 0x00..0x7F for a 128-row panel
 * - end_row: row address, 0x00..0x7F for a 128-row panel
 */
esp_err_t ssd1363_api_set_row_address(uint8_t start_row, uint8_t end_row);

/*
 * Set both column and row address windows.
 *
 * Arguments:
 * - start_column: SSD1363 column address, 0x00..0xFF
 * - end_column: SSD1363 column address, 0x00..0xFF
 * - start_row: row address, 0x00..0x7F for a 128-row panel
 * - end_row: row address, 0x00..0x7F for a 128-row panel
 */
esp_err_t ssd1363_api_set_window(uint8_t start_column, uint8_t end_column, uint8_t start_row, uint8_t end_row);

/*
 * Send a packed framebuffer buffer to GDDRAM using the current window.
 *
 * Arguments:
 * - buffer: pointer to packed display data, must not be NULL when len > 0
 * - len: number of bytes to send, valid range: 0..SIZE_MAX
 */
esp_err_t ssd1363_api_send_buffer(const uint8_t *buffer, size_t len);

/*
 * Fill the active area with a raw packed-byte pattern.
 *
 * Arguments:
 * - pattern: raw packed pixel byte, 0x00..0xFF
 * - width: fill width in pixels, must be non-zero and a multiple of 4
 * - height: fill height in pixels, must be non-zero
 */
esp_err_t ssd1363_api_fill_active_area(uint8_t pattern, uint16_t width, uint16_t height);

#ifdef __cplusplus
}
#endif

#endif