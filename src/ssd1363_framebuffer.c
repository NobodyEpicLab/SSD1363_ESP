#include "ssd1363_framebuffer.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ssd1363_basic.h"

#define SSD1363_FRAMEBUFFER_ROW_BYTES      (SSD1363_FRAMEBUFFER_WIDTH / 2U)

static bool ssd1363_framebuffer_rect_in_bounds(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	if (width == 0U || height == 0U) {
		return false;
	}

	if (x >= SSD1363_FRAMEBUFFER_WIDTH || y >= SSD1363_FRAMEBUFFER_HEIGHT) {
		return false;
	}

	if ((x + width) > SSD1363_FRAMEBUFFER_WIDTH || (y + height) > SSD1363_FRAMEBUFFER_HEIGHT) {
		return false;
	}

	return true;
}

static bool ssd1363_framebuffer_pixel_in_bounds(uint16_t x, uint16_t y)
{
	return (x < SSD1363_FRAMEBUFFER_WIDTH) && (y < SSD1363_FRAMEBUFFER_HEIGHT);
}

static size_t ssd1363_framebuffer_pixel_index(uint16_t x, uint16_t y)
{
	return ((size_t)y * SSD1363_FRAMEBUFFER_ROW_BYTES) + (x / 2U);
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
		#if SSD1363_GDDRAM_EVEN_PIXEL_HIGH_NIBBLE
		framebuffer->data[index] = (uint8_t)((value & 0x0FU) | (gray4 << 4));
		#else
		framebuffer->data[index] = (uint8_t)((value & 0xF0U) | gray4);
		#endif
	} else {
		#if SSD1363_GDDRAM_EVEN_PIXEL_HIGH_NIBBLE
		framebuffer->data[index] = (uint8_t)((value & 0xF0U) | gray4);
		#else
		framebuffer->data[index] = (uint8_t)((value & 0x0FU) | (gray4 << 4));
		#endif
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
		#if SSD1363_GDDRAM_EVEN_PIXEL_HIGH_NIBBLE
		*gray4 = (uint8_t)((value >> 4) & 0x0FU);
		#else
		*gray4 = (uint8_t)(value & 0x0FU);
		#endif
	} else {
		#if SSD1363_GDDRAM_EVEN_PIXEL_HIGH_NIBBLE
		*gray4 = (uint8_t)(value & 0x0FU);
		#else
		*gray4 = (uint8_t)((value >> 4) & 0x0FU);
		#endif
	}

	return ESP_OK;
}

esp_err_t ssd1363_framebuffer_fill_rect(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t gray4)
{
	uint16_t x_end;
	uint16_t y_end;

	if (framebuffer == NULL || gray4 > 0x0FU) {
		return ESP_ERR_INVALID_ARG;
	}

	if (!ssd1363_framebuffer_rect_in_bounds(x, y, width, height)) {
		return ESP_ERR_INVALID_ARG;
	}

	x_end = (uint16_t)(x + width);
	y_end = (uint16_t)(y + height);

	for (uint16_t row = y; row < y_end; ++row) {
		for (uint16_t column = x; column < x_end; ++column) {
			ssd1363_framebuffer_set_pixel(framebuffer, column, row, gray4);
		}
	}

	return ESP_OK;
}

esp_err_t ssd1363_framebuffer_draw_hline(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint16_t width, uint8_t gray4)
{
	return ssd1363_framebuffer_fill_rect(framebuffer, x, y, width, 1U, gray4);
}

esp_err_t ssd1363_framebuffer_draw_vline(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint16_t height, uint8_t gray4)
{
	return ssd1363_framebuffer_fill_rect(framebuffer, x, y, 1U, height, gray4);
}

esp_err_t ssd1363_framebuffer_draw_line(ssd1363_framebuffer_t *framebuffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t gray4)
{
	int x = (int)x0;
	int y = (int)y0;
	const int target_x = (int)x1;
	const int target_y = (int)y1;
	const int delta_x = abs(target_x - x);
	const int step_x = (x < target_x) ? 1 : -1;
	const int delta_y = -abs(target_y - y);
	const int step_y = (y < target_y) ? 1 : -1;
	int error;

	if (framebuffer == NULL || gray4 > 0x0FU) {
		return ESP_ERR_INVALID_ARG;
	}

	if (!ssd1363_framebuffer_pixel_in_bounds(x0, y0) || !ssd1363_framebuffer_pixel_in_bounds(x1, y1)) {
		return ESP_ERR_INVALID_ARG;
	}

	error = delta_x + delta_y;

	while (true) {
		ssd1363_framebuffer_set_pixel(framebuffer, (uint16_t)x, (uint16_t)y, gray4);

		if (x == target_x && y == target_y) {
			break;
		}

		const int doubled_error = error * 2;
		if (doubled_error >= delta_y) {
			error += delta_y;
			x += step_x;
		}

		if (doubled_error <= delta_x) {
			error += delta_x;
			y += step_y;
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

esp_err_t ssd1363_framebuffer_draw_bitmap_1bpp(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *bitmap, uint8_t foreground_gray4, uint8_t background_gray4, ssd1363_framebuffer_bitmap_mode_t mode)
{
	const uint16_t bitmap_stride = (uint16_t)((width + 7U) / 8U);

	if (framebuffer == NULL || bitmap == NULL || foreground_gray4 > 0x0FU || background_gray4 > 0x0FU) {
		return ESP_ERR_INVALID_ARG;
	}

	if (!ssd1363_framebuffer_rect_in_bounds(x, y, width, height)) {
		return ESP_ERR_INVALID_ARG;
	}

	for (uint16_t row = 0; row < height; ++row) {
		for (uint16_t column = 0; column < width; ++column) {
			const size_t byte_index = ((size_t)row * bitmap_stride) + (column / 8U);
			const uint8_t mask = (uint8_t)(0x80U >> (column % 8U));
			const bool pixel_on = (bitmap[byte_index] & mask) != 0U;

			if (pixel_on) {
				ssd1363_framebuffer_set_pixel(framebuffer, (uint16_t)(x + column), (uint16_t)(y + row), foreground_gray4);
			} else if (mode == SSD1363_FRAMEBUFFER_BITMAP_OPAQUE) {
				ssd1363_framebuffer_set_pixel(framebuffer, (uint16_t)(x + column), (uint16_t)(y + row), background_gray4);
			}
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

esp_err_t ssd1363_framebuffer_flush_rect(const ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	uint16_t aligned_x;
	uint16_t aligned_width;
	uint16_t x_end;
	uint8_t row_buffer[SSD1363_FRAMEBUFFER_ROW_BYTES];

	if (framebuffer == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	if (!ssd1363_framebuffer_rect_in_bounds(x, y, width, height)) {
		return ESP_ERR_INVALID_ARG;
	}

	aligned_x = (uint16_t)(x & ~0x3U);
	x_end = (uint16_t)(((x + width + 3U) / 4U) * 4U);
	if (x_end > SSD1363_FRAMEBUFFER_WIDTH) {
		x_end = SSD1363_FRAMEBUFFER_WIDTH;
	}
	aligned_width = (uint16_t)(x_end - aligned_x);

	for (uint16_t row = 0; row < height; ++row) {
		const size_t source_offset = ((size_t)(y + row) * SSD1363_FRAMEBUFFER_ROW_BYTES) + (aligned_x / 2U);
		const size_t row_len = aligned_width / 2U;

		memcpy(row_buffer, &framebuffer->data[source_offset], row_len);

		esp_err_t err = ssd1363_basic_write_area(aligned_x, (uint16_t)(y + row), aligned_width, 1U, row_buffer, row_len);
		if (err != ESP_OK) {
			return err;
		}
	}

	return ESP_OK;
}