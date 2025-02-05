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

		float m_flashlight_fwd_offset = -8.0f;
		float m_flashlight_horz_offset = -5.0f;
		float m_flashlight_vert_offset = -2.0f;

		float m_flashlight_bot_fwd_offset = 22.0f;
		float m_flashlight_bot_horz_offset = 1.0f;
		float m_flashlight_bot_vert_offset = -4.0f;

		float m_flashlight_intensity = 20000.0f;
		float m_flashlight_radius = 0.4f;

		bool m_flashlight_use_custom_dir = false;
		Vector m_flashlight_direction = { 1.0f, 0.0f, 0.0f };
		float m_flashlight_angle = 24.0f;
		float m_flashlight_softness = 0.3f;
		float m_flashlight_exp = 0.8f;

	private:
		void tab_general();
		void tab_map_settings();
		bool m_im_window_focused = false;
		bool m_im_window_hovered = false;
		bool m_im_allow_game_input = false;
		std::string m_devgui_custom_footer_content;

		static void questionmark(const char* desc)
		{
			ImGui::TextDisabled("(?)");
			if (ImGui::BeginItemTooltip())
			{
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(desc);
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
		}
	};
}
