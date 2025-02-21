#include "std_include.hpp"
#include "components/common/imgui/imgui_helper.hpp"
#include "components/common/toml.hpp"
#include "components/common/imgui/font_awesome_solid_900.hpp"
#include "components/common/imgui/font_defines.hpp"
#include "components/common/imgui/font_opensans.hpp"

#include "imgui_internal.h"

// Allow us to directly call the ImGui WndProc function.
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

#define SPACING_INDENT_BEGIN ImGui::Spacing(); ImGui::Indent()
#define SPACING_INDENT_END ImGui::Spacing(); ImGui::Unindent()
#define TT(TXT) ImGui::SetItemTooltipBlur((TXT));

#define SET_CHILD_WIDGET_WIDTH			ImGui::SetNextItemWidth(ImGui::CalcWidgetWidthForChild(80.0f));
#define SET_CHILD_WIDGET_WIDTH_MAN(V)	ImGui::SetNextItemWidth(ImGui::CalcWidgetWidthForChild((V)));

namespace components
{
	WNDPROC g_game_wndproc = nullptr;
	
	LRESULT __stdcall wnd_proc_hk(HWND window, UINT message_type, WPARAM wparam, LPARAM lparam)
	{
		bool pass_msg_to_game = false;

		switch (message_type)
		{
		case WM_KEYUP: // always pass button up events to prevent "stuck" game keys

		// allows user to move the game window via titlebar :>
		case WM_NCLBUTTONDOWN: case WM_NCLBUTTONUP: case WM_NCMOUSEMOVE: case WM_NCMOUSELEAVE:
		case WM_WINDOWPOSCHANGED: //case WM_WINDOWPOSCHANGING:

		//case WM_INPUT:
		//case WM_NCHITTEST: // sets cursor to center
		//case WM_SETCURSOR: case WM_CAPTURECHANGED:

		case WM_NCACTIVATE:
		case WM_SETFOCUS: case WM_KILLFOCUS:
		case WM_SYSCOMMAND:
		
		case WM_GETMINMAXINFO: case WM_ENTERSIZEMOVE: case WM_EXITSIZEMOVE:
		case WM_SIZING: case WM_MOVING: case WM_MOVE:
			pass_msg_to_game = true;
			break;

		case WM_MOUSEACTIVATE: 
			if (ImGui::GetIO().WantCaptureMouse || !imgui::get()->m_menu_active)
			{
				pass_msg_to_game = true;
			}
			break;

		default: break; 
		}

		if (imgui::get()->input_message(message_type, wparam, lparam) && !pass_msg_to_game) {
			return true;
		}

		/*if (message_type == WM_SYSCOMMAND) {
			game::console(); printf("MSG 0x%x -- w: 0x%x -- l: 0x%x\n", message_type, wparam, lparam);
		}
		else {
			game::console(); printf("MSG 0x%x\n", message_type);
		}*/
			
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

	bool imgui::input_message(const UINT message_type, const WPARAM wparam, const LPARAM lparam)
	{
		if (message_type == WM_KEYUP && wparam == VK_F5) 
		{
			const auto& io = ImGui::GetIO();
			if (!io.MouseDown[1])
			{
				m_menu_active = !m_menu_active;

				// reset cursor to center when closing the menu to not affect player angles
				if (interfaces::get()->m_surface->is_cursor_visible() && !m_menu_active)
				{
					center_cursor();
					SendMessage(glob::main_window, WM_ACTIVATEAPP, TRUE, 0);
					SendMessage(glob::main_window, WM_MOUSEACTIVATE, TRUE, 0);
				}

				interfaces::get()->m_surface->set_cursor_always_visible(m_menu_active);
			}

			else {
				ImGui_ImplWin32_WndProcHandler(glob::main_window, message_type, wparam, lparam);
			}
		}

		if (m_menu_active)
		{
			const auto& io = ImGui::GetIO();

			ImGui_ImplWin32_WndProcHandler(glob::main_window, message_type, wparam, lparam);

			// enable game input if no imgui window is hovered and right mouse is held
			if (!m_im_window_hovered && io.MouseDown[1])
			{
				// center cursor and only call set_cursor_always_visible once 
				if (!m_im_allow_game_input)
				{
					center_cursor(); 
					interfaces::get()->m_surface->set_cursor_always_visible(false);
				}

				ImGui::SetWindowFocus(); // unfocus input text
				m_im_allow_game_input = true;
				return false;
			}

			// ^ wait until mouse is up and call set_cursor_always_visible once
			if (m_im_allow_game_input && !io.MouseDown[1])
			{
				m_im_allow_game_input = false;
				interfaces::get()->m_surface->set_cursor_always_visible(true);
				return false;
			}
		}
		else {
			m_im_allow_game_input = false; // always reset if there is no imgui window open
		}

		return m_menu_active;
	}

	// ------

	bool reload_mapsettings_popup()
	{
		bool result = false;
		if (ImGui::BeginPopupModal("Reload MapSettings?", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
		{
			common::imgui::draw_background_blur();
			const auto half_width = ImGui::GetContentRegionMax().x * 0.5f;
			auto line1_str = "You'll loose all unsaved changes if you continue!";
			auto line2_str = "Use the copy to clipboard buttons and manually update  ";
			auto line3_str = "the map_settings.toml file if you've made changes.";

			ImGui::Spacing();
			ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line1_str).x * 0.5f));
			ImGui::TextUnformatted(line1_str);

			ImGui::Spacing();
			ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line2_str).x * 0.5f));
			ImGui::TextUnformatted(line2_str);
			ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line3_str).x * 0.5f));
			ImGui::TextUnformatted(line3_str);

			ImGui::Spacing(0, 8);
			ImGui::Spacing(0, 0); ImGui::SameLine();

			ImVec2 button_size(half_width - 6.0f - ImGui::GetStyle().WindowPadding.x, 0.0f);
			if (ImGui::Button("Reload", button_size))
			{
				result = true;
				map_settings::reload();
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine(0, 6);
			if (ImGui::Button("Cancel", button_size)) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		return result;
	}

	bool reload_mapsettings_button_with_popup(const char* ID)
	{
		ImGui::PushFont(common::imgui::font::BOLD);
		if (ImGui::Button(utils::va("Reload MapSettings  %s##%s", ICON_FA_REDO, ID), ImVec2(ImGui::GetContentRegionAvail().x, 0)))
		{
			if (!ImGui::IsPopupOpen("Reload MapSettings?")) {
				ImGui::OpenPopup("Reload MapSettings?");
			}
		}
		ImGui::PopFont();

		return reload_mapsettings_popup();
	}

	// #
	// #

	void cont_general_quickcommands()
	{
		if (ImGui::Button("Director Start")) {
			interfaces::get()->m_engine->execute_client_cmd_unrestricted("sv_cheats 1; director_start");
		}

		ImGui::SameLine();
		if (ImGui::Button("Director Stop")) {
			interfaces::get()->m_engine->execute_client_cmd_unrestricted("sv_cheats 1; director_stop");
		}

		ImGui::SameLine();
		if (ImGui::Button("Kick Survivor Bots")) {
			interfaces::get()->m_engine->execute_client_cmd_unrestricted("sv_cheats 1; kick rochelle; kick coach; kick ellis; kick roach; kick louis; kick zoey; kick francis; kick bill");
		}

		ImGui::SameLine();
		if (ImGui::Button("Give Autoshotgun")) {
			interfaces::get()->m_engine->execute_client_cmd_unrestricted("sv_cheats 1; give autoshotgun");
		}

		ImGui::Spacing();

		static bool im_zignore_player = false;
		if (ImGui::Checkbox("Infected Ignore Player", &im_zignore_player))
		{
			if (!im_zignore_player) {
				interfaces::get()->m_engine->execute_client_cmd_unrestricted("sv_cheats 1; nb_vision_ignore_survivors 0");
			}
			else {
				interfaces::get()->m_engine->execute_client_cmd_unrestricted("sv_cheats 1; nb_vision_ignore_survivors 1");
			}
		}
	}

	void imgui::tab_general()
	{
		// quick commands
		{
			static float cont_quickcmd_height = 0.0f;
			cont_quickcmd_height = ImGui::Widget_ContainerWithCollapsingTitle("Quick Commands", cont_quickcmd_height, cont_general_quickcommands,
				true, ICON_FA_TERMINAL, &ImGuiCol_ContainerBackground, &ImGuiCol_ContainerBorder);
		}
	}

	// #
	// #

	void cont_mapsettings_general()
	{
		ImGui::PushFont(common::imgui::font::BOLD);
		if (ImGui::Button("Reload rtx.conf    " ICON_FA_REDO, ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
		{
			if (!ImGui::IsPopupOpen("Reload RtxConf?")) {
				ImGui::OpenPopup("Reload RtxConf?");
			}
		} ImGui::PopFont();

		// popup
		if (ImGui::BeginPopupModal("Reload RtxConf?", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
		{
			common::imgui::draw_background_blur();
			ImGui::Spacing(0.0f, 0.0f);

			const auto half_width = ImGui::GetContentRegionMax().x * 0.5f;
			auto line1_str = "This will reload the rtx.conf file and re-apply all of it's variables.  ";
			auto line3_str = "(excluding texture hashes)";

			ImGui::Spacing();
			ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line1_str).x * 0.5f));
			ImGui::TextUnformatted(line1_str);

			ImGui::PushFont(common::imgui::font::BOLD);
			ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line3_str).x * 0.5f));
			ImGui::TextUnformatted(line3_str);
			ImGui::PopFont();

			ImGui::Spacing(0, 8);
			ImGui::Spacing(0, 0); ImGui::SameLine();

			ImVec2 button_size(half_width - 6.0f - ImGui::GetStyle().WindowPadding.x, 0.0f);
			if (ImGui::Button("Reload", button_size))
			{
				remix_vars::xo_vars_parse_options_fn();
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine(0, 6.0f);
			if (ImGui::Button("Cancel", button_size)) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::SameLine();
		reload_mapsettings_button_with_popup("General");

		ImGui::Checkbox("Show Area Debug Info", &cmd::debug_node_vis);
		TT("Toggle bsp node/leaf debug visualization using the remix api\n~~ cmd: xo_debug_toggle_node_vis");

		ImGui::Checkbox("Show Static Prop Debug Info", &cmd::model_info_vis);
		TT("Toggle model name and radius visualizations\nUseful for HIDEMODEL (MapSettings)\n~~ cmd: xo_debug_toggle_model_info");

		SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
		ImGui::SliderInt2("HUD: Area Debug Pos", &main_module::get()->m_hud_debug_node_vis_pos[0], 0, 512);

		{
			auto* default_nocull_dist = &map_settings::get_map_settings().default_nocull_dist;
			SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
			if (ImGui::DragFloat("Def. NoCull Dist", default_nocull_dist, 0.5f, 0.0f)) {
				*default_nocull_dist = *default_nocull_dist < 0.0f ? 0.0f : *default_nocull_dist;
			}
			TT("Default distance value for the default anti-cull mode (distance) if there is no override for the current area");
		}

#if DEBUG
		{
			const auto im = imgui::get();

			ImGui::Spacing(0,8);
			if (ImGui::CollapsingHeader("DEBUG Build Section", ImGuiTreeNodeFlags_SpanFullWidth))
			{
				SET_CHILD_WIDGET_WIDTH; ImGui::Checkbox("Disable R_CullNode", &im->m_disable_cullnode);
				SET_CHILD_WIDGET_WIDTH; ImGui::Checkbox("Enable Area Forcing", &im->m_enable_area_forcing);

				const auto coloredit_flags = ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_Float;

				SET_CHILD_WIDGET_WIDTH; ImGui::ColorEdit4("ContainerBg", &im->ImGuiCol_ContainerBackground.x, coloredit_flags);
				SET_CHILD_WIDGET_WIDTH; ImGui::ColorEdit4("ContainerBorder", &im->ImGuiCol_ContainerBorder.x, coloredit_flags);

				SET_CHILD_WIDGET_WIDTH; ImGui::ColorEdit4("ButtonGreen", &im->ImGuiCol_ButtonGreen.x, coloredit_flags);
				SET_CHILD_WIDGET_WIDTH; ImGui::ColorEdit4("ButtonYellow", &im->ImGuiCol_ButtonYellow.x, coloredit_flags);
				SET_CHILD_WIDGET_WIDTH; ImGui::ColorEdit4("ButtonRed", &im->ImGuiCol_ButtonRed.x, coloredit_flags);

				const auto glob = interfaces::get()->m_globals;
				ImGui::Text("Realtime: %.4f", glob->realtime);
				ImGui::Text("Curtime Abs: %.4f", glob->curtime);
				ImGui::Text("MaxClients: %.4f", glob->maxClients);
				ImGui::Text("Frametime Abs: %.4f", glob->absoluteframetime);
				ImGui::Text("Frametime: %.4f", glob->frametime);
			}
		}
#endif
	}

	void cont_mapsettings_fog()
	{
		auto& ms = map_settings::get_map_settings();

		Vector fog_color = {};
		fog_color.x = static_cast<float>((ms.fog_color >> 16) & 0xFF) / 255.0f * 1.0f;
		fog_color.y = static_cast<float>((ms.fog_color >>  8) & 0xFF) / 255.0f * 1.0f;
		fog_color.z = static_cast<float>((ms.fog_color >>  0) & 0xFF) / 255.0f * 1.0f;

		ImGui::PushFont(common::imgui::font::BOLD);
		if (ImGui::Button("Copy Settings to Clipboard   " ICON_FA_SAVE "##Fog", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
		{
			std::string toml_str = map_settings::get_map_settings().mapname + " = { "s;
			toml_str += "distance = " + std::to_string(map_settings::get_map_settings().fog_dist) + ", "s;
			toml_str += "color = ["
				+ std::to_string(static_cast<int>(std::round(fog_color.x * 255.0f))) + ", "
				+ std::to_string(static_cast<int>(std::round(fog_color.y * 255.0f))) + ", "
				+ std::to_string(static_cast<int>(std::round(fog_color.z * 255.0f))) + "] }"s;

			ImGui::LogToClipboard();
			ImGui::LogText("%s", toml_str.c_str());
			ImGui::LogFinish();
		} ImGui::PopFont();

		ImGui::SameLine();
		reload_mapsettings_button_with_popup("Fog");

		ImGui::Spacing();
		ImGui::Spacing();

		bool fog_enabled = ms.fog_dist != 0.0f;
		static float old_fog_val = 0.0f;
		SET_CHILD_WIDGET_WIDTH;
		if (ImGui::Checkbox("Enable Fog", &fog_enabled))
		{
			if (!fog_enabled) 
			{
				old_fog_val = ms.fog_dist;
				ms.fog_dist = 0.0f;
			}
			else
			{
				ms.fog_dist = old_fog_val > 0.0f ? old_fog_val : 15000.0f;
				if (ms.fog_color == 0xFFFFFFFF) {
					ms.fog_color = 0xFF646464;
				}
			}
		}

		SET_CHILD_WIDGET_WIDTH;
		if (ImGui::DragFloat("Distance", &ms.fog_dist, 1.0f, 0.0f)) {
			ms.fog_dist = ms.fog_dist < 0.0f ? 0.0f : ms.fog_dist;
		}

		SET_CHILD_WIDGET_WIDTH;
		if (ImGui::ColorEdit3("Transmission", &fog_color.x, ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_PickerHueBar)) {
			ms.fog_color = D3DCOLOR_COLORVALUE(fog_color.x, fog_color.y, fog_color.z, 1.0f);
		}
	}

	void cont_mapsettings_marker_manipulation()
	{
		auto& markers = map_settings::get_map_settings().map_markers;
		ImGui::PushFont(common::imgui::font::BOLD);
		if (ImGui::Button("Copy All Markers to Clipboard   " ICON_FA_SAVE, ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
		{
			ImGui::LogToClipboard();
			ImGui::LogText("%s", common::toml::build_map_marker_string_for_current_map(markers).c_str());
			ImGui::LogFinish();
		} ImGui::PopFont();

		ImGui::SameLine();
		reload_mapsettings_button_with_popup("MapMarker");
		//ImGui::Spacing(0, 4);

		constexpr auto in_buflen = 1024u;
		static char in_area_buf[in_buflen], in_nleaf_buf[in_buflen];
		static map_settings::marker_settings_s* selection = nullptr;

		//
		// MARKER TABLE

		ImGui::TableHeaderDropshadow();
		if (ImGui::BeginTable("MarkerTable", 10,
			ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody |
			ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_ScrollY, ImVec2(0, 380)))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // make top row always visible
			ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 12.0f);
			ImGui::TableSetupColumn("Num", ImGuiTableColumnFlags_NoResize, 24.0f);
			ImGui::TableSetupColumn("NC", ImGuiTableColumnFlags_NoResize, 24.0f);
			ImGui::TableSetupColumn("Areas", ImGuiTableColumnFlags_WidthStretch, 80.0f);
			ImGui::TableSetupColumn("NLeafs", ImGuiTableColumnFlags_WidthStretch, 80.0f);
			ImGui::TableSetupColumn("Comment", ImGuiTableColumnFlags_WidthStretch, 200.0f);
			ImGui::TableSetupColumn("Pos", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 200.0f);
			ImGui::TableSetupColumn("Rot", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 180.0f);
			ImGui::TableSetupColumn("Scale", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultHide, 130.0f);
			ImGui::TableSetupColumn("##Delete", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoClip, 16.0f);
			ImGui::TableHeadersRow();

			bool selection_matches_any_entry = false;
			map_settings::marker_settings_s* marked_for_deletion = nullptr;

			for (auto i = 0u; i < markers.size(); i++)
			{
				auto& m = markers[i];

				// default selection
				if (!selection) {
					selection = &m;
				}

				ImGui::TableNextRow();

				// save Y offset
				const auto save_row_min_y_pos = ImGui::GetCursorScreenPos().y - ImGui::GetStyle().FramePadding.y + ImGui::GetStyle().CellPadding.y;

				// handle row background color for selected entry
				const bool is_selected = selection && selection == &m;
				if (is_selected) {
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
				}

				// -
				ImGui::TableNextColumn();
				if (!is_selected) // only selectable if not selected
				{
					ImGui::Style_InvisibleSelectorPush(); // never show selection - we use tablebg
					if (ImGui::Selectable(utils::va("%d", i), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 22 + ImGui::GetStyle().CellPadding.y * 1.0f))) {
						selection = &m;
					}
					ImGui::Style_InvisibleSelectorPop();

					if (ImGui::IsItemHovered()) {
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0.6f)));///*ImGui::GetColorU32(ImGuiCol_TableRowBgAlt)*/);
					}
				}
				else {
					ImGui::Text("%d", i); // if selected
				}

				if (selection && selection == &m) {
					selection_matches_any_entry = true; // check that the selection ptr is up to date
				}

				// - marker num
				ImGui::TableNextColumn();
				ImGui::Text("%d", m.index);

				// - nocull
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(m.no_cull ? "x" : "");

				// - Area Input
				ImGui::TableNextColumn();

				if (is_selected) {
					ImGui::Widget_UnorderedSetModifier("MarkerArea", ImGui::Widget_UnorderedSetModifierFlags_Area, selection->areas, in_area_buf, in_buflen);
				}

				ImGui::Spacing();
				ImGui::TextWrapped_IntegersFromUnorderedSet(m.areas);
				ImGui::Spacing();

				// - NLeaf Input
				ImGui::TableNextColumn();
				if (is_selected) {
					ImGui::Widget_UnorderedSetModifier("MarkerNLeafs", ImGui::Widget_UnorderedSetModifierFlags_Leaf, selection->when_not_in_leafs, in_nleaf_buf, in_buflen);
				}

				ImGui::Spacing();
				ImGui::TextWrapped_IntegersFromUnorderedSet(m.when_not_in_leafs);
				ImGui::Spacing();

				// - comment
				ImGui::TableNextColumn();
				ImGui::TextWrapped(m.comment.c_str());

				const auto row_max_y_pos = ImGui::GetItemRectMax().y;

				// - pos
				ImGui::TableNextColumn(); ImGui::Spacing();
				ImGui::Text("%.2f, %.2f, %.2f", m.origin.x, m.origin.y, m.origin.z);

				// - rot
				ImGui::TableNextColumn(); ImGui::Spacing();
				ImGui::Text("%.2f, %.2f, %.2f", m.rotation.x, m.rotation.y, m.rotation.z);

				// - scale
				ImGui::TableNextColumn(); ImGui::Spacing();
				ImGui::Text("%.2f, %.2f, %.2f", m.scale.x, m.scale.y, m.scale.z);

				// Delete Button
				ImGui::TableNextColumn();
				{
					ImGui::Style_DeleteButtonPush();
					ImGui::PushID((int)i);

					const auto btn_size = ImVec2(16, is_selected ? (row_max_y_pos - save_row_min_y_pos) : 25.0f);
					if (ImGui::Button("x##Marker", btn_size))
					{
						marked_for_deletion = &m;
						main_module::trigger_vis_logic();
					}

					ImGui::Style_DeleteButtonPop();
					ImGui::PopID();
				}

			} // end for loop

			if (!selection_matches_any_entry)
			{
				for (auto& m : markers)
				{
					if (selection && selection == &m)
					{
						selection_matches_any_entry = true;
						break;
					}
				}

				if (!selection_matches_any_entry) {
					selection = nullptr;
				}
			}
			else if (selection) {
				game::debug_add_text_overlay(&selection->origin.x, "[ImGui] Selected Marker", 0, 0.8f, 1.0f, 0.3f, 0.8f);
			}

			// remove entry
			if (marked_for_deletion)
			{
				for (auto it = markers.begin(); it != markers.end(); ++it)
				{
					if (&*it == marked_for_deletion)
					{
						markers.erase(it);
						selection = nullptr;
						break;
					}
				}
			}
			ImGui::EndTable();
		}

		ImGui::Style_ColorButtonPush(imgui::get()->ImGuiCol_ButtonGreen, true);
		if (ImGui::Button("++ Marker"))
		{
			std::uint32_t free_marker = 0u;
			for (auto i = 0u; i < markers.size(); i++)
			{
				if (markers[i].index == free_marker)
				{
					free_marker++;
					i = 0u; // restart loop
				}
			}

			markers.emplace_back(map_settings::marker_settings_s{
					free_marker, *game::get_current_view_origin() - Vector(0,0,1), true
				});

			selection = &markers.back();
		}
		ImGui::Style_ColorButtonPop();

		if (selection)
		{
			ImGui::SameLine();
			ImGui::Style_ColorButtonPush(imgui::get()->ImGuiCol_ButtonYellow, true);
			if (ImGui::Button("Duplicate Current Marker"))
			{
				markers.emplace_back(map_settings::marker_settings_s{
					.index = selection->index,
					.origin = selection->origin,
					.no_cull = selection->no_cull,
					.rotation = selection->rotation,
					.scale = selection->scale,
					.areas = selection->areas,
					.when_not_in_leafs = selection->when_not_in_leafs
					});

				selection = &markers.back();
			}
			ImGui::Style_ColorButtonPop();
		}

		ImGui::SameLine();
		ImGui::BeginDisabled(!selection);
		{
			if (ImGui::Button("TP to Marker")) {
				interfaces::get()->m_engine->execute_client_cmd_unrestricted(utils::va("sv_cheats 1; noclip; setpos %.2f %.2f %.2f", selection->origin.x, selection->origin.y, selection->origin.z - 40.0f));
			}

			ImGui::SameLine();
			if (ImGui::Button("TP Marker to Player", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
			{
				selection->origin = *game::get_current_view_origin();
				selection->origin.z -= 1.0f;
			}
			ImGui::EndDisabled();
		}

		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::SeparatorText("Modify Marker");

		ImGui::Spacing();
		ImGui::Spacing();

		if (selection)
		{
			int temp_num = (int)selection->index;

			SET_CHILD_WIDGET_WIDTH;
			if (ImGui::DragInt("Number", &temp_num, 0.1f, 0))
			{
				if (temp_num < 0) {
					temp_num = 0;
				}
				else if (!selection->no_cull && temp_num > 99) {
					temp_num = 99;
				}

				selection->index = (std::uint32_t)temp_num;
			}

			//ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.6f, 0.5f));
			ImGui::Widget_PrettyDragVec3("Origin", &selection->origin.x, true, 80.0f, 0.5f,
				-FLT_MAX, FLT_MAX, "X", "Y", "Z");
			//ImGui::PopStyleVar();

			// RAD2DEG -> DEG2RAD 
			Vector temp_rot = { RAD2DEG(selection->rotation.x), RAD2DEG(selection->rotation.y), RAD2DEG(selection->rotation.z) };

			ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.6f, 0.5f));
			if (ImGui::Widget_PrettyDragVec3("Rotation", &temp_rot.x, true, 80.0f, 0.1f,
				-360.0f, 360.0f, "Rx", "Ry", "Rz")) 
			{
				selection->rotation = { DEG2RAD(temp_rot.x), DEG2RAD(temp_rot.y), DEG2RAD(temp_rot.z) };
			} ImGui::PopStyleVar();

			if (selection->no_cull) 
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.6f, 0.5f));
				ImGui::Widget_PrettyDragVec3("Scale", &selection->scale.x, true, 80.0f, 0.01f,
					-FLT_MAX, FLT_MAX, "Sx", "Sy", "Sz");
				ImGui::PopStyleVar();
			}

			SET_CHILD_WIDGET_WIDTH;
			ImGui::InputText("Comment", &selection->comment);
		} // selection

		ImGui::Spacing();
		ImGui::Spacing();
		if (ImGui::TreeNodeEx("Help##Marker", ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			ImGui::TextUnformatted(
				"# Spawn unique markers that can be used as anchor meshes (same one can be spawned multiple times)\n"
				"# Parameters ---------------------------------------------------------------------------------------------------------------------------------\n"
				"#\n"
				"# marker    ]     THIS:     number of marker mesh - can get culled BUT that can be controlled via leaf/area forcing (initial spawning can't be forced)[int 0 - 100]\n"
				"# nocull    ]  OR THAT:     number of marker mesh - never getting culled and spawned on map load (eg: useful for distant light) [int 0-inf.]\n"
				"# nocull    |>   areas:     (optional) only show nocull marker when player is in specified area/s [int array]\n"
				"# nocull    |> N_leafs:     (optional) only show nocull marker when player is in ^ and NOT in specified leaf/s [int array]\n"
				"#\n"
				"# position:                 X Y Z position of the marker mesh [3D Vector]\n"
				"# rotation:                 X Y Z rotation of the marker mesh [3D Vector]\n"
				"# scale:                    X Y Z scale of the marker mesh [3D Vector]\n");

			ImGui::TreePop();
		}
	}

	void cont_mapsettings_culling_manipulation()
	{
		auto& areas = map_settings::get_map_settings().area_settings;
		ImGui::PushFont(common::imgui::font::BOLD);
		if (ImGui::Button("Copy Settings to Clipboard   " ICON_FA_SAVE "##Cull", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
		{
			ImGui::LogToClipboard();
			ImGui::LogText("%s", common::toml::build_culling_overrides_string_for_current_map(areas).c_str());
			ImGui::LogFinish();
		} ImGui::PopFont();

		ImGui::SameLine();
		reload_mapsettings_button_with_popup("Cull");
		//ImGui::Spacing(0, 4);

		static map_settings::area_overrides_s* area_selection = nullptr;
		static map_settings::area_overrides_s* area_selection_old = nullptr;
		area_selection_old = area_selection; // we compare at the end of the table

		static map_settings::leaf_tweak_s* tweak_selection = nullptr;
		static map_settings::hide_area_s* hidearea_selection = nullptr;

		constexpr auto in_buflen = 1024u;
		static char in_leafs_buf[in_buflen], in_areas_buf[in_buflen],
			in_twk_in_leafs_buf[in_buflen], in_twk_areas_buf[in_buflen], in_twk_force_leafs_buf[in_buflen],
			in_hide_leafs_buf[in_buflen], in_hide_areas_buf[in_buflen], in_hide_nleafs_buf[in_buflen];

		// # CULL TABLE
		constexpr auto cull_table_num_columns = 6;
		ImGui::TableHeaderDropshadow();

		if (ImGui::BeginTable("CullTable", cull_table_num_columns, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
			ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_ScrollY, ImVec2(0, 480)))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // make top row always visible
			ImGui::TableSetupColumn("Ar", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 16.0f);
			ImGui::TableSetupColumn("Mode", ImGuiTableColumnFlags_WidthStretch, 34.0f);
			ImGui::TableSetupColumn("Leafs", ImGuiTableColumnFlags_WidthStretch, 80.0f);
			ImGui::TableSetupColumn("Areas", ImGuiTableColumnFlags_WidthStretch, 60.0f);
			ImGui::TableSetupColumn("Hide-Leafs", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultHide, 40.0f);
			ImGui::TableSetupColumn("LeafTweaks & HideAreas", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 140.0f);

			const char* cull_table_tooltips[cull_table_num_columns] =
			{
				"The Area the player has to be in to trigger any override functionality.\n",
				"Different anti/culling modes and additional settings for various use-cases.",
				"The Leafs which will have their visibility forced.",
				"The Areas which will have their visibility forced.",
				"The Leafs with forced invisibility.",
				("LeafTweaks: Additional per leaf overrides that only trigger if the player is in specified Leafs.\n"
				 "~ Can be quite useful at area crossings when used in conjunction with area-specific markers that spawn visibility blockers (eg. a flat plane)\n\n"
				 "HideAreas: This can be used to forcefully cull parts of the map when the game is drawing too much.\n")
			};

			ImGui::TableHeadersRowWithTooltip(cull_table_tooltips);

			bool area_selection_matches_any_entry = false;
			auto row_num = 0u;

			for (auto& [area_num, a] : areas)
			{
				ImGui::TableNextRow();

				// default selection
				if (!area_selection) {
					area_selection = &a;
				}

				// save Y offset
				const auto area_table_first_row_y_pos = ImGui::GetCursorScreenPos().y - ImGui::GetStyle().FramePadding.y + ImGui::GetStyle().CellPadding.y;

				// handle row background color for selected entry
				const bool is_area_selected = area_selection && area_selection == &a;
				const bool player_is_in_area = g_player_current_area_override && g_player_current_area_override == &a;

				// -
				ImGui::TableNextColumn();

				float first_col_width = ImGui::GetCursorScreenPos().x;
				float start_y = ImGui::GetCursorScreenPos().y; // save row start of selector at the end of a row

				if (is_area_selected) {
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
				}

				// set background for first column - highlight current area
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
					player_is_in_area ? ImGui::GetColorU32(ImGuiCol_DragDropTarget) : ImGui::GetColorU32(ImGuiCol_TableHeaderBg));

				// - Area
				const auto ar_num_str = utils::va("%d", (int)area_num);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x * 0.5f - ImGui::CalcTextSize(ar_num_str).x * 0.5f));
				ImGui::TextUnformatted(ar_num_str);

				if (is_area_selected) {
					area_selection_matches_any_entry = true; // check that the selection ptr is up to date
				}

				// Mode
				ImGui::TableNextColumn();

				// width of first col
				first_col_width = ImGui::GetCursorScreenPos().x - first_col_width;

				// dropdown
				ImGui::PushID((int)area_num);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				if (ImGui::BeginCombo("##ModeSelector", map_settings::AREA_CULL_MODE_STR[a.cull_mode], ImGuiComboFlags_None))
				{
					for (int n = 0; n < IM_ARRAYSIZE(map_settings::AREA_CULL_MODE_STR); n++)
					{
						const bool is_selected = (a.cull_mode == n);
						if (ImGui::Selectable(map_settings::AREA_CULL_MODE_STR[n], is_selected)) {
							a.cull_mode = (map_settings::AREA_CULL_MODE)n;
						}

						if (is_selected) {
							ImGui::SetItemDefaultFocus();
						}

					}
					ImGui::EndCombo();
				}
				TT( "No Frustum:\t\t\t      Compl. disable frustum culling (everywhere)\n"
					"No Frustum in Area:    Compl. disable frustum culling when in current area\n"
					"Stock:\t\t\t\t\t		 Stock frustum culling\n"
					"Force Area:\t\t\t\t   ^ + force all nodes/leafs in current area\n"
					"Force Area Dist:\t\t   ^ + all outside of current area within certain dist to player\n"
					"Distance:\t\t\t\t       Force all nodes/leafs within certain dist to player");

				if (a.cull_mode >= map_settings::AREA_CULL_INFO_NOCULLDIST_START
					&& a.cull_mode <= map_settings::AREA_CULL_INFO_NOCULLDIST_END)
				{
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::DragFloat("##NocullDist", &a.nocull_distance, 0.1f, 0.0f, FLT_MAX, "%.0f"))
					{
						a.nocull_distance = a.nocull_distance < 0.0f ? 0.0f : a.nocull_distance;
						main_module::trigger_vis_logic();
					} TT("NoCull Distance - Radius around the player where nothing will get culled.")
				}

				auto row_max_y_pos = ImGui::GetItemRectMax().y;
				ImGui::PopID();

				// - Leafs
				ImGui::TableNextColumn();
				{
					// Leaf Input
					if (is_area_selected) {
						ImGui::Widget_UnorderedSetModifier("CullLeafs", ImGui::Widget_UnorderedSetModifierFlags_Leaf, area_selection->leafs, in_leafs_buf, in_buflen);
					}

					ImGui::Spacing();
					ImGui::TextWrapped_IntegersFromUnorderedSet(a.leafs);
					ImGui::Spacing();
					row_max_y_pos = std::max(row_max_y_pos, ImGui::GetItemRectMax().y);
				}

				// - Areas
				ImGui::TableNextColumn();
				{
					// Area Input
					if (is_area_selected) {
						ImGui::Widget_UnorderedSetModifier("CullAreas", ImGui::Widget_UnorderedSetModifierFlags_Area, area_selection->areas, in_areas_buf, in_buflen);
					}

					ImGui::Spacing();
					ImGui::TextWrapped_IntegersFromUnorderedSet(a.areas);
					ImGui::Spacing();
					row_max_y_pos = std::max(row_max_y_pos, ImGui::GetItemRectMax().y);
				}

				// - Hide Leafs
				ImGui::TableNextColumn();
				{
					// Hide Leafs Input
					if (is_area_selected) {
						ImGui::Widget_UnorderedSetModifier("CullHideLeafs", ImGui::Widget_UnorderedSetModifierFlags_Leaf, area_selection->hide_leafs, in_hide_leafs_buf, in_buflen);
					}

					ImGui::Spacing();
					ImGui::TextWrapped_IntegersFromUnorderedSet(a.hide_leafs);
					ImGui::Spacing();
					row_max_y_pos = std::max(row_max_y_pos, ImGui::GetItemRectMax().y);
				}

				// - tweak leafs + hide_areas
				ImGui::TableNextColumn();

				// 
				if (is_area_selected) {
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableRowBg));
				}
				

				bool any_tweak_with_nocull_override = false;
				if (!a.leaf_tweaks.empty())
				{
					// inline table for leaf tweaks
					constexpr auto twk_leaf_num_columns = 5;
					//ImGui::TableHeaderDropshadow(8.0f, 0.6f, 4.0f);

					if (ImGui::BeginTable("tweak_leafs_nested_table", twk_leaf_num_columns, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
						ImGuiTableFlags_Reorderable | ImGuiTableFlags_ContextMenuInBody))
					{
						ImGui::TableSetupColumn("Tweak in Leafs", ImGuiTableColumnFlags_WidthStretch, 100.0f);
						ImGui::TableSetupColumn("Tweak Areas", ImGuiTableColumnFlags_WidthStretch, 100.0f);
						ImGui::TableSetupColumn("Tweak Leafs", ImGuiTableColumnFlags_WidthStretch, 100.0f);
						ImGui::TableSetupColumn("NoCullDist", ImGuiTableColumnFlags_WidthStretch, 44.0f);
						ImGui::TableSetupColumn("##Delete", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoClip, 16.0f);

						const char* twk_leaf_tooltips[twk_leaf_num_columns] =
						{
							"The leaf/s the player has to be in to trigger any leaf tweak functionality.\n",
							"The Areas which will have their visibility forced.",
							"The Leafs which will have their visibility forced.",
							"Can be used to override the 'NoCull Distance' value specified in the area (if > 0).",
							""
						};

						ImGui::TableHeadersRowWithTooltip(twk_leaf_tooltips);

						std::uint32_t cur_row = 0;
						bool selection_matches_any_entry = false;
						map_settings::leaf_tweak_s* marked_for_deletion = nullptr;

						for (auto& lt : a.leaf_tweaks)
						{
							if (is_area_selected && !tweak_selection) {
								tweak_selection = &lt;
							}

							ImGui::TableNextRow();

							// save Y offset because a multiline cell messes with following cells
							const auto tweak_table_first_row_y_pos = ImGui::GetCursorScreenPos().y - ImGui::GetStyle().FramePadding.y + ImGui::GetStyle().CellPadding.y;
							const bool is_tweak_selected = tweak_selection && tweak_selection == &lt;

							if (is_tweak_selected) {
								ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(
									ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_TableRowBgAlt)) + ImVec4(0.1f, 0.1f, 0.1f, 0.0f)));
							}
							else {
								ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBg));
							}

							// ---
							// Twk In Leafs
							ImGui::TableNextColumn();

							// save row start of selector at the end of a row
							float twk_start_y = ImGui::GetCursorPosY();
							{
								// Input
								if (is_tweak_selected) {
									ImGui::Widget_UnorderedSetModifier("CullTwkInLeafs", ImGui::Widget_UnorderedSetModifierFlags_Leaf, tweak_selection->in_leafs, in_twk_in_leafs_buf, in_buflen);
								}

								ImGui::Spacing();
								ImGui::TextWrapped_IntegersFromUnorderedSet(lt.in_leafs);
								ImGui::Spacing();
							}

							auto leaftwk_row_max_y_pos = ImGui::GetItemRectMax().y;

							// Twk Areas
							ImGui::TableNextColumn();
							{
								// Input
								if (is_tweak_selected) {
									ImGui::Widget_UnorderedSetModifier("CullTwkAreas", ImGui::Widget_UnorderedSetModifierFlags_Area, tweak_selection->areas, in_twk_areas_buf, in_buflen);
								}

								ImGui::Spacing();
								ImGui::TextWrapped_IntegersFromUnorderedSet(lt.areas);
								ImGui::Spacing();
								leaftwk_row_max_y_pos = std::max(leaftwk_row_max_y_pos, ImGui::GetItemRectMax().y);
							}

							// Twk Forced Leafs
							ImGui::TableNextColumn();
							{
								// Input
								if (is_tweak_selected) {
									ImGui::Widget_UnorderedSetModifier("CullTwkForcedLeafs", ImGui::Widget_UnorderedSetModifierFlags_Leaf, tweak_selection->leafs, in_twk_force_leafs_buf, in_buflen);
								}

								ImGui::Spacing();
								ImGui::TextWrapped_IntegersFromUnorderedSet(lt.leafs);
								ImGui::Spacing();
								leaftwk_row_max_y_pos = std::max(leaftwk_row_max_y_pos, ImGui::GetItemRectMax().y);
							}

							// Twk NoCullDist
							ImGui::TableNextColumn();
							ImGui::PushID((int)cur_row);

							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
							if (ImGui::DragFloat("##NoCullDist", &lt.nocull_dist, 0.1f, 0.0f, FLT_MAX, "%.0f")) {
								lt.nocull_dist = lt.nocull_dist < 0.0f ? 0.0f : lt.nocull_dist;
							}
							TT("NoCull Distance: Can be used to override the nocull distance for distance based cullmodes.\n"
								"Priority: Leaf NoCull  >  Area NoCull  >  Global (Default) NoCull")

								// this leaf overrides the nocull distance
								any_tweak_with_nocull_override = lt.nocull_dist > 0.0f ? true : any_tweak_with_nocull_override;
							ImGui::PopID();
							leaftwk_row_max_y_pos = std::max(leaftwk_row_max_y_pos, ImGui::GetItemRectMax().y);

							// Delete Button
							ImGui::TableNextColumn();
							{
								if (is_area_selected)
								{
									ImGui::Style_DeleteButtonPush();
									ImGui::PushID((int)cur_row);

									const auto btn_size = ImVec2(16, is_tweak_selected ? (leaftwk_row_max_y_pos - tweak_table_first_row_y_pos) : 25.0f);
									if (ImGui::Button("x##twkleaf", btn_size)) {
										marked_for_deletion = &lt;
									}

									ImGui::Style_DeleteButtonPop();
									ImGui::PopID();
								}

								if (is_area_selected && !is_tweak_selected)
								{
									float content_height = ImGui::GetCursorPosY() - twk_start_y + 2.0f;
									ImGui::SetCursorPosY(twk_start_y - 1.0f);

									ImGui::Style_InvisibleSelectorPush(); // do not show highlight using selector - we use rowbg
									if (ImGui::Selectable(utils::va("##TwkSel%d", cur_row), false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, content_height))) {
										tweak_selection = &lt;
									}
									ImGui::Style_InvisibleSelectorPop();

									if (ImGui::IsItemHovered()) {
										ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0.6f)));///*ImGui::GetColorU32(ImGuiCol_TableRowBgAlt)*/);
									}
								}
							}

							cur_row++;
						}

						if (is_area_selected && !selection_matches_any_entry)
						{
							// re-check for new selection (moving towards the start of the table)
							for (auto& lt : a.leaf_tweaks)
							{
								if (tweak_selection && tweak_selection == &lt)
								{
									selection_matches_any_entry = true;
									break;
								}
							}

							if (!selection_matches_any_entry) {
								tweak_selection = nullptr;
							}
						}

						// remove entry
						if (marked_for_deletion)
						{
							for (auto it = area_selection->leaf_tweaks.begin(); it != area_selection->leaf_tweaks.end(); ++it)
							{
								if (&*it == marked_for_deletion)
								{
									area_selection->leaf_tweaks.erase(it);
									tweak_selection = nullptr;
									break;
								}
							}
						}

						// end inline table for leaf tweaks
						ImGui::EndTable();
					}
				}

				row_max_y_pos = std::max(row_max_y_pos, ImGui::GetItemRectMax().y);

				// has this area any nocull overrides?
				a.nocull_distance_overrides_in_leaf_twk = any_tweak_with_nocull_override;

				if (is_area_selected)
				{
					ImGui::Style_ColorButtonPush(imgui::get()->ImGuiCol_ButtonGreen, true);
					if (ImGui::Button("++ Tweak Leaf Entry", ImVec2(ImGui::GetContentRegionAvail().x, 28))) {
						area_selection->leaf_tweaks.emplace_back();
					}
					ImGui::Style_ColorButtonPop();
					ImGui::Spacing(0, 1.0f);
				}

				// ---
				if (!a.hide_areas.empty())
				{
					ImGui::Spacing(); 
					//ImGui::TableHeaderDropshadow(8.0f, 0.6f, 4.0f);

					// inline table for leaf tweaks
					if (ImGui::BeginTable("hide_areas_nested_table", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_ContextMenuInBody))
					{
						ImGui::TableSetupColumn("Areas", ImGuiTableColumnFlags_WidthStretch, 100.0f);
						ImGui::TableSetupColumn("NLeafs", ImGuiTableColumnFlags_WidthStretch, 73.0f);
						ImGui::TableSetupColumn("##Delete", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoClip, 16.0f);
						ImGui::TableHeadersRow();

						std::uint32_t cur_row = 0;
						bool selection_matches_any_entry = false;
						map_settings::hide_area_s* marked_for_deletion = nullptr;

						for (auto& hda : a.hide_areas)
						{
							if (is_area_selected && !hidearea_selection) {
								hidearea_selection = &hda;
							}

							ImGui::TableNextRow();

							// save Y offset because a multiline cell messes with following cells
							const auto hide_table_first_row_y_pos = ImGui::GetCursorScreenPos().y - ImGui::GetStyle().FramePadding.y + ImGui::GetStyle().CellPadding.y;
							const bool is_hda_selected = hidearea_selection && hidearea_selection == &hda;

							if (is_hda_selected) {
								ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(
									ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_TableRowBgAlt)) /*- ImVec4(0.1f, 0.1f, 0.1f, 0.0f)*/));
							}
							else {
								ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBg));
							}

							// ---
							// Areas
							ImGui::TableNextColumn();

							// save row start of selector at the end of a row
							float hda_start_y = ImGui::GetCursorPosY();
							{
								// Input
								if (is_hda_selected) {
									ImGui::Widget_UnorderedSetModifier("CullHideAreaArea", ImGui::Widget_UnorderedSetModifierFlags_Area, hidearea_selection->areas, in_hide_areas_buf, in_buflen);
								}

								ImGui::Spacing();
								ImGui::TextWrapped_IntegersFromUnorderedSet(hda.areas);
								ImGui::Spacing();
							}

							// Not in Leafs
							ImGui::TableNextColumn();
							{
								// Input
								if (is_hda_selected) {
									ImGui::Widget_UnorderedSetModifier("CullHideAreaNLeafs", ImGui::Widget_UnorderedSetModifierFlags_Leaf, hidearea_selection->when_not_in_leafs, in_hide_nleafs_buf, in_buflen);
								}

								ImGui::Spacing();
								ImGui::TextWrapped_IntegersFromUnorderedSet(hda.when_not_in_leafs);
								ImGui::Spacing();
							}

							// Delete Button
							ImGui::TableNextColumn();
							{
								if (is_area_selected)
								{
									
									ImGui::PushID((int)cur_row);
									ImGui::Style_DeleteButtonPush();

									const auto btn_size = ImVec2(16, is_hda_selected ? (ImGui::GetItemRectMax().y - hide_table_first_row_y_pos) : 25.0f);
									if (ImGui::Button("x##twkhidearea", btn_size)) {
										marked_for_deletion = &hda;
									}

									ImGui::Style_DeleteButtonPop();
									ImGui::PopID();
								}

								if (is_area_selected && !is_hda_selected)
								{
									float content_height = ImGui::GetCursorPosY() - hda_start_y + (!cur_row ? 6.0f : 4.0f);
									ImGui::SetCursorPosY(hda_start_y);

									// never show selection
									if (ImGui::Selectable(utils::va("##HdaSel%d", cur_row), false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, content_height))) {
										hidearea_selection = &hda;
									}
								}
							}

							cur_row++;
						}

						if (is_area_selected && !selection_matches_any_entry)
						{
							// re-check for new selection (moving towards the start of the table)
							for (auto& hda : a.hide_areas)
							{
								if (hidearea_selection && hidearea_selection == &hda)
								{
									selection_matches_any_entry = true;
									break;
								}
							}

							if (!selection_matches_any_entry) {
								hidearea_selection = nullptr;
							}
						}

						// remove entry
						if (marked_for_deletion)
						{
							for (auto it = area_selection->hide_areas.begin(); it != area_selection->hide_areas.end(); ++it)
							{
								if (&*it == marked_for_deletion)
								{
									area_selection->hide_areas.erase(it);
									hidearea_selection = nullptr;
									break;
								}
							}
						}

						// end inline table for leaf tweaks
						ImGui::EndTable();
					}
				}

				row_max_y_pos = std::max(row_max_y_pos, ImGui::GetItemRectMax().y);

				if (is_area_selected)
				{
					ImGui::Style_ColorButtonPush(imgui::get()->ImGuiCol_ButtonGreen, true);
					if (ImGui::Button("++ Tweak Hide Area Entry", ImVec2(ImGui::GetContentRegionAvail().x, 28))) {
						area_selection->hide_areas.emplace_back();
					}
					ImGui::Style_ColorButtonPop();
					//ImGui::Spacing(0, 7.0f); // Hack and fails if window or frame padding changes
				}
				else
				{
					ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, start_y - 2.0f));
					const float content_height = row_max_y_pos - area_table_first_row_y_pos;

					ImGuiWindow* window = ImGui::GetCurrentWindow();
					const float min_x = window->ParentWorkRect.Min.x + first_col_width;
					const float max_x = window->ParentWorkRect.Max.x;

					const auto saved_parent_work_rect_min_x = window->ParentWorkRect.Min.x;
					window->ParentWorkRect.Min.x += first_col_width;

					ImGui::Style_InvisibleSelectorPush();
					if (ImGui::Selectable(utils::va("##CullAreaSelector%d", area_num), false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(max_x - min_x, content_height))) {
						area_selection = &a;
					}
					ImGui::Style_InvisibleSelectorPop();

					if (ImGui::IsItemHovered()) {
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0.6f)));///*ImGui::GetColorU32(ImGuiCol_TableRowBgAlt)*/);
					}

					window->ParentWorkRect.Min.x = saved_parent_work_rect_min_x;
				}

				row_num++;
			} // table end for loop

			if (!area_selection_matches_any_entry)
			{
				// re-check for new selection (moving towards the start of the table)
				for (auto& [a_num, a] : areas)
				{
					if (area_selection && area_selection == &a)
					{
						area_selection_matches_any_entry = true;
						break;
					}
				}

				if (!area_selection_matches_any_entry) {
					area_selection = nullptr;
				}
			}

			ImGui::EndTable();
		} // table end

		bool was_area_removed = false;
		const auto it = areas.find(g_current_area);
		const auto can_area_be_added = it == areas.end();
		{
			ImGui::BeginDisabled(!can_area_be_added);
			ImGui::Style_ColorButtonPush(imgui::get()->ImGuiCol_ButtonGreen, true);
			if (ImGui::Button("Add Current Area##Cull", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
			{
				areas.emplace((std::uint32_t)g_current_area, map_settings::area_overrides_s{
						.cull_mode = map_settings::AREA_CULL_MODE::AREA_CULL_INFO_DEFAULT,
						.nocull_distance = map_settings::get_map_settings().default_nocull_dist,
						.area_index = (std::uint32_t)g_current_area,
					});
			}
			ImGui::Style_ColorButtonPop();
			ImGui::EndDisabled();
			ImGui::SameLine();
		}

		if (area_selection)
		{
			ImGui::Style_ColorButtonPush(imgui::get()->ImGuiCol_ButtonRed, true);
			if (ImGui::Button("X Remove Selected Area Entry##Cull", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
			{
				// if selection = the area the player is in
				if ((int)area_selection->area_index == g_current_area)
				{
					areas.erase(it);
					g_player_current_area_override = nullptr;
				}
				else {
					areas.erase(area_selection->area_index);
				}

				was_area_removed = true;
			}
			ImGui::Style_ColorButtonPop();
		}

		// resets
		if (area_selection_old != area_selection || was_area_removed)
		{
			tweak_selection = nullptr;
			hidearea_selection = nullptr;
		}

		ImGui::Spacing();
		if (ImGui::TreeNodeEx("Help", ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			ImGui::TextUnformatted(
				"# Override culling per game area\n"
				"# :: Useful console command: 'xo_debug_toggle_node_vis'\n"
				"# ~~ Parameters:\n"
				"#\n"
				"# in_area:          the area the player has to be in                [int]\n"
				"# areas:            area/s with forced visibility                   [int array]\n"
				"# leafs:            leaf/s with forced visibility                   [int array]\n"
				"#\n"
				"# cull:             [0] disable frustum culling                     [int 0-5]\n"
				"#                   [1] disable frustum culling in current area\n"
				"#                   [2] stock\n"
				"#                   [3] frustum culling (outside current area) + force all nodes/leafs in current area\n"
				"#                   [4] ^ + outside of current area within certain dist to player (param: nocull_dist)\n"
				"#                   [5] force all leafs/nodes within certain dist to player (param: nocull_dist) << default\n"
				"#\n"
				"# nocull_dist:      |> Distance around the player where objects wont get culled - only used on certain cull modes [float] << defaults to 600.0\n"
				"#\n"
				"# -----------       :: This can be used to disable frustum culling for specified areas when the player is in specified leafs\n"
				"#                   :: Useful at area crossings when used in conjunction with nocull - area-specific markers that block visibility\n"
				"# leaf_tweak:																					[array of structure below]\n"
				"#                   |>    in_leafs:     the leaf/s the player has to be in                     [int array]\n"
				"#                   |>       areas:     area/s with forced visibility                          [int array]\n"
				"#                   |>       leafs:     leaf/s with forced visibility                          [int array]\n"
				"#                   |> nocull_dist:     uses per leaf value instead of area value if defined	[float]	<< defaults to 0.0 (off)\n"
				"#\n"
				"# -----------       :: This can be used to forcefully cull parts of the map\n"
				"# hide_areas :																					[array of structure below]\n"
				"#                   |>    areas:   area/s to hide												[int array]\n"
				"#                   |>  N_leafs:   only hide area/s when NOT in leaf/s							[int array]\n"
				"#\n"
				"# hide_leafs :		force hide leaf/s															[int array]\n");

			ImGui::TreePop();
		}
	}

	bool check_light_for_modifications(const map_settings::remix_light_settings_s& edit_def, const map_settings::remix_light_settings_s& map_def, std::vector<map_settings::remix_light_settings_s::point_s>* mover_pts)
	{
		const auto& pt = mover_pts && !mover_pts->empty() ? *mover_pts : edit_def.points;

		if (edit_def.run_once != map_def.run_once) { return true; }
		if (edit_def.loop != map_def.loop) { return true; }
		if (edit_def.loop_smoothing != map_def.loop_smoothing) { return true; }
		if (edit_def.trigger_always != map_def.trigger_always) { return true; }

		if (edit_def.trigger_choreo_name != map_def.trigger_choreo_name) { return true; }
		if (edit_def.trigger_choreo_actor != map_def.trigger_choreo_actor) { return true; }
		if (edit_def.trigger_choreo_event != map_def.trigger_choreo_event) { return true; }
		if (edit_def.trigger_choreo_param1 != map_def.trigger_choreo_param1) { return true; }
		if (edit_def.trigger_sound_hash != map_def.trigger_sound_hash) { return true; }
		if (!utils::float_equal(edit_def.trigger_delay, map_def.trigger_delay)) { return true; }

		if (edit_def.kill_choreo_name != map_def.kill_choreo_name) { return true; }
		if (edit_def.kill_sound_hash != map_def.kill_sound_hash) { return true; }
		if (!utils::float_equal(edit_def.kill_delay, map_def.kill_delay)) { return true; }

		if (!utils::float_equal(edit_def.attach_prop_radius, map_def.attach_prop_radius)) { return true; }
		if (edit_def.attach_prop_mins != map_def.attach_prop_mins) { return true; }
		if (edit_def.attach_prop_maxs != map_def.attach_prop_maxs) { return true; }
		if (edit_def.attach_prop_name != map_def.attach_prop_name) { return true; }

		if (edit_def.comment != map_def.comment) { return true; }

		if (pt.size() != map_def.points.size()) { return true; }

		for (size_t i = 0u; i < pt.size(); i++)
		{
			const auto& edit_p = pt[i];
			const auto& map_p = map_def.points[i];

			if (edit_p.position != map_p.position) { return true; }
			if (edit_p.radiance != map_p.radiance) { return true; }
			if (!utils::float_equal(edit_p.radiance_scalar, map_p.radiance_scalar)) { return true; }
			if (!utils::float_equal(edit_p.radius, map_p.radius)) { return true; }

			// never check the very first timepoint
			// could also re-calculate timepoints to check if there is a mismatch but we are writing all timepoints for now
			if (!i)
			{
				if (!utils::float_equal(edit_p.timepoint, map_p.timepoint)) {
					return true;
				}
			}

			if (!utils::float_equal(edit_p.smoothness, map_p.smoothness)) { return true; }

			if (edit_p.use_shaping != map_p.use_shaping) { return true; }
			if (edit_p.direction != map_p.direction) { return true; }
			if (!utils::float_equal(edit_p.degrees, map_p.degrees)) { return true; }
			if (!utils::float_equal(edit_p.softness, map_p.softness)) { return true; }
			if (!utils::float_equal(edit_p.exponent, map_p.exponent)) { return true; }
		}

		return false;
	}

	void mapsettings_ls_general_light_settings(remix_lights::remix_light_s* edit_active_light)
	{
		const auto im = imgui::get();
		const auto cont_bg_color = im->ImGuiCol_ContainerBackground + ImVec4(0.05f, 0.05f, 0.05f, 0.0f);

		ImGui::Spacing(0, 12);
		ImGui::PushFont(common::imgui::font::BOLD_LARGE);
		ImGui::SeparatorText(" General Light Settings ");
		ImGui::PopFont();
		ImGui::Spacing(0, 4);

		static float cont_height = 0.0f;
		cont_height = ImGui::Widget_ContainerWithDropdownShadow(cont_height, [edit_active_light]
			{
				ImGui::BeginDisabled(!edit_active_light->mover.is_initialized());
				ImGui::Checkbox("Run Once", &edit_active_light->def.run_once);
				TT("Enabled: Destroy light after reaching the last point");

				ImGui::SameLine(ImGui::GetContentRegionAvail().x * 0.33f, 0);
				ImGui::Checkbox("Loop", &edit_active_light->def.loop);
				TT("Enabled: Looping light that restarts at the first point after reaching the last point.\n"
					"Disabled: Light will stop and stay active when reaching the last point.\n"
					"This does not make a difference when in edit mode.");

				ImGui::SameLine(ImGui::GetContentRegionAvail().x * 0.66f, 0);
				if (ImGui::Checkbox("Loop Smoothing", &edit_active_light->def.loop_smoothing)) {
					edit_active_light->mover.init(edit_active_light->mover.get_points_vec(), true, edit_active_light->def.loop_smoothing);
				}
				TT("Enabled: Automatically connect and smooth the start and end point.\n"
					"[!] requires 'loop' to be true\n"
					"[!] only position + timepoint is used from the last point");

				ImGui::EndDisabled();
				ImGui::Spacing(0, 6);

				//ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 80.0f);
				//ImGui::TextUnformatted(" Comment ");
				//ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
				ImGui::InputText("Comment", &edit_active_light->def.comment);


				ImGui::Spacing(0, 12);

				ImGuiWindow* window = ImGui::GetCurrentWindow();
				const auto s_workrect_max_x = window->WorkRect.Max.x;
				window->WorkRect.Max.x -= (ImGui::GetStyle().WindowPadding.x * 3.0f);

				
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

				ImGui::TableHeaderDropshadow(12.0f, 0.6f, 0.0f, window->WorkRect.Max.x - window->DC.CursorPos.x);

				const auto spos_pre_trigger_header = ImGui::GetCursorScreenPos();
				const auto triggersettings_state = ImGui::CollapsingHeader("Trigger Settings");

				if (edit_active_light->has_spawn_trigger() || edit_active_light->has_kill_trigger())
				{
					const auto spos_post_header = ImGui::GetCursorScreenPos();
					const auto header_dims = ImGui::GetItemRectSize();
					const auto icon_dims = ImGui::CalcTextSize(ICON_FA_CHECK);
					ImGui::SetCursorScreenPos(spos_pre_trigger_header + ImVec2(header_dims.x - icon_dims.x - ImGui::GetStyle().WindowPadding.x - 8.0f, header_dims.y * 0.5f - icon_dims.y * 0.5f));
					ImGui::TextUnformatted(ICON_FA_CHECK);
					ImGui::SetCursorScreenPos(spos_post_header);
				}

				ImGui::PopStyleVar(); // FrameRounding

				if (triggersettings_state)
				{
					SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
					ImGui::InputText("Choreo Name##Trigger", &edit_active_light->def.trigger_choreo_name);
					TT("Trigger light creation when a specified choreography (vcd) starts playing.\n"
						"The choreo trigger has HIGHER precedence over sound triggering.\n"
						"This can be a substring. Use cmd 'xo_debug_scene_print' to get info about playing choreo's.");

					// clear sound trigger if choreo is not empty
					if (!edit_active_light->def.trigger_choreo_name.empty()) {
						edit_active_light->def.trigger_sound_hash = 0u;
					}

					if (!edit_active_light->def.trigger_choreo_name.empty())
					{
						SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
						ImGui::InputText("Choreo Actor##Trigger", &edit_active_light->def.trigger_choreo_actor);
						TT("Use this if the choreo name isn't enough to uniquely identify the choreo that should trigger light creation.\n"
							"This can be a substring. Use cmd 'xo_debug_scene_print' to get info about playing choreo's.");

						SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
						ImGui::InputText("Choreo Event##Trigger", &edit_active_light->def.trigger_choreo_event);
						TT("Use this if the choreo name isn't enough to uniquely identify the choreo that should trigger light creation.\n"
							"This can be a substring. Use cmd 'xo_debug_scene_print' to get info about playing choreo's.");

						SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
						ImGui::InputText("Choreo Param1##Trigger", &edit_active_light->def.trigger_choreo_param1);
						TT("Use this if the choreo name isn't enough to uniquely identify the choreo that should trigger light creation.\n"
							"This can be a substring. Use cmd 'xo_debug_scene_print' to get info about playing choreo's.");
					}

					// --- sound

					SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
					std::string temp_sound_hash_str = edit_active_light->def.trigger_sound_hash ? std::format("0x{:X}", edit_active_light->def.trigger_sound_hash) : "";

					if (ImGui::InputText("Sound Hash##Trigger", &temp_sound_hash_str, ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_EnterReturnsTrue,
						[](ImGuiInputTextCallbackData* data)
						{
							const auto c = static_cast<char>(data->EventChar);
							if (std::isxdigit(c) || c == 'x' || c == 'X') {
								return 0; // allow input
							}
							return 1; // block input
						}))
					{
						edit_active_light->def.trigger_sound_hash = static_cast<uint32_t>(std::strtoul(temp_sound_hash_str.c_str(), nullptr, 16));
						temp_sound_hash_str = std::format("0x{:X}", edit_active_light->def.trigger_sound_hash);
					}
					TT("Trigger light creation when a specified sound starts playing.\n"
						"The sound trigger has LOWER precedence over choreo triggering.\n"
						"Use cmd 'xo_debug_toggle_sound_print' to get info about playing sounds.");

					ImGui::BeginDisabled(!edit_active_light->has_spawn_trigger());
					{
						SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
						if (ImGui::DragFloat("Delay##Trigger", &edit_active_light->def.trigger_delay, 0.05f, 0.0f)) {
							edit_active_light->def.trigger_delay = edit_active_light->def.trigger_delay < 0.0f ? 0.0f : edit_active_light->def.trigger_delay;
						} TT("Delay spawn after trigger in seconds.");

						ImGui::Checkbox("Always", &edit_active_light->def.trigger_always);
						TT("Retriggering the event again will spawn a new light instance everytime.");
					}
					ImGui::EndDisabled();

					// clear choreo trigger if sound hash is not empty
					if (edit_active_light->def.trigger_sound_hash)
					{
						edit_active_light->def.trigger_choreo_name.clear();
						edit_active_light->def.trigger_choreo_actor.clear();
						edit_active_light->def.trigger_choreo_event.clear();
						edit_active_light->def.trigger_choreo_param1.clear();
					}

					// --
					// kill choreo


					ImGui::Spacing(0, 12);
					ImGui::TextUnformatted(" Kill Settings ");

					SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
					ImGui::InputText("Choreo Name##Kill", &edit_active_light->def.kill_choreo_name);
					TT("Trigger light deletion when a specified choreography (vcd) starts playing.\n"
						"The choreo trigger has HIGHER precedence over sound triggering.\n"
						"This can be a substring. Use cmd 'xo_debug_scene_print' to get info about playing choreo's.");

					// clear sound trigger if choreo is not empty
					if (!edit_active_light->def.kill_choreo_name.empty()) {
						edit_active_light->def.kill_sound_hash = 0u;
					}

					// kill sound

					SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
					std::string temp_kill_sound_hash_str = edit_active_light->def.kill_sound_hash ? std::format("0x{:X}", edit_active_light->def.kill_sound_hash) : "";

					if (ImGui::InputText("Sound Hash##Kill", &temp_kill_sound_hash_str, ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_EnterReturnsTrue,
						[](ImGuiInputTextCallbackData* data)
						{
							const auto c = static_cast<char>(data->EventChar);
							if (std::isxdigit(c) || c == 'x' || c == 'X') {
								return 0; // allow input
							}
							return 1; // block input
						}))
					{
						edit_active_light->def.kill_sound_hash = static_cast<uint32_t>(std::strtoul(temp_kill_sound_hash_str.c_str(), nullptr, 16));
						temp_kill_sound_hash_str = std::format("0x{:X}", edit_active_light->def.kill_sound_hash);
					}
					TT( "Trigger light creation when a specified sound starts playing.\n"
						"The sound trigger has LOWER precedence over choreo triggering.\n"
						"Use cmd 'xo_debug_toggle_sound_print' to get info about playing sounds.");

					ImGui::BeginDisabled(!edit_active_light->has_kill_trigger());
					{
						SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
						if (ImGui::DragFloat("Delay##Kill", &edit_active_light->def.kill_delay, 0.05f, 0.0f)) {
							edit_active_light->def.kill_delay = edit_active_light->def.kill_delay < 0.0f ? 0.0f : edit_active_light->def.kill_delay;
						} TT("Delay kill after kill trigger in seconds.");
					}
					ImGui::EndDisabled();
					
					// clear choreo kill trigger if sound hash is not empty
					if (edit_active_light->def.kill_sound_hash) {
						edit_active_light->def.kill_choreo_name.clear();
					}
				}

				ImGui::Spacing();
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

				ImGui::TableHeaderDropshadow(12.0f, 0.6f, 0.0f, window->WorkRect.Max.x - window->DC.CursorPos.x);

				const auto spos_pre_attach_header = ImGui::GetCursorScreenPos();
				const auto attachprop_settings_state = ImGui::CollapsingHeader("Attach to Prop");

				if (edit_active_light->has_attach_parms())
				{
					const auto spos_post_header = ImGui::GetCursorScreenPos();
					const auto header_dims = ImGui::GetItemRectSize();
					const auto icon_dims = ImGui::CalcTextSize(ICON_FA_CHECK);
					ImGui::SetCursorScreenPos(spos_pre_attach_header + ImVec2(header_dims.x - icon_dims.x - ImGui::GetStyle().WindowPadding.x - 8.0f, header_dims.y * 0.5f - icon_dims.y * 0.5f));
					ImGui::TextUnformatted(ICON_FA_CHECK);
					ImGui::SetCursorScreenPos(spos_post_header);
				}


				ImGui::PopStyleVar(); // FrameRounding

				if (attachprop_settings_state)
				{
					SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
					if (ImGui::DragFloat("Prop Radius##Attach", &edit_active_light->def.attach_prop_radius, 0.05f, 0.0f)) 
					{
						edit_active_light->def.attach_prop_radius = edit_active_light->def.attach_prop_radius < 0.0f ? 0.0f : edit_active_light->def.attach_prop_radius;
						if (edit_active_light->def.attach_prop_radius > 0.0f) {
							edit_active_light->def.attach_prop_name.clear();
						}
					}
					TT( "Attach light to a prop with this radius. This + bounds is the recommended way!\n"
						"Use cmd 'xo_debug_toggle_model_info' to get info about nearby props.");

					SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
					if (ImGui::InputText("Prop Name##Attach", &edit_active_light->def.attach_prop_name))
					{
						if (!edit_active_light->def.attach_prop_name.empty()) {
							edit_active_light->def.attach_prop_radius = 0.0f;
						}
					}
					TT( "Attach light to a prop that contains this string within its name.\n"
						"This is slower then using radius + bounds so be aware of that.\n"
						"Use cmd 'xo_debug_toggle_model_info' to get info about nearby props.");

					ImGui::BeginDisabled(!edit_active_light->has_attach_parms());
					ImGui::Widget_PrettyDragVec3("Bounds Min", &edit_active_light->def.attach_prop_mins.x, true, 120.0f, 0.05f);
					ImGui::Widget_PrettyDragVec3("Bounds Max", &edit_active_light->def.attach_prop_maxs.x, true, 120.0f, 0.05f);
					ImGui::EndDisabled();

					ImGui::Spacing(0, 4);

					ImGui::PushFont(common::imgui::font::BOLD);
					ImGui::Indent(4);
					ImGui::Text("Attach Status: %s",
						edit_active_light->has_attach_parms() && edit_active_light->is_attached() ? "Attached." :
						edit_active_light->has_attach_parms() ? "Found no fitting prop to attach to." : "No attach parameters defined.");
					ImGui::Unindent(4);
					ImGui::PopFont();
				}

				window->WorkRect.Max.x = s_workrect_max_x;
				ImGui::Spacing(0, 4);

			}, &cont_bg_color, &im->ImGuiCol_ContainerBorder);

		// debug vis

		if (im->m_debugvis_attach_bounds && edit_active_light->has_attach_parms())
		{
			const auto remixapi = remix_api::get();
			remixapi->debug_draw_box(edit_active_light->def.attach_prop_mins, edit_active_light->def.attach_prop_maxs, 1.0f, 
				edit_active_light->is_attached() ? remix_api::DEBUG_REMIX_LINE_COLOR::WHITE : remix_api::DEBUG_REMIX_LINE_COLOR::RED);
		}
	}

	void mapsettings_ls_playback_visualization_settings(remix_lights::remix_light_s* edit_active_light, const bool is_static_light_with_single_point)
	{
		const auto im = imgui::get();
		const auto cont_bg_color = im->ImGuiCol_ContainerBackground + ImVec4(0.05f, 0.05f, 0.05f, 0.0f);

		ImGui::Spacing(0, 12);
		ImGui::PushFont(common::imgui::font::BOLD_LARGE);
		ImGui::SeparatorText(" Playback / Visualization Settings ");
		ImGui::PopFont();
		ImGui::Spacing(0, 4);

		static float cont_height = 0.0f;
		cont_height = ImGui::Widget_ContainerWithDropdownShadow(cont_height, [edit_active_light, is_static_light_with_single_point]
			{
				const auto im = imgui::get();
				ImGui::Checkbox("Live Vis.", &im->m_debugvis_live); TT("Enable live visualizations instead of static per point visualizations.");
				ImGui::SameLine(ImGui::GetContentRegionAvail().x * 0.49f, 0);
				ImGui::Checkbox("Vis. Light Radius", &im->m_debugvis_radius); TT("Show light radius visualizations.");

				ImGui::Checkbox("Vis. Light Shaping", &im->m_debugvis_shaping); TT("Show light shaping visualizations.");
				ImGui::SameLine(ImGui::GetContentRegionAvail().x * 0.49f, 0);
				ImGui::Checkbox("Vis. Attach Bounds", &im->m_debugvis_attach_bounds); TT("Show bounding box visualization used by light attachment logic.");

				ImGui::SetNextItemWidth(100.0f);
				if (ImGui::InputInt("##Shaping Cone Steps", &im->m_debugvis_cone_steps, 1, 2, ImGuiInputTextFlags_CharsDecimal)) {
					im->m_debugvis_cone_steps = std::clamp(im->m_debugvis_cone_steps, 0, 100);
				} TT("Amount of light shaping cone steps used for debug visualizations.");

				ImGui::SameLine(ImGui::GetContentRegionAvail().x * 0.49f, 0);
				ImGui::SetNextItemWidth(100.0f);
				ImGui::DragFloat("Shaping Cone Height", &im->m_debugvis_cone_height); TT("Height of light shaping cone (debug visualizations).");

				ImGui::Spacing(0, 6);

				ImGui::BeginDisabled(!edit_active_light->mover.is_initialized());
				ImGui::BeginDisabled(is_static_light_with_single_point);
				if (ImGui::Button("Restart Light Loop", ImVec2(ImGui::GetContentRegionAvail().x * 0.49f - ImGui::GetStyle().ItemSpacing.x, 0))) {
					edit_active_light->mover.restart();
				} TT("This resets the current loop to timepoint = 0");
				ImGui::EndDisabled();

				ImGui::SameLine(ImGui::GetContentRegionAvail().x * 0.49f);
				if (ImGui::Button("Evenly distribute all timepoints", ImVec2(ImGui::GetContentRegionAvail().x * 0.98f, 0)))
				{
					auto& pts = edit_active_light->mover.get_points_vec();
					// clear timepoints for all but the very first & very last points:
					for (size_t i = 1u; i < pts.size() - 1u; i++) {
						pts[i].timepoint = 0.0f;
					}

					edit_active_light->mover.init(pts, true, edit_active_light->def.loop_smoothing);
				} TT("This will clear and recalculate the timepoints of all but the last point to evenly distribute time across all point 2 point segments.");
				ImGui::EndDisabled();

			}, &cont_bg_color, &im->ImGuiCol_ContainerBorder);
	}

	void cont_mapsettings_light_spawning()
	{
		const auto im = imgui::get();
		const auto lights = remix_lights::get();

		static map_settings::remix_light_settings_s* ms_light_selection = nullptr;
		static map_settings::remix_light_settings_s* ms_light_selection_pending = nullptr;

		// this will handle the reload popup as long as we are not in edit mode
		if (!im->m_light_edit_mode) 
		{
			ms_light_selection = nullptr;
			ms_light_selection_pending = nullptr;

			if (ImGui::Button("Enable Light Edit Mode", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
			{
				// this uses the reload mapsettings "button" function below
				if (!ImGui::IsPopupOpen("Reload MapSettings?")) {
					ImGui::OpenPopup("Reload MapSettings?");
				}
			} TT("This will enable you to edit lights. However, reloading the mapsettings is required.");

			// code duplication because we need to set im->m_light_edit_mode to true before reloading
			if (ImGui::BeginPopupModal("Reload MapSettings?", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
			{
				common::imgui::draw_background_blur();
				const auto half_width = ImGui::GetContentRegionMax().x * 0.5f;
				auto line1_str = "You'll loose all unsaved changes if you continue!";
				auto line2_str = "Use the copy to clipboard buttons and manually update  ";
				auto line3_str = "the map_settings.toml file if you've made changes.";

				ImGui::Spacing();
				ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line1_str).x * 0.5f));
				ImGui::TextUnformatted(line1_str);

				ImGui::Spacing();
				ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line2_str).x * 0.5f));
				ImGui::TextUnformatted(line2_str);
				ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line3_str).x * 0.5f));
				ImGui::TextUnformatted(line3_str);

				ImGui::Spacing(0, 8);
				ImGui::Spacing(0, 0); ImGui::SameLine();

				ImVec2 button_size(half_width - 6.0f - ImGui::GetStyle().WindowPadding.x, 0.0f);
				if (ImGui::Button("Reload", button_size))
				{
					im->m_light_edit_mode = true;
					map_settings::reload();
					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine(0, 6);
				if (ImGui::Button("Cancel", button_size)) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			return;
		}

		auto& ms_lights = map_settings::get_map_settings().remix_lights;
		ImGui::PushFont(common::imgui::font::BOLD);
		if (ImGui::Button("Copy Selected Light to Clipboard   " ICON_FA_SAVE, ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
		{
			if (const auto edit_light = lights->get_first_active_light(); edit_light)
			{
				auto temp_def = edit_light->def;
				if (edit_light->mover.is_initialized()) {
					temp_def.points = edit_light->mover.get_points_vec();
				}

				ImGui::LogToClipboard();
				ImGui::LogText("%s", common::toml::build_light_string_for_single_light(temp_def).c_str());
				ImGui::LogFinish();
			}
		} ImGui::PopFont();

		ImGui::SameLine();
		reload_mapsettings_button_with_popup("RemixLights");

		

		// default selection (when table not visible and selection empty)
		if (!ms_light_selection && !ms_lights.empty()) 
		{
			ms_light_selection = &ms_lights.front();
			lights->destroy_and_clear_all_active_lights();
		}

		// point table helper - true when the user switched to a different light
		bool reset_point_selection = false;

		//
		// LIGHT TABLE

		ImGui::Spacing(0, 16);
		ImGui::PushFont(common::imgui::font::BOLD_LARGE);
		ImGui::SeparatorText(" Light Selection ");
		ImGui::PopFont();

		ImGui::TableHeaderDropshadow();

		if (ImGui::BeginTable("LightTable", 12,
			ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody |
			ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_ScrollY, ImVec2(0, 134.0f)))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // make top row always visible
			ImGui::TableSetupColumn("##num", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 10.0f);
			ImGui::TableSetupColumn("Comment", ImGuiTableColumnFlags_WidthStretch, 150.0f);
			ImGui::TableSetupColumn("Trig Choreo", ImGuiTableColumnFlags_WidthStretch, 80.0f);
			ImGui::TableSetupColumn("Trig Sound", ImGuiTableColumnFlags_WidthStretch, 10.0f);
			ImGui::TableSetupColumn("Delay Trig", ImGuiTableColumnFlags_WidthStretch, 16.0f);
			ImGui::TableSetupColumn("Always", ImGuiTableColumnFlags_WidthStretch, 10.0f);

			ImGui::TableSetupColumn("Kill Choreo", ImGuiTableColumnFlags_WidthStretch, 80.0f);
			ImGui::TableSetupColumn("Kill Sound", ImGuiTableColumnFlags_WidthStretch, 10.0f);
			ImGui::TableSetupColumn("Delay Kill", ImGuiTableColumnFlags_WidthStretch, 16.0f);

			ImGui::TableSetupColumn("Once", ImGuiTableColumnFlags_WidthStretch, 10.0f);
			ImGui::TableSetupColumn("Loop", ImGuiTableColumnFlags_WidthStretch, 10.0f);
			ImGui::TableSetupColumn("Smooth", ImGuiTableColumnFlags_WidthStretch, 10.0f);
			ImGui::TableHeadersRow();

			bool selection_matches_any_entry = false;
			for (auto i = 0u; i < ms_lights.size(); i++)
			{
				auto& l = ms_lights[i];

				const bool is_selected = ms_light_selection && ms_light_selection == &l;

				ImGui::TableNextRow();

				if (is_selected) { // handle row background color for selected entry
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
				}

				// -
				ImGui::TableNextColumn();
				if (!is_selected) // only selectable if not selected
				{ 
					ImGui::Style_InvisibleSelectorPush(); // never show selection - we use tablebg
					if (ImGui::Selectable(utils::va("%d", i), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) 
					{
						if (const auto edit_light = lights->get_first_active_light(); edit_light)
						{
							if (check_light_for_modifications(edit_light->def, *ms_light_selection, &edit_light->mover.get_points_vec()))
							{
								if (!ImGui::IsPopupOpen("Ignore Changes?", ImGuiPopupFlags_AnyPopup)) {
									ImGui::OpenPopup("Ignore Changes?", ImGuiPopupFlags_AnyPopup);
								}

								ms_light_selection_pending = &l;
							}

							// no modifications on old light -> select new one
							else {
								ms_light_selection = &l;
								ms_light_selection_pending = nullptr;
							}
						}
						else {
							ms_light_selection = &l;
						}
					}
					ImGui::Style_InvisibleSelectorPop();

					if (ImGui::IsItemHovered()) {
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0.6f)));///*ImGui::GetColorU32(ImGuiCol_TableRowBgAlt)*/);
					}
				}
				else {
					ImGui::Text("%d", i); // if selected
				}

				// check if there is at least one valid selection
				if (ms_light_selection && ms_light_selection == &l) 
				{
					// user selected a new light
					if (!is_selected) 
					{
						lights->destroy_and_clear_all_active_lights();
						reset_point_selection = true;
						selection_matches_any_entry = false;
					}
					else
					{
						selection_matches_any_entry = true;
					}
				}

				// comment
				ImGui::TableNextColumn();
				ImGui::TextUnformatted_ClippedByColumnTooltip(l.comment.c_str());
				
				// trig choreo
				ImGui::TableNextColumn();
				ImGui::TextUnformatted_ClippedByColumnTooltip(l.trigger_choreo_name.c_str());

				// trig sound
				ImGui::TableNextColumn();
				ImGui::Text("%s", l.trigger_sound_hash ? "x" : ""); //ImGui::Text("%#x", l.trigger_sound_hash);

				// trig delay
				ImGui::TableNextColumn();
				ImGui::Text("%.1f", l.trigger_delay);

				// trig always
				ImGui::TableNextColumn();
				ImGui::Text("%s", l.trigger_always ? "x" : "");

				// kill choreo
				ImGui::TableNextColumn();
				ImGui::TextUnformatted_ClippedByColumnTooltip(l.kill_choreo_name.c_str());

				// kill sound
				ImGui::TableNextColumn();
				ImGui::Text("%s", l.kill_sound_hash ? "x" : ""); //ImGui::Text("%#x", l.kill_sound_hash);

				// kill delay
				ImGui::TableNextColumn();
				ImGui::Text("%.1f", l.kill_delay);

				// once
				ImGui::TableNextColumn();
				ImGui::Text("%s", l.run_once ? "x" : "");

				// loop
				ImGui::TableNextColumn();
				ImGui::Text("%s", l.loop ? "x" : "");

				// smooth
				ImGui::TableNextColumn();
				ImGui::Text("%s", l.loop_smoothing ? "x" : "");
			} // end for loop

			// no valid selection found in the last loop
			// re-check because user might have selected something new using the selectable 
			if (!selection_matches_any_entry)
			{
				for (auto& l : ms_lights)
				{
					if (ms_light_selection && ms_light_selection == &l)
					{
						selection_matches_any_entry = true;
						break;
					}
				}

				// reset selection ptr if no valid selection was found
				if (!selection_matches_any_entry)
				{
					if (!ms_lights.empty()) {
						ms_light_selection = &ms_lights.front();
					}
					else {
						ms_light_selection = nullptr;
					}
				}
			}

			ImGui::EndTable();
		} // table end



		if (ms_light_selection) 
		{
			// create light
			if (!lights->get_active_light_count()) {
				lights->add_single_map_setting_light_for_editing(ms_light_selection);
			}

			if (auto edit_light = lights->get_first_active_light(); 
				edit_light)
			{
				// debug text
				Vector lpos = &edit_light->ext.position.x;
				Vector screen_pos; common::imgui::world2screen(lpos, screen_pos);
				ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(screen_pos.x, screen_pos.y), 4.0f, ImGui::GetColorU32(ImGuiCol_Text));
				game::debug_add_text_overlay(&lpos.x, "  Selected Light", -1, 0.8f, 0.8f, 0.8f, 0.8f);
			}
		}


		ImGui::Spacing(0, 0);
		ImGui::Style_ColorButtonPush(im->ImGuiCol_ButtonGreen, true);
		if (ImGui::Button("Add Light", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
		{
			// write modified params into the current mapsetting light so changes dont get lost
			if (auto edit_light = lights->get_first_active_light(); edit_light)
			{
				auto temp_def = edit_light->def;
				if (edit_light->mover.is_initialized()) {
					temp_def.points = edit_light->mover.get_points_vec();
				}
				*ms_light_selection = temp_def;
			}

			lights->destroy_and_clear_all_active_lights();

			map_settings::remix_light_settings_s::point_s pt =
			{
				.position = *game::get_current_view_origin(),
				.radiance = { 10.0f, 10.0f, 10.0f },
				.radiance_scalar = 1.0f,
				.radius = 1.0f,
				.timepoint = 0.0f,
				.smoothness = 0.5f,
				.use_shaping = false,
			};

			ms_lights.push_back(
				map_settings::remix_light_settings_s
				{
					.points = { pt },
					.run_once = false,
					.loop = true,
					.loop_smoothing = false,
					.trigger_always = false,
					.trigger_choreo_name = "",
					.trigger_choreo_actor = "",
					.trigger_choreo_event = "",
					.trigger_choreo_param1 = "",
					.trigger_sound_hash = 0u,
					.trigger_delay = 0.0f,
					.kill_choreo_name = "",
					.kill_sound_hash = 0u,
					.kill_delay = 0.0f
				});

			ms_light_selection = &ms_lights.back();
			reset_point_selection = true;

			if (!lights->get_active_light_count()) {
				lights->add_single_map_setting_light_for_editing(ms_light_selection);
			}
		}
		ImGui::Style_ColorButtonPop();

		ImGui::BeginDisabled(!ms_light_selection);
		{
			ImGui::SameLine();
			ImGui::Style_ColorButtonPush(im->ImGuiCol_ButtonRed, true);
			if (ImGui::Button("Delete Selected Light", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
			{
				for (auto it = ms_lights.begin(); it != ms_lights.end(); ++it)
				{
					if (&*it == ms_light_selection)
					{
						lights->destroy_and_clear_all_active_lights();
						ms_lights.erase(it);
						reset_point_selection = true;

						if (!ms_lights.empty()) 
						{
							ms_light_selection = &ms_lights.front();
							if (!lights->get_active_light_count()) {
								lights->add_single_map_setting_light_for_editing(ms_light_selection);
							}
						}
						else {
							ms_light_selection = nullptr;
						}

						break;
					}
				}
			}
			ImGui::Style_ColorButtonPop();

			//ImGui::Style_ColorButtonPush(imgui::get()->ImGuiCol_ButtonYellow, true);
			if (ImGui::Button("Duplicate Selected Light", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
			{
				// write modified params into the current mapsetting light so changes dont get lost
				if (auto edit_light = lights->get_first_active_light(); edit_light)
				{
					auto temp_def = edit_light->def;
					if (edit_light->mover.is_initialized()) {
						temp_def.points = edit_light->mover.get_points_vec();
					}
					*ms_light_selection = temp_def;
				}

				ms_lights.push_back(*ms_light_selection);
				lights->destroy_and_clear_all_active_lights();

				ms_light_selection = &ms_lights.back();
				reset_point_selection = true;

				if (!lights->get_active_light_count()) {
					lights->add_single_map_setting_light_for_editing(ms_light_selection);
				}
			} TT("Duplicates the currently selected light.")
			//ImGui::Style_ColorButtonPop();

			ImGui::SameLine();
			if (ImGui::Button("TP to Light", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
				interfaces::get()->m_engine->execute_client_cmd_unrestricted(utils::va("sv_cheats 1; noclip; setpos %.2f %.2f %.2f", ms_light_selection->points[0].position.x, ms_light_selection->points[0].position.y, ms_light_selection->points[0].position.z - 40.0f));
			}

			ImGui::EndDisabled();
		}


		if (auto edit_active_light = lights->get_first_active_light(); 
			edit_active_light && ms_light_selection)
		{
			const auto is_static_light_with_single_point = !edit_active_light->mover.is_initialized();
			// ---
			// holds mover points OR def point if light is static
			static map_settings::remix_light_settings_s::point_s* active_point_selection = nullptr;

			map_settings::remix_light_settings_s::point_s* active_points = nullptr;
			size_t active_points_count = 0u;

			// if light only has a single point, mover wont be initialized
			if (is_static_light_with_single_point && !edit_active_light->def.points.empty())
			{
				active_points = edit_active_light->def.points.data();
				active_points_count = 1u;
				active_point_selection = active_points;
			}
			else if (edit_active_light->mover.is_initialized())
			{
				active_points = edit_active_light->mover.get_points();
				active_points_count = edit_active_light->mover.get_points_count();
			}

			if (reset_point_selection || !active_point_selection && active_points_count > 0u) {
				active_point_selection = &active_points[0]; // default selection
			}

			ImGui::Spacing(0, 16);
			ImGui::PushFont(common::imgui::font::BOLD_LARGE);
			ImGui::SeparatorText(" Light Points ");
			ImGui::PopFont();

			ImGui::TableHeaderDropshadow();
			
			if (ImGui::BeginTable("PointTable", 12,
				ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody |
				ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_ScrollY, ImVec2(0, 134.0f)))
			{
				ImGui::TableSetupScrollFreeze(0, 1); // make top row always visible
				ImGui::TableSetupScrollFreeze(0, 1); // make top row always visible
				ImGui::TableSetupColumn("##num", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 10.0f);
				ImGui::TableSetupColumn("Timepoint", ImGuiTableColumnFlags_WidthStretch, 30.0f);
				ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthStretch, 100.0f);
				ImGui::TableSetupColumn("Radiance", ImGuiTableColumnFlags_WidthStretch, 60.0f);
				ImGui::TableSetupColumn("Scalar", ImGuiTableColumnFlags_WidthStretch, 30.0f);
				ImGui::TableSetupColumn("Radius", ImGuiTableColumnFlags_WidthStretch, 30.0f);
				ImGui::TableSetupColumn("Smooth", ImGuiTableColumnFlags_WidthStretch, 30.0f);
				ImGui::TableSetupColumn("Shaping", ImGuiTableColumnFlags_WidthStretch, 14.0f);
				ImGui::TableSetupColumn("Direction", ImGuiTableColumnFlags_WidthStretch, 70.0f);
				ImGui::TableSetupColumn("Degrees", ImGuiTableColumnFlags_WidthStretch, 30.0f);
				ImGui::TableSetupColumn("Soft", ImGuiTableColumnFlags_WidthStretch, 30.0f);
				ImGui::TableSetupColumn("Expo", ImGuiTableColumnFlags_WidthStretch, 30.0f);
				ImGui::TableHeadersRow();

				bool selection_matches_any_entry = false;
				for (size_t i = 0u; i < active_points_count; i++)
				{
					auto& p = active_points[i];

					// default selection
					if (!active_point_selection) {
						active_point_selection = &p;
					}

					ImGui::TableNextRow();
					// handle row background color for selected entry
					const bool is_selected = active_point_selection && active_point_selection == &p;
					if (is_selected) {
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
					}

					// -
					ImGui::TableNextColumn();
					if (!is_selected) // only selectable if not selected
					{
						ImGui::Style_InvisibleSelectorPush(); // never show selection - we use tablebg
						if (ImGui::Selectable(utils::va("%d", i), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
							active_point_selection = &p;
						}
						ImGui::Style_InvisibleSelectorPop();

						if (ImGui::IsItemHovered()) {
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0.6f)));///*ImGui::GetColorU32(ImGuiCol_TableRowBgAlt)*/);
						}
					}
					else {
						ImGui::Text("%d", i); // if selected
					}

					if (active_point_selection && active_point_selection == &p) {
						selection_matches_any_entry = true; // check that the selection ptr is up to date
					}

					// timepoint
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", p.timepoint);

					// pos
					ImGui::TableNextColumn();
					ImGui::TextUnformatted_ClippedByColumnTooltip(utils::va("%.0f, %.0f, %.0f", p.position.x, p.position.y, p.position.z));

					// rad
					ImGui::TableNextColumn();
					ImGui::TextUnformatted_ClippedByColumnTooltip(utils::va("%.0f, %.0f, %.0f", p.radiance.x, p.radiance.y, p.radiance.z));

					// scalar
					ImGui::TableNextColumn();
					ImGui::Text("%.1f", p.radiance_scalar);

					// radius
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", p.radius);

					// smooth
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", p.smoothness);

					// shaping
					ImGui::TableNextColumn();
					ImGui::Text("%s", p.use_shaping ? "x" : "");

					// dir
					ImGui::TableNextColumn();
					ImGui::TextUnformatted_ClippedByColumnTooltip(utils::va("%.2f, %.2f, %.2f", p.direction.x, p.direction.y, p.direction.z));

					// deg
					ImGui::TableNextColumn();
					ImGui::Text("%.1f", p.degrees);

					// soft
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", p.softness);

					// smooth
					ImGui::TableNextColumn();
					ImGui::Text("%.2f", p.smoothness);
				}

				if (!selection_matches_any_entry) {
					active_point_selection = nullptr;
				}

				ImGui::EndTable();
			} // ----------------------------------------

			ImGui::Spacing(0, 0);

			ImGui::Style_ColorButtonPush(im->ImGuiCol_ButtonGreen, true);
			if (ImGui::Button("Add Point", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
			{
				//if (active_points_count <= 1)
				{
					// copy light def because we dont want to directly edit the mapsettings def
					auto new_def = edit_active_light->def;

					// use mover points if light has more then 1 pt
					new_def.points = edit_active_light->mover.is_initialized()
						? edit_active_light->mover.get_points_vec()
						: new_def.points;

					lights->destroy_and_clear_all_active_lights(); 

					std::uint32_t prev_point_index = new_def.points.size() - 1u;
					new_def.points.push_back(new_def.points[prev_point_index]); // copy first point
					new_def.points.back().timepoint += 2.0f;
					
					if (!lights->get_active_light_count()) {
						lights->add_single_map_setting_light_for_editing(&new_def);
					}

					// --

					if (edit_active_light->mover.is_initialized())
					{
						active_points = edit_active_light->mover.get_points();
						active_points_count = edit_active_light->mover.get_points_count();
						active_point_selection = &active_points[active_points_count - 1u];
					}
				}
			}
			ImGui::Style_ColorButtonPop();

			ImGui::BeginDisabled( !active_point_selection 
								|| active_points_count <= 1u
								|| active_point_selection == active_points); // if first point
			{
				ImGui::SameLine();
				ImGui::Style_ColorButtonPush(im->ImGuiCol_ButtonRed, true);
				if (ImGui::Button("Delete Selected Point", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
				{
					// copy light def because we dont want to directly edit the mapsettings def
					auto new_def = edit_active_light->def;
					 
					// use mover points if light has more then 1 pt
					new_def.points = edit_active_light->mover.is_initialized()
						? edit_active_light->mover.get_points_vec()
						: new_def.points;

					// disabled handles single points ..
					std::ptrdiff_t pt_index = active_point_selection - active_points;

					if ((size_t) pt_index < new_def.points.size()) {
						new_def.points.erase(new_def.points.begin() + pt_index);
					}

					lights->destroy_and_clear_all_active_lights();

					if (!lights->get_active_light_count()) {
						lights->add_single_map_setting_light_for_editing(&new_def);
					}

					if (edit_active_light->mover.is_initialized())
					{
						active_points = edit_active_light->mover.get_points();
						active_points_count = edit_active_light->mover.get_points_count();
						active_point_selection = &active_points[active_points_count - 1u];
					}
				}
				ImGui::Style_ColorButtonPop();
				ImGui::EndDisabled();
			}

			ImGui::Spacing(0, 4);

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
			ImGui::TableHeaderDropshadow();
			const bool light_transform_state = ImGui::CollapsingHeader("Light Transform (all points)");
			ImGui::PopStyleVar();

			if (light_transform_state)
			{
				const auto cont_bg_color = im->ImGuiCol_ContainerBackground + ImVec4(0.05f, 0.05f, 0.05f, 0.0f);

				//static float cont_height = 0.0f;
				//cont_height = ImGui::Widget_ContainerWithDropdownShadow(cont_height, [active_points, edit_active_light, is_static_light_with_single_point]
					{
						if (ImGui::Button("Teleport Light To Player", ImVec2(ImGui::CalcWidgetWidthForChild(120.0f), 0)))
						{
							if (is_static_light_with_single_point) {
								active_points->position = *game::get_current_view_origin();
							}
							else
							{
								auto& pts = edit_active_light->mover.get_points_vec();
								Vector current_center = {};

								for (const auto& point : pts) {
									current_center += point.position;
								} current_center /= (float)pts.size();

								Vector teleport_offset = *game::get_current_view_origin() - current_center;

								for (auto& point : pts) {
									point.position += teleport_offset;
								}
							}
						}

						static float offs_step_offset = 0.5f;

						float offs[3] = {};
						if (ImGui::Widget_PrettyStepVec3("Offset Light", offs, true, 80.0f, offs_step_offset))
						{
							if (is_static_light_with_single_point) {
								active_points->position += offs;
							}
							else
							{
								for (auto& point : edit_active_light->mover.get_points_vec()) {
									point.position += offs;
								}
							}
						}

						SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
						if (ImGui::DragFloat("Step Offset", &offs_step_offset, 0.005f, 0.0f, 100.0f)) {
							offs_step_offset = std::clamp(offs_step_offset, 0.0f, 100.0f);
						}

						ImGui::Spacing(0, 4);
						ImGui::Separator();
						ImGui::Spacing(0, 4);

					}//, &cont_bg_color, &im->ImGuiCol_ContainerBorder);
			}

			ImGui::Spacing(0, 8);

			// -------

			if (active_point_selection)
			{
				std::ptrdiff_t pt_index = 0;
				if (!is_static_light_with_single_point)
				{
					pt_index = active_point_selection - active_points;
					pt_index = pt_index < 0 ? 0 : pt_index;
				}

				ImGui::Widget_PrettyDragVec3("Position", &active_point_selection->position.x, true, 120.0f, 0.25f);

				Vector normalized_radiance = im->m_debugvis_live ? &edit_active_light->info.radiance.x : active_point_selection->radiance; 
				normalized_radiance.Normalize();

				//const auto debug_color_bg = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
				const auto debug_color = ImGui::ColorConvertFloat4ToU32(ImVec4(normalized_radiance.x, normalized_radiance.y, normalized_radiance.z, 1.0f));

				// draw position as circle
				Vector screen_pos; common::imgui::world2screen((im->m_debugvis_live ? &edit_active_light->ext.position.x : active_point_selection->position) + edit_active_light->attached_offset, screen_pos);
				ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(screen_pos.x, screen_pos.y), 8.0f, debug_color);


				ImGui::Widget_PrettyDragVec3("Radiance", &active_point_selection->radiance.x, true, 120.0f, 0.1f, 0.0f, FLT_MAX,
					"R", "G", "B");

				SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
				if (ImGui::DragFloat("Radiance Scale", &active_point_selection->radiance_scalar, 0.1f, 0.0f, FLT_MAX, "%.1f")) {
					active_point_selection->radiance_scalar = active_point_selection->radiance_scalar < 0.0f ? 0.0f : active_point_selection->radiance_scalar;
				} TT("General radiance scalar");


				SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
				if (ImGui::DragFloat("Radius", &active_point_selection->radius, 0.005f, 0.0f, FLT_MAX, "%.1f")) {
					active_point_selection->radius = active_point_selection->radius < 0.0f ? 0.0f : active_point_selection->radius;
				} TT("Radius of light (defaults to 1.0)");

				//ImGui::Draw3DCircle(ImGui::GetBackgroundDrawList(), &edit_active_light->ext.position.x, Vector(0.0f, 0.0f, 1.0f), active_point_selection->radius, false, debug_color, 2.0f);
				//ImGui::Draw3DCircle(ImGui::GetBackgroundDrawList(), &edit_active_light->ext.position.x, Vector(0.0f, 1.0f, 0.0f), active_point_selection->radius, false, debug_color, 2.0f);
				//ImGui::Draw3DCircle(ImGui::GetBackgroundDrawList(), &edit_active_light->ext.position.x, Vector(1.0f, 0.0f, 0.0f), active_point_selection->radius, false, debug_color, 2.0f);

				if (im->m_debugvis_radius)
				{
					const Vector circle_pos = (im->m_debugvis_live ? &edit_active_light->ext.position.x : active_point_selection->position) + edit_active_light->attached_offset;
					const float radius = im->m_debugvis_live ? edit_active_light->ext.radius : active_point_selection->radius;

					const auto remixapi = remix_api::get();
					remixapi->add_debug_circle(circle_pos, Vector(0.0f, 0.0f, 1.0f), radius - 0.02f, radius * 0.1f, normalized_radiance);
					remixapi->add_debug_circle_based_on_previous(circle_pos, Vector(0, 90, 0), Vector(1.0f, 1.0f, 1.0f));
					remixapi->add_debug_circle_based_on_previous(circle_pos, Vector(90, 0, 90), Vector(1.0f, 1.0f, 1.0f));
				}

				// cant edit time of first point
				ImGui::BeginDisabled(!pt_index);
				{
					// get min and max timepoint for current point
					float min_timepoint = 0.0f, max_timepoint = FLT_MAX;
					if (pt_index > 0) {
						min_timepoint = active_points[pt_index - 1].timepoint + 0.1f; // at least 0.1 diff
					}

					if (pt_index + 1u < active_points_count) {
						max_timepoint = active_points[pt_index + 1].timepoint - 0.1f; // at least 0.1 diff
					}

					SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
					if (ImGui::DragFloat("Timepoint", &active_point_selection->timepoint, 0.005f, min_timepoint, max_timepoint, "%.1f")) 
					{
						active_point_selection->timepoint = std::clamp(active_point_selection->timepoint, min_timepoint, max_timepoint);
						edit_active_light->mover.calculate_segment_durations();
					}
					TT("Time in seconds at which the light arrives at the point\n"
					   "Last point defines the total duration.");

					ImGui::EndDisabled();
				}

				SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
				if (ImGui::DragFloat("Smoothness", &active_point_selection->smoothness, 0.005f, 0.0f, 3.0f, "%.1f")) {
					active_point_selection->smoothness = active_point_selection->smoothness < 0.0f ? 0.0f : active_point_selection->smoothness;
				} TT("Curve smoothness (defaults to 0.5 - values above 1 might produce odd results)");

				ImGui::Spacing(0, 8);

				bool light_shaping_state = active_point_selection->use_shaping;
				if (ImGui::Checkbox("Use Light Shaping", &active_point_selection->use_shaping))
				{
					if (!active_point_selection->use_shaping)
					{
						active_point_selection->direction.x = 0.0f;
						active_point_selection->direction.y = 0.0f;
						active_point_selection->direction.z = 1.0f;
						active_point_selection->degrees = 180.0f;
					}

					// was just toggled on -> set default val
					if (active_point_selection->use_shaping && light_shaping_state != active_point_selection->use_shaping) {
						active_point_selection->degrees = 90.0f;
					}
				}

				if (active_point_selection->use_shaping)
				{
					if (ImGui::Widget_PrettyDragVec3("Direction", &active_point_selection->direction.x, true, 120.0f, 0.1f, -1.0f, 1.0f)) {
						active_point_selection->direction.Normalize();
					}

					SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
					if (ImGui::DragFloat("Degrees", &active_point_selection->degrees, 0.25f, 0.0f, 180.0f, "%.1f")) {
						active_point_selection->degrees = std::clamp(active_point_selection->degrees, 0.0f, 180.0f);
					} TT("Cone Angle");

					SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
					if (ImGui::DragFloat("Softness", &active_point_selection->softness, 0.005f, 0.0f, 1.0f, "%.1f")) {
						active_point_selection->softness = std::clamp(active_point_selection->softness, 0.0f, 1.0f);
					} TT("Cone Softness");

					SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
					if (ImGui::DragFloat("Exponent", &active_point_selection->exponent, 0.005f, 0.0f, 1.0f, "%.1f")) {
						active_point_selection->exponent = std::clamp(active_point_selection->exponent, 0.0f, 1.0f);
					} TT("Cone Focus Exponent");

					if (im->m_debugvis_shaping)
					{
						const auto remixapi = remix_api::get();
						const float cone_deg = im->m_debugvis_live ? edit_active_light->ext.shaping_value.coneAngleDegrees : active_point_selection->degrees;

						if (cone_deg <= 90.0f)
						{
							const float cone_rad_tan = std::tan(DEG2RAD(cone_deg));
							const float scaled_height = im->m_debugvis_cone_height / (1.0f + cone_rad_tan);

							// draw cone
							for (auto i = 1; i <= im->m_debugvis_cone_steps; ++i)
							{
								const float step_fraction = (float)i / (float)im->m_debugvis_cone_steps;

								float radius = (step_fraction * scaled_height) * cone_rad_tan;

								Vector circle_pos =
									(im->m_debugvis_live ? Vector(&edit_active_light->ext.position.x) + Vector(&edit_active_light->ext.shaping_value.direction.x) * (step_fraction * scaled_height)
									: active_point_selection->position + active_point_selection->direction * (step_fraction * scaled_height)) + edit_active_light->attached_offset;

								remixapi->add_debug_circle(
									circle_pos,
									im->m_debugvis_live ? &edit_active_light->ext.shaping_value.direction.x : active_point_selection->direction,
									radius, radius * 0.025f, normalized_radiance, false);
							}
						}

						// draw dir line
						remixapi->add_debug_line(
							im->m_debugvis_live ?	&edit_active_light->ext.position.x
														: active_point_selection->position,
							im->m_debugvis_live	? Vector(&edit_active_light->ext.position.x) + Vector(&edit_active_light->ext.shaping_value.direction.x).Scale(im->m_debugvis_cone_height)
													: active_point_selection->position + active_point_selection->direction.Scale(im->m_debugvis_cone_height),
							1.0f, remix_api::WHITE);
					}
				} // end use shaping

				ImGui::Spacing(0, 8);
				mapsettings_ls_general_light_settings(edit_active_light);

				ImGui::Spacing(0, 8);
				mapsettings_ls_playback_visualization_settings(edit_active_light, is_static_light_with_single_point);
				ImGui::Spacing(0, 4);

				// update light def for static light
				if (is_static_light_with_single_point) {
					remix_lights::get()->update_static_remix_light(edit_active_light, active_point_selection);
				}
			} // end if active_point_selection
			

			// popup frame
			ImGui::PushID("LightTable");
			if (ImGui::BeginPopupModal("Ignore Changes?", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
			{
				common::imgui::draw_background_blur();
				ImGui::Spacing(0.0f, 0.0f);

				const auto half_width = ImGui::GetContentRegionMax().x * 0.5f;
				auto line1_str = "You'll loose all unsaved changes if you continue!";
				auto line2_str = "Copy to clipboard and manually change map_settings.toml!   ";
				auto line3_str = "Do not forget to save the file ;)";

				ImGui::Spacing();
				ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line1_str).x * 0.5f));
				ImGui::TextUnformatted(line1_str);

				

				ImGui::Spacing(0, 2);
				ImGui::PushFont(common::imgui::font::BOLD);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x * 0.5f - 120.0f));
				if (ImGui::Button("Copy Selected Light to Clipboard   " ICON_FA_SAVE, ImVec2(240.0f, 0)))
				{
					if (const auto edit_light = lights->get_first_active_light(); edit_light)
					{
						auto temp_def = edit_light->def;
						if (edit_light->mover.is_initialized()) {
							temp_def.points = edit_light->mover.get_points_vec();
						}

						ImGui::LogToClipboard();
						ImGui::LogText("%s", common::toml::build_light_string_for_single_light(temp_def).c_str());
						ImGui::LogFinish();
					}
				} ImGui::PopFont();
				ImGui::Spacing(0, 2);

				ImGui::Spacing();
				ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line2_str).x * 0.5f));
				ImGui::TextUnformatted(line2_str);
				ImGui::Spacing(0, 2);

				ImGui::PushFont(common::imgui::font::BOLD);
				ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line3_str).x * 0.5f));
				ImGui::TextUnformatted(line3_str);
				ImGui::PopFont();

				ImGui::Spacing(0, 8);
				ImGui::Spacing(0, 0); ImGui::SameLine();

				ImVec2 button_size(half_width - 6.0f - ImGui::GetStyle().WindowPadding.x, 0.0f);
				if (ImGui::Button("Ignore", button_size))
				{
					ms_light_selection = ms_light_selection_pending;
					ms_light_selection_pending = nullptr;
					lights->destroy_and_clear_all_active_lights();
					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine(0, 6.0f);
				if (ImGui::Button("Cancel", button_size)) 
				{
					ms_light_selection_pending = nullptr;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			ImGui::PopID();
		} // end if edit_active_light && ms_light_selection
	}

	void cont_mapsettings_confvar()
	{
		const auto& var = remix_vars::get();

		ImGui::PushFont(common::imgui::font::BOLD);
		if (ImGui::Button("Reset Vars to Level State   " ICON_FA_REPLY_ALL "##ConfvarReset", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0))) {
			var->reset_all_modified(true);
		} ImGui::PopFont(); TT("This resets all remix vars back to level state (same as when map loads)");

		ImGui::SameLine();
		reload_mapsettings_button_with_popup("Confvar");
		ImGui::Spacing(0, 2);

		static std::string conf_str1, conf_str2;
		static float conf1_transition_time = 0.0f, conf2_transition_time = 0.0f;
		static remix_vars::EASE_TYPE conf1_mode = remix_vars::EASE_TYPE_SIN_IN, conf2_mode = remix_vars::EASE_TYPE_SIN_IN;
		static std::vector<std::string> configs;
		static bool loaded_configs = false;

		ImGui::PushFont(common::imgui::font::BOLD);
		if (ImGui::Button("Refresh Configs   " ICON_FA_REDO, ImVec2(ImGui::GetContentRegionAvail().x, 0)) || !loaded_configs)
		{
			configs.clear();
			if (!game::root_path.empty())
			{
				std::string conf_path = game::root_path + "l4d2-rtx\\map_configs\\";
				if (std::filesystem::exists(conf_path))
				{
					for (const auto& d : std::filesystem::directory_iterator(conf_path))
					{
						if (d.path().extension() == ".conf")
						{
							auto file = std::filesystem::path(d.path());
							configs.push_back(file.filename().string());
						}
					}
				}
				loaded_configs = true;
			}
		}
		ImGui::PopFont();
		ImGui::Spacing(0, 6);

		{

			ImGui::TableHeaderDropshadow();
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
			ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetColorU32(ImGuiCol_FrameBgActive));
			if (ImGui::BeginListBox("##listbox1", ImVec2(ImGui::GetContentRegionAvail().x, 100.0f)))
			{
				for (const auto& str : configs)
				{
					const bool is_selected = conf_str1 == str;
					if (ImGui::Selectable(str.c_str(), is_selected)) {
						conf_str1 = str;
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndListBox();
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::Spacing(0, 4);

			SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
			if (ImGui::DragFloat("Transition Time##1", &conf1_transition_time, 0.005f, 0.0f)) {
				conf1_transition_time = std::clamp(conf1_transition_time, 0.0f, FLT_MAX);
			}

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
			if (ImGui::BeginCombo("##ModeSelector1", remix_vars::EASE_TYPE_STR[conf1_mode], ImGuiComboFlags_None))
			{
				for (std::uint32_t n = 0u; n < (std::uint32_t)IM_ARRAYSIZE(remix_vars::EASE_TYPE_STR); n++)
				{
					const bool is_selected = conf1_mode == n;
					if (ImGui::Selectable(remix_vars::EASE_TYPE_STR[n], is_selected)) {
						conf1_mode = (remix_vars::EASE_TYPE)n;
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}

				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			ImGui::BeginDisabled(conf_str1.empty());

			const auto btn_to_label_size = ImGui::CalcWidgetWidthForChild(120.0f);
			if (ImGui::Button("Trigger##1", ImVec2(btn_to_label_size, 0)))
			{
				std::string conf_name = conf_str1;
				if (!conf_name.ends_with(".conf")) {
					conf_name += ".conf";
				}

				var->parse_and_apply_conf_with_lerp(
					conf_name,
					utils::string_hash64(conf_name),
					conf1_mode,
					conf1_transition_time);
			}
			ImGui::EndDisabled();
		}

		ImGui::Spacing(0, 4);
		ImGui::Separator();
		ImGui::Spacing(0, 4);

		{
			ImGui::TableHeaderDropshadow();
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
			ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetColorU32(ImGuiCol_FrameBgActive));
			if (ImGui::BeginListBox("##listbox2", ImVec2(ImGui::GetContentRegionAvail().x, 100.0f)))
			{
				for (const auto& str : configs)
				{
					const bool is_selected = conf_str2 == str;
					if (ImGui::Selectable(str.c_str(), is_selected)) {
						conf_str2 = str;
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndListBox();
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::Spacing(0, 4);

			SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
			if (ImGui::DragFloat("Transition Time##2", &conf2_transition_time, 0.005f, 0.0f)) {
				conf2_transition_time = std::clamp(conf2_transition_time, 0.0f, FLT_MAX);
			}

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
			if (ImGui::BeginCombo("##ModeSelector2", remix_vars::EASE_TYPE_STR[conf2_mode], ImGuiComboFlags_None))
			{
				for (std::uint32_t n = 0u; n < (std::uint32_t)IM_ARRAYSIZE(remix_vars::EASE_TYPE_STR); n++)
				{
					const bool is_selected = conf2_mode == n;
					if (ImGui::Selectable(remix_vars::EASE_TYPE_STR[n], is_selected)) {
						conf2_mode = (remix_vars::EASE_TYPE)n;
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}

				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			ImGui::BeginDisabled(conf_str2.empty());

			const auto btn_to_label_size = ImGui::CalcWidgetWidthForChild(120.0f);
			if (ImGui::Button("Trigger##2", ImVec2(btn_to_label_size, 0)))
			{
				std::string conf_name = conf_str2;
				if (!conf_name.ends_with(".conf")) {
					conf_name += ".conf";
				}

				var->parse_and_apply_conf_with_lerp(
					conf_name,
					utils::string_hash64(conf_name),
					conf2_mode,
					conf2_transition_time);
			}
			ImGui::EndDisabled();
		}

		ImGui::Spacing(0, 4);
	}

	void imgui::tab_map_settings()
	{
		// general settings
		{
			static float cont_general_height = 0.0f;
			cont_general_height = ImGui::Widget_ContainerWithCollapsingTitle("General Settings", cont_general_height, cont_mapsettings_general,
				true, ICON_FA_ELLIPSIS_H, &ImGuiCol_ContainerBackground, &ImGuiCol_ContainerBorder);
		}

		ImGui::Spacing(0, 6.0f);
		ImGui::SeparatorText("The following settings do NOT auto-save.");
		ImGui::TextDisabled("Export to clipboard and override the settings manually!");
		ImGui::Spacing(0, 6.0f);

		// fog settings
		{
			static float cont_fog_height = 0.0f;
			cont_fog_height = ImGui::Widget_ContainerWithCollapsingTitle("Fog Settings", cont_fog_height, cont_mapsettings_fog, 
				false, ICON_FA_WATER, &ImGuiCol_ContainerBackground, &ImGuiCol_ContainerBorder);
		}

		// marker manipulation
		{
			static float cont_marker_manip_height = 0.0f;
			cont_marker_manip_height = ImGui::Widget_ContainerWithCollapsingTitle("Marker Manipulation", cont_marker_manip_height, cont_mapsettings_marker_manipulation, 
				false, ICON_FA_DICE_D6, &ImGuiCol_ContainerBackground, &ImGuiCol_ContainerBorder);
		}

		// culling manipulation
		{
			static float cont_cull_manip_height = 0.0f;
			cont_cull_manip_height = ImGui::Widget_ContainerWithCollapsingTitle("Culling Manipulation", cont_cull_manip_height, cont_mapsettings_culling_manipulation, 
				false, ICON_FA_EYE_SLASH, &ImGuiCol_ContainerBackground, &ImGuiCol_ContainerBorder);
		}

		// lights
		{
			static float cont_lights_height = 0.0f;
			cont_lights_height = ImGui::Widget_ContainerWithCollapsingTitle("Lights", cont_lights_height, cont_mapsettings_light_spawning,
				false, ICON_FA_LIGHTBULB, &ImGuiCol_ContainerBackground, &ImGuiCol_ContainerBorder);
		}

		// config vars
		{
			static float cont_vars_height = 0.0f;
			cont_vars_height = ImGui::Widget_ContainerWithCollapsingTitle("Configvars / Transitions", cont_vars_height, cont_mapsettings_confvar,
				false, ICON_FA_PAINT_BRUSH, &ImGuiCol_ContainerBackground, &ImGuiCol_ContainerBorder);
		}

		m_devgui_custom_footer_content = "Area: " + std::to_string(g_current_area) + "\nLeaf: " + std::to_string(g_current_leaf);
	}

	// #
	// #

	void cont_gamesettings_flashlight()
	{
		const auto gs = game_settings::get();

		ImGui::Widget_PrettyDragVec3("Offsets Player", gs->flashlight_offset_player.get_as<float*>(), true, 80.0f, 0.1f, -1000.0f, 1000.0f, "F", "H", "V");
		TT(gs->flashlight_offset_player.get_tooltip_string().c_str());

		ImGui::Widget_PrettyDragVec3("Offsets Bot", gs->flashlight_offset_bot.get_as<float*>(), true, 80.0f, 0.1f, -1000.0f, 1000.0f, "F", "H", "V");
		TT(gs->flashlight_offset_bot.get_tooltip_string().c_str());

		SET_CHILD_WIDGET_WIDTH_MAN(80);
		ImGui::DragFloat("Intensity", gs->flashlight_intensity.get_as<float*>(), 0.1f);

		SET_CHILD_WIDGET_WIDTH_MAN(80);
		ImGui::DragFloat("Radius", gs->flashlight_radius.get_as<float*>(), 0.005f);

		SET_CHILD_WIDGET_WIDTH_MAN(80);
		ImGui::DragFloat("Spot Angle", gs->flashlight_angle.get_as<float*>(), 0.001f);

		SET_CHILD_WIDGET_WIDTH_MAN(80);
		ImGui::DragFloat("Spot Softness", gs->flashlight_softness.get_as<float*>(), 0.001f);

		SET_CHILD_WIDGET_WIDTH_MAN(80);
		ImGui::DragFloat("Spot Expo", gs->flashlight_expo.get_as<float*>(), 0.001f);
	}

	void cont_gamesettings_quick_cmd()
	{
		if (ImGui::Button("Save Current Settings", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0))) {
			game_settings::write_toml();
		}

		ImGui::SameLine();
		if (ImGui::Button("Reload GameSettings", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
		{
			if (!ImGui::IsPopupOpen("Reload GameSettings?")) {
				ImGui::OpenPopup("Reload GameSettings?");
			}
		}

		// popup
		if (ImGui::BeginPopupModal("Reload GameSettings?", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
		{
			common::imgui::draw_background_blur();
			ImGui::Spacing(0.0f, 0.0f);

			const auto half_width = ImGui::GetContentRegionMax().x * 0.5f;
			auto line1_str = "You'll loose all unsaved changes if you continue!   ";
			auto line2_str = "To save your changes, use:";
			auto line3_str = "Save Current Settings";

			ImGui::Spacing();
			ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line1_str).x * 0.5f));
			ImGui::TextUnformatted(line1_str);

			ImGui::Spacing();
			ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line2_str).x * 0.5f));
			ImGui::TextUnformatted(line2_str);

			ImGui::PushFont(common::imgui::font::BOLD);
			ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line3_str).x * 0.5f));
			ImGui::TextUnformatted(line3_str);
			ImGui::PopFont();

			ImGui::Spacing(0, 8);
			ImGui::Spacing(0, 0); ImGui::SameLine();

			ImVec2 button_size(half_width - 6.0f - ImGui::GetStyle().WindowPadding.x, 0.0f);
			if (ImGui::Button("Reload", button_size))
			{
				game_settings::xo_gamesettings_update_fn();
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine(0, 6.0f);
			if (ImGui::Button("Cancel", button_size)) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void cont_gamesettings_renderer_settings()
	{
		const auto gs = game_settings::get();
		ImGui::Checkbox("Enable LOD Forcing", gs->lod_forcing.get_as<bool*>()); TT(gs->lod_forcing.get_tooltip_string().c_str());

		if (ImGui::Checkbox("Enable 3D Skybox (very unstable)", gs->enable_3d_sky.get_as<bool*>())) {
			remix_vars::set_option(remix_vars::get_option("rtx.skyAutoDetect"), remix_vars::string_to_option_value(remix_vars::OPTION_TYPE_FLOAT, gs->enable_3d_sky.get_as<bool>() ? "1" : "0"));
		}
		TT(gs->enable_3d_sky.get_tooltip_string().c_str());

		SET_CHILD_WIDGET_WIDTH_MAN(120.0f);
		auto gs_nocull_dist_ptr = game_settings::get()->default_nocull_distance.get_as<float*>();
		if (ImGui::DragFloat("Def. NoCull Dist", gs_nocull_dist_ptr, 0.5f, 0.0f, FLT_MAX, "%.2f")) 
		{
			*gs_nocull_dist_ptr = *gs_nocull_dist_ptr < 0.0f ? 0.0f : *gs_nocull_dist_ptr;
			map_settings::get_map_settings().default_nocull_dist = *gs_nocull_dist_ptr;
		}
		TT(gs->default_nocull_distance.get_tooltip_string().c_str());
	}

	void imgui::tab_game_settings()
	{
		// quick commands
		{
			static float cont_quickcmd_height = 0.0f;
			cont_quickcmd_height = ImGui::Widget_ContainerWithCollapsingTitle("Quick Commands", cont_quickcmd_height, cont_gamesettings_quick_cmd,
				true, ICON_FA_TERMINAL, &ImGuiCol_ContainerBackground, &ImGuiCol_ContainerBorder);
		}

		// renderer related settings
		{
			static float cont_rendersettings_height = 0.0f;
			cont_rendersettings_height = ImGui::Widget_ContainerWithCollapsingTitle("Renderer Related Settings", cont_rendersettings_height, cont_gamesettings_renderer_settings,
				true, ICON_FA_CAMERA, &ImGuiCol_ContainerBackground, &ImGuiCol_ContainerBorder);
		}

		// flashlight
		{
			static float cont_flashlight_height = 0.0f;
			cont_flashlight_height = ImGui::Widget_ContainerWithCollapsingTitle("Flashlight", cont_flashlight_height, cont_gamesettings_flashlight,
				true, ICON_FA_LIGHTBULB, &ImGuiCol_ContainerBackground, &ImGuiCol_ContainerBorder);
		}
	}

	// #
	// #

	void imgui::devgui()
	{
		ImGui::SetNextWindowSize(ImVec2(900, 800), ImGuiCond_FirstUseEver);

		bool old_active_state = m_menu_active;
		if (!ImGui::Begin("Devgui", &m_menu_active, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse, &common::imgui::draw_window_blur_callback))
		{
			ImGui::End();
			return;
		}

		// HACK when using the menu close button instead of the hotkey to close the devgui
		// :: use logic in 'imgui::input_message' to toggle the menu by sending a msg
		if (old_active_state != m_menu_active && !m_menu_active) 
		{ 
			m_menu_active = true; // we have to re-set this back to true. We would instantly reopen the gui otherwise
			SendMessage(glob::main_window, WM_KEYUP, VK_F5, 0);
		}

		m_im_window_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
		m_im_window_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);

		static bool im_demo_menu = false;
		if (im_demo_menu) {
			ImGui::ShowDemoWindow(&im_demo_menu);
		}

#define ADD_TAB(NAME, FUNC) \
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0)));			\
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 8));			\
	if (ImGui::BeginTabItem(NAME)) {																		\
		ImGui::PopStyleVar(1);																				\
		if (ImGui::BeginChild("##child_" NAME, ImVec2(0, ImGui::GetContentRegionAvail().y - 38), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_AlwaysVerticalScrollbar )) {	\
			FUNC(); ImGui::EndChild();																		\
		} else {																							\
			ImGui::EndChild();																				\
		} ImGui::EndTabItem();																				\
	} else { ImGui::PopStyleVar(1); } ImGui::PopStyleColor();

		// ---------------------------------------

		const auto col_top = ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0.0f));
		const auto col_bottom = ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0.4f));
		const auto col_border = ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0.8f));
		const auto pre_tabbar_spos = ImGui::GetCursorScreenPos() - ImGui::GetStyle().WindowPadding;

		ImGui::GetWindowDrawList()->AddRectFilledMultiColor(pre_tabbar_spos, pre_tabbar_spos + ImVec2(ImGui::GetWindowWidth(), 40.0f),
			col_top, col_top, col_bottom, col_bottom);

		ImGui::GetWindowDrawList()->AddLine(pre_tabbar_spos + ImVec2(0, 40.0f), pre_tabbar_spos + ImVec2(ImGui::GetWindowWidth(), 40.0f),
			col_border, 1.0f);

		ImGui::SetCursorScreenPos(pre_tabbar_spos + ImVec2(12,8));

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 8));
		ImGui::PushStyleColor(ImGuiCol_TabSelected, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		if (ImGui::BeginTabBar("devgui_tabs"))
		{
			ImGui::PopStyleColor();
			ImGui::PopStyleVar(1);
			ADD_TAB("General", tab_general);
			ADD_TAB("Map Settings", tab_map_settings);
			ADD_TAB("Game Settings", tab_game_settings);
			ImGui::EndTabBar();
		}
		else {
			ImGui::PopStyleColor();
			ImGui::PopStyleVar(1);
		}
#undef ADD_TAB

		{

			ImGui::Separator();
			//ImGui::Spacing();

			const char* movement_hint_str = "Press and Hold the Right Mouse Button outside ImGui to allow for Game Input ";
			const auto avail_width = ImGui::GetContentRegionAvail().x;
			float cur_pos = avail_width - 54.0f;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			{
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().ItemSpacing.y);
				const auto spos = ImGui::GetCursorScreenPos();
				ImGui::TextUnformatted(m_devgui_custom_footer_content.c_str());
				ImGui::SetCursorScreenPos(spos);
				m_devgui_custom_footer_content.clear();
			}
			

			ImGui::SetCursorPos(ImVec2(cur_pos, ImGui::GetCursorPosY() + 2.0f));
			if (ImGui::Button("Demo", ImVec2(50, 0))) {
				im_demo_menu = !im_demo_menu;
			}

			ImGui::SameLine();
			cur_pos = cur_pos - ImGui::CalcTextSize(movement_hint_str).x - 6.0f;
			ImGui::SetCursorPosX(cur_pos);
			ImGui::TextUnformatted(movement_hint_str);
		}
		ImGui::PopStyleVar(1);
		ImGui::End();
	}

	void imgui::endscene_stub()
	{
		if (auto* im = imgui::get(); im)
		{
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

					if (im->m_menu_active) {
						im->devgui();
					}

					ImGui::EndFrame();
					ImGui::Render();
					ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
				}
			}
		}
	}

	// called before mapsettings are applied 
	void imgui::on_map_load()
	{
		get()->m_light_edit_mode = false;
	}

	void imgui::style_xo()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.5f;

		style.WindowPadding = ImVec2(8.0f, 10.0f);
		style.FramePadding = ImVec2(7.0f, 6.0f);
		style.ItemSpacing = ImVec2(3.0f, 3.0f);
		style.ItemInnerSpacing = ImVec2(3.0f, 8.0f);
		style.IndentSpacing = 0.0f;
		style.ColumnsMinSpacing = 10.0f;
		style.ScrollbarSize = 10.0f;
		style.GrabMinSize = 10.0f;

		style.WindowBorderSize = 1.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupBorderSize = 1.0f;
		style.FrameBorderSize = 1.0f;
		style.TabBorderSize = 0.0f;

		style.WindowRounding = 0.0f;
		style.ChildRounding = 2.0f;
		style.FrameRounding = 4.0f;
		style.PopupRounding = 2.0f;
		style.ScrollbarRounding = 2.0f;
		style.GrabRounding = 1.0f;
		style.TabRounding = 2.0f;
		
		style.CellPadding = ImVec2(5.0f, 4.0f);

		auto& colors = style.Colors;
		colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.26f, 0.26f, 0.26f, 0.78f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.28f, 0.28f, 0.28f, 0.92f);
		colors[ImGuiCol_Border] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.23f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.17f, 0.25f, 0.27f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.07f, 0.39f, 0.47f, 0.59f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.98f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 0.98f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.15f, 0.15f, 0.98f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.39f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.54f, 0.54f, 0.54f, 0.47f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.78f, 0.78f, 0.78f, 0.33f);
		colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.39f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.31f);
		colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.45f, 0.45f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.17f, 0.25f, 0.27f, 0.78f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.17f, 0.25f, 0.27f, 0.78f);
		colors[ImGuiCol_Separator] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.15f, 0.52f, 0.66f, 0.30f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.30f, 0.69f, 0.84f, 0.39f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.43f, 0.43f, 0.43f, 0.51f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.07f, 0.39f, 0.47f, 0.59f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.30f, 0.69f, 0.84f, 0.39f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.19f, 0.53f, 0.66f, 0.39f);
		colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.37f);
		colors[ImGuiCol_TabSelected] = ImVec4(0.11f, 0.39f, 0.51f, 0.64f);
		colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.10f, 0.34f, 0.43f, 0.30f);
		colors[ImGuiCol_TabDimmed] = ImVec4(0.00f, 0.00f, 0.00f, 0.16f);
		colors[ImGuiCol_TabDimmedSelected] = ImVec4(1.00f, 1.00f, 1.00f, 0.24f);
		colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 0.35f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 1.00f, 1.00f, 0.35f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.11f, 0.42f, 0.51f, 0.35f);
		colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.00f, 0.51f, 0.39f, 0.31f);
		colors[ImGuiCol_NavCursor] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.56f);

		// custom colors
		ImGuiCol_ButtonGreen = ImVec4(0.3f, 0.4f, 0.05f, 0.7f);
		ImGuiCol_ButtonYellow = ImVec4(0.4f, 0.3f, 0.1f, 0.8f);
		ImGuiCol_ButtonRed = ImVec4(0.48f, 0.15f, 0.15f, 1.00f);
		ImGuiCol_ContainerBackground = ImVec4(0.220f, 0.220f, 0.220f, 0.863f);
		ImGuiCol_ContainerBorder = ImVec4(0.099f, 0.099f, 0.099f, 0.901f);
	}

	void init_fonts()
	{
		using namespace common::imgui::font;

		auto merge_icons_with_latest_font = [](const float& font_size, const bool font_data_owned_by_atlas = false)
			{
				static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0 };

				ImFontConfig icons_config;
				icons_config.MergeMode = true;
				icons_config.PixelSnapH = true;
				icons_config.FontDataOwnedByAtlas = font_data_owned_by_atlas;

				ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)fa_solid_900, sizeof(fa_solid_900), font_size, &icons_config, icons_ranges);
			};

		ImGuiIO& io = ImGui::GetIO();

		io.Fonts->AddFontFromMemoryCompressedTTF(opensans_bold_compressed_data, opensans_bold_compressed_size, 18.0f);
		merge_icons_with_latest_font(12.0f, false);

		io.Fonts->AddFontFromMemoryCompressedTTF(opensans_bold_compressed_data, opensans_bold_compressed_size, 17.0f);
		merge_icons_with_latest_font(12.0f, false);

		io.Fonts->AddFontFromMemoryCompressedTTF(opensans_regular_compressed_data, opensans_regular_compressed_size, 18.0f);
		io.Fonts->AddFontFromMemoryCompressedTTF(opensans_regular_compressed_data, opensans_regular_compressed_size, 16.0f);

		ImFontConfig font_cfg;
		font_cfg.FontDataOwnedByAtlas = false;

		io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(opensans_regular_compressed_data, opensans_regular_compressed_size, 17.0f, &font_cfg);
		merge_icons_with_latest_font(17.0f, false);
	}

	imgui::imgui()
	{
		p_this = this;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		init_fonts();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

		style_xo();

		ImGui_ImplWin32_Init(glob::main_window);
		g_game_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(glob::main_window, GWLP_WNDPROC, LONG_PTR(wnd_proc_hk)));
	}

	imgui::~imgui()
	{ }
}
