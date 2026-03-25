# SSD1363 OLED Library for ESP-IDF / PlatformIO

SSD1363 OLED library project for ESP32, focused on real SSD1363 controller support with **4-bit grayscale** support.

This project was created because usable SSD1363 libraries are limited, and because grayscale support is important for these displays.

THIS PROJECT IS STILL VERY MUCH IN PROGRESS, but some of the function works already, gray scale and text for example. I could not find any librarys that would support gray for this display scale so decided to write it myself with strong GPT support :)

## What This Is

This repository is an in-progress SSD1363 driver and graphics library for:

- ESP32
- ESP-IDF
- PlatformIO
- 256x128 SSD1363 OLED displays

Current development has been done primarily against an I2C-connected SSD1363 module.

## Important Features

- **Works with SSD1363**
- **Supports grayscale**
- 4bpp framebuffer for `256x128`
- Pixel, line, rectangle, and bitmap drawing
- Text rendering layer
- PlatformIO + ESP-IDF project structure
- I2C transport working on real hardware
- SPI path scaffolded for future validation

## Current Status

This project is working, but it is still under active development.

What is currently confirmed:

- SSD1363 panel bring-up works on real hardware
- Full-screen output works
- Grayscale output works
- Text rendering works cleanly after matching the panel's real GDDRAM packing format
- The project builds with PlatformIO using ESP-IDF

What is not final yet:

- The public API still needs cleanup
- SPI support is scaffolded, but not yet hardware-validated
- More SSD1363 commands still need dedicated wrappers
- Documentation still needs to be expanded

## Minimum Usable API

If you want to use the library without touching low-level controller commands, start with the `ssd1363_basic`, `ssd1363_framebuffer`, and `ssd1363_text` layers.

Recommended starting point:

- `ssd1363_basic_init(bus)`
	- initialize transport and panel
- `ssd1363_basic_display_on()` / `ssd1363_basic_display_off()`
	- panel output control
- `ssd1363_basic_set_contrast(value)`
	- quick visible brightness/current tuning
- `ssd1363_basic_set_master_contrast(value)`
	- coarse global contrast control
- `ssd1363_framebuffer_init(&fb)`
	- initialize framebuffer state
- `ssd1363_framebuffer_fill(&fb, gray)`
	- clear or fill the whole framebuffer
- `ssd1363_framebuffer_set_pixel(...)`
- `ssd1363_framebuffer_draw_line(...)`
- `ssd1363_framebuffer_draw_rect(...)`
- `ssd1363_framebuffer_fill_rect(...)`
	- basic drawing primitives
- `ssd1363_text_draw_string(...)`
	- draw text into the framebuffer
- `ssd1363_framebuffer_flush(&fb)`
	- write the full framebuffer to the display
- `ssd1363_framebuffer_flush_rect(&fb, x, y, w, h)`
	- update only part of the display for smaller changes

If you need more tuning but still want to stay above the raw command layer, the `basic` layer now also exposes helpers for:

- start line
- display offset
- remap
- default gray table
- display clock
- multiplex ratio
- phase length
- precharge voltage
- second precharge
- VCOMH
- segment low voltage
- display enhancement

The lower `ssd1363_api` layer is still available when you want direct SSD1363 register-level control.

## Tested Setup

Current known working setup during development:

- Board: `az-delivery-devkit-v4`
- Framework: `ESP-IDF`
- Environment: `PlatformIO`
- Display: `SSD1363`, `256x128`, `16 grayscale levels`
- Active bus: `I2C`
- I2C address: `0x3C`
- SDA: `21`
- SCL: `22`
- Reset pin: not connected in the tested setup

## Project Layout

The project intentionally uses a flat structure:

- Public headers in `include/`
- Source files in `src/`

Main modules currently include:

- `ssd1363_interface` - transport layer for I2C/SPI
- `ssd1363_api` - low-level SSD1363 command layer
- `ssd1363_basic` - simpler high-level display write layer
- `ssd1363_framebuffer` - 4bpp framebuffer and drawing primitives
- `ssd1363_text` / `ssd1363_fonts` - text rendering and bundled font support

## Why Grayscale Matters

This library is specifically intended for **SSD1363 grayscale OLED displays**.

That means this is not just a monochrome text library with SSD1363 naming. The project is built around:

- `16` grayscale levels
- packed `4bpp` framebuffer storage
- SSD1363 GDDRAM-oriented writes
- future grayscale-focused controller features

## Credits and Upstream References

This project is not created in isolation. Important ideas, structure, and reference material came from earlier work by other authors.

### SSD1322 OLED Library

A major structural reference for this project was:

- [wjklimek1/SSD1322_OLED_library](https://github.com/wjklimek1/SSD1322_OLED_library/tree/master)

Credit goes to:

- `wjklimek1`
- all contributors to `SSD1322_OLED_library`

This project especially benefited from that library's organization and general driver / graphics layering approach.

### Adafruit GFX Font Format

The current readable font backend uses the `GFXglyph` / `GFXfont` style that is widely known from Adafruit GFX font assets.

Credit goes to:

- Adafruit Industries
- contributors to the Adafruit GFX ecosystem

The bundled `FreeMono12pt7b` font asset included in this repository carries its own BSD license text in the font header.

### U8g2 Reference Work

During SSD1363 bring-up and mapping validation, SSD1363-related behavior was also compared against known public SSD1363 work from:

- [olikraus/u8g2](https://github.com/olikraus/u8g2)

Credit goes to:

- `olikraus`
- all contributors to `u8g2`

## Goal of This Repository

The goal is to grow this into a clean, reusable public SSD1363 library that:

- actually works on real hardware
- supports grayscale properly
- is understandable to maintain
- is useful for other people working with SSD1363 displays

## License Notes

Please review the licenses of this repository and bundled assets before reuse.

In particular:

- this project should keep upstream credit visible
- bundled font assets may have their own license text
- upstream reference projects remain the work of their original authors and contributors

## Development Notes

The project tracker is here:

- `SSD1363_PORTING_TRACKER.md`

That file contains implementation progress, current assumptions, and planned next steps.
