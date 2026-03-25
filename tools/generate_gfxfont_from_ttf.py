import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def format_byte_array(name: str, values: list[int]) -> str:
    lines: list[str] = []
    if not values:
        lines.append("    0x00")
    else:
        for index in range(0, len(values), 12):
            chunk = values[index:index + 12]
            lines.append("    " + ", ".join(f"0x{value:02X}" for value in chunk))
    return f"static const uint8_t {name}[] = {{\n" + ",\n".join(lines) + "\n};\n"


def pack_bitmap(image: Image.Image) -> list[int]:
    packed: list[int] = []
    width, height = image.size
    current_byte = 0
    bit_count = 0

    for y in range(height):
        for x in range(width):
            current_byte <<= 1
            if image.getpixel((x, y)) != 0:
                current_byte |= 0x01

            bit_count += 1
            if bit_count == 8:
                packed.append(current_byte)
                current_byte = 0
                bit_count = 0

    if bit_count != 0:
        current_byte <<= 8 - bit_count
        packed.append(current_byte)

    return packed


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--symbol", required=True)
    parser.add_argument("--size", type=int, default=12)
    parser.add_argument("--first", type=int, default=32)
    parser.add_argument("--last", type=int, default=126)
    args = parser.parse_args()

    input_path = Path(args.input)
    output_path = Path(args.output)
    symbol = args.symbol

    font = ImageFont.truetype(str(input_path), size=args.size)
    ascent, descent = font.getmetrics()
    y_advance = ascent + descent
    probe = Image.new("L", (1, 1), 0)
    probe_draw = ImageDraw.Draw(probe)

    bitmap_values: list[int] = []
    glyph_lines: list[str] = []

    for code in range(args.first, args.last + 1):
        ch = chr(code)
        left, top, right, bottom = probe_draw.textbbox((0, 0), ch, font=font, anchor="ls")
        width = max(0, right - left)
        height = max(0, bottom - top)
        x_advance = int(round(probe_draw.textlength(ch, font=font)))
        bitmap_offset = len(bitmap_values)

        if width > 0 and height > 0:
            glyph_image = Image.new("1", (width, height), 0)
            glyph_draw = ImageDraw.Draw(glyph_image)
            glyph_draw.text((-left, -top), ch, font=font, fill=1, anchor="ls")
            bitmap_values.extend(pack_bitmap(glyph_image))

        glyph_lines.append(
            "    {"
            f"{bitmap_offset}, {width}, {height}, {x_advance}, {left}, {top}"
            "}"
        )

    bitmap_array = format_byte_array(f"{symbol}Bitmaps", bitmap_values)
    glyph_array = (
        f"static const GFXglyph {symbol}Glyphs[] = {{\n"
        + ",\n".join(glyph_lines)
        + "\n};\n"
    )

    header = f"""#ifndef {symbol.upper()}_H
#define {symbol.upper()}_H

/* Generated from {input_path.name}. */

#include <stdint.h>

#include \"ssd1363_gfxfont.h\"

#ifdef __cplusplus
extern \"C\" {{
#endif

#define {symbol}_ASCENT {ascent}
#define {symbol}_Y_ADVANCE {y_advance}

{bitmap_array}

{glyph_array}

static const GFXfont {symbol} = {{
    {symbol}Bitmaps,
    {symbol}Glyphs,
    {args.first},
    {args.last},
    {y_advance}
}};

#ifdef __cplusplus
}}
#endif

#endif
"""

    output_path.write_text(header, encoding="ascii")


if __name__ == "__main__":
    main()