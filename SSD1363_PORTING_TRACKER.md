# SSD1363 Porting Tracker

## Goal

Port the SSD1322-oriented library structure into this ESP-IDF PlatformIO project for an SSD1363-based display, while keeping the project layout flat:

- Headers in `include`
- Sources in `src`
- Current priority on I2C
- SPI scaffolded now so it can be enabled later without redesigning the codebase
- Long-term goal is to turn this into a reusable SSD1363 library that is clean enough for other users to adopt, especially because SSD1363 library options appear limited

## Current Decisions

### Project structure

- Keep all public headers in `include`
- Keep all C source files in `src`
- Do not use a nested library folder under `lib`
- Register `include` explicitly in `src/CMakeLists.txt`

### Communication model

- Implement a shared SSD1363 interface layer that supports both I2C and SPI
- Use I2C as the current active path because the existing display only responds on I2C
- Keep SPI code compileable from the start so the future hardware migration is limited to configuration and validation
- Leave SPI pin assignment unfixed for now; keep it scaffolded only

### Current implementation scope

- Phase 1 is transport and API scaffolding only
- Full SSD1363 initialization values are not finalized yet
- Graphics/framebuffer porting is deferred until transport and command paths are validated
- A later driver phase should expose a more complete SSD1363 command-table-oriented API so the project can use more of the controller's native features, especially grayscale-related controls
- The future reusable-library phase should also include text rendering support with multiple fonts and font sizes
- Font support should start with a small, clearly licensed, optional font set rather than a large bundled font dump

## Progress

| Phase | Status | Notes |
|---|---|---|
| 1. Match project structure | Done | Headers stay in `include`, sources stay in `src` |
| 2. Enable build to see `include` | Done | `src/CMakeLists.txt` updated |
| 3. Create porting tracker | Done | This file is the single source of truth |
| 4. Add dual-bus transport scaffold | Done | I2C implemented, SPI scaffolded and compileable |
| 5. Add SSD1363 API scaffold | Done | Basic command/data/window/buffer functions added |
| 5a. Validate scaffold build | Done | PlatformIO build reached configuration and workspace diagnostics report no errors |
| 6. Confirm panel wiring and parameters | In progress | I2C pins accepted, no reset pin confirmed, I2C address now confirmed as `0x3C` |
| 7. Finalize SSD1363 init sequence | In progress | Working baseline is now being frozen into the core driver API; remaining work is to validate which analog/power values are strictly required |
| 8. Bring up display with smoke test | Done | Full display is now responding across the whole panel |
| 9. Port framebuffer/GFX layer | In progress | The framebuffer layer now has pixel access, rectangle primitives, line drawing, icon-ready bitmap blit support, and a first partial-flush path for small updates |
| 9d. Add text layer | In progress | A tiny internal font format, grayscale text rendering, and the first built-in small and larger font descriptors are being added on top of the framebuffer layer |
| 9a. Expand SSD1363 command coverage | In progress | The baseline init path is being converted from raw writes into named command wrappers, starting with display, timing, and analog-tuning commands |
| 9b. Turn project into reusable library | In progress | Main entry is being reduced toward user code only, with demo and driver logic moved into dedicated files |
| 9c. Add example-layer API | In progress | A thin `basic` layer is being introduced above the core driver so user code and demos do not need to talk directly to low-level controller functions |
| 10. Validate future SPI migration path | Pending | Depends on final SPI wiring |

## Files Added In This Step

- `include/ssd1363_config.h`
- `include/ssd1363_interface.h`
- `include/ssd1363_api.h`
- `include/ssd1363_basic.h`
- `include/ssd1363_fonts.h`
- `include/ssd1363_framebuffer.h`
- `include/ssd1363_text.h`
- `src/ssd1363_interface.c`
- `src/ssd1363_api.c`
- `src/ssd1363_basic.c`
- `src/ssd1363_fonts.c`
- `src/ssd1363_framebuffer.c`
- `src/ssd1363_text.c`
- `src/main.c`

## Assumptions In The Current Scaffold

These are placeholders and must be validated against the real hardware:

- I2C port: `I2C_NUM_0`
- I2C pins: SDA `21`, SCL `22`
- I2C speed: `400000`
- Panel geometry currently assumed from the product listing: `256x128`, `16 grayscale levels`
- I2C address confirmed on hardware: `0x3C`
- I2C control bytes placeholder: command `0x00`, data `0x40`
- No reset pin is currently connected
- Current smoke-test tuning restored to `256x128` active height and now follows a known U8g2 SSD1363 `256x128` baseline more closely: display offset `0x20`, remap bytes `0x32 0x00`, column offset `8`
- SSD1363 column addressing is now treated as `4-pixel` address units for the `0x15` command, matching the known U8g2 SSD1363 write path
- SPI host placeholder: `SPI2_HOST`
- SPI default pins are placeholders intentionally left unresolved for now
- Reset/DC/CS pins are optional and disabled by default with `-1`
- SSD1363 column offset placeholder is `0`

## What Is Implemented Right Now

### Transport layer

- I2C init/deinit
- I2C write with control byte
- SPI init/deinit
- SPI transfer with DC line support
- Optional reset pin toggling
- Delay helper
- Runtime bus selection

### API layer

- Command send
- Single-byte data send
- Multi-byte data send
- Set column/row window
- Send raw display buffer to GDDRAM with I2C chunking
- Transport-level init hook
- Baseline SSD1363 panel initialization sequence frozen from the currently working smoke-test result
- Named command wrappers for display control, panel mapping, timing, and analog-tuning commands already used by the working baseline init

### Basic example layer

- A thin `ssd1363_basic` layer now sits above the low-level API
- This layer is intended to give demos and future user code a stable starting point without exposing raw controller details too early
- The current `basic` layer handles init, display on/off, clear, full-screen fill, and full-buffer write
- The basic layer now also provides bounded area writes for optimized partial screen updates when the pixel region is aligned to SSD1363 column-addressing requirements

### Framebuffer layer

- A dedicated fixed-size `256x128` `4bpp` framebuffer module now sits above the basic display layer
- This module provides buffer ownership, full-buffer clear/fill, single-pixel read/write, rectangle outline/fill primitives, line drawing, monochrome bitmap blit support for small icons, and both full-frame and partial flush paths
- Pixel packing is now confirmed for this module as: even `x` pixel in the low nibble, odd `x` pixel in the high nibble, plus a byte swap inside each SSD1363 `4-pixel` column group before data is written to GDDRAM

### Text layer

- A small internal font format now sits above the framebuffer layer, with sparse glyph lookup and string measurement/draw helpers
- Text rendering supports grayscale foreground values plus either transparent or opaque background mode
- The first built-in font set currently uses a compact bitmap asset with two size descriptors: a small `5x7` presentation and a larger doubled `10x14` presentation
- Text drawing is designed to work with partial flush by returning changed bounds that can be passed to the framebuffer rectangle flush path
- The text layer is being moved to an SSD1322-style `GFXfont` backend so bundled fonts stay readable in source form instead of opaque byte blobs

### Future driver goal

- Keep the low-level transport layer minimal
- Build a fuller SSD1363 driver layer above it around controller commands instead of only smoke-test helpers
- Organize the future API so command-table features can be exposed cleanly without putting raw command bytes everywhere in application code
- Include first-class support for grayscale-related controls such as contrast/current/gray table configuration where the hardware supports them
- Keep the public API understandable enough that other users can pick up the library without reverse-engineering the whole codebase
- Include a practical text layer with multiple built-in fonts and size options so applications do not need to provide their own text rendering from scratch
- Keep the first bundled font set compact: only a few practical sizes, not dozens of fonts
- Prefer clearly licensed fonts for bundled assets, especially public-domain fonts, SIL OFL fonts, and Uwe-created fonts with clear redistribution terms
- Keep bundled fonts optional so the core driver stays small
- Document the license for each bundled font in the repository
- Treat true grayscale or antialiased text as a separate advanced feature that will require a different rendering path or a custom font pipeline

## Future To-Do List

These items are planned for the future driver/library phase:

1. Freeze the working SSD1363 initialization baseline and document which values are proven on hardware
2. Replace smoke-test-oriented API naming with stable driver naming
3. Add named wrappers for more of the SSD1363 command table instead of relying on raw command bytes in higher-level code
4. Add controller functions for display on/off, remap, start line, display offset, multiplex ratio, contrast, master contrast, phase length, precharge, VCOMH, and related tuning commands
5. Add grayscale-focused functions, especially default gray table selection and custom gray table support if confirmed on this module
6. Build a proper 4bpp framebuffer layer for `256x128`
7. Add basic drawing helpers on top of the framebuffer layer
8. Add a text-rendering layer with multiple built-in fonts
9. Include multiple font sizes so the library covers small UI text and larger headings without extra user work
10. Start with only `3` or `4` bundled font sizes instead of a broad font pack
11. Curate bundled fonts from clearly licensed sources only
12. Keep font files optional assets so applications that do not need them are not forced to carry the extra footprint
13. Document the license and source of each bundled font in the repository
14. Decide on a clean font format and API so text rendering fits the rest of the library cleanly
15. Plan a separate future path for true grayscale or antialiased font rendering instead of treating monochrome bitmap fonts as the final solution
16. Refactor `main.c` so it becomes only a demo/example entry point instead of containing bring-up/debug logic
17. Separate smoke-test code from the reusable driver code
18. Improve naming, comments, and file organization so the library feels complete and understandable to external users
19. Add documentation for configuration, wiring, initialization, usage, and font support
20. Preserve I2C as the working baseline while keeping SPI support ready for later validation
21. Validate and document which controller features are fully implemented versus intentionally deferred
22. Prepare the project so it can be published as a useful SSD1363 library rather than only a one-off experiment
23. Refactor the confirmed panel-specific GDDRAM packing settings into a cleaner panel-format abstraction so the driver reads as explicit encoding rather than ad hoc conditionals
24. Remove or simplify the now-unused smoke-test demo helpers and leave a smaller stable example scene
25. Document the difference between logical framebuffer coordinates and panel-specific GDDRAM byte packing so future maintenance does not confuse encoding with pixel offsets

### Smoke test

- I2C bus scan in `main.c`
- Direct probe of the configured address
- Tentative SSD1363 init sequence
- Full-screen fill pattern loop using `0x00`, `0xFF`, `0xF0`, `0x0F`, `0x55`, `0xAA`
- Hardware serial output confirms the display module ACKs at `0x3C`
- Hardware serial output confirms the tentative init sequence completes without I2C transaction errors
- Hardware serial output confirms the fill-pattern loop is executing continuously
- First visual result: the image fills full width but only about half height, with black bands at top and bottom
- This symptom suggests the next debug target is active row count and/or display offset, not transport
- Second visual result: even less of the display is active, with apparent response only on roughly every fourth horizontal line band
- This symptom suggests COM-line mapping/remap is a better target than shrinking the active height
- Third visual result: full width is correct, with the middle 64 rows active and top/bottom 32 rows inactive
- Fourth visual result: the startup frame can light the lower region as white-noise while the top stays black, and the intended stripe diagnostics appear as columns rather than expected rows
- Final smoke-test result: the whole display is now working
- Final text result: text is now sharp and clean after matching the panel's real packed-pixel and `4-pixel` group byte order
- The user has now confirmed the module itself is labeled `256x128`, so this symptom is no longer treated as evidence of a true 64-row panel
- The added column-oriented symptom points to row/column remap or address increment being wrong, not just the panel height
- The newest narrow-strip symptom points to the RAM column window itself being programmed in the wrong units

## What Is Explicitly Not Final Yet

- Final SSD1363 power-on init sequence
- Verified I2C command/data control bytes for this exact display module
- Correct SSD1363 column offset
- Whether the tentative smoke-test init values match the exact module hardware
- Whether the module really exposes only I2C in the currently-owned hardware configuration
- What the panel actually shows for the current smoke-test pattern loop
- Whether the U8g2-like baseline of remap `0x32 0x00`, display offset `0x20`, and column offset `8` maps the panel correctly when the panel is known to be physically `256x128`
- Whether the combination of U8g2-like init values plus `4-pixel` column address units fully restores full-screen GDDRAM writes
- Which parts of the current smoke-test init sequence should be preserved as the stable baseline for the real SSD1363 driver layer
- How to organize the SSD1363 API so more of the command table can be exposed cleanly without turning the application layer into raw register writes
- What minimum documentation and API cleanup is required before this should be treated as a reusable public library for other SSD1363 users
- Which font set and font sizes should be included by default so the library is useful without becoming unnecessarily heavy
- Which clearly licensed fonts should be chosen for the first bundled set
- What rendering approach should be used later for true grayscale or antialiased fonts on the SSD1363 instead of simple monochrome bitmap text

## Open Questions

1. What does the panel actually show during the current smoke-test pattern loop?
2. Which ESP32 pins will be used later for SPI: MOSI, SCLK, CS, DC, RESET?
3. Does the display module require any special power-enable or initialization timing outside the controller reset?
4. Does the current I2C implementation for this module use standard command/data control bytes?
5. The product page title says `256x128`, but one generated spec block shows `128x64`; the title and variant selection are currently treated as more trustworthy.
6. The current init sequence is a best-effort SSD1363 smoke-test sequence and must be validated on hardware before treating it as final.

## Latest Interpretation

The first live test indicates:

- I2C transport is good
- Address `0x3C` is correct
- The display responds to command/data traffic
- Vertical mapping is likely wrong

Current working hypothesis:

- The module is using a `256x64` active region inside a controller space previously assumed to be `256x128`
- A centered display offset is required to map the active rows correctly

Updated working hypothesis:

- The controller is responding across full width, but COM mapping is interleaved incorrectly
- The next useful test is full 128-row multiplex with altered remap bytes and explicit stripe diagnostics

- Refined working hypothesis:

- The panel is physically `256x128`
- The centered 64-row image plus column-oriented stripe symptom points to incorrect remap and/or offset, not an incorrect panel-size assumption
- A known U8g2 SSD1363 `256x128` baseline uses `display offset 0x20`, `remap 0x32 0x00`, and `column offset 8`
- A known U8g2 SSD1363 `256x128` write path sets RAM columns in `4-pixel` units, not `2-pixel` framebuffer-byte units
- That baseline plus corrected `0x15` column units is sufficient to drive the whole display in the current smoke test

## Next Recommended Step

Build a minimal smoke test that:

1. Selects I2C transport
2. Performs a reset if a reset pin exists
3. Sends a minimal SSD1363 init sequence
4. Sets the display window
5. Writes repeating test patterns to GDDRAM

## Change Log

### 2026-03-23

- Adopted the flat `include` + `src` project structure used by the reference ESP_OLED project
- Updated the ESP-IDF component registration to include the `include` folder
- Added a dual-bus SSD1363 transport scaffold with I2C-first implementation and SPI-ready structure
- Added an SSD1363 API scaffold for command, data, window, and raw buffer writes
- Verified that the new scaffold introduces no current workspace diagnostics errors
- Accepted the current I2C pin choice as-is for this project stage
- Recorded the product listing evidence that the purchased module is most likely a `2.7 inch SSD1363 256x128 16-gray 4-pin I2C` variant
- Kept SPI explicitly deferred, with compile-only placeholders remaining in config
- Recorded that no reset pin is connected in the current hardware setup
- Added an I2C scanner and a first smoke test in `main.c`
- Added a tentative SSD1363 initialization sequence plus chunked I2C framebuffer writes for visible pattern testing
- Confirmed from hardware serial log that the module responds on I2C address `0x3C`
- Confirmed from hardware serial log that the smoke-test init and pattern loop run without reported ESP-IDF or I2C write failures
- Recorded the first visual symptom: full width response but only about half height with black top/bottom bands
- Retuned the smoke test to a `64-row` active height and a centered display offset of `0x20`
- Replaced the reduced-height test with a full-height remap-focused test using `0x30 0x00`
- Added direct fill and stripe diagnostics to distinguish row-skipping from data-packing issues
- Added PlatformIO monitor/upload settings intended to reduce serial-line interference and prefer standard reset behavior during flashing
- The user confirmed the module itself is marked `256x128`, overriding the temporary working assumption that the visible target should be treated as 64 rows
- Restored the smoke-test target to full `256x128` while keeping the improved remap setting for the next hardware run
- Recorded the new symptom that startup noise appears in the lower region while the top remains black and that stripe tests appear column-oriented
- Updated the smoke test to match a known U8g2 SSD1363 `256x128` baseline more closely by setting display offset `0x20`, remap `0x32 0x00`, and column offset `8`
- Corrected the smoke-test RAM window programming so command `0x15` uses `4-pixel` SSD1363 column units instead of `2-pixel` framebuffer-byte units
- The user confirmed that, after the column-window fix, the whole display is working
- Recorded the future goal of making the driver more complete and professional by exposing more of the SSD1363 command table, especially grayscale-related functionality
- Expanded the tracker with an explicit future to-do list for turning the current bring-up into a reusable SSD1363 library other people could use
- Added future library goals for built-in text rendering with multiple fonts and font sizes
- Added font-selection constraints: start compact, use clearly licensed fonts, keep fonts optional, and document per-font licensing

### 2026-03-24

- Renamed the temporary smoke-test-style panel init path toward a stable driver-facing baseline initialization entry point
- Started freezing the currently working hardware configuration into reusable API calls instead of keeping it only as ad hoc setup code
- Added named driver wrappers for several panel-control commands already used by the working initialization sequence
- Moved the I2C smoke-test and pattern-loop logic out of `main.c` into a dedicated demo module
- Reduced `main.c` to a thin application entry point so the project can move toward user-code-only main logic later
- Added a first `basic` example layer inspired by the structure of external display libraries, but kept all SSD1363 controller behavior local to this project
- Continued converting the working SSD1363 init sequence from raw command writes into named API wrappers so the public driver header better matches a command-table-oriented library
- Added the first framebuffer/GFX building block with a dedicated `4bpp` framebuffer module, pixel helpers, rectangle primitives, and flush-through-basic-display support
- Added line primitives, 1bpp bitmap blit support for icon/text building blocks, and a first alignment-aware partial flush path to reduce full-screen updates for small changes
- Added the long-term requirement for true grayscale or antialiased text as a separate future rendering task
- Established this tracker file as the required running project log