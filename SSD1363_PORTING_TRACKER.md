# SSD1363 Porting Tracker

## Goal

Port the SSD1322-oriented library structure into this ESP-IDF PlatformIO project for an SSD1363-based display, while keeping the project layout flat:

- Headers in `include`
- Sources in `src`
- Current priority on I2C
- SPI scaffolded now so it can be enabled later without redesigning the codebase

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
| 7. Finalize SSD1363 init sequence | In progress | Full-panel smoke test now works; remaining work is to clean up and validate which init values are strictly required |
| 8. Bring up display with smoke test | Done | Full display is now responding across the whole panel |
| 9. Port framebuffer/GFX layer | Pending | After command path is proven |
| 10. Validate future SPI migration path | Pending | Depends on final SPI wiring |

## Files Added In This Step

- `include/ssd1363_config.h`
- `include/ssd1363_interface.h`
- `include/ssd1363_api.h`
- `src/ssd1363_interface.c`
- `src/ssd1363_api.c`
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
- Tentative SSD1363 smoke-test initialization sequence

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
- Established this tracker file as the required running project log