#ifndef SSD1363_FRAMEBUFFER_H
#define SSD1363_FRAMEBUFFER_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SSD1363_FRAMEBUFFER_WIDTH       256U
#define SSD1363_FRAMEBUFFER_HEIGHT      128U
#define SSD1363_FRAMEBUFFER_BPP         4U
#define SSD1363_FRAMEBUFFER_SIZE_BYTES  ((SSD1363_FRAMEBUFFER_WIDTH * SSD1363_FRAMEBUFFER_HEIGHT) / 2U)

typedef struct {
	uint8_t data[SSD1363_FRAMEBUFFER_SIZE_BYTES];
} ssd1363_framebuffer_t;

void ssd1363_framebuffer_init(ssd1363_framebuffer_t *framebuffer);
void ssd1363_framebuffer_clear(ssd1363_framebuffer_t *framebuffer);
void ssd1363_framebuffer_fill(ssd1363_framebuffer_t *framebuffer, uint8_t gray4);
esp_err_t ssd1363_framebuffer_set_pixel(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint8_t gray4);
esp_err_t ssd1363_framebuffer_get_pixel(const ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint8_t *gray4);
esp_err_t ssd1363_framebuffer_fill_rect(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t gray4);
esp_err_t ssd1363_framebuffer_draw_rect(ssd1363_framebuffer_t *framebuffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t gray4);
esp_err_t ssd1363_framebuffer_flush(const ssd1363_framebuffer_t *framebuffer);

#ifdef __cplusplus
}
#endif

#endif