#include "ssd1363_framebuffer.h"

#include <string.h>

#include "ssd1363_basic.h"

static bool ssd1363_framebuffer_pixel_in_bounds(uint16_t x, uint16_t y)
{
	return (x < SSD1363_FRAMEBUFFER_WIDTH) && (y < SSD1363_FRAMEBUFFER_HEIGHT);
}

static size_t ssd1363_framebuffer_pixel_index(uint16_t x, uint16_t y)
{
	return ((size_t)y * (SSD1363_FRAMEBUFFER_WIDTH / 2U)) + (x / 2U);
}

void ssd1363_framebuffer_init(ssd1363_framebuffer_t *framebuffer)
{
	if (framebuffer == NULL) {
		return;
	}

	ssd1363_framebuffer_clear(framebuffer);
}

void ssd1363_framebuffer_clear(ssd1363_framebuffer_t *framebuffer)
{
	if (framebuffer == NULL) {
		return;
	}

	memset(framebuffer->data, 0x00, sizeof(framebuffer->data));
}

void ssd1363_framebuffer_fill(ssd1363_framebuffer_t *framebuffer, uint8_t gray4)
{
	uint8_t packed_gray;

	if (framebuffer == NULL) {
		return;
	}

	gray4 &= 0x0FU;
	packed_gray = (uint8_t)((gray4 << 4) | gray4);
	memset(framebuffer->data, packed_gray, sizeof(framebuffer->data));
}

esp_err_t ssd1363_framebuffer_set_pixel(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint8_t gray4)
{
	size_t index;
	uint8_t value;

	if (framebuffer == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	if (!ssd1363_framebuffer_pixel_in_bounds(x, y) || gray4 > 0x0FU) {
		return ESP_ERR_INVALID_ARG;
	}

	index = ssd1363_framebuffer_pixel_index(x, y);
	value = framebuffer->data[index];

	if ((x & 1U) == 0U) {
		framebuffer->data[index] = (uint8_t)((value & 0x0FU) | (gray4 << 4));
	} else {
		framebuffer->data[index] = (uint8_t)((value & 0xF0U) | gray4);
	}

	return ESP_OK;
}

esp_err_t ssd1363_framebuffer_get_pixel(const ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint8_t *gray4)
{
	size_t index;
	uint8_t value;

	if (framebuffer == NULL || gray4 == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	if (!ssd1363_framebuffer_pixel_in_bounds(x, y)) {
		return ESP_ERR_INVALID_ARG;
	}

	index = ssd1363_framebuffer_pixel_index(x, y);
	value = framebuffer->data[index];

	if ((x & 1U) == 0U) {
		*gray4 = (uint8_t)((value >> 4) & 0x0FU);
	} else {
		*gray4 = (uint8_t)(value & 0x0FU);
	}

	return ESP_OK;
}

esp_err_t ssd1363_framebuffer_fill_rect(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t gray4)
{
	uint16_t x_end;
	uint16_t y_end;

	if (framebuffer == NULL || width == 0U || height == 0U || gray4 > 0x0FU) {
		return ESP_ERR_INVALID_ARG;
	}

	if (x >= SSD1363_FRAMEBUFFER_WIDTH || y >= SSD1363_FRAMEBUFFER_HEIGHT) {
		return ESP_ERR_INVALID_ARG;
	}

	x_end = (uint16_t)(x + width);
	y_end = (uint16_t)(y + height);
	if (x_end > SSD1363_FRAMEBUFFER_WIDTH || y_end > SSD1363_FRAMEBUFFER_HEIGHT) {
		return ESP_ERR_INVALID_ARG;
	}

	for (uint16_t row = y; row < y_end; ++row) {
		for (uint16_t column = x; column < x_end; ++column) {
			ssd1363_framebuffer_set_pixel(framebuffer, column, row, gray4);
		}
	}

	return ESP_OK;
}

esp_err_t ssd1363_framebuffer_draw_rect(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t gray4)
{
	esp_err_t err;

	if (framebuffer == NULL || width == 0U || height == 0U || gray4 > 0x0FU) {
		return ESP_ERR_INVALID_ARG;
	}

	err = ssd1363_framebuffer_fill_rect(framebuffer, x, y, width, 1U, gray4);
	if (err != ESP_OK) {
		return err;
	}

	if (height > 1U) {
		err = ssd1363_framebuffer_fill_rect(framebuffer, x, (uint16_t)(y + height - 1U), width, 1U, gray4);
		if (err != ESP_OK) {
			return err;
		}
	}

	if (height > 2U) {
		err = ssd1363_framebuffer_fill_rect(framebuffer, x, (uint16_t)(y + 1U), 1U, (uint16_t)(height - 2U), gray4);
		if (err != ESP_OK) {
			return err;
		}

		if (width > 1U) {
			return ssd1363_framebuffer_fill_rect(framebuffer, (uint16_t)(x + width - 1U), (uint16_t)(y + 1U), 1U, (uint16_t)(height - 2U), gray4);
		}
	}

	return ESP_OK;
}

esp_err_t ssd1363_framebuffer_flush(const ssd1363_framebuffer_t *framebuffer)
{
	if (framebuffer == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	return ssd1363_basic_write_buffer(framebuffer->data, sizeof(framebuffer->data));
}