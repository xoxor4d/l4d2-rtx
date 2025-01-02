#include "std_include.hpp"

#ifdef USE_IMGUI
#include "imgui_internal.h"

// Allow us to directly call the ImGui WndProc function.
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

#define SPACING_INDENT_BEGIN ImGui::Spacing(); ImGui::Indent()
#define SPACING_INDENT_END ImGui::Spacing(); ImGui::Unindent()

#endif

namespace components
{
#if USE_IMGUI
	WNDPROC g_game_wndproc = nullptr;
	
	LRESULT __stdcall wnd_proc_hk(HWND window, UINT message_type, WPARAM wparam, LPARAM lparam)
	{
		if (imgui::get()->input_message(message_type, wparam, lparam)) {
			return true;
		}

		return CallWindowProc(g_game_wndproc, window, message_type, wparam, lparam);
	}

	void center_cursor()
	{
		RECT rect;
		if (GetClientRect(glob::main_window, &rect))
		{
			POINT center;
			center.x = (rect.right - rect.left) / 2;
			center.y = (rect.bottom - rect.top) / 2;

			ClientToScreen(glob::main_window, &center);
			SetCursorPos(center.x, center.y);
		}
	}

	void show_cursor(bool show)
	{
		if (show) {
			while (ShowCursor(TRUE) < 0);
		}
		else {
			while (ShowCursor(FALSE) >= 0);
		}
	}

	bool imgui::input_message(const UINT message_type, const WPARAM wparam, const LPARAM lparam)
	{
		if (message_type == WM_KEYUP && wparam == VK_F5) 
		{
			m_menu_active = !m_menu_active;

			// reset cursor to center when closing the menu to not affect player angles
			// ! not when game input is already locked (menu)
			if (ImGui::GetIO().MouseDrawCursor && !m_menu_active) 
			{
				center_cursor();
				ImGui::GetIO().MouseDrawCursor = false;
			}
		}

		if (m_menu_active) {
			ImGui_ImplWin32_WndProcHandler(glob::main_window, message_type, wparam, lparam);
		}

		return m_menu_active;
	}

	void imgui::devgui()
	{
		if (!ImGui::Begin("Devgui", &m_menu_active))
		{
			ImGui::End();
			return;
		}

		static bool im_demo_menu = false;
		if (ImGui::Button("Demo Menu")) {
			ImGui::ShowDemoWindow(&im_demo_menu);
		}

		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Game Settings", ImGuiTreeNodeFlags_DefaultOpen))
		{
			SPACING_INDENT_BEGIN;

			if (ImGui::Button("Director Start")) {
				interfaces::get()->m_engine->execute_client_cmd_unrestricted("director_start");
			}
			
			ImGui::SameLine();
			if (ImGui::Button("Director Stop")) {
				interfaces::get()->m_engine->execute_client_cmd_unrestricted("director_stop");
			}

			ImGui::Spacing();

			static bool im_zignore_player = false;
			if (ImGui::Checkbox("Z ignore Player", &im_zignore_player))
			{
				if (!im_zignore_player)  {
					interfaces::get()->m_engine->execute_client_cmd_unrestricted("nb_vision_ignore_survivors 0");
				}
				else {
					interfaces::get()->m_engine->execute_client_cmd_unrestricted("nb_vision_ignore_survivors 1");
				}
			}

			ImGui::Spacing();

			if (ImGui::Button("Kick Survivors")) {
				interfaces::get()->m_engine->execute_client_cmd_unrestricted("kick rochelle; kick coach; kick ellis; kick roach");
			}

			ImGui::SameLine();

			if (ImGui::Button("Give Autoshotgun")) {
				interfaces::get()->m_engine->execute_client_cmd_unrestricted("give autoshotgun");
			}

			SPACING_INDENT_END;
		}

		if (ImGui::CollapsingHeader("Culling", ImGuiTreeNodeFlags_DefaultOpen))
		{
			SPACING_INDENT_BEGIN;

			ImGui::Checkbox("Disable R_CullNode", &m_disable_cullnode);
			ImGui::Checkbox("Enable Area Forcing", &m_enable_area_forcing);

			SPACING_INDENT_END;
		}

		if (ImGui::CollapsingHeader("Flashlight"))
		{
			SPACING_INDENT_BEGIN;

			if (ImGui::TreeNodeEx("Player Flashlight", ImGuiTreeNodeFlags_DefaultOpen))
			{
				SPACING_INDENT_BEGIN;

				ImGui::DragFloat("Forward Offset", &m_flashlight_fwd_offset, 0.1f);
				ImGui::DragFloat("Horizontal Offset", &m_flashlight_horz_offset, 0.1f);
				ImGui::DragFloat("Vertical Offset", &m_flashlight_vert_offset, 0.1f);

				ImGui::DragFloat("Intensity", &m_flashlight_intensity, 0.1f);
				ImGui::DragFloat("Radius", &m_flashlight_radius, 0.1f);

				ImGui::Checkbox("Custom Direction", &m_flashlight_use_custom_dir);
				ImGui::DragFloat3("Direction", &m_flashlight_direction.x, 0.01f, 0, 0, "%.2f");
				ImGui::DragFloat("Spot Angle", &m_flashlight_angle, 0.1f);
				ImGui::DragFloat("Spot Softness", &m_flashlight_softness, 0.1f);
				ImGui::DragFloat("Spot Expo", &m_flashlight_exp, 0.1f);

				ImGui::TreePop();
				SPACING_INDENT_END;
			}

			if (ImGui::TreeNodeEx("Bot Flashlight", ImGuiTreeNodeFlags_DefaultOpen))
			{
				SPACING_INDENT_BEGIN;

				ImGui::PushID("bot");
				ImGui::DragFloat("Forward Offset", &m_flashlight_bot_fwd_offset, 0.1f);
				ImGui::DragFloat("Horizontal Offset", &m_flashlight_bot_horz_offset, 0.1f);
				ImGui::DragFloat("Vertical Offset", &m_flashlight_bot_vert_offset, 0.1f);
				ImGui::PopID();

				ImGui::TreePop();
				SPACING_INDENT_END;
			}

			SPACING_INDENT_END;
		}

		ImGui::End();
	}
#endif

	void imgui::endscene_stub()
	{
#if USE_IMGUI
		auto* im = imgui::get();
		if (const auto dev = game::get_d3d_device(); dev)
		{
			if (!im->m_initialized_device)
			{
				ImGui_ImplDX9_Init(dev);
				im->m_initialized_device = true;
			}

			if (im->m_initialized_device)
			{
				ImGui_ImplDX9_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();

				if (!interfaces::get()->m_surface->is_cursor_visible()) {
					ImGui::GetIO().MouseDrawCursor = im->m_menu_active;
				}

				if (!im->m_menu_active) {
					//show_cursor(false);
					ImGui::GetIO().MouseDrawCursor = false;
				}

				//ImGui::GetIO().MouseDrawCursor = im->m_menu_active;
				interfaces::get()->m_surface->set_cursor_always_visible(im->m_menu_active);

				if (im->m_menu_active) {
					im->devgui(); //ImGui::ShowDemoWindow(&im->m_menu_active);
				}

				ImGui::EndFrame();
				ImGui::Render();
				ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			}
		}
#endif
	}

	imgui::imgui()
	{
		p_this = this;

#if USE_IMGUI

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
		//ImGui::StyleColorsDark();

		{	//https://github.com/ocornut/imgui/pull/7826
			ImGuiStyle* style = &ImGui::GetStyle();
			ImVec4* colors = style->Colors;

			colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.94f);
			colors[ImGuiCol_Border] = ImVec4(0.15f, 0.15f, 0.21f, 0.50f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.02f, 0.07f, 0.20f, 0.54f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.05f, 0.31f, 0.96f, 0.40f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.05f, 0.31f, 0.96f, 0.67f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.02f, 0.07f, 0.20f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.53f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.05f, 0.31f, 0.96f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.05f, 0.23f, 0.75f, 1.00f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.05f, 0.31f, 0.96f, 1.00f);
			colors[ImGuiCol_Button] = ImVec4(0.05f, 0.31f, 0.96f, 0.40f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.05f, 0.31f, 0.96f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.24f, 0.96f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.05f, 0.31f, 0.96f, 0.31f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.05f, 0.31f, 0.96f, 0.80f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.05f, 0.31f, 0.96f, 1.00f);
			colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.01f, 0.13f, 0.52f, 0.78f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.01f, 0.13f, 0.52f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.05f, 0.31f, 0.96f, 0.20f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.05f, 0.31f, 0.96f, 0.67f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.05f, 0.31f, 0.96f, 0.95f);
			colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
			colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
			colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
			colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
			colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
			colors[ImGuiCol_PlotLines] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.15f, 0.10f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.79f, 0.45f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.32f, 0.00f, 1.00f);
			colors[ImGuiCol_TableHeaderBg] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
			colors[ImGuiCol_TableBorderStrong] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);   // Prefer using Alpha=1.0 here
			colors[ImGuiCol_TableBorderLight] = ImVec4(0.04f, 0.04f, 0.05f, 1.00f);   // Prefer using Alpha=1.0 here
			colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.05f, 0.31f, 0.96f, 0.35f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
			colors[ImGuiCol_NavHighlight] = ImVec4(0.05f, 0.31f, 0.96f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.60f, 0.60f, 0.60f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.60f, 0.60f, 0.60f, 0.35f);
		}

		ImGui_ImplWin32_Init(glob::main_window);
		g_game_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(glob::main_window, GWLP_WNDPROC, LONG_PTR(wnd_proc_hk)));
#endif
	}

	imgui::~imgui()
	{ }
}
