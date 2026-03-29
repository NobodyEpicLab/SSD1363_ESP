#include "ssd1363_fonts.h"

#include "font_assets/CascadiaCode15px.h"
#include "font_assets/CascadiaCode29px.h"
#include "font_assets/CascadiaCode43px.h"
#include "font_assets/FreeMono12pt7b.h"
#include "font_assets/IBMPlexMonoLight17px.h"
#include "font_assets/IBMPlexMonoLight32px.h"
#include "font_assets/IBMPlexMonoLight47px.h"
#include "font_assets/JetBrainsMonoLight17px.h"
#include "font_assets/JetBrainsMonoLight33px.h"
#include "font_assets/JetBrainsMonoLight48px.h"
#include "font_assets/Michroma18px.h"
#include "font_assets/Michroma35px.h"
#include "font_assets/Michroma52px.h"
#include "font_assets/Orbitron16px.h"
#include "font_assets/Orbitron31px.h"
#include "font_assets/Orbitron46px.h"
#include "font_assets/ShareTechMono14px.h"
#include "font_assets/ShareTechMono28px.h"
#include "font_assets/ShareTechMono41px.h"

#define SSD1363_GLYPH(code, bitmap_name) {code, 5U, bitmap_name}
#define SSD1363_DEFINE_GFX_FONT(symbol, gfx_name, ascent_macro, y_advance_macro, scale_x_value, scale_y_value, line_spacing_value) \
	const ssd1363_font_t symbol = { \
		.backend = SSD1363_FONT_BACKEND_GFX, \
		.glyphs = NULL, \
		.glyph_count = 0U, \
		.gfx_font = &gfx_name, \
		.glyph_height = y_advance_macro, \
		.baseline = ascent_macro, \
		.scale_x = scale_x_value, \
		.scale_y = scale_y_value, \
		.letter_spacing = 0U, \
		.line_spacing = line_spacing_value, \
		.fallback_char = '?', \
	};

static const uint8_t glyph_space[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t glyph_dash[] = {0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00};
static const uint8_t glyph_period[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0};
static const uint8_t glyph_colon[] = {0x00, 0xC0, 0xC0, 0x00, 0xC0, 0xC0, 0x00};

static const uint8_t glyph_0[] = {0x70, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x70};
static const uint8_t glyph_1[] = {0x20, 0x60, 0x20, 0x20, 0x20, 0x20, 0x70};
static const uint8_t glyph_2[] = {0x70, 0x88, 0x08, 0x10, 0x20, 0x40, 0xF8};
static const uint8_t glyph_3[] = {0xF0, 0x08, 0x08, 0x70, 0x08, 0x08, 0xF0};
static const uint8_t glyph_4[] = {0x10, 0x30, 0x50, 0x90, 0xF8, 0x10, 0x10};
static const uint8_t glyph_5[] = {0xF8, 0x80, 0x80, 0xF0, 0x08, 0x08, 0xF0};
static const uint8_t glyph_6[] = {0x70, 0x80, 0x80, 0xF0, 0x88, 0x88, 0x70};
static const uint8_t glyph_7[] = {0xF8, 0x08, 0x10, 0x20, 0x40, 0x40, 0x40};
static const uint8_t glyph_8[] = {0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x70};
static const uint8_t glyph_9[] = {0x70, 0x88, 0x88, 0x78, 0x08, 0x08, 0x70};

static const uint8_t glyph_A[] = {0x70, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88};
static const uint8_t glyph_B[] = {0xF0, 0x88, 0x88, 0xF0, 0x88, 0x88, 0xF0};
static const uint8_t glyph_C[] = {0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70};
static const uint8_t glyph_D[] = {0xF0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0};
static const uint8_t glyph_E[] = {0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8};
static const uint8_t glyph_F[] = {0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0x80};
static const uint8_t glyph_G[] = {0x70, 0x88, 0x80, 0xB8, 0x88, 0x88, 0x70};
static const uint8_t glyph_H[] = {0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88};
static const uint8_t glyph_I[] = {0x70, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70};
static const uint8_t glyph_J[] = {0x38, 0x10, 0x10, 0x10, 0x10, 0x90, 0x60};
static const uint8_t glyph_K[] = {0x88, 0x90, 0xA0, 0xC0, 0xA0, 0x90, 0x88};
static const uint8_t glyph_L[] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF8};
static const uint8_t glyph_M[] = {0x88, 0xD8, 0xA8, 0xA8, 0x88, 0x88, 0x88};
static const uint8_t glyph_N[] = {0x88, 0xC8, 0xA8, 0x98, 0x88, 0x88, 0x88};
static const uint8_t glyph_O[] = {0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70};
static const uint8_t glyph_P[] = {0xF0, 0x88, 0x88, 0xF0, 0x80, 0x80, 0x80};
static const uint8_t glyph_Q[] = {0x70, 0x88, 0x88, 0x88, 0xA8, 0x90, 0x68};
static const uint8_t glyph_R[] = {0xF0, 0x88, 0x88, 0xF0, 0xA0, 0x90, 0x88};
static const uint8_t glyph_S[] = {0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xF0};
static const uint8_t glyph_T[] = {0xF8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};
static const uint8_t glyph_U[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70};
static const uint8_t glyph_V[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x50, 0x20};
static const uint8_t glyph_W[] = {0x88, 0x88, 0x88, 0xA8, 0xA8, 0xA8, 0x50};
static const uint8_t glyph_X[] = {0x88, 0x88, 0x50, 0x20, 0x50, 0x88, 0x88};
static const uint8_t glyph_Y[] = {0x88, 0x88, 0x50, 0x20, 0x20, 0x20, 0x20};
static const uint8_t glyph_Z[] = {0xF8, 0x08, 0x10, 0x20, 0x40, 0x80, 0xF8};

static const ssd1363_font_glyph_t builtin_glyphs[] = {
	{'.', 2U, glyph_period},
	{':', 2U, glyph_colon},
	SSD1363_GLYPH('-', glyph_dash),
	{' ', 3U, glyph_space},
	SSD1363_GLYPH('0', glyph_0),
	SSD1363_GLYPH('1', glyph_1),
	SSD1363_GLYPH('2', glyph_2),
	SSD1363_GLYPH('3', glyph_3),
	SSD1363_GLYPH('4', glyph_4),
	SSD1363_GLYPH('5', glyph_5),
	SSD1363_GLYPH('6', glyph_6),
	SSD1363_GLYPH('7', glyph_7),
	SSD1363_GLYPH('8', glyph_8),
	SSD1363_GLYPH('9', glyph_9),
	SSD1363_GLYPH('A', glyph_A),
	SSD1363_GLYPH('B', glyph_B),
	SSD1363_GLYPH('C', glyph_C),
	SSD1363_GLYPH('D', glyph_D),
	SSD1363_GLYPH('E', glyph_E),
	SSD1363_GLYPH('F', glyph_F),
	SSD1363_GLYPH('G', glyph_G),
	SSD1363_GLYPH('H', glyph_H),
	SSD1363_GLYPH('I', glyph_I),
	SSD1363_GLYPH('J', glyph_J),
	SSD1363_GLYPH('K', glyph_K),
	SSD1363_GLYPH('L', glyph_L),
	SSD1363_GLYPH('M', glyph_M),
	SSD1363_GLYPH('N', glyph_N),
	SSD1363_GLYPH('O', glyph_O),
	SSD1363_GLYPH('P', glyph_P),
	SSD1363_GLYPH('Q', glyph_Q),
	SSD1363_GLYPH('R', glyph_R),
	SSD1363_GLYPH('S', glyph_S),
	SSD1363_GLYPH('T', glyph_T),
	SSD1363_GLYPH('U', glyph_U),
	SSD1363_GLYPH('V', glyph_V),
	SSD1363_GLYPH('W', glyph_W),
	SSD1363_GLYPH('X', glyph_X),
	SSD1363_GLYPH('Y', glyph_Y),
	SSD1363_GLYPH('Z', glyph_Z),
};

const ssd1363_font_t ssd1363_font_builtin_5x7 = {
	.backend = SSD1363_FONT_BACKEND_BITMAP,
	.glyphs = builtin_glyphs,
	.glyph_count = sizeof(builtin_glyphs) / sizeof(builtin_glyphs[0]),
	.gfx_font = NULL,
	.glyph_height = 7U,
	.baseline = 0U,
	.scale_x = 1U,
	.scale_y = 1U,
	.letter_spacing = 1U,
	.line_spacing = 2U,
	.fallback_char = ' ',
};

const ssd1363_font_t ssd1363_font_builtin_10x14 = {
	.backend = SSD1363_FONT_BACKEND_BITMAP,
	.glyphs = builtin_glyphs,
	.glyph_count = sizeof(builtin_glyphs) / sizeof(builtin_glyphs[0]),
	.gfx_font = NULL,
	.glyph_height = 7U,
	.baseline = 0U,
	.scale_x = 2U,
	.scale_y = 2U,
	.letter_spacing = 2U,
	.line_spacing = 3U,
	.fallback_char = ' ',
};

const ssd1363_font_t ssd1363_font_builtin_15x21 = {
	.backend = SSD1363_FONT_BACKEND_BITMAP,
	.glyphs = builtin_glyphs,
	.glyph_count = sizeof(builtin_glyphs) / sizeof(builtin_glyphs[0]),
	.gfx_font = NULL,
	.glyph_height = 7U,
	.baseline = 0U,
	.scale_x = 3U,
	.scale_y = 3U,
	.letter_spacing = 3U,
	.line_spacing = 4U,
	.fallback_char = ' ',
};

SSD1363_DEFINE_GFX_FONT(ssd1363_font_freemono_24px, FreeMono12pt7b, 15U, 24U, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_freemono_48px, FreeMono12pt7b, 15U, 24U, 2U, 2U, 2U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_freemono_72px, FreeMono12pt7b, 15U, 24U, 3U, 3U, 4U)

/* Legacy FreeMono aliases preserved for compatibility. */
SSD1363_DEFINE_GFX_FONT(ssd1363_font_freemono12pt7b, FreeMono12pt7b, 15U, 24U, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_freemono12pt7b_x2, FreeMono12pt7b, 15U, 24U, 2U, 2U, 2U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_freemono12pt7b_x3, FreeMono12pt7b, 15U, 24U, 3U, 3U, 4U)

/* Preferred pixel-height font exports. */
SSD1363_DEFINE_GFX_FONT(ssd1363_font_cascadiacode_15px, CascadiaCode15px, CascadiaCode15px_ASCENT, CascadiaCode15px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_cascadiacode_29px, CascadiaCode29px, CascadiaCode29px_ASCENT, CascadiaCode29px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_cascadiacode_43px, CascadiaCode43px, CascadiaCode43px_ASCENT, CascadiaCode43px_Y_ADVANCE, 1U, 1U, 0U)

SSD1363_DEFINE_GFX_FONT(ssd1363_font_jetbrainsmonolight_17px, JetBrainsMonoLight17px, JetBrainsMonoLight17px_ASCENT, JetBrainsMonoLight17px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_jetbrainsmonolight_33px, JetBrainsMonoLight33px, JetBrainsMonoLight33px_ASCENT, JetBrainsMonoLight33px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_jetbrainsmonolight_48px, JetBrainsMonoLight48px, JetBrainsMonoLight48px_ASCENT, JetBrainsMonoLight48px_Y_ADVANCE, 1U, 1U, 0U)

SSD1363_DEFINE_GFX_FONT(ssd1363_font_ibmplexmonolight_17px, IBMPlexMonoLight17px, IBMPlexMonoLight17px_ASCENT, IBMPlexMonoLight17px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_ibmplexmonolight_32px, IBMPlexMonoLight32px, IBMPlexMonoLight32px_ASCENT, IBMPlexMonoLight32px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_ibmplexmonolight_47px, IBMPlexMonoLight47px, IBMPlexMonoLight47px_ASCENT, IBMPlexMonoLight47px_Y_ADVANCE, 1U, 1U, 0U)

SSD1363_DEFINE_GFX_FONT(ssd1363_font_michroma_18px, Michroma18px, Michroma18px_ASCENT, Michroma18px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_michroma_35px, Michroma35px, Michroma35px_ASCENT, Michroma35px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_michroma_52px, Michroma52px, Michroma52px_ASCENT, Michroma52px_Y_ADVANCE, 1U, 1U, 0U)

SSD1363_DEFINE_GFX_FONT(ssd1363_font_orbitron_16px, Orbitron16px, Orbitron16px_ASCENT, Orbitron16px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_orbitron_31px, Orbitron31px, Orbitron31px_ASCENT, Orbitron31px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_orbitron_46px, Orbitron46px, Orbitron46px_ASCENT, Orbitron46px_Y_ADVANCE, 1U, 1U, 0U)

SSD1363_DEFINE_GFX_FONT(ssd1363_font_sharetechmono_14px, ShareTechMono14px, ShareTechMono14px_ASCENT, ShareTechMono14px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_sharetechmono_28px, ShareTechMono28px, ShareTechMono28px_ASCENT, ShareTechMono28px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_sharetechmono_41px, ShareTechMono41px, ShareTechMono41px_ASCENT, ShareTechMono41px_Y_ADVANCE, 1U, 1U, 0U)

/* Legacy 12pt/x2/x3 aliases preserved for compatibility with older examples. */
SSD1363_DEFINE_GFX_FONT(ssd1363_font_cascadiacode12pt, CascadiaCode15px, CascadiaCode15px_ASCENT, CascadiaCode15px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_cascadiacode12pt_x2, CascadiaCode29px, CascadiaCode29px_ASCENT, CascadiaCode29px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_cascadiacode12pt_x3, CascadiaCode43px, CascadiaCode43px_ASCENT, CascadiaCode43px_Y_ADVANCE, 1U, 1U, 0U)

SSD1363_DEFINE_GFX_FONT(ssd1363_font_jetbrainsmonolight12pt, JetBrainsMonoLight17px, JetBrainsMonoLight17px_ASCENT, JetBrainsMonoLight17px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_jetbrainsmonolight12pt_x2, JetBrainsMonoLight33px, JetBrainsMonoLight33px_ASCENT, JetBrainsMonoLight33px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_jetbrainsmonolight12pt_x3, JetBrainsMonoLight48px, JetBrainsMonoLight48px_ASCENT, JetBrainsMonoLight48px_Y_ADVANCE, 1U, 1U, 0U)

SSD1363_DEFINE_GFX_FONT(ssd1363_font_ibmplexmonolight12pt, IBMPlexMonoLight17px, IBMPlexMonoLight17px_ASCENT, IBMPlexMonoLight17px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_ibmplexmonolight12pt_x2, IBMPlexMonoLight32px, IBMPlexMonoLight32px_ASCENT, IBMPlexMonoLight32px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_ibmplexmonolight12pt_x3, IBMPlexMonoLight47px, IBMPlexMonoLight47px_ASCENT, IBMPlexMonoLight47px_Y_ADVANCE, 1U, 1U, 0U)

SSD1363_DEFINE_GFX_FONT(ssd1363_font_michroma12pt, Michroma18px, Michroma18px_ASCENT, Michroma18px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_michroma12pt_x2, Michroma35px, Michroma35px_ASCENT, Michroma35px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_michroma12pt_x3, Michroma52px, Michroma52px_ASCENT, Michroma52px_Y_ADVANCE, 1U, 1U, 0U)

SSD1363_DEFINE_GFX_FONT(ssd1363_font_orbitron12pt, Orbitron16px, Orbitron16px_ASCENT, Orbitron16px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_orbitron12pt_x2, Orbitron31px, Orbitron31px_ASCENT, Orbitron31px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_orbitron12pt_x3, Orbitron46px, Orbitron46px_ASCENT, Orbitron46px_Y_ADVANCE, 1U, 1U, 0U)

SSD1363_DEFINE_GFX_FONT(ssd1363_font_sharetechmono12pt, ShareTechMono14px, ShareTechMono14px_ASCENT, ShareTechMono14px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_sharetechmono12pt_x2, ShareTechMono28px, ShareTechMono28px_ASCENT, ShareTechMono28px_Y_ADVANCE, 1U, 1U, 0U)
SSD1363_DEFINE_GFX_FONT(ssd1363_font_sharetechmono12pt_x3, ShareTechMono41px, ShareTechMono41px_ASCENT, ShareTechMono41px_Y_ADVANCE, 1U, 1U, 0U)