#ifndef SSD1363_CONFIG_H
#define SSD1363_CONFIG_H

/*
 * Placeholders below are chosen to keep the scaffold compileable.
 * They must be validated against the actual SSD1363 module wiring.
 */

#define SSD1363_PANEL_WIDTH           256
#define SSD1363_PANEL_HEIGHT          128
#define SSD1363_GRAYSCALE_LEVELS      16

/*
 * Smoke-test geometry is intentionally separated from the nominal panel size.
 * The module itself is marked 256x128, so the current centered-64-row symptom
 * is treated as a controller initialization or COM-mapping issue rather than
 * a true panel-size fact.
 */
#define SSD1363_ACTIVE_WIDTH          256
#define SSD1363_ACTIVE_HEIGHT         128
#define SSD1363_DISPLAY_OFFSET        0x20
#define SSD1363_DISPLAY_START_LINE    0x00
#define SSD1363_MULTIPLEX_RATIO       (SSD1363_ACTIVE_HEIGHT - 1)
#define SSD1363_REMAP_BYTE0           0x32
#define SSD1363_REMAP_BYTE1           0x00

#define SSD1363_I2C_PORT               0
#define SSD1363_I2C_SDA_PIN            21
#define SSD1363_I2C_SCL_PIN            22
#define SSD1363_I2C_CLOCK_HZ           400000
#define SSD1363_I2C_ADDRESS            0x3C
#define SSD1363_I2C_TIMEOUT_MS         2000
#define SSD1363_I2C_CONTROL_COMMAND    0x00
#define SSD1363_I2C_CONTROL_DATA       0x40
#define SSD1363_I2C_DATA_CHUNK_SIZE    32

#define SSD1363_SPI_HOST               2
#define SSD1363_SPI_MOSI_PIN           23
#define SSD1363_SPI_SCLK_PIN           18
#define SSD1363_SPI_CS_PIN             -1
#define SSD1363_SPI_DC_PIN             -1
#define SSD1363_RESET_PIN              -1
#define SSD1363_SPI_CLOCK_HZ           10000000
#define SSD1363_SPI_QUEUE_SIZE         4

/*
 * SPI remains intentionally uncommitted for now.
 * The placeholders above exist only so the dual-bus scaffold keeps compiling.
 */

#define SSD1363_COLUMN_OFFSET          8

#endif