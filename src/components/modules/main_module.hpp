#pragma once

namespace components
{
	namespace cmd
	{
		extern bool debug_node_vis;
	}

	extern int g_current_leaf;
	extern int g_current_area;
	extern map_settings::area_overrides_s* g_player_current_area_override;

	class main_module : public component
	{
	public:
		main_module();
		~main_module();

		static inline main_module* p_this = nullptr;
		static main_module* get() { return p_this; }

		static inline std::uint64_t framecount = 0u;
		static inline LPD3DXFONT d3d_font = nullptr;

		static void iterate_entities();
		static void force_cvars();
		static void cross_handle_map_and_game_settings();

		static void hud_draw_area_info();
		static void trigger_vis_logic();
		static void xo_debug_toggle_node_vis_fn();

		int m_sky3d_scale = 0;
		Vector m_sky3d_origin = {};
		Vector m_sky3d_camera_origin = {};

		int  m_hud_debug_node_vis_pos[2] = { 250, 135 };
		bool m_hud_debug_node_vis_has_forced_leafs = false;
		bool m_hud_debug_node_vis_has_forced_arealeafs = false;
	};
}
