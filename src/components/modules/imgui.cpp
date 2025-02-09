#include "std_include.hpp"
#include "map_settings.hpp"
#include "components/common/imgui/imgui_helper.hpp"
#include "components/common/toml.hpp"
#include "components/common/imgui/font_awesome_solid_900.hpp"
#include "components/common/imgui/font_defines.hpp"
#include "components/common/imgui/font_opensans.hpp"

#ifdef USE_IMGUI
#include "imgui_internal.h"

// Allow us to directly call the ImGui WndProc function.
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

#define SPACING_INDENT_BEGIN ImGui::Spacing(); ImGui::Indent()
#define SPACING_INDENT_END ImGui::Spacing(); ImGui::Unindent()
#define TT(TXT) ImGui::SetItemTooltipBlur((TXT));

#define SET_CHILD_WIDGET_WIDTH			ImGui::SetNextItemWidth(ImGui::CalcWidgetWidthForChild(80.0f));
#define SET_CHILD_WIDGET_WIDTH_MAN(V)	ImGui::SetNextItemWidth(ImGui::CalcWidgetWidthForChild((V)));
#endif

namespace components
{
#if USE_IMGUI
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

	void reload_mapsettings_button_with_popup(const char* ID)
	{
		if (ImGui::Button(utils::va("Reload MapSettings##%s", ID), ImVec2(ImGui::GetContentRegionAvail().x, 0)))
		{
			if (!ImGui::IsPopupOpen("Reload MapSettings?")) {
				ImGui::OpenPopup("Reload MapSettings?");
			}
		}

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
				map_settings::reload();
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine(0, 6);
			if (ImGui::Button("Cancel", button_size)) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	// #
	// #

	void cont_general_quickcommands()
	{
		if (ImGui::Button("Director Start")) {
			interfaces::get()->m_engine->execute_client_cmd_unrestricted("director_start");
		}

		ImGui::SameLine();
		if (ImGui::Button("Director Stop")) {
			interfaces::get()->m_engine->execute_client_cmd_unrestricted("director_stop");
		}

		ImGui::SameLine();
		if (ImGui::Button("Kick Survivor Bots")) {
			interfaces::get()->m_engine->execute_client_cmd_unrestricted("kick rochelle; kick coach; kick ellis; kick roach; kick louis; kick zoey; kick francis; kick bill");
		}

		ImGui::SameLine();
		if (ImGui::Button("Give Autoshotgun")) {
			interfaces::get()->m_engine->execute_client_cmd_unrestricted("give autoshotgun");
		}

		ImGui::Spacing();

		static bool im_zignore_player = false;
		if (ImGui::Checkbox("Infected Ignore Player", &im_zignore_player))
		{
			if (!im_zignore_player) {
				interfaces::get()->m_engine->execute_client_cmd_unrestricted("nb_vision_ignore_survivors 0");
			}
			else {
				interfaces::get()->m_engine->execute_client_cmd_unrestricted("nb_vision_ignore_survivors 1");
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
		if (ImGui::Button("Reload rtx.conf", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
		{
			if (!ImGui::IsPopupOpen("Reload RtxConf?")) {
				ImGui::OpenPopup("Reload RtxConf?");
			}
		}

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

		if (ImGui::Button("Copy Settings to Clipboard##Fog", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
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
		}

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
		if (ImGui::Button("Copy All Markers to Clipboard", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
		{
			ImGui::LogToClipboard();
			ImGui::LogText("%s", common::toml::build_map_marker_string_for_current_map(markers).c_str());
			ImGui::LogFinish();
		}

		ImGui::SameLine();
		reload_mapsettings_button_with_popup("MapMarker");
		ImGui::Spacing(0, 4);

		constexpr auto in_buflen = 1024u;
		static char in_area_buf[in_buflen], in_nleaf_buf[in_buflen];
		static map_settings::marker_settings_s* selection = nullptr;

		//
		// MARKER TABLE

		if (ImGui::BeginTable("MarkerTable", 9,
			ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody |
			ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_ScrollY, ImVec2(0, 380)))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // make top row always visible
			ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 12.0f);
			ImGui::TableSetupColumn("Num", ImGuiTableColumnFlags_NoResize, 24.0f);
			ImGui::TableSetupColumn("NC", ImGuiTableColumnFlags_NoResize, 24.0f);
			ImGui::TableSetupColumn("Areas", ImGuiTableColumnFlags_WidthStretch, 80.0f);
			ImGui::TableSetupColumn("NLeafs", ImGuiTableColumnFlags_WidthStretch, 80.0f);
			ImGui::TableSetupColumn("Pos", ImGuiTableColumnFlags_WidthFixed, 200.0f);
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
					if (ImGui::Selectable(utils::va("%d", i), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 22 + ImGui::GetStyle().CellPadding.y * 1.0f)))
					{
						selection = &m;
						m.imgui_is_selected = true;
					}
					ImGui::Style_InvisibleSelectorPop();

					if (ImGui::IsItemHovered()) {
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0.6f)));///*ImGui::GetColorU32(ImGuiCol_TableRowBgAlt)*/);
					}
				}
				else {
					ImGui::Text("%d", i); // if selected
				}

				if (selection && selection != &m) {
					m.imgui_is_selected = false;
				}
				else {
					selection_matches_any_entry = true; // check that the selection ptr is up to date
				}

				// -
				ImGui::TableNextColumn();
				ImGui::Text("%d", m.index);

				// -
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

				const auto row_max_y_pos = ImGui::GetItemRectMax().y;

				// -
				ImGui::TableNextColumn(); ImGui::Spacing();
				ImGui::Text("%.2f, %.2f, %.2f", m.origin.x, m.origin.y, m.origin.z);

				// -
				ImGui::TableNextColumn(); ImGui::Spacing();
				ImGui::Text("%.2f, %.2f, %.2f", m.rotation.x, m.rotation.y, m.rotation.z);

				// -
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
				interfaces::get()->m_engine->execute_client_cmd_unrestricted(utils::va("noclip; setpos %.2f %.2f %.2f", selection->origin.x, selection->origin.y, selection->origin.z - 40.0f));
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
			ImGui::Widget_PrettyDragVec3("Origin", &selection->origin.x, true, 0.5f,
				-FLT_MAX, FLT_MAX, "X", "Y", "Z");
			//ImGui::PopStyleVar();

			// RAD2DEG -> DEG2RAD 
			Vector temp_rot = { RAD2DEG(selection->rotation.x), RAD2DEG(selection->rotation.y), RAD2DEG(selection->rotation.z) };

			ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.6f, 0.5f));
			if (ImGui::Widget_PrettyDragVec3("Rotation", &temp_rot.x, true, 0.1f, 
				-360.0f, 360.0f, "Rx", "Ry", "Rz")) 
			{
				selection->rotation = { DEG2RAD(temp_rot.x), DEG2RAD(temp_rot.y), DEG2RAD(temp_rot.z) };
			} ImGui::PopStyleVar();

			if (selection->no_cull) 
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.6f, 0.5f));
				ImGui::Widget_PrettyDragVec3("Scale", &selection->scale.x, true, 0.01f,
					-FLT_MAX, FLT_MAX, "Sx", "Sy", "Sz");
				ImGui::PopStyleVar();
			}
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
		if (ImGui::Button("Copy Settings to Clipboard##Cull", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
		{
			ImGui::LogToClipboard();
			ImGui::LogText("%s", common::toml::build_culling_overrides_string_for_current_map(areas).c_str());
			ImGui::LogFinish();
		} // end copy to clipboard

		ImGui::SameLine();
		reload_mapsettings_button_with_popup("Cull");
		ImGui::Spacing(0, 4);

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

		m_devgui_custom_footer_content = "Area: " + std::to_string(g_current_area) + "\nLeaf: " + std::to_string(g_current_leaf);
	}

	// #
	// #

	void cont_gamesettings_flashlight()
	{
		const auto gs = game_settings::get();

		ImGui::Widget_PrettyDragVec3("Offsets Player", gs->flashlight_offset_player.get_as<float*>(), true, 0.1f, -1000.0f, 1000.0f, "F", "H", "V");
		TT(gs->flashlight_offset_player.get_tooltip_string().c_str());

		ImGui::Widget_PrettyDragVec3("Offsets Bot", gs->flashlight_offset_bot.get_as<float*>(), true, 0.1f, -1000.0f, 1000.0f, "F", "H", "V");
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

		if (!ImGui::Begin("Devgui", &m_menu_active, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, &common::imgui::draw_window_blur_callback))
		{
			ImGui::End();
			return;
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
#endif

	void imgui::endscene_stub()
	{
#if USE_IMGUI
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

#if 0
					ImGui::GetIO().MouseDrawCursor = false;
					if (menu_state_changed) 
					{
						if (im->m_menu_active ^ interfaces::get()->m_surface->is_cursor_visible()) {
							interfaces::get()->m_engine->execute_client_cmd_unrestricted("debugsystemui");
						}
					}
#endif

					if (im->m_menu_active) {
						im->devgui();
					}

					ImGui::EndFrame();
					ImGui::Render();
					ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
				}
			}
		}
#endif
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
		colors[ImGuiCol_WindowBg] = ImVec4(0.26f, 0.26f, 0.26f, 0.90f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.28f, 0.28f, 0.28f, 0.92f);
		colors[ImGuiCol_Border] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.23f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.17f, 0.25f, 0.27f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.07f, 0.39f, 0.47f, 0.59f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.94f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 0.94f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.15f, 0.15f, 0.94f);
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
		colors[ImGuiCol_Header] = ImVec4(0.02f, 0.02f, 0.02f, 0.18f);
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

#if USE_IMGUI

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		init_fonts();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

		style_xo();

		ImGui_ImplWin32_Init(glob::main_window);
		g_game_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(glob::main_window, GWLP_WNDPROC, LONG_PTR(wnd_proc_hk)));
#endif
	}

	imgui::~imgui()
	{ }
}
