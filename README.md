# SSD1363 OLED Library for ESP-IDF / PlatformIO

SSD1363 grayscale OLED driver and graphics library for ESP32, built around real hardware bring-up instead of datasheet-only assumptions.

The main reason this project exists is simple: usable SSD1363 libraries are very hard to find, especially ones that actually handle grayscale and work in a modern PlatformIO + ESP-IDF setup.

## What This Library Provides

- Real SSD1363 initialization and display output on tested hardware
- `4bpp` grayscale framebuffer for `256x128`
- Pixel, line, rectangle, divider, and bitmap drawing
- Text rendering with built-in and imported GFXfont-based fonts
- Layered architecture for transport, controller commands, practical helpers, framebuffer, and text
- PlatformIO project structure ready for ESP-IDF development

## Current State

This library is already usable.

Confirmed working:

- SSD1363 panel bring-up on real hardware
- Full-screen drawing
- Partial updates
- `16` grayscale levels
- Clean text rendering after matching the panel's real GDDRAM packing format
- PlatformIO build and upload flow

Not finalized yet:

- SPI is scaffolded but not hardware-validated
- Some SSD1363 features still only exist at the raw command level
- Public API naming is stable enough to use, but may still evolve slowly over time

## Tested Hardware

Validated development setup:

- Board: `az-delivery-devkit-v4`
- Framework: `ESP-IDF`
- Environment: `PlatformIO`
- Display: `SSD1363`, `256x128`, `16 grayscale levels`
- Active bus: `I2C`
- I2C address: `0x3C`
- SDA: `21`
- SCL: `22`
- Reset pin: not connected in the tested setup

## Why This Matters

This is not just a monochrome OLED library with SSD1363 naming.

The library is built around real SSD1363 grayscale behavior:

- packed `4bpp` framebuffer storage
- SSD1363-specific windowing and writes
- panel-specific GDDRAM packing behavior
- support for grayscale-oriented UI work

That makes it useful for dashboards, ECU-style displays, test equipment screens, and similar applications where grayscale and larger fonts matter.

## Project Layout

The project intentionally keeps a flat layout:

- public headers in `include/`
- source files in `src/`

Main modules:

- `ssd1363_interface`: low-level transport layer for I2C and future SPI
- `ssd1363_api`: raw SSD1363 command layer
- `ssd1363_basic`: practical high-level controller helpers
- `ssd1363_framebuffer`: local `4bpp` framebuffer and drawing primitives
- `ssd1363_text`: text measurement and rendering
- `ssd1363_fonts`: public font registry
- `ssd1363_demo`: smoke test and refresh benchmark
- `ssd1363_font_presentation`: separate font preview application

## Recommended Starting Point

If you want to use the display without thinking about raw controller commands, work with these layers:

- `ssd1363_basic`
- `ssd1363_framebuffer`
- `ssd1363_text`

Typical workflow:

1. Initialize the display with `ssd1363_basic_init(...)`
2. Create or clear a framebuffer with `ssd1363_framebuffer_init(...)` or `ssd1363_framebuffer_fill(...)`
3. Draw graphics with framebuffer helpers
4. Draw text with `ssd1363_text_draw_string(...)`
5. Push the result to the display with `ssd1363_framebuffer_flush(...)` or `ssd1363_framebuffer_flush_rect(...)`

Useful high-level functions include:

- `ssd1363_basic_init(...)`
- `ssd1363_basic_display_on()` / `ssd1363_basic_display_off()`
- `ssd1363_basic_set_contrast(...)`
- `ssd1363_basic_set_master_contrast(...)`
- `ssd1363_framebuffer_set_pixel(...)`
- `ssd1363_framebuffer_draw_line(...)`
- `ssd1363_framebuffer_draw_rect(...)`
- `ssd1363_framebuffer_fill_rect(...)`
- `ssd1363_framebuffer_draw_divider(...)`
- `ssd1363_text_draw_string(...)`
- `ssd1363_framebuffer_flush(...)`
- `ssd1363_framebuffer_flush_rect(...)`

If you want direct datasheet-level control, drop down into `ssd1363_api`.

## Font Naming

Preferred public font names use pixel height in the symbol name, for example:

- `ssd1363_font_freemono_24px`
- `ssd1363_font_cascadiacode_29px`
- `ssd1363_font_ibmplexmonolight_32px`

Older `12pt`, `x2`, and `x3` names are still exported as compatibility aliases so existing code does not break.

## Why Another SSD1363 Library

Because there are very few SSD1363 libraries that:

- clearly target this controller
- support grayscale properly
- are structured for ESP-IDF / PlatformIO
- are known to run on real hardware

This repository is intended to be a practical starting point, not just a partial port or an unverified command list.

## Credits and References

This project was informed by earlier public work and should keep that credit visible.

Structural and architectural inspiration:

- [wjklimek1/SSD1322_OLED_library](https://github.com/wjklimek1/SSD1322_OLED_library/tree/master)

Font backend format:

- Adafruit GFX `GFXglyph` / `GFXfont`

Bring-up comparison and SSD1363 behavior cross-checking:

- [olikraus/u8g2](https://github.com/olikraus/u8g2)

The bundled `FreeMono12pt7b` asset includes its own BSD license text in the font header.

## Development Notes

Project progress and working assumptions are tracked in:

- `SSD1363_PORTING_TRACKER.md`

## License Notes

Please review licenses before reuse, especially for bundled font assets and upstream reference material.
