#pragma once

namespace sdk
{
	class Color
	{
	public:
		Color() {
			*((int*)this) = 0;
		}

		Color(int _r, int _g, int _b) {
			SetColor(_r, _g, _b, 0);
		}

		Color(int _r, int _g, int _b, int _a) {
			SetColor(_r, _g, _b, _a);
		}

		void SetColor(int _r, int _g, int _b, int _a = 0);
		void GetColor(int& _r, int& _g, int& _b, int& _a) const;

		void SetRawColor(int color32) {
			*((int*)this) = color32;
		}

		int GetRawColor() const {
			return *((int*)this);
		}

		inline void getFloatArray(float* arr) const;

		int r() const { return _color[0]; }
		int g() const { return _color[1]; }
		int b() const { return _color[2]; }
		int a() const { return _color[3]; }

		unsigned char& operator[](int index) {
			return _color[index];
		}

		const unsigned char& operator[](int index) const {
			return _color[index];
		}

		bool operator==(const Color& rhs) const {
			return (*((int*)this) == *((int*)&rhs));
		}

		bool operator!=(const Color& rhs) const {
			return !(operator==(rhs));
		}

		Color& operator=(const Color& rhs)
		{
			SetRawColor(rhs.GetRawColor());
			return *this;
		}

		float* Base();
		float Hue() const;
		float Saturation() const;
		float Brightness() const;
		Color FromHSB(float hue, float saturation, float brightness);

		static Color Red() { return Color(255, 0, 0); }
		static Color Green() { return Color(70, 255, 70); }
		static Color Blue() { return Color(0, 0, 255); }
		static Color LightBlue() { return Color(50, 160, 255); }
		static Color Grey() { return Color(128, 128, 128); }
		static Color DarkGrey() { return Color(45, 45, 45); }
		static Color Black() { return Color(0, 0, 0); }
		static Color White() { return Color(255, 255, 255); }
		static Color Purple() { return Color(220, 0, 220); }

	private:
		unsigned char _color[4];
	};
}
