#include "ssd1363_basic.h"

#include <stddef.h>
#include <string.h>

#include "ssd1363_api.h"
#include "ssd1363_config.h"

#define SSD1363_BASIC_FULL_BUFFER_SIZE ((SSD1363_ACTIVE_WIDTH * SSD1363_ACTIVE_HEIGHT) / 2U)

/*
 * Convert one logical framebuffer row into the byte order expected by this
 * panel's SSD1363 GDDRAM interface. This is a serialization step, not a pixel
 * position adjustment.
 */
static void ssd1363_basic_encode_row_for_gddram(uint8_t *dest, const uint8_t *src, size_t row_len)
{

#if SSD1363_PANEL_SWAP_4PX_GROUP_BYTES
	for (size_t offset = 0; offset < row_len; offset += 2U) {
		dest[offset] = src[offset + 1U];
		dest[offset + 1U] = src[offset];
	}
#else
	memcpy(dest, src, row_len);
#endif
}

static bool ssd1363_basic_area_is_valid(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	if (width == 0U || height == 0U) {
		return false;
	}

	if (x >= SSD1363_ACTIVE_WIDTH || y >= SSD1363_ACTIVE_HEIGHT) {
		return false;
	}

	if ((x + width) > SSD1363_ACTIVE_WIDTH || (y + height) > SSD1363_ACTIVE_HEIGHT) {
		return false;
	}

	if ((x % SSD1363_PANEL_COLUMN_ADDR_UNIT_PIXELS) != 0U ||
		(width % SSD1363_PANEL_COLUMN_ADDR_UNIT_PIXELS) != 0U) {
		return false;
	}

	return true;
}

static esp_err_t ssd1363_basic_set_full_window(void)
{
	return ssd1363_api_set_window(
		0,
		(SSD1363_ACTIVE_WIDTH / SSD1363_PANEL_COLUMN_ADDR_UNIT_PIXELS) - 1U,
		0,
		SSD1363_ACTIVE_HEIGHT - 1U
	);
}

static esp_err_t ssd1363_basic_set_window_pixels_internal(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	const uint8_t start_column = (uint8_t)(x / SSD1363_PANEL_COLUMN_ADDR_UNIT_PIXELS);
	const uint8_t end_column = (uint8_t)(((x + width) / SSD1363_PANEL_COLUMN_ADDR_UNIT_PIXELS) - 1U);
	const uint8_t start_row = (uint8_t)y;
	const uint8_t end_row = (uint8_t)(y + height - 1U);

	if (!ssd1363_basic_area_is_valid(x, y, width, height)) {
		return ESP_ERR_INVALID_ARG;
	}

	return ssd1363_api_set_window(start_column, end_column, start_row, end_row);
}

/*
 * Initialize the selected bus, run the panel init sequence, and select the
 * full-screen drawing window.
 */
esp_err_t ssd1363_basic_init(ssd1363_bus_t bus)
{
	esp_err_t err = ssd1363_api_init(bus);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_api_panel_init();
	if (err != ESP_OK) {
		return err;
	}

	return ssd1363_basic_set_full_window();
}

/* Deinitialize the active transport backend. */
esp_err_t ssd1363_basic_deinit(void)
{
	return ssd1363_interface_deinit();
}

/* Enable visible panel output. */
esp_err_t ssd1363_basic_display_on(void)
{
	return ssd1363_api_display_on();
}

/* Disable visible panel output. */
esp_err_t ssd1363_basic_display_off(void)
{
	return ssd1363_api_display_off();
}

/* Restore normal display mode. */
esp_err_t ssd1363_basic_set_display_mode_normal(void)
{
	return ssd1363_api_set_display_mode_normal();
}

/* Clear the active area to black. */
esp_err_t ssd1363_basic_clear(void)
{
	return ssd1363_basic_fill(0x00);
}

/* Fill the active area with a raw packed-byte pattern. */
esp_err_t ssd1363_basic_fill(uint8_t pattern)
{
	return ssd1363_api_fill_active_area(pattern, SSD1363_ACTIVE_WIDTH, SSD1363_ACTIVE_HEIGHT);
}

/* Set panel contrast current using command C1h. */
esp_err_t ssd1363_basic_set_contrast(uint8_t contrast)
{
	return ssd1363_api_set_contrast(contrast);
}

/* Set panel master contrast using command C7h. */
esp_err_t ssd1363_basic_set_master_contrast(uint8_t contrast)
{
	if (contrast > 0x0FU) {
		return ESP_ERR_INVALID_ARG;
	}

	return ssd1363_api_set_master_contrast(contrast);
}

/* Set the panel start line. */
esp_err_t ssd1363_basic_set_start_line(uint8_t line)
{
	if (line >= SSD1363_ACTIVE_HEIGHT) {
		return ESP_ERR_INVALID_ARG;
	}

	return ssd1363_api_set_start_line(line);
}

/* Set the panel display offset. */
esp_err_t ssd1363_basic_set_display_offset(uint8_t offset)
{
	if (offset >= SSD1363_ACTIVE_HEIGHT) {
		return ESP_ERR_INVALID_ARG;
	}

	return ssd1363_api_set_display_offset(offset);
}

/* Set the raw SSD1363 remap register bytes. */
esp_err_t ssd1363_basic_set_remap(uint8_t remap_byte0, uint8_t remap_byte1)
{
	return ssd1363_api_set_remap(remap_byte0, remap_byte1);
}

/* Select the controller's default grayscale table. */
esp_err_t ssd1363_basic_use_default_gray_table(void)
{
	return ssd1363_api_use_default_gray_table();
}

/* Set the raw SSD1363 display clock register. */
esp_err_t ssd1363_basic_set_display_clock(uint8_t clock_setting)
{
	return ssd1363_api_set_display_clock(clock_setting);
}

/* Set the panel multiplex ratio. */
esp_err_t ssd1363_basic_set_multiplex_ratio(uint8_t ratio)
{
	if (ratio >= SSD1363_ACTIVE_HEIGHT) {
		return ESP_ERR_INVALID_ARG;
	}

	return ssd1363_api_set_multiplex_ratio(ratio);
}

/* Set the SSD1363 phase length register. */
esp_err_t ssd1363_basic_set_phase_length(uint8_t phase_length)
{
	return ssd1363_api_set_phase_length(phase_length);
}

/* Set the SSD1363 precharge voltage register. */
esp_err_t ssd1363_basic_set_precharge_voltage(uint8_t voltage)
{
	return ssd1363_api_set_precharge_voltage(voltage);
}

/* Set the SSD1363 second-precharge period register. */
esp_err_t ssd1363_basic_set_second_precharge(uint8_t period)
{
	return ssd1363_api_set_second_precharge(period);
}

/* Set the SSD1363 VCOMH register. */
esp_err_t ssd1363_basic_set_vcomh(uint8_t level)
{
	return ssd1363_api_set_vcomh(level);
}

/* Set the SSD1363 segment low-voltage register pair. */
esp_err_t ssd1363_basic_set_segment_low_voltage(uint8_t level0, uint8_t level1)
{
	return ssd1363_api_set_segment_low_voltage(level0, level1);
}

/* Set the SSD1363 display-enhancement register pair. */
esp_err_t ssd1363_basic_set_display_enhancement(uint8_t value0, uint8_t value1)
{
	return ssd1363_api_set_display_enhancement(value0, value1);
}

/* Set the active drawing window using pixel coordinates. */
esp_err_t ssd1363_basic_set_window_pixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	return ssd1363_basic_set_window_pixels_internal(x, y, width, height);
}

/* Write a full logical framebuffer to the panel. */
esp_err_t ssd1363_basic_write_buffer(const uint8_t *buffer, size_t len)
{
	uint8_t row_buffer[SSD1363_ACTIVE_WIDTH / 2U];
	const size_t row_len = SSD1363_ACTIVE_WIDTH / 2U;
	const size_t row_count = SSD1363_ACTIVE_HEIGHT;

	if (buffer == NULL || len != SSD1363_BASIC_FULL_BUFFER_SIZE) {
		return ESP_ERR_INVALID_ARG;
	}

	esp_err_t err = ssd1363_basic_set_full_window();
	if (err != ESP_OK) {
		return err;
	}

	for (size_t row = 0; row < row_count; ++row) {
		const uint8_t *src = buffer + (row * row_len);
		ssd1363_basic_encode_row_for_gddram(row_buffer, src, row_len);

		err = ssd1363_api_send_buffer(row_buffer, row_len);
		if (err != ESP_OK) {
			return err;
		}
	}

	return ESP_OK;
}

/* Write an SSD1363-aligned logical framebuffer rectangle to the panel. */
esp_err_t ssd1363_basic_write_area(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *buffer, size_t len)
{
	const size_t expected_len = ((size_t)width * (size_t)height) / 2U;
	const size_t row_len = width / 2U;
	uint8_t row_buffer[SSD1363_ACTIVE_WIDTH / 2U];

	if (buffer == NULL || len != expected_len || !ssd1363_basic_area_is_valid(x, y, width, height)) {
		return ESP_ERR_INVALID_ARG;
	}

	esp_err_t err = ssd1363_basic_set_window_pixels_internal(x, y, width, height);
	if (err != ESP_OK) {
		return err;
	}

	for (uint16_t row = 0; row < height; ++row) {
		const uint8_t *src = buffer + ((size_t)row * row_len);
		ssd1363_basic_encode_row_for_gddram(row_buffer, src, row_len);

		err = ssd1363_api_send_buffer(row_buffer, row_len);
		if (err != ESP_OK) {
			return err;
		}
	}

	return ESP_OK;
}

/* Write a pixel-addressed packed 4bpp rectangle to the panel. */
esp_err_t ssd1363_basic_write_rect_pixels(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *buffer, size_t len)
{
	return ssd1363_basic_write_area(x, y, width, height, buffer, len);
}