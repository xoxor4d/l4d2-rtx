#pragma once

namespace components
{
	extern int g_current_leaf;
	extern int g_current_area;

	class main_module : public component
	{
	public:
		main_module();
		~main_module();

		static inline std::uint64_t framecount = 0u;
		static inline LPD3DXFONT d3d_font = nullptr;

		static void iterate_entities();
		static void force_cvars();
	};
}
