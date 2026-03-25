#ifndef SSD1363_GFXFONT_H
#define SSD1363_GFXFONT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint16_t bitmapOffset;
	uint8_t width;
	uint8_t height;
	uint8_t xAdvance;
	int8_t xOffset;
	int8_t yOffset;
} GFXglyph;

typedef struct {
	const uint8_t *bitmap;
	const GFXglyph *glyph;
	uint16_t first;
	uint16_t last;
	uint8_t yAdvance;
} GFXfont;

#ifdef __cplusplus
}
#endif

#endif