#include <std_include.hpp>
#include "c_surface_mgr.hpp"

namespace sdk
{
	void surface::draw_set_color(Color color)
	{
		using original_fn = void(__thiscall*)(surface*, Color);
		return (*(original_fn**)this)[11](this, color);
	}

	void surface::draw_filled_rect(int x0, int y0, int x1, int y1)
	{
		using original_fn = void(__thiscall*)(surface*, int, int, int, int);
		return (*(original_fn**)this)[12](this, x0, y0, x1, y1);
	}

	void surface::draw_outlined_rect(int x0, int y0, int x1, int y1)
	{
		using original_fn = void(__thiscall*)(surface*, int, int, int, int);
		return (*(original_fn**)this)[14](this, x0, y0, x1, y1);
	}

	void surface::draw_line(int x0, int y0, int x1, int y1)
	{
		using original_fn = void(__thiscall*)(surface*, int, int, int, int);
		return (*(original_fn**)this)[15](this, x0, y0, x1, y1);
	}

	void surface::draw_set_text_font(m_font font)
	{
		using original_fn = void(__thiscall*)(surface*, m_font);
		return (*(original_fn**)this)[17](this, font);
	}

	void surface::draw_set_text_color(Color color)
	{
		using original_fn = void(__thiscall*)(surface*, Color);
		return (*(original_fn**)this)[18](this, color);
	}

	void surface::draw_set_text_pos(int x, int y)
	{
		using original_fn = void(__thiscall*)(surface*, int, int);
		return (*(original_fn**)this)[20](this, x, y);
	}

	void surface::draw_get_text_pos(int& x, int& y)
	{
		using original_fn = void(__thiscall*)(surface*, int&, int&);
		return (*(original_fn**)this)[21](this, x, y);
	}

	void surface::draw_print_text(const wchar_t* text, int len, font_draw_type_t type)
	{
		using original_fn = void(__thiscall*)(surface*, const wchar_t*, int, font_draw_type_t);
		return (*(original_fn**)this)[22](this, text, len, type);
	}

	void surface::set_cursor_always_visible(bool v)
	{
		using original_fn = void(__thiscall*)(surface*, bool);
		return (*(original_fn**)this)[49](this, v);
	}

	bool surface::is_cursor_visible()
	{
		using original_fn = bool(__thiscall*)(surface*);
		return (*(original_fn**)this)[50](this);
	}

	void surface::unlock_cursor()
	{
		using original_fn = void(__thiscall*)(surface*);
		return (*(original_fn**)this)[58](this);
	}

	void surface::lock_cursor()
	{
		using original_fn = void(__thiscall*)(surface*);
		return (*(original_fn**)this)[59](this);
	}

	m_font surface::create_font()
	{
		using original_fn = m_font(__thiscall*)(surface*);
		return (*(original_fn**)this)[63](this);
	}
}
