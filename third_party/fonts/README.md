# Font Import Staging

This folder is the staging location for external font files that are intended to be converted into the SSD1363 GFXfont format.

## Requested Families

The current requested font set is:

- DSEG
- Cascadia Mono
- JetBrains Mono
- IBM Plex Mono
- Share Tech Mono
- Eurostile or Microgramma

## Files To Provide

Place one regular style font file for each family in this folder.

Recommended filenames:

- `DSEG-Regular.ttf`
- `CascadiaMono-Regular.ttf`
- `JetBrainsMonoNL-Regular.ttf`
- `IBMPlexMono-Regular.ttf`
- `ShareTechMono-Regular.ttf`
- `Eurostile-Regular.ttf` or `Microgramma-Regular.ttf`

## Important Licensing Note

Open-font families such as DSEG, JetBrains Mono, IBM Plex Mono, and Share Tech Mono can generally be bundled when their license terms are preserved.

`Eurostile` and `Microgramma` are not treated as safely redistributable by default in this project. Only place those files here if you already have the legal right to use and redistribute them in this repository.

If you do not want to provide licensed Eurostile or Microgramma assets, choose an open substitute instead.

Reasonable substitutes:

- `Michroma`
- `Orbitron`
- `Oxanium`
- `Aldrich`

## Planned Output

Once the font files are available, the project can generate:

- one base GFXfont asset per family
- three exposed native GFXfont sizes per family with pixel-height public names

Target public names will follow the existing style used by `ssd1363_fonts.h`.