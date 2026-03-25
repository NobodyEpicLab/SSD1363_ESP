#ifndef SSD1363_TEXT_H
#define SSD1363_TEXT_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "ssd1363_framebuffer.h"
#include "ssd1363_gfxfont.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char code;
	uint8_t width;
	const uint8_t *bitmap;
} ssd1363_font_glyph_t;

typedef enum {
	SSD1363_FONT_BACKEND_BITMAP = 0,
	SSD1363_FONT_BACKEND_GFX = 1,
} ssd1363_font_backend_t;

typedef struct {
	ssd1363_font_backend_t backend;
	const ssd1363_font_glyph_t *glyphs;
	size_t glyph_count;
	const GFXfont *gfx_font;
	uint8_t glyph_height;
	uint8_t baseline;
	uint8_t scale_x;
	uint8_t scale_y;
	uint8_t letter_spacing;
	uint8_t line_spacing;
	char fallback_char;
} ssd1363_font_t;

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
} ssd1363_text_bounds_t;

void ssd1363_text_bounds_clear(ssd1363_text_bounds_t *bounds);
esp_err_t ssd1363_text_measure_string(const ssd1363_font_t *font, const char *text, uint16_t *width, uint16_t *height);
esp_err_t ssd1363_text_draw_char(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, char ch, const ssd1363_font_t *font, uint8_t foreground_gray4, uint8_t background_gray4, ssd1363_framebuffer_bitmap_mode_t background_mode, ssd1363_text_bounds_t *updated_bounds);
esp_err_t ssd1363_text_draw_string(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, const char *text, const ssd1363_font_t *font, uint8_t foreground_gray4, uint8_t background_gray4, ssd1363_framebuffer_bitmap_mode_t background_mode, ssd1363_text_bounds_t *updated_bounds);
esp_err_t ssd1363_text_write_string_full(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, const char *text, const ssd1363_font_t *font, uint8_t foreground_gray4, uint8_t background_gray4, ssd1363_framebuffer_bitmap_mode_t background_mode);
esp_err_t ssd1363_text_write_string_partial(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, const char *text, const ssd1363_font_t *font, uint8_t foreground_gray4, uint8_t background_gray4, ssd1363_framebuffer_bitmap_mode_t background_mode, ssd1363_text_bounds_t *updated_bounds);

#ifdef __cplusplus
}
#endif

#endif