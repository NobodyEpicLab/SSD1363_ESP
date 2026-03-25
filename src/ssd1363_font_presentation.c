#include "ssd1363_font_presentation.h"

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ssd1363_api.h"
#include "ssd1363_basic.h"
#include "ssd1363_fonts.h"
#include "ssd1363_framebuffer.h"
#include "ssd1363_text.h"

#define SSD1363_FONT_PRESENTATION_CONTRAST 0xFF
#define SSD1363_FONT_PRESENTATION_PAGE_HOLD_MS 3500U

typedef struct {
	const char *name;
	const ssd1363_font_t *font;
	const char *sample;
	uint16_t sample_y;
} ssd1363_font_preview_t;

static ssd1363_framebuffer_t g_font_preview_framebuffer;

static const ssd1363_font_preview_t g_font_previews[] = {
	{"BUILTIN 15X21", &ssd1363_font_builtin_15x21, "OIL 105C", 34U},
	{"FREEMONO 24PX", &ssd1363_font_freemono_24px, "PSI 4.2", 30U},
	{"CASCADIA 29PX", &ssd1363_font_cascadiacode_29px, "TEST FONTA 112934.4", 36U},
	{"JETBRAINS 33PX", &ssd1363_font_jetbrainsmonolight_33px, "TEST FONTA 112934.4", 34U},
	{"IBM PLEX 32PX", &ssd1363_font_ibmplexmonolight_32px, "TEST FONTA 112934.4", 34U},
	{"SHARETECH 28PX", &ssd1363_font_sharetechmono_28px, "TEST FONTA 112934.4", 38U},
	{"MICHROMA 35PX", &ssd1363_font_michroma_35px, "TEST FONTA 112934.4", 32U},
	{"ORBITRON 31PX", &ssd1363_font_orbitron_31px, "TEST FONTA 112934.4", 36U},
    {"IBM PLEX 17PX", &ssd1363_font_ibmplexmonolight_17px, "TEST FONTA 112934.4", 34U},
    {"IBM PLEX 47PX", &ssd1363_font_ibmplexmonolight_47px, "TEST FONTA 112934.4", 34U},

};

static esp_err_t ssd1363_font_presentation_draw_page(const ssd1363_font_preview_t *preview)
{
	esp_err_t err;
	char footer[32];

	if (preview == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	ssd1363_framebuffer_fill(&g_font_preview_framebuffer, 0x00U);

	err = ssd1363_text_write_string_full(
		&g_font_preview_framebuffer,
		8U,
		8U,
		preview->name,
		&ssd1363_font_builtin_10x14,
		0x0FU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_text_write_string_full(
		&g_font_preview_framebuffer,
		8U,
		preview->sample_y,
		preview->sample,
		preview->font,
		0x0FU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_TRANSPARENT
	);
	if (err != ESP_OK) {
		return err;
	}

	snprintf(footer, sizeof(footer), "1234567890 TEMP PSI");
	err = ssd1363_text_write_string_full(
		&g_font_preview_framebuffer,
		8U,
		110U,
		footer,
		&ssd1363_font_builtin_5x7,
		0x08U,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	return ssd1363_framebuffer_flush(&g_font_preview_framebuffer);
}

void ssd1363_font_presentation_run_i2c(void)
{
	printf("SSD1363 font presentation start\n");

	esp_err_t err = ssd1363_basic_init(SSD1363_BUS_I2C);
	if (err != ESP_OK) {
		printf("ssd1363_basic_init failed: %s\n", esp_err_to_name(err));
		return;
	}

	err = ssd1363_api_set_contrast(SSD1363_FONT_PRESENTATION_CONTRAST);
	if (err != ESP_OK) {
		printf("ssd1363_api_set_contrast failed: %s\n", esp_err_to_name(err));
		return;
	}

	ssd1363_framebuffer_init(&g_font_preview_framebuffer);

	while (true) {
		for (size_t index = 0; index < (sizeof(g_font_previews) / sizeof(g_font_previews[0])); ++index) {
			err = ssd1363_font_presentation_draw_page(&g_font_previews[index]);
			if (err != ESP_OK) {
				printf("font preview page failed: %s\n", esp_err_to_name(err));
				return;
			}

			printf("Showing font preview: %s\n", g_font_previews[index].name);
			vTaskDelay(pdMS_TO_TICKS(SSD1363_FONT_PRESENTATION_PAGE_HOLD_MS));
		}
	}
}