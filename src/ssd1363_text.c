#include "ssd1363_text.h"

#include <stdbool.h>
#include <stddef.h>

/* Expand a bounds box so partial updates can flush the touched region. */
static void ssd1363_text_bounds_include(ssd1363_text_bounds_t *bounds, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	if (bounds == NULL || width == 0U || height == 0U) {
		return;
	}

	if (bounds->width == 0U || bounds->height == 0U) {
		bounds->x = x;
		bounds->y = y;
		bounds->width = width;
		bounds->height = height;
		return;
	}

	const uint16_t x1 = bounds->x;
	const uint16_t y1 = bounds->y;
	const uint16_t x2 = (uint16_t)(bounds->x + bounds->width);
	const uint16_t y2 = (uint16_t)(bounds->y + bounds->height);
	const uint16_t new_x2 = (uint16_t)(x + width);
	const uint16_t new_y2 = (uint16_t)(y + height);
	const uint16_t merged_x1 = (x < x1) ? x : x1;
	const uint16_t merged_y1 = (y < y1) ? y : y1;
	const uint16_t merged_x2 = (new_x2 > x2) ? new_x2 : x2;
	const uint16_t merged_y2 = (new_y2 > y2) ? new_y2 : y2;

	bounds->x = merged_x1;
	bounds->y = merged_y1;
	bounds->width = (uint16_t)(merged_x2 - merged_x1);
	bounds->height = (uint16_t)(merged_y2 - merged_y1);
}

/* Normalize bitmap-font input to uppercase for the built-in glyph tables. */
static char ssd1363_text_normalize_char(char ch)
{
	if (ch >= 'a' && ch <= 'z') {
		return (char)(ch - ('a' - 'A'));
	}

	return ch;
}

/* Look up one glyph in the bitmap-font table, falling back when needed. */
static const ssd1363_font_glyph_t *ssd1363_text_find_bitmap_glyph(const ssd1363_font_t *font, char ch)
{
	const char normalized = ssd1363_text_normalize_char(ch);

	for (size_t index = 0; index < font->glyph_count; ++index) {
		if (font->glyphs[index].code == normalized) {
			return &font->glyphs[index];
		}
	}

	for (size_t index = 0; index < font->glyph_count; ++index) {
		if (font->glyphs[index].code == font->fallback_char) {
			return &font->glyphs[index];
		}
	}

	return NULL;
}

/* Check whether the current font uses the Adafruit GFX backend. */
static bool ssd1363_text_font_is_gfx(const ssd1363_font_t *font)
{
	return font != NULL && font->backend == SSD1363_FONT_BACKEND_GFX && font->gfx_font != NULL;
}

/* Read one pixel bit from a packed 1bpp bitmap-font glyph. */
static bool ssd1363_text_bitmap_pixel_on(const uint8_t *bitmap, uint8_t width, uint8_t x, uint8_t y)
{
	const uint8_t stride = (uint8_t)((width + 7U) / 8U);
	const size_t index = ((size_t)y * stride) + (x / 8U);
	const uint8_t mask = (uint8_t)(0x80U >> (x % 8U));

	return (bitmap[index] & mask) != 0U;
}

/* Look up one glyph in a GFXfont, applying the configured fallback character. */
static const GFXglyph *ssd1363_text_find_gfx_glyph(const ssd1363_font_t *font, char ch)
{
	if (!ssd1363_text_font_is_gfx(font)) {
		return NULL;
	}

	uint8_t glyph_code = (uint8_t)ch;
	if (glyph_code < font->gfx_font->first || glyph_code > font->gfx_font->last) {
		glyph_code = (uint8_t)font->fallback_char;
		if (glyph_code < font->gfx_font->first || glyph_code > font->gfx_font->last) {
			return NULL;
		}
	}

	return &font->gfx_font->glyph[glyph_code - font->gfx_font->first];
}

/* Fill a rectangle but clip it to the framebuffer bounds first. */
static void ssd1363_text_fill_clipped_rect(ssd1363_framebuffer_t *framebuffer, int32_t x, int32_t y, uint16_t width, uint16_t height, uint8_t gray4, ssd1363_text_bounds_t *updated_bounds)
{
	if (framebuffer == NULL || width == 0U || height == 0U) {
		return;
	}

	const int32_t start_x = (x < 0) ? 0 : x;
	const int32_t start_y = (y < 0) ? 0 : y;
	const int32_t end_x = ((x + width) > SSD1363_FRAMEBUFFER_WIDTH) ? SSD1363_FRAMEBUFFER_WIDTH : (x + width);
	const int32_t end_y = ((y + height) > SSD1363_FRAMEBUFFER_HEIGHT) ? SSD1363_FRAMEBUFFER_HEIGHT : (y + height);

	if (start_x >= end_x || start_y >= end_y) {
		return;
	}

	for (int32_t draw_y = start_y; draw_y < end_y; ++draw_y) {
		for (int32_t draw_x = start_x; draw_x < end_x; ++draw_x) {
			(void)ssd1363_framebuffer_set_pixel(framebuffer, (uint16_t)draw_x, (uint16_t)draw_y, gray4);
		}
	}

	ssd1363_text_bounds_include(
		updated_bounds,
		(uint16_t)start_x,
		(uint16_t)start_y,
		(uint16_t)(end_x - start_x),
		(uint16_t)(end_y - start_y)
	);
}

/* Draw one GFXfont character and optionally report its advance width. */
static esp_err_t ssd1363_text_gfx_draw_char(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, char ch, const ssd1363_font_t *font, uint8_t foreground_gray4, uint8_t background_gray4, ssd1363_framebuffer_bitmap_mode_t background_mode, ssd1363_text_bounds_t *updated_bounds, uint16_t *advance_width)
{
	const GFXglyph *glyph = ssd1363_text_find_gfx_glyph(font, ch);
	if (glyph == NULL) {
		return ESP_ERR_NOT_FOUND;
	}

	const uint16_t scaled_advance = (uint16_t)(glyph->xAdvance * font->scale_x);
	const uint16_t line_height = (uint16_t)(font->glyph_height * font->scale_y);
	const int32_t baseline_y = (int32_t)y + ((int32_t)font->baseline * font->scale_y);

	if (advance_width != NULL) {
		*advance_width = scaled_advance;
	}

	if (background_mode == SSD1363_FRAMEBUFFER_BITMAP_OPAQUE && scaled_advance > 0U) {
		ssd1363_text_fill_clipped_rect(framebuffer, x, y, scaled_advance, line_height, background_gray4, updated_bounds);
	}

	if (glyph->width == 0U || glyph->height == 0U) {
		return ESP_OK;
	}

	const uint8_t *bitmap = font->gfx_font->bitmap + glyph->bitmapOffset;
	uint16_t bitmap_index = 0U;
	uint8_t bit_mask = 0U;
	uint8_t current_byte = 0U;

	for (uint8_t glyph_y = 0U; glyph_y < glyph->height; ++glyph_y) {
		for (uint8_t glyph_x = 0U; glyph_x < glyph->width; ++glyph_x) {
			if (bit_mask == 0U) {
				current_byte = bitmap[bitmap_index++];
				bit_mask = 0x80U;
			}

			if ((current_byte & bit_mask) != 0U) {
				const int32_t pixel_x = (int32_t)x + ((int32_t)(glyph->xOffset + glyph_x) * font->scale_x);
				const int32_t pixel_y = baseline_y + ((int32_t)(glyph->yOffset + glyph_y) * font->scale_y);
				ssd1363_text_fill_clipped_rect(framebuffer, pixel_x, pixel_y, font->scale_x, font->scale_y, foreground_gray4, updated_bounds);
			}

			bit_mask >>= 1;
		}
	}

	return ESP_OK;
}

/*
 * Clear a bounds structure used for partial text updates.
 *
 * Arguments:
 * - bounds: bounds object to reset
 *   - must not be NULL
 *
 * Returns:
 * - none
 */
void ssd1363_text_bounds_clear(ssd1363_text_bounds_t *bounds)
{
	if (bounds == NULL) {
		return;
	}

	bounds->x = 0U;
	bounds->y = 0U;
	bounds->width = 0U;
	bounds->height = 0U;
}

/*
 * Measure the pixel size of a string for the selected font.
 *
 * Arguments:
 * - font: font definition to use
 *   - must not be NULL
 * - text: zero-terminated string to measure
 *   - must not be NULL
 * - width: destination for measured width in pixels
 *   - must not be NULL
 * - height: destination for measured height in pixels
 *   - must not be NULL
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid pointers
 * - ESP_ERR_NOT_FOUND if a glyph cannot be resolved
 */
esp_err_t ssd1363_text_measure_string(const ssd1363_font_t *font, const char *text, uint16_t *width, uint16_t *height)
{
	if (font == NULL || text == NULL || width == NULL || height == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	if (ssd1363_text_font_is_gfx(font)) {
		uint16_t cursor_x = 0U;
		uint16_t max_width_px = 0U;
		uint16_t total_height_px = (uint16_t)(font->glyph_height * font->scale_y);

		for (const char *ptr = text; *ptr != '\0'; ++ptr) {
			if (*ptr == '\n') {
				if (cursor_x > 0U) {
					cursor_x = (uint16_t)(cursor_x - font->letter_spacing);
				}

				if (cursor_x > max_width_px) {
					max_width_px = cursor_x;
				}

				cursor_x = 0U;
				total_height_px = (uint16_t)(total_height_px + (font->glyph_height * font->scale_y) + font->line_spacing);
				continue;
			}

			const GFXglyph *glyph = ssd1363_text_find_gfx_glyph(font, *ptr);
			if (glyph == NULL) {
				return ESP_ERR_NOT_FOUND;
			}

			cursor_x = (uint16_t)(cursor_x + (glyph->xAdvance * font->scale_x) + font->letter_spacing);
		}

		if (cursor_x > 0U) {
			cursor_x = (uint16_t)(cursor_x - font->letter_spacing);
		}

		if (cursor_x > max_width_px) {
			max_width_px = cursor_x;
		}

		*width = max_width_px;
		*height = total_height_px;
		return ESP_OK;
	}

	uint16_t cursor_x = 0U;
	uint16_t max_width = 0U;
	uint16_t total_height = (uint16_t)(font->glyph_height * font->scale_y);

	for (const char *ptr = text; *ptr != '\0'; ++ptr) {
		if (*ptr == '\n') {
			if (cursor_x > max_width) {
				max_width = cursor_x;
			}

			cursor_x = 0U;
			total_height = (uint16_t)(total_height + (font->glyph_height * font->scale_y) + font->line_spacing);
			continue;
		}

		const ssd1363_font_glyph_t *glyph = ssd1363_text_find_bitmap_glyph(font, *ptr);
		if (glyph == NULL) {
			return ESP_ERR_NOT_FOUND;
		}

		cursor_x = (uint16_t)(cursor_x + (glyph->width * font->scale_x) + font->letter_spacing);
	}

	if (cursor_x > 0U) {
		cursor_x = (uint16_t)(cursor_x - font->letter_spacing);
	}

	if (cursor_x > max_width) {
		max_width = cursor_x;
	}

	*width = max_width;
	*height = total_height;
	return ESP_OK;
}

/*
 * Draw one character into the local framebuffer.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x: left pixel coordinate of the character cell
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: top pixel coordinate of the character cell
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - ch: character to draw
 *   - valid range: any 8-bit char supported by the active font
 * - font: font definition to use
 *   - must not be NULL
 * - foreground_gray4: grayscale value for lit glyph pixels
 *   - valid range: 0x00..0x0F
 * - background_gray4: grayscale value for background fill in opaque mode
 *   - valid range: 0x00..0x0F
 * - background_mode: transparent or opaque background handling
 *   - valid values: SSD1363_FRAMEBUFFER_BITMAP_TRANSPARENT or SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
 * - updated_bounds: optional bounds accumulator for partial updates
 *   - may be NULL
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid framebuffer, font, or gray values
 * - ESP_ERR_NOT_FOUND if a glyph cannot be resolved
 * - an ESP-IDF error code if framebuffer drawing fails
 */
esp_err_t ssd1363_text_draw_char(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, char ch, const ssd1363_font_t *font, uint8_t foreground_gray4, uint8_t background_gray4, ssd1363_framebuffer_bitmap_mode_t background_mode, ssd1363_text_bounds_t *updated_bounds)
{
	if (framebuffer == NULL || font == NULL || foreground_gray4 > 0x0FU || background_gray4 > 0x0FU) {
		return ESP_ERR_INVALID_ARG;
	}

	if (ssd1363_text_font_is_gfx(font)) {
		return ssd1363_text_gfx_draw_char(framebuffer, x, y, ch, font, foreground_gray4, background_gray4, background_mode, updated_bounds, NULL);
	}

	const ssd1363_font_glyph_t *glyph = ssd1363_text_find_bitmap_glyph(font, ch);
	if (glyph == NULL) {
		return ESP_ERR_NOT_FOUND;
	}

	const uint16_t draw_width = (uint16_t)(glyph->width * font->scale_x);
	const uint16_t draw_height = (uint16_t)(font->glyph_height * font->scale_y);

	if (font->scale_x == 1U && font->scale_y == 1U) {
		esp_err_t err = ssd1363_framebuffer_draw_bitmap_1bpp(framebuffer, x, y, glyph->width, font->glyph_height, glyph->bitmap, foreground_gray4, background_gray4, background_mode);
		if (err != ESP_OK) {
			return err;
		}
	} else {
		for (uint8_t glyph_y = 0; glyph_y < font->glyph_height; ++glyph_y) {
			for (uint8_t glyph_x = 0; glyph_x < glyph->width; ++glyph_x) {
				const bool pixel_on = ssd1363_text_bitmap_pixel_on(glyph->bitmap, glyph->width, glyph_x, glyph_y);
				if (pixel_on || background_mode == SSD1363_FRAMEBUFFER_BITMAP_OPAQUE) {
					const uint8_t gray = pixel_on ? foreground_gray4 : background_gray4;
					esp_err_t err = ssd1363_framebuffer_fill_rect(
						framebuffer,
						(uint16_t)(x + (glyph_x * font->scale_x)),
						(uint16_t)(y + (glyph_y * font->scale_y)),
						font->scale_x,
						font->scale_y,
						gray
					);
					if (err != ESP_OK) {
						return err;
					}
				}
			}
		}
	}

	ssd1363_text_bounds_include(updated_bounds, x, y, draw_width, draw_height);
	return ESP_OK;
}

/*
 * Draw a zero-terminated string into the local framebuffer.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x: left pixel coordinate of the first character
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: top pixel coordinate of the first line
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - text: zero-terminated string to draw
 *   - must not be NULL
 * - font: font definition to use
 *   - must not be NULL
 * - foreground_gray4: grayscale value for lit glyph pixels
 *   - valid range: 0x00..0x0F
 * - background_gray4: grayscale value for background fill in opaque mode
 *   - valid range: 0x00..0x0F
 * - background_mode: transparent or opaque background handling
 *   - valid values: SSD1363_FRAMEBUFFER_BITMAP_TRANSPARENT or SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
 * - updated_bounds: optional bounds accumulator for partial updates
 *   - may be NULL
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid pointers or gray values
 * - ESP_ERR_NOT_FOUND if a glyph cannot be resolved
 * - an ESP-IDF error code if framebuffer drawing fails
 */
esp_err_t ssd1363_text_draw_string(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, const char *text, const ssd1363_font_t *font, uint8_t foreground_gray4, uint8_t background_gray4, ssd1363_framebuffer_bitmap_mode_t background_mode, ssd1363_text_bounds_t *updated_bounds)
{
	uint16_t cursor_x = x;
	uint16_t cursor_y = y;

	if (framebuffer == NULL || text == NULL || font == NULL || foreground_gray4 > 0x0FU || background_gray4 > 0x0FU) {
		return ESP_ERR_INVALID_ARG;
	}

	ssd1363_text_bounds_clear(updated_bounds);

	for (const char *ptr = text; *ptr != '\0'; ++ptr) {
		if (*ptr == '\n') {
			cursor_x = x;
			cursor_y = (uint16_t)(cursor_y + (font->glyph_height * font->scale_y) + font->line_spacing);
			continue;
		}

		if (ssd1363_text_font_is_gfx(font)) {
			uint16_t advance_width = 0U;
			esp_err_t err = ssd1363_text_gfx_draw_char(framebuffer, cursor_x, cursor_y, *ptr, font, foreground_gray4, background_gray4, background_mode, updated_bounds, &advance_width);
			if (err != ESP_OK) {
				return err;
			}

			cursor_x = (uint16_t)(cursor_x + advance_width + font->letter_spacing);
			continue;
		}

		const ssd1363_font_glyph_t *glyph = ssd1363_text_find_bitmap_glyph(font, *ptr);
		if (glyph == NULL) {
			return ESP_ERR_NOT_FOUND;
		}

		esp_err_t err = ssd1363_text_draw_char(framebuffer, cursor_x, cursor_y, *ptr, font, foreground_gray4, background_gray4, background_mode, updated_bounds);
		if (err != ESP_OK) {
			return err;
		}

		cursor_x = (uint16_t)(cursor_x + (glyph->width * font->scale_x) + font->letter_spacing);
	}

	return ESP_OK;
}

/*
 * Draw a string and flush the entire framebuffer to the display.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x: left pixel coordinate of the first character
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: top pixel coordinate of the first line
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - text: zero-terminated string to draw
 *   - must not be NULL
 * - font: font definition to use
 *   - must not be NULL
 * - foreground_gray4: grayscale value for lit glyph pixels
 *   - valid range: 0x00..0x0F
 * - background_gray4: grayscale value for background fill in opaque mode
 *   - valid range: 0x00..0x0F
 * - background_mode: transparent or opaque background handling
 *   - valid values: SSD1363_FRAMEBUFFER_BITMAP_TRANSPARENT or SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid pointers or gray values
 * - ESP_ERR_NOT_FOUND if a glyph cannot be resolved
 * - an ESP-IDF error code if drawing or flushing fails
 */
esp_err_t ssd1363_text_write_string_full(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, const char *text, const ssd1363_font_t *font, uint8_t foreground_gray4, uint8_t background_gray4, ssd1363_framebuffer_bitmap_mode_t background_mode)
{
	esp_err_t err = ssd1363_text_draw_string(framebuffer, x, y, text, font, foreground_gray4, background_gray4, background_mode, NULL);
	if (err != ESP_OK) {
		return err;
	}

	return ssd1363_framebuffer_flush(framebuffer);
}

/*
 * Draw a string and flush only the updated bounds.
 *
 * Arguments:
 * - framebuffer: target framebuffer object
 *   - must not be NULL
 * - x: left pixel coordinate of the first character
 *   - valid range: 0..SSD1363_FRAMEBUFFER_WIDTH-1
 * - y: top pixel coordinate of the first line
 *   - valid range: 0..SSD1363_FRAMEBUFFER_HEIGHT-1
 * - text: zero-terminated string to draw
 *   - must not be NULL
 * - font: font definition to use
 *   - must not be NULL
 * - foreground_gray4: grayscale value for lit glyph pixels
 *   - valid range: 0x00..0x0F
 * - background_gray4: grayscale value for background fill in opaque mode
 *   - valid range: 0x00..0x0F
 * - background_mode: transparent or opaque background handling
 *   - valid values: SSD1363_FRAMEBUFFER_BITMAP_TRANSPARENT or SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
 * - updated_bounds: optional output for the flushed bounds
 *   - may be NULL
 *
 * Returns:
 * - ESP_OK on success
 * - ESP_ERR_INVALID_ARG for invalid pointers or gray values
 * - ESP_ERR_NOT_FOUND if a glyph cannot be resolved
 * - an ESP-IDF error code if drawing or flushing fails
 */
esp_err_t ssd1363_text_write_string_partial(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, const char *text, const ssd1363_font_t *font, uint8_t foreground_gray4, uint8_t background_gray4, ssd1363_framebuffer_bitmap_mode_t background_mode, ssd1363_text_bounds_t *updated_bounds)
{
	ssd1363_text_bounds_t local_bounds;
	ssd1363_text_bounds_t *bounds = (updated_bounds != NULL) ? updated_bounds : &local_bounds;

	esp_err_t err = ssd1363_text_draw_string(framebuffer, x, y, text, font, foreground_gray4, background_gray4, background_mode, bounds);
	if (err != ESP_OK) {
		return err;
	}

	if (bounds->width == 0U || bounds->height == 0U) {
		return ESP_OK;
	}

	return ssd1363_framebuffer_flush_rect(framebuffer, bounds->x, bounds->y, bounds->width, bounds->height);
}