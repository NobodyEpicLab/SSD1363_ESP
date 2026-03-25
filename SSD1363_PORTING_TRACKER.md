# SSD1363 Porting Tracker

## Goal

Build a clean, reusable SSD1363 OLED library for ESP32 with ESP-IDF and PlatformIO, using SSD1322-style structural ideas where useful but keeping the implementation specific to the real SSD1363 controller and the tested hardware.

Project layout remains intentionally flat:

- Headers in `include/`
- Sources in `src/`
- I2C is the current validated path
- SPI remains scaffolded for future hardware validation

Long-term goal:

- turn this into a public SSD1363 library that is understandable, reusable, and useful for other people working with grayscale SSD1363 modules

## Current Verified State

The following is now confirmed on real hardware:

- SSD1363 panel bring-up works
- Full-screen output works across the whole `256x128` display
- Grayscale output works
- Text rendering works cleanly
- The project builds and uploads successfully with PlatformIO + ESP-IDF
- Current working transport is I2C

This project is no longer only a smoke-test scaffold. It now has a working layered structure with transport, controller API, higher-level helpers, framebuffer drawing, and text rendering.

## Confirmed Hardware / Runtime Parameters

- Board: `az-delivery-devkit-v4`
- Framework: `ESP-IDF`
- Environment: `PlatformIO`
- Display: `SSD1363`, `256x128`, `16 grayscale levels`
- Active bus: `I2C`
- I2C address: `0x3C`
- SDA: `21`
- SCL: `22`
- I2C speed: `400000`
- Reset pin: not connected in the tested setup
- Working display offset: `0x20`
- Working remap: `0x32 0x00`
- Working column offset: `8`

## Confirmed Panel-Specific GDDRAM Format

This was the real root cause behind the earlier text corruption and rendering artifacts.

Confirmed behavior for the current module:

- SSD1363 command `0x15` column addressing must be treated in `4-pixel` units for this write path
- Logical framebuffer pixels are stored as `4bpp`
- Even `x` pixel uses the low nibble
- Odd `x` pixel uses the high nibble
- The two bytes inside each SSD1363 `4-pixel` group must be swapped before writing to GDDRAM

Important interpretation:

- this is panel-specific packed-pixel encoding, not a logical coordinate shift
- the framebuffer uses normal logical coordinates
- the translation into panel-specific byte order belongs in the display write/encoding path

## Current Architecture

### Project structure

- Keep all public headers in `include`
- Keep all source files in `src`
- Do not use a nested library folder under `lib`
- Keep `src/CMakeLists.txt` aware of the flat header layout

### Module layout

- `ssd1363_interface`
  - transport layer for I2C and SPI
- `ssd1363_api`
  - low-level SSD1363 command wrappers and initialization
- `ssd1363_basic`
  - higher-level, easier-to-use display helpers above the raw API
- `ssd1363_framebuffer`
  - fixed-size `256x128` `4bpp` framebuffer and drawing primitives
- `ssd1363_text`
  - text drawing and bounds handling
- `ssd1363_fonts`
  - bundled font descriptors and assets
- `ssd1363_demo`
  - example/demo scene code separated from the core driver

## Progress

| Phase | Status | Notes |
|---|---|---|
| 1. Match project structure | Done | Flat `include` + `src` layout is in place |
| 2. Enable build to see `include` | Done | Build system recognizes the public headers |
| 3. Create and maintain tracker | Done | This file remains the project log and has now been refreshed |
| 4. Add dual-bus transport scaffold | Done | I2C works, SPI scaffold compiles |
| 5. Add SSD1363 API layer | Done | Command/data/window/init wrappers exist |
| 6. Confirm panel wiring and parameters | Done | I2C `0x3C`, SDA `21`, SCL `22`, no reset pin |
| 7. Finalize working init baseline | Done | Current known-good baseline is frozen into the driver |
| 8. Bring up display with hardware smoke test | Done | Full display now responds correctly |
| 9. Build framebuffer layer | Done | `4bpp` framebuffer and drawing primitives implemented |
| 10. Build text layer | Done | Text renders cleanly using the current GFXfont-based path |
| 11. Fix panel-specific GDDRAM mapping | Done | Root cause of text/rendering glitches identified and fixed |
| 12. Add higher-level basic layer | Done | Simpler wrapper layer exists for demos and user code |
| 13. Expand and document public headers | Done | `basic` and `api` headers now have clearer documentation |
| 14. Prepare public-facing repo docs | In progress | README exists, more polish is still possible |
| 15. Validate future SPI migration path | Pending | Needs real SPI wiring and hardware test |

## Current Files

### Headers in `include/`

- `FreeMono12pt7b.h`
- `ssd1363_api.h`
- `ssd1363_basic.h`
- `ssd1363_config.h`
- `ssd1363_demo.h`
- `ssd1363_fonts.h`
- `ssd1363_framebuffer.h`
- `ssd1363_gfxfont.h`
- `ssd1363_interface.h`
- `ssd1363_text.h`

### Sources in `src/`

- `main.c`
- `ssd1363_api.c`
- `ssd1363_basic.c`
- `ssd1363_demo.c`
- `ssd1363_fonts.c`
- `ssd1363_framebuffer.c`
- `ssd1363_interface.c`
- `ssd1363_text.c`

## What Is Implemented Right Now

### Transport layer

- I2C init/deinit
- I2C command/data writes with control bytes
- SPI init/deinit scaffold
- SPI transfer path with DC-line handling
- Optional reset handling
- Runtime bus selection

### API layer

- Raw command send
- Raw single-byte data send
- Raw data-buffer send
- Column/row/window setup
- Packed buffer send to GDDRAM
- Frozen working panel init path
- Named wrappers for the SSD1363 commands already needed by the current implementation
- Contrast and master-contrast wrappers

### Basic layer

- Driver init/deinit
- Display on/off
- Normal display mode selection
- Clear/fill helpers
- Contrast helper
- Master-contrast helper
- Start-line helper
- Display-offset helper
- Remap helper
- Default gray-table helper
- Full framebuffer write
- Bounded partial-area write
- Public header comments describing behavior, arguments, and ranges

### Framebuffer layer

- Fixed `256x128` `4bpp` framebuffer ownership
- Fill / clear
- Pixel read/write
- Rectangle outline/fill
- Line drawing
- Monochrome bitmap blit support
- Full flush
- Partial flush for changed rectangular regions
- Panel-specific row encoding before writeout

### Text layer

- String drawing
- Character drawing
- Changed-bounds reporting for partial flush
- Grayscale foreground values
- Transparent and opaque background behavior
- Readable GFXfont/GFXglyph-style backend
- Bundled `FreeMono12pt7b`-based font support

### Demo layer

- Dedicated demo/example code separated from `main.c`
- Text-oriented demo scene
- Demo-level direct contrast control can be set from the demo code

## What Is No Longer Accurate

The following older assumptions or tracker statements are no longer current:

- initialization is no longer merely tentative bring-up scaffolding
- framebuffer work is no longer deferred
- text support is no longer just planned
- the project is no longer limited to raw smoke-test command/data writes
- the display is no longer only partially working
- the main unresolved rendering problem was not font quality but GDDRAM packing format
- the temporary SSD1322 reference clone is no longer part of the project

## Remaining Work

These are the real remaining tasks based on the current project state.

1. Continue public API cleanup so higher-level code rarely needs low-level register calls
2. Add more safe high-level wrappers where they make sense, especially for clock, phase length, precharge, second precharge, and VCOMH
3. Keep improving header/source documentation so the repo is easier for external users to understand
4. Validate the SPI transport path on real hardware
5. Expand documentation around configuration, wiring, example usage, and grayscale behavior
6. Decide whether to expose custom gray-table support beyond the default gray-table helper
7. Keep polishing the demo so it remains a small stable example rather than a debug scratchpad
8. Consider a cleaner explicit abstraction for panel-specific GDDRAM packing if future displays need different formats

## Open Questions

1. Which SPI pins will be used when the hardware is moved from I2C to SPI?
2. Does this exact display module require any extra timing or power sequencing beyond what is already working in the current tested setup?
3. Which additional SSD1363 tuning commands are worth exposing in the `basic` layer versus leaving in the low-level API?
4. Should the public release keep only one bundled readable font first, or immediately include more sizes?

## Current Interpretation

The project has moved out of the risky bring-up phase.

Current status should be understood as:

- the hardware path is proven on I2C
- the panel mapping issue is solved for the tested module
- the layered driver architecture is established
- the main remaining work is API polish, documentation, and future SPI validation

## Next Recommended Steps

1. Continue expanding the higher-level API only where it improves usability without hiding important controller behavior
2. Keep the current I2C path as the validated baseline while documenting the exact tested configuration clearly
3. Validate SPI on real hardware when pins and module wiring are available
4. Do a final public-release cleanup pass on documentation, naming consistency, and examples

## Change Log

### 2026-03-23

- Adopted the flat `include` + `src` project structure
- Updated build registration so public headers are visible to the project
- Added dual-bus SSD1363 transport scaffolding with I2C-first support
- Added the initial SSD1363 API layer for commands, data, windows, and raw buffer writes
- Confirmed the module responds on I2C address `0x3C`
- Established the real tested setup with SDA `21`, SCL `22`, and no reset pin
- Performed the early smoke-test iterations that exposed row/remap/window mapping problems
- Confirmed the working baseline around display offset `0x20`, remap `0x32 0x00`, and column offset `8`
- Corrected column-window programming so `0x15` uses `4-pixel` addressing units in the current write path
- Reached full-panel working output on the real display

### 2026-03-24

- Moved panel bring-up into a reusable driver-facing initialization path
- Added more named SSD1363 command wrappers instead of relying only on raw writes
- Moved demo/smoke-test logic out of `main.c` into a dedicated demo module
- Added the `basic` helper layer above the low-level API
- Added a dedicated `4bpp` framebuffer with drawing primitives and partial flush support
- Added the text layer and iterated through multiple font approaches
- Switched to a readable GFXfont-style backend rather than opaque embedded font blobs
- Identified the real text-glitch root cause as panel-specific packed-pixel byte ordering
- Fixed the panel write path so text became sharp and clean on hardware
- Removed the temporary SSD1322 reference clone after it was no longer needed
- Added a public-facing README with upstream credits and project description

### 2026-03-25

- Restored demo-level contrast control through the SSD1363 contrast command path
- Expanded the `basic` layer with more user-facing helpers
- Added documentation comments to the public `basic` and `api` headers
- Refreshed this tracker so it reflects the current real project state instead of earlier bring-up assumptions
