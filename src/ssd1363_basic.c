#include "ssd1363_basic.h"

#include <stddef.h>

#include "ssd1363_api.h"
#include "ssd1363_config.h"

#define SSD1363_BASIC_FULL_BUFFER_SIZE ((SSD1363_ACTIVE_WIDTH * SSD1363_ACTIVE_HEIGHT) / 2U)

static esp_err_t ssd1363_basic_set_full_window(void)
{
	return ssd1363_api_set_window(
		0,
		(SSD1363_ACTIVE_WIDTH / 4U) - 1U,
		0,
		SSD1363_ACTIVE_HEIGHT - 1U
	);
}

esp_err_t ssd1363_basic_init(ssd1363_bus_t bus)
{
	esp_err_t err = ssd1363_api_init(bus);
	if (err != ESP_OK) {
		return err;
	}

	err = ssd1363_api_panel_init();
	if (err != ESP_OK) {
		return err;
	}

	return ssd1363_basic_set_full_window();
}

esp_err_t ssd1363_basic_deinit(void)
{
	return ssd1363_interface_deinit();
}

esp_err_t ssd1363_basic_display_on(void)
{
	return ssd1363_api_display_on();
}

esp_err_t ssd1363_basic_display_off(void)
{
	return ssd1363_api_display_off();
}

esp_err_t ssd1363_basic_clear(void)
{
	return ssd1363_basic_fill(0x00);
}

esp_err_t ssd1363_basic_fill(uint8_t pattern)
{
	return ssd1363_api_fill_active_area(pattern, SSD1363_ACTIVE_WIDTH, SSD1363_ACTIVE_HEIGHT);
}

esp_err_t ssd1363_basic_write_buffer(const uint8_t *buffer, size_t len)
{
	if (buffer == NULL || len != SSD1363_BASIC_FULL_BUFFER_SIZE) {
		return ESP_ERR_INVALID_ARG;
	}

	esp_err_t err = ssd1363_basic_set_full_window();
	if (err != ESP_OK) {
		return err;
	}

	return ssd1363_api_send_buffer(buffer, len);
}