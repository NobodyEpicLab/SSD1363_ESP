#include "ssd1363_demo.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ssd1363_api.h"
#include "ssd1363_config.h"

#define SSD1363_FRAMEBUFFER_SIZE ((SSD1363_ACTIVE_WIDTH * SSD1363_ACTIVE_HEIGHT) / 2)

static uint8_t g_framebuffer[SSD1363_FRAMEBUFFER_SIZE];

static void build_vertical_stripe_pattern(uint8_t on_nibble, uint8_t off_nibble)
{
	for (size_t index = 0; index < sizeof(g_framebuffer); ++index) {
		g_framebuffer[index] = (uint8_t)((on_nibble << 4) | off_nibble);
	}
}

static void build_horizontal_stripe_pattern(void)
{
	const size_t bytes_per_row = SSD1363_ACTIVE_WIDTH / 2;

	for (uint16_t row = 0; row < SSD1363_ACTIVE_HEIGHT; ++row) {
		uint8_t value = (row % 2 == 0) ? 0xFF : 0x00;
		memset(&g_framebuffer[row * bytes_per_row], value, bytes_per_row);
	}
}

static esp_err_t probe_i2c_address(uint8_t address)
{
	i2c_cmd_handle_t command = i2c_cmd_link_create();
	if (command == NULL) {
		return ESP_ERR_NO_MEM;
	}

	i2c_master_start(command);
	i2c_master_write_byte(command, (address << 1) | I2C_MASTER_WRITE, true);
	i2c_master_stop(command);

	esp_err_t err = i2c_master_cmd_begin(
		(i2c_port_t)SSD1363_I2C_PORT,
		command,
		pdMS_TO_TICKS(50)
	);

	i2c_cmd_link_delete(command);
	return err;
}

static void scan_i2c_bus(void)
{
	printf("Scanning I2C bus on port %d...\n", SSD1363_I2C_PORT);

	for (uint8_t address = 0x03; address < 0x78; ++address) {
		if (probe_i2c_address(address) == ESP_OK) {
			printf("I2C device found at 0x%02X\n", address);
		}
	}
}

static esp_err_t write_fullscreen_pattern(uint8_t pattern)
{
	memset(g_framebuffer, pattern, sizeof(g_framebuffer));

	return ssd1363_api_send_buffer(g_framebuffer, sizeof(g_framebuffer));
}

static esp_err_t write_framebuffer(void)
{
	return ssd1363_api_send_buffer(g_framebuffer, sizeof(g_framebuffer));
}

void ssd1363_demo_run_i2c_smoke_test(void)
{
	static const uint8_t patterns[] = {0x00, 0xFF, 0xF0, 0x0F, 0x55, 0xAA};

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

	esp_err_t err = ssd1363_api_init(SSD1363_BUS_I2C);
	if (err != ESP_OK) {
		printf("ssd1363_api_init failed: %s\n", esp_err_to_name(err));
		return;
	}

	scan_i2c_bus();

	err = probe_i2c_address(SSD1363_I2C_ADDRESS);
	if (err != ESP_OK) {
		printf("Configured address 0x%02X did not ACK: %s\n", SSD1363_I2C_ADDRESS, esp_err_to_name(err));
		return;
	}

	printf("Address 0x%02X acknowledged, sending baseline panel init\n", SSD1363_I2C_ADDRESS);

	err = ssd1363_api_panel_init();
	if (err != ESP_OK) {
		printf("Panel init failed: %s\n", esp_err_to_name(err));
		return;
	}

	err = ssd1363_api_set_window(
		0,
		(SSD1363_ACTIVE_WIDTH / 4) - 1,
		0,
		SSD1363_ACTIVE_HEIGHT - 1
	);
	if (err != ESP_OK) {
		printf("Initial window setup failed: %s\n", esp_err_to_name(err));
		return;
	}

	printf("Panel init complete, cycling fill and stripe patterns\n");

	while (true) {
		err = ssd1363_api_fill_active_area(0x00, SSD1363_ACTIVE_WIDTH, SSD1363_ACTIVE_HEIGHT);
		if (err != ESP_OK) {
			printf("Full black fill failed: %s\n", esp_err_to_name(err));
			return;
		}
		printf("Displayed direct fill 0x00\n");
		vTaskDelay(pdMS_TO_TICKS(1000));

		err = ssd1363_api_fill_active_area(0xFF, SSD1363_ACTIVE_WIDTH, SSD1363_ACTIVE_HEIGHT);
		if (err != ESP_OK) {
			printf("Full white fill failed: %s\n", esp_err_to_name(err));
			return;
		}
		printf("Displayed direct fill 0xFF\n");
		vTaskDelay(pdMS_TO_TICKS(1000));

		build_horizontal_stripe_pattern();
		err = write_framebuffer();
		if (err != ESP_OK) {
			printf("Horizontal stripe pattern failed: %s\n", esp_err_to_name(err));
			return;
		}
		printf("Displayed horizontal stripe pattern\n");
		vTaskDelay(pdMS_TO_TICKS(1000));

		build_vertical_stripe_pattern(0xF, 0x0);
		err = write_framebuffer();
		if (err != ESP_OK) {
			printf("Vertical stripe pattern failed: %s\n", esp_err_to_name(err));
			return;
		}
		printf("Displayed vertical stripe pattern\n");
		vTaskDelay(pdMS_TO_TICKS(1000));

		for (size_t index = 0; index < (sizeof(patterns) / sizeof(patterns[0])); ++index) {
			err = write_fullscreen_pattern(patterns[index]);
			if (err != ESP_OK) {
				printf("Pattern 0x%02X write failed: %s\n", patterns[index], esp_err_to_name(err));
				return;
			}

			printf("Displayed pattern 0x%02X\n", patterns[index]);
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}
}