#include "ssd1363_framebuffer.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "ssd1363_basic.h"
#include "ssd1363_config.h"

#define SSD1363_FRAMEBUFFER_ROW_BYTES      (SSD1363_FRAMEBUFFER_WIDTH / 2U)

/* Validate that a rectangle stays inside the fixed framebuffer area. */
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

/* Validate a single pixel coordinate against framebuffer bounds. */
static bool ssd1363_framebuffer_pixel_in_bounds(uint16_t x, uint16_t y)
{
	return (x < SSD1363_FRAMEBUFFER_WIDTH) && (y < SSD1363_FRAMEBUFFER_HEIGHT);
}

/* Translate a pixel coordinate into the packed 4bpp byte index. */
static size_t ssd1363_framebuffer_pixel_index(uint16_t x, uint16_t y)
{
	return ((size_t)y * SSD1363_FRAMEBUFFER_ROW_BYTES) + (x / 2U);
}

/* Stamp a square brush around one pixel for thick-line drawing. */
static void ssd1363_framebuffer_stamp_brush(ssd1363_framebuffer_t *framebuffer, int center_x, int center_y, uint8_t gray4, uint16_t thickness)
{
	const int radius_before = (int)((thickness - 1U) / 2U);
	const int radius_after = (int)(thickness / 2U);

	for (int brush_y = center_y - radius_before; brush_y <= center_y + radius_after; ++brush_y) {
		for (int brush_x = center_x - radius_before; brush_x <= center_x + radius_after; ++brush_x) {
			if (brush_x >= 0 && brush_y >= 0 &&
				brush_x < (int)SSD1363_FRAMEBUFFER_WIDTH &&
				brush_y < (int)SSD1363_FRAMEBUFFER_HEIGHT) {
				ssd1363_framebuffer_set_pixel(framebuffer, (uint16_t)brush_x, (uint16_t)brush_y, gray4);
			}
		}
	}
}

/*
 * Initialize the framebuffer to a known cleared state.
 *
 * What it does:
 * - Validates the pointer
 * - Clears the whole local framebuffer to black
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 *
 * Returns:
 * - none
 */
void ssd1363_framebuffer_init(ssd1363_framebuffer_t *framebuffer)
{
	if (framebuffer == NULL) {
		return;
	}

	ssd1363_framebuffer_clear(framebuffer);
}

/*
 * Clear the whole framebuffer to black.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 *
 * Returns:
 * - none
 */
void ssd1363_framebuffer_clear(ssd1363_framebuffer_t *framebuffer)
{
	if (framebuffer == NULL) {
		return;
	}

	memset(framebuffer->data, 0x00, sizeof(framebuffer->data));
}

/*
 * Fill the whole framebuffer with one 4-bit grayscale value.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - gray4: 4-bit grayscale value
 *   - valid range: 0x00..0x0F
 *   - values above 0x0F are masked to the low 4 bits
 *
 * Returns:
 * - none
 */
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

/*
 * Write one logical pixel into the packed local framebuffer.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x: horizontal pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: vertical pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - gray4: 4-bit grayscale value
 *   - valid range: 0x00..0x0F
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer, coordinates, or gray value
 */
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
		#if SSD1363_PANEL_EVEN_PIXEL_HIGH_NIBBLE
		framebuffer->data[index] = (uint8_t)((value & 0x0FU) | (gray4 << 4));
		#else
		framebuffer->data[index] = (uint8_t)((value & 0xF0U) | gray4);
		#endif
	} else {
		#if SSD1363_PANEL_EVEN_PIXEL_HIGH_NIBBLE
		framebuffer->data[index] = (uint8_t)((value & 0xF0U) | gray4);
		#else
		framebuffer->data[index] = (uint8_t)((value & 0x0FU) | (gray4 << 4));
		#endif
	}

	return ESP_OK;
}

/*
 * Read one logical pixel from the packed local framebuffer.
 *
 * Arguments:
 * - framebuffer: source framebuffer object
 *   - must not be NULL
 * - x: horizontal pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: vertical pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - gray4: destination for the returned grayscale value
 *   - must not be NULL
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer, coordinates, or output pointer
 */
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
		#if SSD1363_PANEL_EVEN_PIXEL_HIGH_NIBBLE
		*gray4 = (uint8_t)((value >> 4) & 0x0FU);
		#else
		*gray4 = (uint8_t)(value & 0x0FU);
		#endif
	} else {
		#if SSD1363_PANEL_EVEN_PIXEL_HIGH_NIBBLE
		*gray4 = (uint8_t)(value & 0x0FU);
		#else
		*gray4 = (uint8_t)((value >> 4) & 0x0FU);
		#endif
	}

	return ESP_OK;
}

/*
 * Fill a solid rectangle in the local framebuffer.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x: left pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: top pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - width: rectangle width in pixels
 *   - valid range: 1..SSD1363_FRAMEBUFFER_WIDTH
 * - height: rectangle height in pixels
 *   - valid range: 1..SSD1363_FRAMEBUFFER_HEIGHT
 * - gray4: 4-bit grayscale value
 *   - valid range: 0x00..0x0F
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer, rectangle, or gray value
 */
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

/*
 * Draw a 1-pixel horizontal UI divider using menu-friendly naming.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x: left pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: row coordinate of the divider
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - width: divider width in pixels
 *   - valid range: 1..SSD1363_FRAMEBUFFER_WIDTH
 * - gray4: 4-bit grayscale value
 *   - valid range: 0x00..0x0F
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer, rectangle, or gray value
 */
esp_err_t ssd1363_framebuffer_draw_divider(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint16_t width, uint8_t gray4)
{
	return ssd1363_framebuffer_draw_hline(framebuffer, x, y, width, gray4);
}

/*
 * Draw a 1-pixel horizontal line.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x: left pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: row coordinate of the line
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - width: line width in pixels
 *   - valid range: 1..SSD1363_FRAMEBUFFER_WIDTH
 * - gray4: 4-bit grayscale value
 *   - valid range: 0x00..0x0F
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer, rectangle, or gray value
 */
esp_err_t ssd1363_framebuffer_draw_hline(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint16_t width, uint8_t gray4)
{
	return ssd1363_framebuffer_fill_rect(framebuffer, x, y, width, 1U, gray4);
}

/*
 * Draw a 1-pixel vertical line.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x: column coordinate of the line
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: top pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - height: line height in pixels
 *   - valid range: 1..SSD1363_FRAMEBUFFER_HEIGHT
 * - gray4: 4-bit grayscale value
 *   - valid range: 0x00..0x0F
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer, rectangle, or gray value
 */
esp_err_t ssd1363_framebuffer_draw_vline(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint16_t height, uint8_t gray4)
{
	return ssd1363_framebuffer_fill_rect(framebuffer, x, y, 1U, height, gray4);
}

/*
 * Draw a 1-pixel line between two endpoints using Bresenham stepping.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x0: start x coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y0: start y coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - x1: end x coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y1: end y coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - gray4: 4-bit grayscale value
 *   - valid range: 0x00..0x0F
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer, endpoints, or gray value
 */
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

/*
 * Draw a thicker line by stamping a square brush along the line path.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x0: start x coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y0: start y coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - x1: end x coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y1: end y coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - gray4: 4-bit grayscale value
 *   - valid range: 0x00..0x0F
 * - thickness: brush width in pixels
 *   - valid range: 1..min(SSD1363_FRAMEBUFFER_WIDTH, SSD1363_FRAMEBUFFER_HEIGHT)
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer, endpoints, gray value, or thickness
 */
esp_err_t ssd1363_framebuffer_draw_line_thick(ssd1363_framebuffer_t *framebuffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t gray4, uint16_t thickness)
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

	if (framebuffer == NULL || gray4 > 0x0FU || thickness == 0U) {
		return ESP_ERR_INVALID_ARG;
	}

	if (!ssd1363_framebuffer_pixel_in_bounds(x0, y0) || !ssd1363_framebuffer_pixel_in_bounds(x1, y1)) {
		return ESP_ERR_INVALID_ARG;
	}

	if (thickness == 1U) {
		return ssd1363_framebuffer_draw_line(framebuffer, x0, y0, x1, y1, gray4);
	}

	error = delta_x + delta_y;

	while (true) {
		ssd1363_framebuffer_stamp_brush(framebuffer, x, y, gray4, thickness);

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

/*
 * Draw a 1-pixel rectangle outline.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x: left pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: top pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - width: rectangle width in pixels
 *   - valid range: 1..SSD1363_FRAMEBUFFER_WIDTH
 * - height: rectangle height in pixels
 *   - valid range: 1..SSD1363_FRAMEBUFFER_HEIGHT
 * - gray4: 4-bit grayscale value
 *   - valid range: 0x00..0x0F
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer, rectangle, or gray value
 */
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

/*
 * Draw a thicker rectangle border, or fill if the border consumes the shape.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x: left pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: top pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - width: rectangle width in pixels
 *   - valid range: 1..SSD1363_FRAMEBUFFER_WIDTH
 * - height: rectangle height in pixels
 *   - valid range: 1..SSD1363_FRAMEBUFFER_HEIGHT
 * - gray4: 4-bit grayscale value
 *   - valid range: 0x00..0x0F
 * - thickness: border thickness in pixels
 *   - valid range: 1..min(width, height)
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer, rectangle, gray value, or thickness
 */
esp_err_t ssd1363_framebuffer_draw_rect_thick(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t gray4, uint16_t thickness)
{
	esp_err_t err;

	if (framebuffer == NULL || width == 0U || height == 0U || gray4 > 0x0FU || thickness == 0U) {
		return ESP_ERR_INVALID_ARG;
	}

	if (!ssd1363_framebuffer_rect_in_bounds(x, y, width, height)) {
		return ESP_ERR_INVALID_ARG;
	}

	if (thickness == 1U) {
		return ssd1363_framebuffer_draw_rect(framebuffer, x, y, width, height, gray4);
	}

	if ((thickness * 2U) >= width || (thickness * 2U) >= height) {
		return ssd1363_framebuffer_fill_rect(framebuffer, x, y, width, height, gray4);
	}

	err = ssd1363_framebuffer_fill_rect(framebuffer, x, y, width, thickness, gray4);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_framebuffer_fill_rect(framebuffer, x, (uint16_t)(y + height - thickness), width, thickness, gray4);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_framebuffer_fill_rect(framebuffer, x, (uint16_t)(y + thickness), thickness, (uint16_t)(height - (2U * thickness)), gray4);
	if (err != ESP_OK) {
		return err;
	}

	return ssd1363_framebuffer_fill_rect(
		framebuffer,
		(uint16_t)(x + width - thickness),
		(uint16_t)(y + thickness),
		thickness,
		(uint16_t)(height - (2U * thickness)),
		gray4
	);
}

/*
 * Draw a 1bpp bitmap into the local framebuffer with optional background fill.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x: left pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: top pixel coordinate
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - width: bitmap width in pixels
 *   - valid range: 1..SSD1363_FRAMEBUFFER_WIDTH
 * - height: bitmap height in pixels
 *   - valid range: 1..SSD1363_FRAMEBUFFER_HEIGHT
 * - bitmap: source 1bpp bitmap data
 *   - must not be NULL
 * - foreground_gray4: grayscale value for set bits
 *   - valid range: 0x00..0x0F
 * - background_gray4: grayscale value for cleared bits when opaque mode is used
 *   - valid range: 0x00..0x0F
 * - mode: transparent or opaque background handling
 *   - valid values: SSD1363_FRAMEBUFFER_BITMAP_TRANSPARENT or SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer, rectangle, bitmap pointer, or gray values
 */
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

/*
 * Flush the entire local framebuffer to the display.
 *
 * Arguments:
 * - framebuffer: source framebuffer object
 *   - must not be NULL
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer
 * - an ESP-IDF error code if the display write fails
 */
esp_err_t ssd1363_framebuffer_flush(const ssd1363_framebuffer_t *framebuffer)
{
	if (framebuffer == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	return ssd1363_basic_write_buffer(framebuffer->data, sizeof(framebuffer->data));
}

/*
 * Flush a rectangle after aligning it to the SSD1363 4-pixel column groups.
 *
 * What it does:
 * - Expands the requested x-range to the controller's 4-pixel write alignment
 * - Flushes one aligned row at a time to the display
 *
 * Arguments:
 * - framebuffer: source framebuffer object
 *   - must not be NULL
 * - x: left pixel coordinate of the logical update region
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: top pixel coordinate of the logical update region
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - width: logical update width in pixels
 *   - valid range: 1..SSD1363_FRAMEBUFFER_WIDTH
 * - height: logical update height in pixels
 *   - valid range: 1..SSD1363_FRAMEBUFFER_HEIGHT
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer or rectangle
 * - an ESP-IDF error code if the display write fails
 */
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

	aligned_x = (uint16_t)((x / SSD1363_PANEL_COLUMN_ADDR_UNIT_PIXELS) * SSD1363_PANEL_COLUMN_ADDR_UNIT_PIXELS);
	x_end = (uint16_t)((((x + width + (SSD1363_PANEL_COLUMN_ADDR_UNIT_PIXELS - 1U)) /
		SSD1363_PANEL_COLUMN_ADDR_UNIT_PIXELS) * SSD1363_PANEL_COLUMN_ADDR_UNIT_PIXELS));
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