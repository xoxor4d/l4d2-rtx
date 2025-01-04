#pragma once

namespace components
{
	class remix_api : public component
	{
	public:
		remix_api();

		static inline remix_api* p_this = nullptr;
		static remix_api* get() { return p_this; }

		void on_renderview();
		static bool is_initialized() { return get()->m_initialized; }

		static constexpr std::uint32_t M_MAX_DEBUG_LINES = 512u;
		enum DEBUG_REMIX_LINE_COLOR
		{
			RED = 0u,
			GREEN = 1u,
			TEAL = 2u,
		};

		void init_debug_lines();
		void create_quad(remixapi_HardcodedVertex* v_out, uint32_t* i_out, float scale);
		void create_line_quad(remixapi_HardcodedVertex* v_out, uint32_t* i_out, const Vector& p1, const Vector& p2, float width);
		void add_debug_line(const Vector& p1, const Vector& p2, float width, DEBUG_REMIX_LINE_COLOR color);
		static bool can_add_debug_lines() { return get()->m_debug_line_amount + 1u < M_MAX_DEBUG_LINES; }
		void debug_draw_box(const VectorAligned& center, const VectorAligned& half_diagonal, float width, const DEBUG_REMIX_LINE_COLOR& color);

		void flashlight_create_or_update(const char* player_name, const Vector& pos, const Vector& fwd, const Vector& rt, const Vector& up, bool is_enabled, bool is_player = false);

		remixapi_Interface m_bridge;

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
			bool is_player = false;
			bool is_enabled = false;
		};
		std::unordered_map<std::string, flashlight_s> m_flashlights;

	private:
		static void begin_scene_callback();
		static void end_scene_callback();
		static void on_present_callback();

		bool m_initialized = false;

		bool m_debug_lines_initialized = false;
		remixapi_MaterialHandle m_debug_line_materials[3];
		remixapi_MeshHandle m_debug_line_list[M_MAX_DEBUG_LINES];
		std::uint32_t m_debug_line_amount = 0u;
		std::uint64_t m_debug_last_line_hash = 0u;
	};
}