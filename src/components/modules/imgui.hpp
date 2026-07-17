#pragma once

namespace components
{
	class imgui final : public loader::component_module
	{
	public:
		imgui();
		~imgui();

		static inline imgui* p_this = nullptr;
		static imgui* get() { return p_this; }

		static void endscene_stub();
		static void on_map_load();

		void devgui();
		bool input_message(UINT message_type, WPARAM wparam, LPARAM lparam);

		bool m_menu_active = false;
		bool m_initialized_device = false;

		void style_xo();

		static bool cvar_toggle_button_bool(const char* cvar_str, const char* btn_text, ImVec2 btn_size = ImVec2(0, 0), const char* tt_text = nullptr, bool invert = false);
		static bool toggle_button_bool(bool* bool_ptr, const char* btn_text, ImVec2 btn_size = ImVec2(0, 0), const char* tt_text = nullptr, bool invert = false);
		static bool cvar_toggle_button_int(const char* cvar_str, const char* btn_text, ImVec2 btn_size = ImVec2(0, 0), const char* tt_text = nullptr, int off_override = 0, int on_override = 0);

		ImVec4 ImGuiCol_ButtonGreen = ImVec4(0.3f, 0.4f, 0.05f, 0.7f);
		ImVec4 ImGuiCol_ButtonYellow = ImVec4(0.4f, 0.3f, 0.1f, 0.8f);
		ImVec4 ImGuiCol_ButtonRed = ImVec4(0.48f, 0.15f, 0.15f, 1.00f);
		ImVec4 ImGuiCol_ContainerBackground = ImVec4(0.220f, 0.220f, 0.220f, 0.863f);
		ImVec4 ImGuiCol_ContainerBorder = ImVec4(0.099f, 0.099f, 0.099f, 0.901f);

		// the following default values will be used on release builds
		bool m_disable_cullnode = false;
		bool m_enable_area_forcing = true;
		bool m_light_edit_mode = false;
		bool m_debug_disable_unbake = false;
		bool m_debug_unbake_all_single_bones = false;

		bool m_debugvis_live = false;
		bool m_debugvis_radius = true;
		bool m_debugvis_shaping = true;
		bool m_debugvis_attach_bounds = true;
		float m_debugvis_cone_height = 60.0f;
		int m_debugvis_cone_steps = 3u;

		float m_debug_float_vec4[4] = {};
		int m_debug_int_vec4[4] = {};

	private:
		void tab_general();
		void tab_map_settings();
		void tab_game_settings();
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
