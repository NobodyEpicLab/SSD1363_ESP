#ifndef SSD1363_BASIC_H
#define SSD1363_BASIC_H

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "ssd1363_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ssd1363_basic_init(ssd1363_bus_t bus);
esp_err_t ssd1363_basic_deinit(void);
esp_err_t ssd1363_basic_display_on(void);
esp_err_t ssd1363_basic_display_off(void);
esp_err_t ssd1363_basic_clear(void);
esp_err_t ssd1363_basic_fill(uint8_t pattern);
esp_err_t ssd1363_basic_write_buffer(const uint8_t *buffer, size_t len);

#ifdef __cplusplus
}
#endif

#endif