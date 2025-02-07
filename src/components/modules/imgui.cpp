#include "std_include.hpp"
#include "map_settings.hpp"
#include "components/common/imgui_helper.hpp"
#include "components/common/toml.hpp"

#ifdef USE_IMGUI
#include "imgui_internal.h"

// Allow us to directly call the ImGui WndProc function.
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

#define SPACING_INDENT_BEGIN ImGui::Spacing(); ImGui::Indent()
#define SPACING_INDENT_END ImGui::Spacing(); ImGui::Unindent()
#define TT(TXT) ImGui::SetItemTooltip((TXT));
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

	void imgui::tab_general()
	{
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
			if (ImGui::Checkbox("Z ignore Player", &im_zignore_player))
			{
				if (!im_zignore_player) {
					interfaces::get()->m_engine->execute_client_cmd_unrestricted("nb_vision_ignore_survivors 0");
				}
				else {
					interfaces::get()->m_engine->execute_client_cmd_unrestricted("nb_vision_ignore_survivors 1");
				}
			}

			SPACING_INDENT_END;
		}

		ImGui::Spacing(); ImGui::Spacing();

		if (ImGui::CollapsingHeader("Flashlight"))
		{
			SPACING_INDENT_BEGIN;

			ImGui::SeparatorText("Player");
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

				SPACING_INDENT_END;
			}

			ImGui::SeparatorText("Bots");
			{
				SPACING_INDENT_BEGIN;

				ImGui::PushID("bot");
				ImGui::DragFloat("Forward Offset", &m_flashlight_bot_fwd_offset, 0.1f);
				ImGui::DragFloat("Horizontal Offset", &m_flashlight_bot_horz_offset, 0.1f);
				ImGui::DragFloat("Vertical Offset", &m_flashlight_bot_vert_offset, 0.1f);
				ImGui::PopID();

				SPACING_INDENT_END;
			}

			SPACING_INDENT_END;
		}
	}

	void ms_fog()
	{
		if (ImGui::CollapsingHeader("Fog Settings"))
		{
			SPACING_INDENT_BEGIN;

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
				ImGui::LogText(toml_str.c_str());
				ImGui::LogFinish();
			}

			ImGui::SameLine();
			if (ImGui::Button("Reload MapSettings##Marker", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
			{
				if (!ImGui::IsPopupOpen("Reload MapSettings?")) {
					ImGui::OpenPopup("Reload MapSettings?");
				}
			}

			ImGui::TextDisabled("No auto-save or file writing. You need to export to clipboard and override the settings yourself!");
			ImGui::Spacing();
			ImGui::Spacing();

			bool fog_enabled = ms.fog_dist != 0.0f;
			if (ImGui::Checkbox("Enable Fog", &fog_enabled))
			{
				if (!fog_enabled) {
					ms.fog_dist = 0.0f;
				}
				else
				{
					ms.fog_dist = 15000.0f;
					if (ms.fog_color == 0xFFFFFFFF) {
						ms.fog_color = 0xFF646464;
					}
				}
			}

			if (ImGui::DragFloat("Distance", &ms.fog_dist, 1.0f, 0.0f)) {
				ms.fog_dist = ms.fog_dist < 0.0f ? 0.0f : ms.fog_dist;
			}

			if (ImGui::ColorEdit3("Transmission", &fog_color.x, ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_PickerHueBar)) {
				ms.fog_color = D3DCOLOR_COLORVALUE(fog_color.x, fog_color.y, fog_color.z, 1.0f);
			}

			SPACING_INDENT_END;

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Separator();
			SPACING_INDENT_END;
			ImGui::ItemSize(ImVec2(0, 10));
		}
	}

	void imgui::tab_map_settings()
	{
		{
			SPACING_INDENT_BEGIN;
			ImGui::Checkbox("Disable R_CullNode", &m_disable_cullnode);
			ImGui::Checkbox("Enable Area Forcing", &m_enable_area_forcing);

			bool im_area_debug_state = main_module::is_node_debug_enabled();
			if (ImGui::Checkbox("Toggle Area Debug Info", &im_area_debug_state)) {
				main_module::set_node_vis_info(im_area_debug_state);
			}

			ImGui::SliderInt2("HUD: Area Debug Position", &main_module::get()->m_hud_debug_node_vis_pos[0], 0, 512);

			{
				auto* default_nocull_dist = &map_settings::get_map_settings().default_nocull_dist;
				if (ImGui::DragFloat("Default NoCull Distance", default_nocull_dist, 0.5f, 0.0f)) {
					*default_nocull_dist = *default_nocull_dist < 0.0f ? 0.0f : *default_nocull_dist;
				}
				TT("Default distance value for the default anti-cull mode (distance) if there is no override for the current area");
			}
			

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			// fog settings
			ms_fog();

			ImGui::Spacing();

			if (ImGui::CollapsingHeader("Marker Manipulation"))
			{
				SPACING_INDENT_BEGIN;

				auto& markers = map_settings::get_map_settings().map_markers;
				if (ImGui::Button("Copy All Markers to Clipboard", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
				{
					ImGui::LogToClipboard();
					ImGui::LogText("%s", common::toml::build_map_marker_string_for_current_map(markers).c_str());
					ImGui::LogFinish();
				}

				ImGui::SameLine();
				if (ImGui::Button("Reload MapSettings##Marker", ImVec2(ImGui::GetContentRegionAvail().x, 0))) 
				{
					if (!ImGui::IsPopupOpen("Reload MapSettings?")) {
						ImGui::OpenPopup("Reload MapSettings?");
					}
				}

				ImGui::TextDisabled("No auto-save or file writing. You need to export to clipboard and override the settings yourself!");
				ImGui::Spacing();
				ImGui::Spacing();

				constexpr auto in_buflen = 1024u;
				static char in_area_buf[in_buflen], in_nleaf_buf[in_buflen];
				static map_settings::marker_settings_s* selection = nullptr;

				//
				// MARKER TABLE

				if (ImGui::BeginTable("MarkerTable", 9,
					ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody |
					ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_ScrollY, ImVec2(0, 150)))
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

#define IMGUI_FIX_CELL_Y_OFFSET(IS_SELECTED_V, SPOS) \
	if ((IS_SELECTED_V)) { ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, (SPOS))); }

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
							// never show selection
							if (ImGui::Selectable(utils::va("%d", i), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 21)))
							{
								selection = &m;
								m.imgui_is_selected = true;
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
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.05f, 0.05f, 0.8f));
							ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
							ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
							ImGui::PushID((int)i);

							const auto btn_size = ImVec2(16, is_selected ? (row_max_y_pos - save_row_min_y_pos) : 16.0f);
							if (ImGui::Button("x##Marker", btn_size)) 
							{
								marked_for_deletion = &m;
								main_module::trigger_vis_logic();
							}
							ImGui::PopStyleVar(2);
							ImGui::PopStyleColor();
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

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.3f, 0.05f, 0.7f));
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
				ImGui::PopStyleColor();

				if (selection)
				{
					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.3f, 0.1f, 0.8f));
					if (ImGui::Button("Duplicate Current Marker"))
					{
						markers.emplace_back(map_settings::marker_settings_s {
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
					ImGui::PopStyleColor();
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
					if (ImGui::DragInt("Marker Number", &temp_num, 0.1f, 0))
					{
						if (temp_num < 0) {
							temp_num = 0;
						}
						else if (!selection->no_cull && temp_num > 99) {
							temp_num = 99;
						}

						selection->index = (std::uint32_t)temp_num;
					}
					ImGui::DragFloat3("Marker Position", &selection->origin.x, 0.5f);

					// RAD2DEG -> DEG2RAD 
					Vector temp_rot = { RAD2DEG(selection->rotation.x), RAD2DEG(selection->rotation.y), RAD2DEG(selection->rotation.z) };
					if (ImGui::DragFloat3("Marker Rotation", &temp_rot.x, 0.1f)) {
						selection->rotation = { DEG2RAD(temp_rot.x), DEG2RAD(temp_rot.y), DEG2RAD(temp_rot.z) };
					}

					if (selection->no_cull) {
						ImGui::DragFloat3("Marker Scale", &selection->scale.x, 0.01f);
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

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Separator();
				SPACING_INDENT_END;
				ImGui::ItemSize(ImVec2(0, 10));
			}

			ImGui::Spacing();

			// #
			// CULLING Section

			if (ImGui::CollapsingHeader("Culling Manipulation"))
			{
				SPACING_INDENT_BEGIN;

				auto& areas = map_settings::get_map_settings().area_settings;
				if (ImGui::Button("Copy Settings to Clipboard##Cull", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
				{
					ImGui::LogToClipboard();
					ImGui::LogText("%s", common::toml::build_culling_overrides_string_for_current_map(areas).c_str());
					ImGui::LogFinish();
				} // end copy to clipboard

				ImGui::SameLine();
				if (ImGui::Button("Reload MapSettings##Cull", ImVec2(ImGui::GetContentRegionAvail().x, 0))) 
				{
					if (!ImGui::IsPopupOpen("Reload MapSettings?")) {
						ImGui::OpenPopup("Reload MapSettings?");
					}
				}

				ImGui::TextDisabled("No auto-save or file writing. You need to export to clipboard and override the settings yourself!");
				ImGui::SameLine();
				ImGui::Spacing();
				ImGui::Spacing();

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
				if (ImGui::BeginTable("CullTable", 6, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
					ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_ScrollY, ImVec2(0, 380)))
				{
					ImGui::TableSetupScrollFreeze(0, 1); // make top row always visible
					ImGui::TableSetupColumn("Ar", ImGuiTableColumnFlags_NoResize, 18.0f);
					ImGui::TableSetupColumn("Mode", ImGuiTableColumnFlags_WidthStretch, 34.0f);
					ImGui::TableSetupColumn("Leafs", ImGuiTableColumnFlags_WidthStretch, 80.0f);
					ImGui::TableSetupColumn("Areas", ImGuiTableColumnFlags_WidthStretch, 60.0f);
					ImGui::TableSetupColumn("Hide-Leafs", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultHide, 40.0f);
					ImGui::TableSetupColumn("LeafTweaks & HideAreas", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 140.0f);
					ImGui::TableHeadersRow();

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

						if (is_area_selected && !player_is_in_area) {
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
						} else if (player_is_in_area) {
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(ImVec4(0.05f, 0.1f, 0.35f, 0.15f))); // current
						}

						// -
						ImGui::TableNextColumn();

						float start_y = ImGui::GetCursorScreenPos().y; // save row start of selector at the end of a row
						ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg)); // set background for first column

						// - Area
						ImGui::Text("%d", (int)area_num);

						if (is_area_selected) {
							area_selection_matches_any_entry = true; // check that the selection ptr is up to date
						}

						// Mode
						ImGui::TableNextColumn();

						ImGui::PushID((int)area_num);
						ImGui::SetNextItemWidth(122.0f);
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
						TT(	"No Frustum:\t\t   compl. disable frustum culling (everywhere)\n"
							"No Frustum in Area:   compl. disable frustum culling when in current area\n"
							"Stock:\t\t\t    stock frustum culling\n"
							"Force Area:\t\t   ^ + force all nodes/leafs in current area\n"
							"Force Area Dist:\t  ^ + all outside of current area within certain dist to player\n"
							"Distance:\t\t\t force all nodes/leafs within certain dist to player");

						if (   a.cull_mode >= map_settings::AREA_CULL_INFO_NOCULLDIST_START
							&& a.cull_mode <= map_settings::AREA_CULL_INFO_NOCULLDIST_END)
						{
							if (ImGui::DragFloat("Dist##NocullDist", &a.nocull_distance, 0.1f, 0.0f, FLT_MAX, "%.2f"))
							{
								a.nocull_distance = a.nocull_distance < 0.0f ? 0.0f : a.nocull_distance;
								main_module::trigger_vis_logic();
							}
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
								ImGui::Widget_UnorderedSetModifier("CullAreas", ImGui::Widget_UnorderedSetModifierFlags_Leaf, area_selection->areas, in_areas_buf, in_buflen);
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

						bool any_tweak_with_nocull_override = false;
						if (!a.leaf_tweaks.empty())
						{
							// inline table for leaf tweaks
							if (ImGui::BeginTable("tweak_leafs_nested_table", 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
								ImGuiTableFlags_Reorderable | ImGuiTableFlags_ContextMenuInBody))
							{
								ImGui::TableSetupColumn("Tweak in Leafs", ImGuiTableColumnFlags_WidthStretch, 100.0f);
								ImGui::TableSetupColumn("Tweak Areas", ImGuiTableColumnFlags_WidthStretch, 100.0f);
								ImGui::TableSetupColumn("Tweak Leafs", ImGuiTableColumnFlags_WidthStretch, 100.0f);
								ImGui::TableSetupColumn("NoCullDist", ImGuiTableColumnFlags_WidthStretch, 44.0f);
								ImGui::TableSetupColumn("##Delete", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoClip, 16.0f);
								ImGui::TableHeadersRow();

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
											ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.05f, 0.05f, 0.8f));
											ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
											ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
											ImGui::PushID((int)cur_row);

											const auto btn_size = ImVec2(16, is_tweak_selected ? (leaftwk_row_max_y_pos - tweak_table_first_row_y_pos) : 18.0f);
											if (ImGui::Button("x##twkleaf", btn_size)) {
												marked_for_deletion = &lt;
											}

											ImGui::PopStyleVar(2);
											ImGui::PopStyleColor();
											ImGui::PopID();
										}

										if (is_area_selected && !is_tweak_selected)
										{
											float content_height = ImGui::GetCursorPosY() - twk_start_y + 2.0f;
											ImGui::SetCursorPosY(twk_start_y - 1.0f);

											// never show selection
											if (ImGui::Selectable(utils::va("##TwkSel%d", cur_row), false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, content_height))) {
												tweak_selection = &lt;
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
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.3f, 0.05f, 0.7f));
							if (ImGui::Button("++ Tweak Leaf Entry", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
								area_selection->leaf_tweaks.emplace_back();
							} ImGui::PopStyleColor();
						}

						// ---
						if (!a.hide_areas.empty())
						{
							ImGui::Spacing();

							// inline table for leaf tweaks
							if (ImGui::BeginTable("hide_areas_nested_table", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_ContextMenuInBody))
							{
								ImGui::TableSetupColumn("Areas", ImGuiTableColumnFlags_WidthStretch, 100.0f);
								ImGui::TableSetupColumn("NLeafs", ImGuiTableColumnFlags_WidthStretch, 60.0f);
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
											ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_TableRowBgAlt)) + ImVec4(0.1f, 0.1f, 0.1f, 0.0f)));
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
											ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.05f, 0.05f, 0.8f));
											ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
											ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
											ImGui::PushID((int)cur_row);

											const auto btn_size = ImVec2(16, is_hda_selected ? (ImGui::GetItemRectMax().y - hide_table_first_row_y_pos) : 18.0f);
											if (ImGui::Button("x##twkhidearea", btn_size)) {
												marked_for_deletion = &hda;
											}
											ImGui::PopStyleVar(2);
											ImGui::PopStyleColor();
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
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.3f, 0.05f, 0.7f)); 
							if (ImGui::Button("++ Tweak Hide Area Entry", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
								area_selection->hide_areas.emplace_back();
							} ImGui::PopStyleColor();
						}
						else
						{
							ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().y, start_y - (!row_num ? 0.0f : 5.0f)));
							const float content_height = row_max_y_pos - area_table_first_row_y_pos;

							// never show selection
							if (ImGui::Selectable(utils::va("##TEST%d", area_num), false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, content_height))) {
								area_selection = &a;
							}
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
				if (it == areas.end())
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.3f, 0.05f, 0.7f));
					if (ImGui::Button("Add current Area##Cull", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
					{
						areas.emplace((std::uint32_t)g_current_area, map_settings::area_overrides_s{
								.cull_mode = map_settings::AREA_CULL_MODE::AREA_CULL_INFO_DEFAULT,
								.nocull_distance = map_settings::get_map_settings().default_nocull_dist,
								.area_index = (std::uint32_t)g_current_area,
							});
					}
					ImGui::PopStyleColor(); 
				}

				if (area_selection)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.05f, 0.05f, 0.8f));
					if (ImGui::Button("Remove Selected Area Entry##Cull", ImVec2(ImGui::GetContentRegionAvail().x, 0))) 
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
					ImGui::PopStyleColor();
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
						"# Useful console command: 'xo_debug_toggle_node_vis'\n"
						"# Parameters ---------------------------------------------------------------------------------------------------------------------------------\n"
						"#\n"
						"# in_area ]                 the area the player has to be in    [int]\n"
						"#         |>       areas:   area/s with forced visibility       [int array]\n"
						"#         |>       leafs:   leaf/s with forced visibility       [int array]\n"
						"#         |>        cull:   [0] disable frustum culling\n"
						"#         |>                [1] frustum culling                 (still forces leaf/s)\n"
						"#         |>                [2] frustum culling (default)       (still forces leaf/s + forces all leafs in current area)\n"
						"#\n"
						"#                           # This can be used to disable frustum culling for specified areas when the player in specified leafs\n"
						"#                           # - Useful at area crossings when used in conjunction with nocull-area-specific markers that block visibility\n"
						"#         |>  leaf_tweak:   per leaf overrides                  [array of structure below]\n"
						"#         :: |> in_leafs:   the leaf/s the player has to be in  [int array]\n"
						"#         :: |>    areas:   area/s with forced visibility       [int array]\n"
						"#\n"
						"#                           # This can be used to forcefully cull parts of the map\n"
						"#         |>  hide_areas:   force hide area/s                   [array of structure below]\n"
						"#         :: |>    areas:   area/s to hide                      [int array]\n"
						"#         :: |>  N_leafs:   only hide area/s when NOT in leaf/s [int array]\n"
						"#         |>  hide_leafs:   force hide leaf/s                   [int array]\n");

					ImGui::TreePop();
				}

				//ImGui::TreePop();
				SPACING_INDENT_END;
			}

			SPACING_INDENT_END;
		}

		if (ImGui::BeginPopupModal("Reload MapSettings?", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
		{
			const auto half_width = ImGui::GetContentRegionMax().x * 0.5f;
			auto line1_str = "You'll loose all unsaved changes if you continue!";
			auto line2_str = "Use the copy to clipboard buttons and manually update";
			auto line3_str = "the map_settings.toml file if you've made changes.";

			ImGui::Spacing();
			ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line1_str).x * 0.5f));
			ImGui::Text(line1_str);

			ImGui::Spacing();
			ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line2_str).x * 0.5f));
			ImGui::Text(line2_str);
			ImGui::SetCursorPosX(5.0f + half_width - (ImGui::CalcTextSize(line3_str).x * 0.5f));
			ImGui::Text(line3_str);

			ImGui::Spacing();
			ImGui::Spacing();

			ImVec2 button_size(half_width - 10.0f, 0.0f);
			if (ImGui::Button("Reload", button_size)) 
			{
				map_settings::reload();
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel", button_size)) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		m_devgui_custom_footer_content = "Area: " + std::to_string(g_current_area) + "\nLeaf: " + std::to_string(g_current_leaf);
	}

	void imgui::devgui()
	{
		ImGui::SetNextWindowSize(ImVec2(900, 800), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Devgui", &m_menu_active/*, ImGuiWindowFlags_AlwaysVerticalScrollbar*/))
		{
			ImGui::End();
			return;
		}

		common::draw_background_blur(ImGui::GetWindowDrawList(), game::get_d3d_device());//

		m_im_window_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
		m_im_window_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);

		static bool im_demo_menu = false;

		if (im_demo_menu) {
			ImGui::ShowDemoWindow(&im_demo_menu);
		}

		/*ImGui::SameLine();
		ImGui::Text("Focused: %s", (m_im_window_focused ? "true" : "false"));

		ImGui::SameLine();
		ImGui::Spacing();

		ImGui::SameLine();
		ImGui::Text("Hovered: %s", (m_im_window_hovered ? "true" : "false"));*/

		
		

#define ADD_TAB(NAME, FUNC) \
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0)));			\
	if (ImGui::BeginTabItem(NAME)) {																		\
		if (ImGui::BeginChild("##child_" NAME, ImVec2(0, ImGui::GetContentRegionAvail().y - 38), ImGuiChildFlags_AlwaysUseWindowPadding )) {	\
			FUNC(); ImGui::EndChild();																		\
		} else {																							\
			ImGui::EndChild();																				\
		} ImGui::EndTabItem();																				\
	} ImGui::PopStyleColor();

		if (ImGui::BeginTabBar("devgui_tabs"))
		{
			ADD_TAB("General", tab_general);
			ADD_TAB("Map Settings", tab_map_settings)
			ImGui::EndTabBar();
		}

#undef ADD_TAB

		{

			ImGui::Separator();
			ImGui::Spacing();

			const char* movement_hint_str = "Press and Hold the Right Mouse Button outside ImGui to allow for Game Input";
			const auto avail_width = ImGui::GetContentRegionAvail().x;
			float cur_pos = avail_width - 50.0f;

			{
				const auto spos = ImGui::GetCursorScreenPos();
				ImGui::TextUnformatted(m_devgui_custom_footer_content.c_str());
				ImGui::SetCursorScreenPos(spos);
				m_devgui_custom_footer_content.clear();
			}
			

			ImGui::SetCursorPosX(cur_pos);
			if (ImGui::Button("Demo", ImVec2(50, 0))) {
				im_demo_menu = !im_demo_menu;
			}

			ImGui::SameLine();
			cur_pos = cur_pos - ImGui::CalcTextSize(movement_hint_str).x - 8.0f;
			ImGui::SetCursorPosX(cur_pos);
			ImGui::TextUnformatted(movement_hint_str);
		}

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

	imgui::imgui()
	{
		p_this = this;

#if USE_IMGUI

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

#if 1
		{
			ImGuiStyle* style = &ImGui::GetStyle();
			ImVec4* colors = style->Colors;

			style->WindowPadding = ImVec2(10, 6);
			style->FramePadding = ImVec2(6, 5);
			style->ItemSpacing = ImVec2(8, 4);
			style->IndentSpacing = 5;
			style->FrameRounding = 4.0f;
			style->GrabRounding = 4.0f;
			style->CellPadding = ImVec2(4, 4);

			colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.93f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.94f);
			colors[ImGuiCol_Border] = ImVec4(0.15f, 0.15f, 0.21f, 0.50f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.04f, 0.04f, 0.04f, 0.54f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.11f, 0.03f, 0.03f, 1.00f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.17f, 0.05f, 0.05f, 1.00f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.03f, 0.03f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.02f, 0.02f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.53f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.25f, 0.12f, 0.12f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
			colors[ImGuiCol_Button] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.10f, 0.02f, 0.02f, 0.66f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.05f, 0.05f, 0.49f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.05f, 0.05f, 1.00f);
			colors[ImGuiCol_Separator] = ImVec4(0.15f, 0.15f, 0.21f, 0.50f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.20f, 0.05f, 0.05f, 1.00f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.20f, 0.05f, 0.05f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.11f, 0.01f, 0.01f, 1.00f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.28f, 0.07f, 0.07f, 1.00f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.56f, 0.08f, 0.08f, 1.00f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.45f, 0.07f, 0.07f, 1.00f);
			colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.02f, 0.02f, 1.00f);
			colors[ImGuiCol_TabSelected] = ImVec4(0.50f, 0.12f, 0.12f, 1.00f);
			colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.56f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_TabDimmed] = ImVec4(0.06f, 0.02f, 0.02f, 1.00f);
			colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.02f, 0.10f, 0.30f, 1.00f);
			colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
			colors[ImGuiCol_PlotLines] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.15f, 0.10f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.79f, 0.45f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.32f, 0.00f, 1.00f);
			colors[ImGuiCol_TableHeaderBg] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
			colors[ImGuiCol_TableBorderStrong] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
			colors[ImGuiCol_TableBorderLight] = ImVec4(0.04f, 0.04f, 0.05f, 1.00f);
			colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.49f);
			colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.10f, 0.10f, 0.10f, 0.44f);
			colors[ImGuiCol_TextLink] = ImVec4(0.54f, 0.02f, 0.02f, 1.00f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.22f, 0.04f, 0.04f, 1.00f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
			colors[ImGuiCol_NavCursor] = ImVec4(0.05f, 0.31f, 0.96f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.60f, 0.60f, 0.60f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.60f, 0.60f, 0.60f, 0.35f);
		}
#endif

		ImGui_ImplWin32_Init(glob::main_window);
		g_game_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(glob::main_window, GWLP_WNDPROC, LONG_PTR(wnd_proc_hk)));
#endif
	}

	imgui::~imgui()
	{ }
}
