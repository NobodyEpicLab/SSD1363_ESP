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
#define SSD1363_COUNTER_END 100UL

static ssd1363_framebuffer_t g_gfx_framebuffer;

static void ssd1363_demo_run_i2c_smoke_test_impl(void);

typedef struct {
	const char *label;
	double elapsed_seconds;
	double updates_per_second;
} ssd1363_demo_benchmark_result_t;

static esp_err_t ssd1363_demo_show_line_example(void);
static esp_err_t ssd1363_demo_prepare_benchmark_scene(const char *mode_label, const ssd1363_font_t *counter_font, uint16_t *number_width, uint16_t *number_height);
static esp_err_t ssd1363_demo_run_counter_benchmark(const char *mode_label, bool use_partial_flush, ssd1363_demo_benchmark_result_t *result);

static esp_err_t ssd1363_demo_show_line_example(void)
{
	esp_err_t err;

	ssd1363_framebuffer_fill(&g_gfx_framebuffer, 0x00U);

	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		8U,
		12U,
		"UI DIVIDER EXAMPLE",
		&ssd1363_font_builtin_10x14,
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
		98U,
		"draw_divider(&fb, 0, 64, 256, 0x0F)",
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
		110U,
		"flush_rect(&fb, 0, 64, 256, 1)",
		&ssd1363_font_builtin_5x7,
		0x08U,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_framebuffer_draw_divider(&g_gfx_framebuffer, 0U, 64U, 256U, 0x0FU);
	if (err != ESP_OK) {
		return err;
	}

	return ssd1363_framebuffer_flush(&g_gfx_framebuffer);
}

static esp_err_t ssd1363_demo_show_refresh_results(const ssd1363_demo_benchmark_result_t *rect_result, const ssd1363_demo_benchmark_result_t *full_result)
{
	esp_err_t err;
	char line_buffer[32];

	ssd1363_framebuffer_fill(&g_gfx_framebuffer, 0x00U);

	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		8U,
		8U,
		"RESULTS",
		&ssd1363_font_builtin_10x14,
		0x0FU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	snprintf(line_buffer, sizeof(line_buffer), "COUNT %lu", (unsigned long)SSD1363_COUNTER_END);
	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		8U,
		30U,
		line_buffer,
		&ssd1363_font_builtin_10x14,
		0x0EU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	snprintf(line_buffer, sizeof(line_buffer), "RECT %.1F UPS", rect_result->updates_per_second);
	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		8U,
		52U,
		line_buffer,
		&ssd1363_font_builtin_10x14,
		0x0EU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	snprintf(line_buffer, sizeof(line_buffer), "TIME %.3FS", rect_result->elapsed_seconds);
	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		8U,
		70U,
		line_buffer,
		&ssd1363_font_builtin_10x14,
		0x08U,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	snprintf(line_buffer, sizeof(line_buffer), "FULL %.1F UPS", full_result->updates_per_second);
	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		8U,
		92U,
		line_buffer,
		&ssd1363_font_builtin_10x14,
		0x0FU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	snprintf(line_buffer, sizeof(line_buffer), "TIME %.3FS", full_result->elapsed_seconds);
	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		8U,
		110U,
		line_buffer,
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
		132U,
		110U,
		"SEE SERIAL FOR FULL LOG",
		&ssd1363_font_builtin_5x7,
		0x08U,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	return ssd1363_framebuffer_flush(&g_gfx_framebuffer);
}

static esp_err_t ssd1363_demo_prepare_benchmark_scene(const char *mode_label, const ssd1363_font_t *counter_font, uint16_t *number_width, uint16_t *number_height)
{
	esp_err_t err;

	ssd1363_framebuffer_fill(&g_gfx_framebuffer, 0x00U);

	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		8U,
		20U,
		"REFRESH TEST",
		&ssd1363_font_builtin_10x14,
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
		&ssd1363_font_builtin_10x14,
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
		118U,
		"0 -> 100",
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
		112U,
		8U,
		mode_label,
		&ssd1363_font_builtin_5x7,
		0x0EU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	return ssd1363_text_measure_string(counter_font, "100", number_width, number_height);
}

static esp_err_t ssd1363_demo_run_counter_benchmark(const char *mode_label, bool use_partial_flush, ssd1363_demo_benchmark_result_t *result)
{
	esp_err_t err;
	ssd1363_text_bounds_t updated_bounds;
	uint16_t number_width = 0U;
	uint16_t number_height = 0U;
	char counter_text[16];
	int64_t start_time_us;
	int64_t end_time_us;
	const ssd1363_font_t *counter_font = &ssd1363_font_builtin_15x21;

	if (result == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	err = ssd1363_demo_prepare_benchmark_scene(mode_label, counter_font, &number_width, &number_height);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_framebuffer_fill_rect(&g_gfx_framebuffer, 8U, 58U, number_width, number_height, 0x00U);
	if (err != ESP_OK) {
		return err;
	}

	snprintf(counter_text, sizeof(counter_text), "%lu", SSD1363_COUNTER_START);
	err = ssd1363_text_write_string_full(
		&g_gfx_framebuffer,
		8U,
		58U,
		counter_text,
		counter_font,
		0x0FU,
		0x00U,
		SSD1363_FRAMEBUFFER_BITMAP_OPAQUE
	);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_framebuffer_flush(&g_gfx_framebuffer);
	if (err != ESP_OK) {
		return err;
	}

	start_time_us = esp_timer_get_time();

	for (uint32_t counter = SSD1363_COUNTER_START; counter <= SSD1363_COUNTER_END; ++counter) {
		snprintf(counter_text, sizeof(counter_text), "%lu", (unsigned long)counter);
		ssd1363_text_bounds_clear(&updated_bounds);

		err = ssd1363_framebuffer_fill_rect(&g_gfx_framebuffer, 8U, 58U, number_width, number_height, 0x00U);
		if (err != ESP_OK) {
			return err;
		}

		err = ssd1363_text_draw_string(
			&g_gfx_framebuffer,
			8U,
			58U,
			counter_text,
			counter_font,
			0x0FU,
			0x00U,
			SSD1363_FRAMEBUFFER_BITMAP_OPAQUE,
			&updated_bounds
		);
		if (err != ESP_OK) {
			return err;
		}

		if (use_partial_flush) {
			err = ssd1363_framebuffer_flush_rect(&g_gfx_framebuffer, 8U, 58U, number_width, number_height);
		} else {
			err = ssd1363_framebuffer_flush(&g_gfx_framebuffer);
		}
		if (err != ESP_OK) {
			return err;
		}
	}

	end_time_us = esp_timer_get_time();
	result->label = mode_label;
	result->elapsed_seconds = (double)(end_time_us - start_time_us) / 1000000.0;
	result->updates_per_second = (result->elapsed_seconds > 0.0)
		? ((double)(SSD1363_COUNTER_END - SSD1363_COUNTER_START + 1UL) / result->elapsed_seconds)
		: 0.0;

	printf("%s finished: %lu updates in %.3f s (%.2f updates/s)\n",
		mode_label,
		(unsigned long)(SSD1363_COUNTER_END - SSD1363_COUNTER_START + 1UL),
		result->elapsed_seconds,
		result->updates_per_second);

	return ESP_OK;
}

void ssd1363_demo_run_i2c_smoke_test(void)
{
	ssd1363_demo_run_i2c_smoke_test_impl();
}

static esp_err_t show_grayscale_text_scene(void)
{
	ssd1363_demo_benchmark_result_t rect_result = {0};
	ssd1363_demo_benchmark_result_t full_result = {0};
	esp_err_t err;

	ssd1363_framebuffer_init(&g_gfx_framebuffer);
	printf("Running partial update benchmark with flush_rect()\n");
	err = ssd1363_demo_run_counter_benchmark("MODE RECT", true, &rect_result);
	if (err != ESP_OK) {
		return err;
	}

	vTaskDelay(pdMS_TO_TICKS(750));

	printf("Running full-screen benchmark with flush()\n");
	err = ssd1363_demo_run_counter_benchmark("MODE FULL", false, &full_result);
	if (err != ESP_OK) {
		return err;
	}

	printf("REFRESH SUMMARY RECT %.2f UPS FULL %.2f UPS\n",
		rect_result.updates_per_second,
		full_result.updates_per_second);

	return ssd1363_demo_show_refresh_results(&rect_result, &full_result);
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
	printf("Displaying 1-pixel horizontal line example\n");

	err = ssd1363_demo_show_line_example();
	if (err != ESP_OK) {
		printf("Line example failed: %s\n", esp_err_to_name(err));
		return;
	}

	vTaskDelay(pdMS_TO_TICKS(2000));

	printf("Displaying partial vs full refresh benchmark\n");

	err = show_grayscale_text_scene();
	if (err != ESP_OK) {
		printf("Counter refresh test failed: %s\n", esp_err_to_name(err));
		return;
	}

	printf("Refresh benchmark displayed and completed\n");

	while (true) {
		vTaskDelay(pdMS_TO_TICKS(3000));
	}
}