#pragma once

namespace components
{
	extern int g_current_leaf;
	extern int g_current_area;

	namespace api
	{
		enum DEBUG_REMIX_LINE_COLOR
		{
			RED = 0u,
			GREEN = 1u,
			TEAL = 2u,
		};

		extern void init();
		extern void create_quad(remixapi_HardcodedVertex* v_out, uint32_t* i_out, float scale);
		extern void add_debug_line(const Vector& p1, const Vector& p2, float width, DEBUG_REMIX_LINE_COLOR color);

		extern bool m_initialized;
		extern remixapi_Interface bridge;

		extern remixapi_MaterialHandle remix_debug_line_materials[3];
		extern remixapi_MeshHandle remix_debug_line_list[128];
		extern std::uint32_t remix_debug_line_amount;
		extern std::uint64_t remix_debug_last_line_hash;


		struct flashlight_def_s
		{
			Vector pos;
			Vector fwd = { 0.0f, 1.0f, 0.0f };
			Vector rt;
			Vector up;
		};

		struct flashlight_s
		{
			remixapi_LightHandle handle = nullptr;
			remixapi_LightInfoSphereEXT ext = {};
			remixapi_LightInfo info = {};
			flashlight_def_s def = {};
			//std::string_view player_name;
			bool is_player = false;
			bool is_enabled = false;
		};
		extern std::unordered_map<std::string, flashlight_s> m_flashlights;


		/*struct remix_light_s
		{
			remixapi_LightHandle handle = nullptr;
			remixapi_LightInfoSphereEXT ext = {};
			remixapi_LightInfo info = {};
		};
		extern remix_light_s flashlight;*/
	}

	class main_module : public component
	{
	public:
		main_module();
		~main_module();

		static inline std::uint64_t framecount = 0u;
		static inline LPD3DXFONT d3d_font = nullptr;

		static void iterate_entities();
		static void debug_draw_box(const VectorAligned& center, const VectorAligned& half_diagonal, float width, const api::DEBUG_REMIX_LINE_COLOR& color);
		static void force_cvars();
	private:
	};
}
