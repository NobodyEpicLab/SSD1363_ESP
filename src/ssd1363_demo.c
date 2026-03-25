#include "ssd1363_demo.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ssd1363_api.h"
#include "ssd1363_basic.h"
#include "ssd1363_config.h"
#include "ssd1363_fonts.h"
#include "ssd1363_framebuffer.h"
#include "ssd1363_text.h"

#define SSD1363_DEMO_CONTRAST 0xFF
#define SSD1363_COUNTER_START 0UL
#define SSD1363_COUNTER_END 10000UL

static ssd1363_framebuffer_t g_gfx_framebuffer;

static void ssd1363_demo_run_i2c_smoke_test_impl(void);

void ssd1363_demo_run_i2c_smoke_test(void)
{
	ssd1363_demo_run_i2c_smoke_test_impl();
}

static esp_err_t show_grayscale_text_scene(void)
{
	esp_err_t err;
	ssd1363_text_bounds_t updated_bounds;
	uint16_t number_width = 0U;
	uint16_t number_height = 0U;
	char counter_text[16];
	int64_t start_time_us;
	int64_t end_time_us;
	double elapsed_seconds;
	double updates_per_second;

	ssd1363_framebuffer_init(&g_gfx_framebuffer);
	ssd1363_framebuffer_fill(&g_gfx_framebuffer, 0x0);

	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		8U,
		20U,
		"REFRESH TEST",
		&ssd1363_font_builtin_5x7,
		0x0FU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		8U,
		40U,
		"COUNT:",
		&ssd1363_font_builtin_5x7,
		0x08U,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		8U,
		60U,
		"0 -> 10000",
		&ssd1363_font_builtin_5x7,
		0x0FU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		8U,
		80U,
		"SPEED: SERIAL LOG",
		&ssd1363_font_builtin_5x7,
		0x0EU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_text_measure_string(&ssd1363_font_builtin_5x7, "10000", &number_width, &number_height);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_framebuffer_fill_rect(&g_gfx_framebuffer, 56U, 40U, number_width, number_height, 0x00U);
	if (err != ESP_OK) {
		return err;
	}

	snprintf(counter_text, sizeof(counter_text), "%lu", SSD1363_COUNTER_START);
	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		56U,
		40U,
		counter_text,
		&ssd1363_font_builtin_5x7,
		0x0FU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	start_time_us = esp_timer_get_time();

	for (uint32_t counter = SSD1363_COUNTER_START; counter <= SSD1363_COUNTER_END; ++counter) {
		snprintf(counter_text, sizeof(counter_text), "%lu", (unsigned long)counter);
		ssd1363_text_bounds_clear(&updated_bounds);

		err = ssd1363_framebuffer_fill_rect(&g_gfx_framebuffer, 56U, 40U, number_width, number_height, 0x00U);
		if (err != ESP_OK) {
			return err;
		}

		err = ssd1363_text_draw_string(
			&g_gfx_framebuffer,
			56U,
			40U,
			counter_text,
			&ssd1363_font_builtin_5x7,
			0x0FU,
			0x00U,
			SSD1363_FRAMEBUFFER_BITMAP_OPAQUE,
			&updated_bounds
		);
		if (err != ESP_OK) {
			return err;
		}

		err = ssd1363_framebuffer_flush_rect(
			&g_gfx_framebuffer,
			56U,
			40U,
			number_width,
			number_height
		);
		if (err != ESP_OK) {
			return err;
		}
	}

	end_time_us = esp_timer_get_time();
	elapsed_seconds = (double)(end_time_us - start_time_us) / 1000000.0;
	updates_per_second = (elapsed_seconds > 0.0)
		? ((double)(SSD1363_COUNTER_END - SSD1363_COUNTER_START + 1UL) / elapsed_seconds)
		: 0.0;

	printf("Counter refresh test finished: %lu updates in %.3f s (%.2f updates/s)\n",
		(unsigned long)(SSD1363_COUNTER_END - SSD1363_COUNTER_START + 1UL),
		elapsed_seconds,
		updates_per_second);

	return ESP_OK;
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
		SSD1363_PANEL_COLUMN_OFFSET,
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
	printf("Displaying fast counter refresh test\n");

	err = show_grayscale_text_scene();
	if (err != ESP_OK) {
		printf("Counter refresh test failed: %s\n", esp_err_to_name(err));
		return;
	}

	printf("Counter refresh test displayed and completed\n");

	while (true) {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}