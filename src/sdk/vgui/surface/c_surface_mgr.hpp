#pragma once
#include "../../utils/color.hpp"

namespace sdk
{
#define VGUI_MAT_SURFACE_INTERFACE_VERSION "VGUI_Surface031"

	typedef unsigned long m_font;

	enum font_draw_type_t
	{
		// Use the "additive" value from the scheme file
		FONT_DRAW_DEFAULT = 0,

		// Overrides
		FONT_DRAW_NONADDITIVE,
		FONT_DRAW_ADDITIVE,

		FONT_DRAW_TYPE_COUNT = 2,
	};

	enum font_flag_t
	{
		FONTFLAG_NONE,
		FONTFLAG_ITALIC = 0x001,
		FONTFLAG_UNDERLINE = 0x002,
		FONTFLAG_STRIKEOUT = 0x004,
		FONTFLAG_SYMBOL = 0x008,
		FONTFLAG_ANTIALIAS = 0x010,
		FONTFLAG_GAUSSIANBLUR = 0x020,
		FONTFLAG_ROTARY = 0x040,
		FONTFLAG_DROPSHADOW = 0x080,
		FONTFLAG_ADDITIVE = 0x100,
		FONTFLAG_OUTLINE = 0x200,
		FONTFLAG_CUSTOM = 0x400,		// custom generated font - never fall back to asian compatibility mode
		FONTFLAG_BITMAP = 0x800,		// compiled bitmap font - no fallbacks
	};

	class surface {
	public:
		void draw_set_color(Color color);
		void draw_filled_rect(int x0, int y0, int x1, int y1);
		void draw_outlined_rect(int x0, int y0, int x1, int y1);
		void draw_line(int x0, int y0, int x1, int y1);
		void draw_set_text_font(m_font font);
		void draw_set_text_color(Color color);
		void draw_set_text_pos(int x, int y);
		void draw_get_text_pos(int& x, int& y);
		void draw_print_text(const wchar_t* text, int len, font_draw_type_t type = FONT_DRAW_DEFAULT);
		void set_cursor_always_visible(bool v);
		bool is_cursor_visible();

		m_font create_font();
	};
}
