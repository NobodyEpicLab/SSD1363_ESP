#include "ssd1363_demo.h"

#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ssd1363_api.h"
#include "ssd1363_basic.h"
#include "ssd1363_config.h"
#include "ssd1363_fonts.h"
#include "ssd1363_framebuffer.h"
#include "ssd1363_text.h"

#define SSD1363_DEMO_CONTRAST 0x0F

static ssd1363_framebuffer_t g_gfx_framebuffer;

static void ssd1363_demo_run_i2c_smoke_test_impl(void);

void ssd1363_demo_run_i2c_smoke_test(void)
{
	ssd1363_demo_run_i2c_smoke_test_impl();
}

static esp_err_t show_grayscale_text_scene(void)
{
	esp_err_t err;

	ssd1363_framebuffer_init(&g_gfx_framebuffer);
	ssd1363_framebuffer_fill(&g_gfx_framebuffer, 0x0);

	for (uint16_t gray = 0; gray < 16U; ++gray) {
		err = ssd1363_framebuffer_fill_rect(&g_gfx_framebuffer, (uint16_t)(gray * 16U), 8U, 16U, 28U, (uint8_t)gray);
		if (err != ESP_OK) {
			return err;
		}
	}

	err = ssd1363_framebuffer_draw_rect(&g_gfx_framebuffer, 0U, 8U, SSD1363_FRAMEBUFFER_WIDTH, 28U, 0xFU);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_text_draw_string(
		&g_gfx_framebuffer,
		50U,
		42U,
		"GRAY SCALE",
		&ssd1363_font_builtin_5x7,
		0x01U,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE,
		NULL
	);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_text_draw_string(
		&g_gfx_framebuffer,
		120U,
		42U,
		"GRAY SCALE 01",
		&ssd1363_font_builtin_5x7,
		0x0FU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE,
		NULL
	);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_text_draw_string(
		&g_gfx_framebuffer,
		8U,
		70U,
		"HELLO WORLD 01",
		&ssd1363_font_freemono12pt7b,
		0x0EU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_TRANSPARENT,
		NULL
	);
	if (err != ESP_OK) {
		return err;
	}

	return ssd1363_framebuffer_flush(&g_gfx_framebuffer);
}

static void ssd1363_demo_run_i2c_smoke_test_impl(void)
{
	printf("SSD1363 smoke test starting\n");
	printf("Configured I2C pins: SDA=%d SCL=%d\n", SSD1363_I2C_SDA_PIN, SSD1363_I2C_SCL_PIN);
	printf("Configured I2C address: 0x%02X\n", SSD1363_I2C_ADDRESS);
	printf("Configured panel size: %dx%d\n", SSD1363_PANEL_WIDTH, SSD1363_PANEL_HEIGHT);
	printf("Active area: %dx%d, multiplex=0x%02X, offset=0x%02X, column_offset=%u, remap=%02X %02X\n",
		SSD1363_ACTIVE_WIDTH,
		SSD1363_ACTIVE_HEIGHT,
		SSD1363_MULTIPLEX_RATIO,
		SSD1363_DISPLAY_OFFSET,
		SSD1363_COLUMN_OFFSET,
		SSD1363_REMAP_BYTE0,
		SSD1363_REMAP_BYTE1);

	esp_err_t err = ssd1363_basic_init(SSD1363_BUS_I2C);
	if (err != ESP_OK) {
		printf("ssd1363_basic_init failed: %s\n", esp_err_to_name(err));
		return;
	}

	err = ssd1363_api_set_contrast(SSD1363_DEMO_CONTRAST);
	if (err != ESP_OK) {
		printf("ssd1363_api_set_contrast failed: %s\n", esp_err_to_name(err));
		return;
	}

	printf("SSD1363 basic layer initialized over I2C\n");
	printf("Demo contrast set to 0x%02X\n", SSD1363_DEMO_CONTRAST);
	printf("Displaying text-only diagnostic scene\n");

	err = show_grayscale_text_scene();
	if (err != ESP_OK) {
		printf("Text-only diagnostic scene failed: %s\n", esp_err_to_name(err));
		return;
	}

	printf("Text-only diagnostic scene displayed and held on screen\n");

	while (true) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}