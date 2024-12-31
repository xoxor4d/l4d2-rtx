#include <std_include.hpp>
#include "color.hpp"

namespace sdk
{
	void Color::SetColor(int _r, int _g, int _b, int _a)
	{
		_color[0] = (unsigned char)_r;
		_color[1] = (unsigned char)_g;
		_color[2] = (unsigned char)_b;
		_color[3] = (unsigned char)_a;
	}

	void Color::GetColor(int& _r, int& _g, int& _b, int& _a) const
	{
		_r = _color[0];
		_g = _color[1];
		_b = _color[2];
		_a = _color[3];
	}

	void Color::getFloatArray(float* arr) const
	{

		arr[0] = (float)r() / (float)255;
		arr[1] = (float)g() / (float)255;
		arr[2] = (float)b() / (float)255;
		arr[3] = (float)a() / (float)255;

		return;
	}

	float* Color::Base()
	{
		static float clr[3];

		clr[0] = _color[0] / 255.0f;
		clr[1] = _color[1] / 255.0f;
		clr[2] = _color[2] / 255.0f;

		return &clr[0];
	}

	float Color::Hue() const
	{
		if (_color[0] == _color[1] && _color[1] == _color[2]) {
			return 0.0f;
		}

		float r = (float)_color[0] / 255.0f;
		float g = (float)_color[1] / 255.0f;
		float b = (float)_color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			  min = r < g ? r : g < b ? g : b;

		float delta = max - min;
		float hue = 0.0f;

		if (r == max) {
			hue = (g - b) / delta;
		}
		else if (g == max) {
			hue = 2 + (b - r) / delta;
		}
		else if (b == max) {
			hue = 4 + (r - g) / delta;
		}

		hue *= 60;

		if (hue < 0.0f) {
			hue += 360.0f;
		}

		return hue;
	}

	float Color::Saturation() const
	{
		float r = _color[0] / 255.0f;
		float g = _color[1] / 255.0f;
		float b = _color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			  min = r < g ? r : g < b ? g : b;

		float l, s = 0;

		if (max != min)
		{
			l = (max + min) / 2;

			if (l <= 0.5f) {
				s = (max - min) / (max + min);
			}
			else {
				s = (max - min) / (2 - max - min);
			}
		}

		return s;
	}

	float Color::Brightness() const
	{
		float r = _color[0] / 255.0f;
		float g = _color[1] / 255.0f;
		float b = _color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			  min = r < g ? r : g < b ? g : b;

		return (max + min) / 2;
	}

	Color Color::FromHSB(float hue, float saturation, float brightness)
	{
		float h = hue == 1.0f ? 0 : hue * 6.0f;
		float f = h - (int)h;
		float p = brightness * (1.0f - saturation);
		float q = brightness * (1.0f - saturation * f);
		float t = brightness * (1.0f - (saturation * (1.0f - f)));

		if (h < 1)
		{
			return Color(
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255),
				(unsigned char)(p * 255)
			);
		}

		if (h < 2)
		{
			return Color(
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255)
			);
		}

		if (h < 3)
		{
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255)
			);
		}

		if (h < 4)
		{
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255)
			);
		}

		if (h < 5)
		{
			return Color(
				(unsigned char)(t * 255),
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255)
			);
		}

		return Color(
			(unsigned char)(brightness * 255),
			(unsigned char)(p * 255),
			(unsigned char)(q * 255)
		);
	}
}
