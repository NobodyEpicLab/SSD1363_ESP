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
#if SSD1363_GDDRAM_SWAP_4PX_GROUP_BYTES
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

	if ((x % 4U) != 0U || (width % 4U) != 0U) {
		return false;
	}

	return true;
}

static esp_err_t ssd1363_basic_set_full_window(void)
{
	return ssd1363_api_set_window(
		0,
		(SSD1363_ACTIVE_WIDTH / 4U) - 1U,
		0,
		SSD1363_ACTIVE_HEIGHT - 1U
	);
}

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

esp_err_t ssd1363_basic_deinit(void)
{
	return ssd1363_interface_deinit();
}

esp_err_t ssd1363_basic_display_on(void)
{
	return ssd1363_api_display_on();
}

esp_err_t ssd1363_basic_display_off(void)
{
	return ssd1363_api_display_off();
}

esp_err_t ssd1363_basic_clear(void)
{
	return ssd1363_basic_fill(0x00);
}

esp_err_t ssd1363_basic_fill(uint8_t pattern)
{
	return ssd1363_api_fill_active_area(pattern, SSD1363_ACTIVE_WIDTH, SSD1363_ACTIVE_HEIGHT);
}

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

esp_err_t ssd1363_basic_write_area(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *buffer, size_t len)
{
	const size_t expected_len = ((size_t)width * (size_t)height) / 2U;
	const size_t row_len = width / 2U;
	const uint8_t start_column = (uint8_t)(x / 4U);
	const uint8_t end_column = (uint8_t)(((x + width) / 4U) - 1U);
	const uint8_t start_row = (uint8_t)y;
	const uint8_t end_row = (uint8_t)(y + height - 1U);
	uint8_t row_buffer[SSD1363_ACTIVE_WIDTH / 2U];

	if (buffer == NULL || len != expected_len || !ssd1363_basic_area_is_valid(x, y, width, height)) {
		return ESP_ERR_INVALID_ARG;
	}

	esp_err_t err = ssd1363_api_set_window(start_column, end_column, start_row, end_row);
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