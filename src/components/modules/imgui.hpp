#pragma once

namespace components
{
	class imgui : public component
	{
	public:
		imgui();
		~imgui();

		static inline imgui* p_this = nullptr;
		static imgui* get() { return p_this; }

		static void endscene_stub();

#if USE_IMGUI
		void devgui();
		bool input_message(UINT message_type, WPARAM wparam, LPARAM lparam);

		bool m_menu_active = false;
		bool m_initialized_device = false;
#endif

		// the following default values will be used on release builds

		bool m_disable_cullnode = false;
		bool m_enable_area_forcing = true;
		bool m_enable_3dsky = false;

		float m_flashlight_fwd_offset = 0.0f;
		float m_flashlight_horz_offset = 0.0f;
		float m_flashlight_vert_offset = 0.0f;

		float m_flashlight_bot_fwd_offset = 22.0f;
		float m_flashlight_bot_horz_offset = 1.0f;
		float m_flashlight_bot_vert_offset = -4.0f;

		float m_flashlight_intensity = 20000.0f;
		float m_flashlight_radius = 0.4f;

		bool m_flashlight_use_custom_dir = false;
		Vector m_flashlight_direction = { 1.0f, 0.0f, 0.0f };
		float m_flashlight_angle = 23.0f;
		float m_flashlight_softness = 0.3f;
		float m_flashlight_exp = 0.8f;

	private:
	};
}
