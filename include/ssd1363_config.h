#ifndef SSD1363_CONFIG_H
#define SSD1363_CONFIG_H

/*
 * Core panel geometry for the tested SSD1363 module.
 */

#define SSD1363_PANEL_WIDTH           256
#define SSD1363_PANEL_HEIGHT          128
#define SSD1363_GRAYSCALE_LEVELS      16

#define SSD1363_ACTIVE_WIDTH          256
#define SSD1363_ACTIVE_HEIGHT         128

/*
 * Tested panel profile values for the currently working module.
 *
 * These values describe the controller setup and GDDRAM packing behavior that
 * has been validated on the current 256x128 I2C module.
 */
#define SSD1363_DISPLAY_OFFSET        0x20
#define SSD1363_DISPLAY_START_LINE    0x00
#define SSD1363_MULTIPLEX_RATIO       (SSD1363_ACTIVE_HEIGHT - 1)
#define SSD1363_REMAP_BYTE0           0x32
#define SSD1363_REMAP_BYTE1           0x00
#define SSD1363_PANEL_COLUMN_OFFSET   8
#define SSD1363_PANEL_COLUMN_ADDR_UNIT_PIXELS 4

#define SSD1363_PANEL_INIT_UNLOCK_CODE        0x12
#define SSD1363_PANEL_INIT_DISPLAY_CLOCK      0x30
#define SSD1363_PANEL_INIT_GPIO_CONFIG        0x00
#define SSD1363_PANEL_INIT_FUNCTION_SELECTION 0x01
#define SSD1363_PANEL_INIT_VSL0               0xA0
#define SSD1363_PANEL_INIT_VSL1               0xFD
#define SSD1363_PANEL_INIT_CONTRAST           0x7F
#define SSD1363_PANEL_INIT_MASTER_CONTRAST    0x0F
#define SSD1363_PANEL_INIT_PHASE_LENGTH       0x74
#define SSD1363_PANEL_INIT_ENHANCEMENT0       0x82
#define SSD1363_PANEL_INIT_ENHANCEMENT1       0x20
#define SSD1363_PANEL_INIT_PRECHARGE_VOLTAGE  0x1F
#define SSD1363_PANEL_INIT_SECOND_PRECHARGE   0x08
#define SSD1363_PANEL_INIT_VCOMH              0x07

/*
 * These values describe how this specific panel expects packed grayscale pixel
 * data in SSD1363 GDDRAM. They do not add visual offsets or move logical pixel
 * coordinates around; they only define how a logical framebuffer row is encoded
 * before being sent over the bus.
 *
 * - If set to 1, an even x pixel is stored in the high nibble of its byte.
 * - If set to 0, an even x pixel is stored in the low nibble of its byte.
 */
#define SSD1363_PANEL_EVEN_PIXEL_HIGH_NIBBLE  0

/*
 * SSD1363 column addressing operates in 4-pixel groups. Some panels wire the
 * two bytes inside each 4-pixel group in reverse bus order. When enabled, the
 * driver encodes each logical 4-pixel group into the byte order expected by
 * this panel before sending it to GDDRAM.
 */
#define SSD1363_PANEL_SWAP_4PX_GROUP_BYTES    1

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

#define SSD1363_COLUMN_OFFSET          SSD1363_PANEL_COLUMN_OFFSET
#define SSD1363_GDDRAM_EVEN_PIXEL_HIGH_NIBBLE SSD1363_PANEL_EVEN_PIXEL_HIGH_NIBBLE
#define SSD1363_GDDRAM_SWAP_4PX_GROUP_BYTES   SSD1363_PANEL_SWAP_4PX_GROUP_BYTES

#endif