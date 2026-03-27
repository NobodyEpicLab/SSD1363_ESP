#ifndef SSD1363_BASIC_H
#define SSD1363_BASIC_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "ssd1363_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* Initialization                                                              */
/* -------------------------------------------------------------------------- */

/*
 * Initialize the SSD1363 driver stack and panel.
 *
 * What it does:
 * - Initializes the selected transport bus
 * - Runs the panel initialization sequence
 * - Sets the active drawing window to the full screen
 *
 * Arguments:
 * - bus: transport backend to use
 *   - valid values: SSD1363_BUS_I2C or SSD1363_BUS_SPI
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if initialization fails
 */
esp_err_t ssd1363_basic_init(ssd1363_bus_t bus);

/*
 * Deinitialize the currently active transport backend.
 *
 * Arguments:
 * - none
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if deinitialization fails
 */
esp_err_t ssd1363_basic_deinit(void);

/* -------------------------------------------------------------------------- */
/* Display Control                                                             */
/* -------------------------------------------------------------------------- */

/*
 * Turn panel output on.
 *
 * Arguments:
 * - none
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_display_on(void);

/*
 * Turn panel output off.
 *
 * Arguments:
 * - none
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_display_off(void);

/*
 * Restore the normal SSD1363 display mode.
 *
 * What it does:
 * - Sends the command that disables inverse / non-normal display mode
 *
 * Arguments:
 * - none
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_display_mode_normal(void);

/*
 * Clear the whole active screen to black.
 *
 * Arguments:
 * - none
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the write fails
 */
esp_err_t ssd1363_basic_clear(void);

/*
 * Fill the whole active screen with a raw packed 4bpp byte pattern.
 *
 * What it does:
 * - Writes the same byte repeatedly to the full active area
 * - Each byte contains two grayscale pixels
 *
 * Arguments:
 * - pattern: raw packed pixel byte
 *   - valid range: 0x00..0xFF
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the write fails
 */
esp_err_t ssd1363_basic_fill(uint8_t pattern);

/* -------------------------------------------------------------------------- */
/* Panel Tuning                                                                */
/* -------------------------------------------------------------------------- */

/*
 * Set SSD1363 contrast current using command C1h.
 *
 * Arguments:
 * - contrast: panel contrast current value
 *   - valid range: 0x00..0xFF
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_contrast(uint8_t contrast);

/*
 * Set SSD1363 master contrast using command C7h.
 *
 * Arguments:
 * - contrast: master contrast level
 *   - valid range: 0x00..0x0F
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_master_contrast(uint8_t contrast);

/*
 * Set the display start line.
 *
 * Arguments:
 * - line: first display line mapped to row 0
 *   - valid range for a 128-row panel: 0x00..0x7F
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_start_line(uint8_t line);

/*
 * Set the display offset.
 *
 * Arguments:
 * - offset: COM output shift value
 *   - valid range for a 128-row panel: 0x00..0x7F
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_display_offset(uint8_t offset);

/*
 * Set the panel remap configuration bytes.
 *
 * Arguments:
 * - remap_byte0: first SSD1363 remap register byte
 *   - valid range: 0x00..0xFF
 * - remap_byte1: second SSD1363 remap register byte
 *   - valid range: 0x00..0xFF
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_remap(uint8_t remap_byte0, uint8_t remap_byte1);

/*
 * Select the controller's default grayscale table.
 *
 * Arguments:
 * - none
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_use_default_gray_table(void);

/*
 * Set the display clock divider / oscillator register.
 *
 * Arguments:
 * - clock_setting: raw SSD1363 clock register value
 *   - valid range: 0x00..0xFF
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_display_clock(uint8_t clock_setting);

/*
 * Set multiplex ratio.
 *
 * Arguments:
 * - ratio: COM multiplex ratio register value
 *   - valid range for a 128-row panel: 0x00..0x7F
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG if ratio is outside the active height range
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_multiplex_ratio(uint8_t ratio);

/*
 * Set phase length.
 *
 * Arguments:
 * - phase_length: raw SSD1363 phase length register value
 *   - valid range: 0x00..0xFF
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_phase_length(uint8_t phase_length);

/*
 * Set precharge voltage.
 *
 * Arguments:
 * - voltage: raw SSD1363 precharge voltage register value
 *   - valid range: 0x00..0xFF
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_precharge_voltage(uint8_t voltage);

/*
 * Set second precharge period.
 *
 * Arguments:
 * - period: raw SSD1363 second-precharge register value
 *   - valid range: 0x00..0xFF
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_second_precharge(uint8_t period);

/*
 * Set VCOMH level.
 *
 * Arguments:
 * - level: raw SSD1363 VCOMH register value
 *   - valid range: 0x00..0xFF
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_vcomh(uint8_t level);

/*
 * Set the SSD1363 segment low-voltage register pair.
 *
 * Arguments:
 * - level0: first raw segment low-voltage register byte
 *   - valid range: 0x00..0xFF
 * - level1: second raw segment low-voltage register byte
 *   - valid range: 0x00..0xFF
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_segment_low_voltage(uint8_t level0, uint8_t level1);

/*
 * Set the SSD1363 display-enhancement register pair.
 *
 * Arguments:
 * - value0: first raw display-enhancement register byte
 *   - valid range: 0x00..0xFF
 * - value1: second raw display-enhancement register byte
 *   - valid range: 0x00..0xFF
 *
 * Returns:
 * - ESP_OK on success
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_display_enhancement(uint8_t value0, uint8_t value1);

/* -------------------------------------------------------------------------- */
/* Buffer Writes                                                               */
/* -------------------------------------------------------------------------- */

/*
 * Set the active drawing window using pixel coordinates.
 *
 * What it does:
 * - Converts pixel coordinates into SSD1363 column-address units
 * - Programs the controller window directly
 *
 * Arguments:
 * - x: left pixel coordinate
 *   - valid range: 0..SSD1363_ACTIVE_WIDTH-1
 *   - alignment: must be a multiple of 4
 * - y: top pixel coordinate
 *   - valid range: 0..SSD1363_ACTIVE_HEIGHT-1
 * - width: window width in pixels
 *   - valid range: 4..SSD1363_ACTIVE_WIDTH
 *   - alignment: must be a multiple of 4
 * - height: window height in pixels
 *   - valid range: 1..SSD1363_ACTIVE_HEIGHT
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid coordinates or alignment
 * - an ESP-IDF error code if the command fails
 */
esp_err_t ssd1363_basic_set_window_pixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/*
 * Write a full packed 4bpp framebuffer to the display.
 *
 * What it does:
 * - Encodes one logical row at a time into the panel's GDDRAM byte order
 * - Writes the full active area
 *
 * Arguments:
 * - buffer: pointer to packed framebuffer data
 *   - must not be NULL
 * - len: size of buffer in bytes
 *   - required value: (SSD1363_ACTIVE_WIDTH * SSD1363_ACTIVE_HEIGHT) / 2
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid buffer or length
 * - an ESP-IDF error code if the write fails
 */
esp_err_t ssd1363_basic_write_buffer(const uint8_t *buffer, size_t len);

/*
 * Write a rectangular packed 4bpp area to the display.
 *
 * What it does:
 * - Encodes each logical row into the panel's GDDRAM byte order
 * - Writes only the requested SSD1363-aligned area
 *
 * Arguments:
 * - x: left pixel coordinate
 *   - valid range: 0..SSD1363_ACTIVE_WIDTH-1
 *   - alignment: must be a multiple of 4
 * - y: top pixel coordinate
 *   - valid range: 0..SSD1363_ACTIVE_HEIGHT-1
 * - width: area width in pixels
 *   - valid range: 4..SSD1363_ACTIVE_WIDTH
 *   - alignment: must be a multiple of 4
 * - height: area height in pixels
 *   - valid range: 1..SSD1363_ACTIVE_HEIGHT
 * - buffer: pointer to packed 4bpp area data
 *   - must not be NULL
 * - len: size of buffer in bytes
 *   - required value: (width * height) / 2
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid coordinates, alignment, buffer, or length
 * - an ESP-IDF error code if the write fails
 */
esp_err_t ssd1363_basic_write_area(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *buffer, size_t len);

/*
 * Write a rectangular packed 4bpp area to the display using pixel coordinates.
 *
 * What it does:
 * - Uses the same packed buffer format as the framebuffer module
 * - Converts pixel coordinates into controller column units internally
 *
 * Arguments:
 * - x: left pixel coordinate
 *   - valid range: 0..SSD1363_ACTIVE_WIDTH-1
 *   - alignment: must be a multiple of 4
 * - y: top pixel coordinate
 *   - valid range: 0..SSD1363_ACTIVE_HEIGHT-1
 * - width: area width in pixels
 *   - valid range: 4..SSD1363_ACTIVE_WIDTH
 *   - alignment: must be a multiple of 4
 * - height: area height in pixels
 *   - valid range: 1..SSD1363_ACTIVE_HEIGHT
 * - buffer: pointer to packed 4bpp area data
 *   - must not be NULL
 * - len: size of buffer in bytes
 *   - required value: (width * height) / 2
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid coordinates, alignment, buffer, or length
 * - an ESP-IDF error code if the write fails
 */
esp_err_t ssd1363_basic_write_rect_pixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *buffer, size_t len);

#ifdef __cplusplus
}
#endif

#endif