#ifndef SSD1363_INTERFACE_H
#define SSD1363_INTERFACE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SSD1363_BUS_I2C = 0,
    SSD1363_BUS_SPI = 1,
} ssd1363_bus_t;

esp_err_t ssd1363_interface_init(ssd1363_bus_t bus);
esp_err_t ssd1363_interface_deinit(void);
ssd1363_bus_t ssd1363_interface_get_bus(void);

esp_err_t ssd1363_interface_write_command(const uint8_t *data, size_t len);
esp_err_t ssd1363_interface_write_data(const uint8_t *data, size_t len);

esp_err_t ssd1363_interface_reset(bool high);
void ssd1363_interface_delay_ms(uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif