#include "std_include.hpp"
#include "map_settings.hpp"
#include <set>

#include "map_settings.hpp"

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
				interfaces::get()->m_engine->execute_client_cmd_unrestricted("kick rochelle; kick coach; kick ellis; kick roach");
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

	void imgui::tab_marker_culling()
	{
		//if (ImGui::CollapsingHeader("Culling / Rendering", ImGuiTreeNodeFlags_DefaultOpen))
		{
			SPACING_INDENT_BEGIN;
			ImGui::Checkbox("Disable R_CullNode", &m_disable_cullnode);
			ImGui::Checkbox("Enable Area Forcing", &m_enable_area_forcing);

			bool im_area_debug_state = main_module::is_node_debug_enabled();
			if (ImGui::Checkbox("Toggle Area Debug Info", &im_area_debug_state)) {
				main_module::set_node_vis_info(im_area_debug_state);
			}

			ImGui::SliderInt2("HUD: Area Debug Position", &main_module::get()->m_hud_debug_node_vis_pos[0], 0, 512);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			const auto txt_input_full = "Add/Remove..";
			const auto txt_input_full_width = ImGui::CalcTextSize(txt_input_full).x;
			const auto txt_input_min = "...";
			const auto txt_input_min_width = ImGui::CalcTextSize(txt_input_min).x;

			if (ImGui::CollapsingHeader("Marker Manipulation"))
			{
				SPACING_INDENT_BEGIN;

				auto& markers = map_settings::get_map_settings().map_markers;
				if (ImGui::Button("Copy All Markers to Clipboard", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
				{
					std::string toml_str = "    "s + map_settings::get_map_settings().mapname + " = [\n"s;
					for (auto& m : markers)
					{
						toml_str += "        { " + (m.no_cull ? "nocull = "s : "marker = "s) + std::to_string(m.index);

						if (m.no_cull)
						{
							toml_str += ", areas = [";
							for (auto it = m.areas.begin(); it != m.areas.end(); ++it)
							{
								if (it != m.areas.begin()) {
									toml_str += ", ";
								}
								toml_str += std::to_string(*it);
							}
							toml_str += "]";

							toml_str += ", N_leafs = [";
							for (auto it = m.when_not_in_leafs.begin(); it != m.when_not_in_leafs.end(); ++it)
							{
								if (it != m.when_not_in_leafs.begin()) {
									toml_str += ", ";
								}
								toml_str += std::to_string(*it);
							}
							toml_str += "]";
						}

						toml_str += ", position = [" + std::to_string(m.origin.x) + ", " + std::to_string(m.origin.y) + ", " + std::to_string(m.origin.z) + "]";
						toml_str += ", rotation = [" + std::to_string(RAD2DEG(m.rotation.x)) + ", " + std::to_string(RAD2DEG(m.rotation.y)) + ", " + std::to_string(RAD2DEG(m.rotation.z)) + "]";

						if (m.no_cull) {
							toml_str += ", scale = [" + std::to_string(m.scale.x) + ", " + std::to_string(m.scale.y) + ", " + std::to_string(m.scale.z) + "]";
						}

						toml_str += " },\n";
					}
					toml_str += "    ]";

					ImGui::LogToClipboard();
					ImGui::LogText(toml_str.c_str());
					ImGui::LogFinish();
				}

				ImGui::SameLine();
				if (ImGui::Button("Reload MapSettings##Marker", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
					map_settings::reload();
				}

				ImGui::TextDisabled("No auto-save or file writing. You need to export to clipboard and override the settings yourself!");
				ImGui::Spacing();
				ImGui::Spacing();


				static map_settings::marker_settings_s* selection = nullptr;
				constexpr auto in_buflen = 256u;
				static char in_area_buf[in_buflen], in_nleaf_buf[in_buflen];
				ImGuiInputTextFlags input_text_flags = /*ImGuiInputTextFlags_CharsScientific |*/ ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll;

				auto get_and_add_integers_to_marker = [](char* str, std::unordered_set<std::uint32_t>& set, bool clear_buf = true)
					{
						std::vector<int> temp_area;
						utils::extract_integer_words(str, temp_area, true);
						set.insert(temp_area.begin(), temp_area.end());

						if (clear_buf) {
							memset(str, 0, in_buflen);
						}
					};

				auto get_and_remove_integers_from_marker = [](char* str, std::unordered_set<std::uint32_t>& set, bool clear_buf = true)
					{
						std::vector<int> temp_area;
						utils::extract_integer_words(str, temp_area, true);

						for (const auto& v : temp_area) {
							set.erase(v);
						}

						if (clear_buf) {
							memset(str, 0, in_buflen);
						}
					};

				//
				// MARKER TABLE

				if (ImGui::BeginTable("MarkerTable", 9,
					ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
					ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_ScrollY, ImVec2(0, 240)))
				{
					ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoHide, 12.0f);
					ImGui::TableSetupColumn("Num", ImGuiTableColumnFlags_NoResize, 24.0f);
					ImGui::TableSetupColumn("NC", ImGuiTableColumnFlags_NoResize, 24.0f);
					ImGui::TableSetupColumn("Areas", ImGuiTableColumnFlags_WidthStretch, 80.0f);
					ImGui::TableSetupColumn("NLeafs", ImGuiTableColumnFlags_WidthStretch, 80.0f);
					ImGui::TableSetupColumn("Pos", ImGuiTableColumnFlags_WidthFixed, 200.0f);
					ImGui::TableSetupColumn("Rot", ImGuiTableColumnFlags_WidthFixed, 180.0f);
					ImGui::TableSetupColumn("Scale", ImGuiTableColumnFlags_WidthFixed, 130.0f);
					ImGui::TableSetupColumn("##Delete", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoClip, 16.0f);
					ImGui::TableHeadersRow();

					bool selection_matches_any_entry = false;
					map_settings::marker_settings_s* marked_for_deletion = nullptr;

#define IMGUI_FIX_CELL_Y_OFFSET(IS_SELECTED_V, SPOS) \
	if ((IS_SELECTED_V)) { ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, (SPOS))); }

					for (auto i = 0u; i < markers.size(); i++)
					{
						auto& m = markers[i];
						{
							// default selection
							if (!selection) {
								selection = &m;
							}

							ImGui::TableNextRow();
							// save Y offset because a multiline cell messes with following cells
							const auto table_first_row_y_pos = ImGui::GetCursorScreenPos().y - ImGui::GetStyle().FramePadding.y + ImGui::GetStyle().CellPadding.y;

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
								if (ImGui::Selectable(utils::va("%d", i), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
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

							// -
							ImGui::TableNextColumn();
							std::string area_str;
							for (auto it = m.areas.begin(); it != m.areas.end(); ++it)
							{
								if (it != m.areas.begin()) {
									area_str += ", ";
								} area_str += std::to_string(*it);
							}
							ImGui::TextUnformatted(area_str.c_str());

							// Area Input
							if (is_selected)
							{
								if (ImGui::Button("-##Area"))
								{
									get_and_remove_integers_from_marker(in_area_buf, selection->areas);
									main_module::trigger_vis_logic();
								}

								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								const auto spos = ImGui::GetCursorScreenPos();
								ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
								if (ImGui::InputText("##AreaMarker", in_area_buf, IM_ARRAYSIZE(in_area_buf), input_text_flags)) {
									get_and_add_integers_to_marker(in_area_buf, selection->areas);
								}

								ImGui::SetCursorScreenPos(spos);
								if (!in_area_buf[0])
								{
									const auto min_content_area_width = ImGui::GetContentRegionAvail().x - 40.0f;
									ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(8.0f, ImGui::CalcTextSize("A").y * 0.45f);
									if (min_content_area_width > txt_input_full_width) {
										ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_full);
									}
									else if (min_content_area_width > txt_input_min_width) {
										ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_min);
									}
								}

								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								if (ImGui::Button("+##Area")) {
									get_and_add_integers_to_marker(in_area_buf, selection->areas);
								}
								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								if (ImGui::Button("P##Areas"))
								{
									auto c_leaf_str = utils::va("%d", g_current_leaf);
									get_and_add_integers_to_marker((char*)c_leaf_str, selection->areas, false);
								} TT("Pick Current Area");
							} // End Area Input

							// -
							ImGui::TableNextColumn();
							IMGUI_FIX_CELL_Y_OFFSET(is_selected, table_first_row_y_pos);

							std::string nleaf_str;
							for (auto it = m.when_not_in_leafs.begin(); it != m.when_not_in_leafs.end(); ++it)
							{
								if (it != m.when_not_in_leafs.begin()) {
									nleaf_str += ", ";
								} nleaf_str += std::to_string(*it);
							}
							ImGui::TextUnformatted(nleaf_str.c_str());

#if 1
							// NLeaf Input
							if (is_selected)
							{
								if (ImGui::Button("-##NLeafs"))
								{
									get_and_remove_integers_from_marker(in_nleaf_buf, selection->when_not_in_leafs);
									main_module::trigger_vis_logic();
								}

								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								const auto spos = ImGui::GetCursorScreenPos();
								ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
								if (ImGui::InputText("##NLeafsMarker", in_nleaf_buf, IM_ARRAYSIZE(in_nleaf_buf), input_text_flags))
								{
									get_and_add_integers_to_marker(in_nleaf_buf, selection->when_not_in_leafs);
									main_module::trigger_vis_logic();
								}

								ImGui::SetCursorScreenPos(spos);
								if (!in_nleaf_buf[0])
								{
									const auto min_content_area_width = ImGui::GetContentRegionAvail().x - 40.0f;
									ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(8.0f, ImGui::CalcTextSize("A").y * 0.45f);
									if (min_content_area_width > txt_input_full_width) {
										ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_full);
									}
									else if (min_content_area_width > txt_input_min_width) {
										ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_min);
									}
								}

								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								if (ImGui::Button("+##NLeafs"))
								{
									get_and_add_integers_to_marker(in_nleaf_buf, selection->when_not_in_leafs);
									main_module::trigger_vis_logic();
								}
								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								if (ImGui::Button("P##Leafs"))
								{
									auto c_leaf_str = utils::va("%d", g_current_leaf);
									get_and_add_integers_to_marker((char*)c_leaf_str, selection->when_not_in_leafs, false);
								} TT("Pick Current Leaf");
							} // end Input

							const auto row_max_y_pos = ImGui::GetItemRectMax().y;
#endif
							// -
							ImGui::TableNextColumn();
							IMGUI_FIX_CELL_Y_OFFSET(is_selected, table_first_row_y_pos);
							ImGui::Text("%.2f, %.2f, %.2f", m.origin.x, m.origin.y, m.origin.z);

							// -
							ImGui::TableNextColumn();
							IMGUI_FIX_CELL_Y_OFFSET(is_selected, table_first_row_y_pos);
							ImGui::Text("%.2f, %.2f, %.2f", m.rotation.x, m.rotation.y, m.rotation.z);

							// -
							ImGui::TableNextColumn();
							IMGUI_FIX_CELL_Y_OFFSET(is_selected, table_first_row_y_pos);
							ImGui::Text("%.2f, %.2f, %.2f", m.scale.x, m.scale.y, m.scale.z);

							// Delete Button
							ImGui::TableNextColumn();
							{
								ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.05f, 0.05f, 0.8f));
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
								ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
								ImGui::PushID((int)i);

								const auto btn_size = ImVec2(16, is_selected ? (row_max_y_pos - table_first_row_y_pos - 8.0f) : 16.0f);
								if (ImGui::Button("x##Marker", btn_size)) {
									marked_for_deletion = &m;
								}
								ImGui::PopStyleVar(2);
								ImGui::PopStyleColor();
								ImGui::PopID();
							}
						}
					}

					if (!selection_matches_any_entry) {
						selection = nullptr;
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

				//ImGui::EndChild();

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
						markers.emplace_back(map_settings::marker_settings_s{
								selection->index, selection->origin, selection->no_cull, selection->rotation, selection->scale, selection->areas, selection->when_not_in_leafs
							});

						selection = &markers.back();
					}
					ImGui::PopStyleColor();
				}

				ImGui::SameLine();
				ImGui::BeginDisabled(!selection);
				{
					if (ImGui::Button("Teleport to Marker", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
						interfaces::get()->m_engine->execute_client_cmd_unrestricted(utils::va("noclip; setpos %.2f %.2f %.2f", selection->origin.x, selection->origin.y, selection->origin.z - 40.0f));
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
			ImGui::Spacing();


			// #
			// CULLING Section

			if (ImGui::CollapsingHeader("Culling Manipulation"))
			{
				SPACING_INDENT_BEGIN;

				auto& areas = map_settings::get_map_settings().area_settings;
				if (ImGui::Button("Copy Settings to Clipboard", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
				{
					// temp map to sort areas by area number
					std::map<int, map_settings::area_overrides_s> sorted_area_settings(areas.begin(), areas.end());

					std::string toml_str = "    "s + map_settings::get_map_settings().mapname + " = [\n"s;
					for (auto& [ar_num, a] : sorted_area_settings)
					{
						bool is_multiline = false;

						// {
						{
							const auto num_spaces = ar_num < 10 ? 2u : ar_num < 100 ? 1u : 0u;
							toml_str += "        { in_area = ";

							for (auto i = 0u; i < num_spaces; i++) {
								toml_str += " ";
							}

							toml_str += std::to_string(ar_num);
						}

						if (!a.areas.empty())
						{
							// temp set to sort areas
							std::set<int> sorted_areas(a.areas.begin(), a.areas.end());

							toml_str += ", areas = [";
							for (auto it = sorted_areas.begin(); it != sorted_areas.end(); ++it)
							{
								if (it != sorted_areas.begin()) {
									toml_str += ", ";
								}
								toml_str += std::to_string(*it);
							}
							toml_str += "]";
						}

						if (!a.leafs.empty())
						{
							// temp set to sort leafs
							std::set<int> sorted_leafs(a.leafs.begin(), a.leafs.end());

							auto leaf_count = 0u;
							toml_str += ", leafs = [\n                ";
							for (auto it = sorted_leafs.begin(); it != sorted_leafs.end(); ++it)
							{
								if (it != sorted_leafs.begin())
								{
									toml_str += ", ";

									// new line every X leafs
									if (!(leaf_count % 8)) 
									{
										toml_str += "\n                ";
										is_multiline = true;
									}
								}

								toml_str += std::to_string(*it);
								leaf_count++;
							}

							toml_str += "\n            ]";
						} // end leafs

						if (a.cull_mode != map_settings::AREA_CULL_MODE::AREA_CULL_MODE_DEFAULT) {
							toml_str += ", cull = " + std::to_string(a.cull_mode);
						}

						if (!a.leaf_tweaks.empty())
						{
							is_multiline = true;
							toml_str += ", leaf_tweak = [\n";
							for (auto& lf : a.leaf_tweaks)
							{
								if (!lf.areas.empty() && !lf.in_leafs.empty())
								{
									// temp set to sort in_leafs
									std::set<int> sorted_twk_inleafs(lf.in_leafs.begin(), lf.in_leafs.end());

									// {
									toml_str += "                { in_leafs = [";
									for (auto it = sorted_twk_inleafs.begin(); it != sorted_twk_inleafs.end(); ++it)
									{
										if (it != sorted_twk_inleafs.begin()) {
											toml_str += ", ";
										}
										toml_str += std::to_string(*it);
									}

									// temp set to sort areas
									std::set<int> sorted_twk_areas(lf.areas.begin(), lf.areas.end());

									toml_str += "], areas = [";
									for (auto it = sorted_twk_areas.begin(); it != sorted_twk_areas.end(); ++it)
									{
										if (it != sorted_twk_areas.begin()) {
											toml_str += ", ";
										}
										toml_str += std::to_string(*it);
									}
									// }
									toml_str += "] },\n";
								}
							}
							toml_str += "            ]";
						} // end leaf_tweaks

						if (!a.hide_areas.empty())
						{
							is_multiline = true;
							toml_str += ", hide_areas = [\n";
							for (auto& hidearea : a.hide_areas)
							{
								if (!hidearea.areas.empty())
								{
									// temp set to sort hide-areas
									std::set<int> sorted_hide_areas(hidearea.areas.begin(), hidearea.areas.end());

									// { 
									toml_str += "                { areas = [";
									for (auto it = sorted_hide_areas.begin(); it != sorted_hide_areas.end(); ++it)
									{
										if (it != sorted_hide_areas.begin()) {
											toml_str += ", ";
										}
										toml_str += std::to_string(*it);
									}
									toml_str += "]";

									if (!hidearea.when_not_in_leafs.empty())
									{
										// temp set to sort hide-nleafs
										std::set<int> sorted_hide_nleafs(hidearea.when_not_in_leafs.begin(), hidearea.when_not_in_leafs.end());

										toml_str += ", N_leafs = [";
										for (auto it = sorted_hide_nleafs.begin(); it != sorted_hide_nleafs.end(); ++it)
										{
											if (it != sorted_hide_nleafs.begin()) {
												toml_str += ", ";
											}
											toml_str += std::to_string(*it);
										}
										toml_str += "]";
									}
									// }
									toml_str += " },\n";
								}
							}
							toml_str += "            ]";
						} // end hide_areas

						if (!a.hide_leafs.empty())
						{
							// temp set to sort hide-leafs
							std::set<int> sorted_hide_leafs(a.hide_leafs.begin(), a.hide_leafs.end());


							toml_str += ", hide_leafs = [";
							for (auto it = sorted_hide_leafs.begin(); it != sorted_hide_leafs.end(); ++it)
							{
								if (it != sorted_hide_leafs.begin()) {
									toml_str += ", ";
								}
								toml_str += std::to_string(*it);
							}
							toml_str += "]";
						} // end hide_leafs

						// }
						toml_str += " },\n";

						if (is_multiline) {
							toml_str += "\n";
						}

					} // end loop
					toml_str += "    ]";

					ImGui::LogToClipboard();
					ImGui::LogText(toml_str.c_str());
					ImGui::LogFinish();
				} // end copy to clipboard

				ImGui::SameLine();
				if (ImGui::Button("Reload MapSettings##Cull", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
					map_settings::reload();
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

				constexpr auto in_buflen = 256u;
				static char in_leafs_buf[in_buflen], in_areas_buf[in_buflen], in_twk_in_leafs_buf[in_buflen], in_twk_areas_buf[in_buflen], in_hide_leafs_buf[in_buflen], in_hide_areas_buf[in_buflen], in_hide_nleafs_buf[in_buflen];

				auto get_and_add_integers_to_set = [](char* str, std::unordered_set<std::uint32_t>& set, bool clear_buf = true)
					{
						std::vector<int> temp_vec;
						utils::extract_integer_words(str, temp_vec, true);
						set.insert(temp_vec.begin(), temp_vec.end());
						if (clear_buf) {
							memset(str, 0, in_buflen);
						}
					};

				auto get_and_remove_integers_from_set = [](char* str, std::unordered_set<std::uint32_t>& set, bool clear_buf = true)
					{
						std::vector<int> temp_vec;
						utils::extract_integer_words(str, temp_vec, true);

						for (const auto& v : temp_vec) {
							set.erase(v);
						}

						if (clear_buf) {
							memset(str, 0, in_buflen);
						}
					};

				
				ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll;
				static float culltable_saved_row_y_pos_last_button = 0.0f;

				// # CULL TABLE
				if (ImGui::BeginTable("CullTable", 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
					ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_ScrollY, ImVec2(0, 580)))
				{
					ImGui::TableSetupScrollFreeze(0, 1); // make top row always visible
					ImGui::TableSetupColumn("Ar", ImGuiTableColumnFlags_NoResize, 18.0f);
					ImGui::TableSetupColumn("Leafs", ImGuiTableColumnFlags_WidthStretch, 120.0f);
					ImGui::TableSetupColumn("Areas", ImGuiTableColumnFlags_WidthStretch, 60.0f);
					ImGui::TableSetupColumn("Hide-Leafs", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultHide, 40.0f);
					ImGui::TableSetupColumn("LeafTweaks & HideAreas", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoHide, 140.0f);
					ImGui::TableHeadersRow();

					bool area_selection_matches_any_entry = false;

					for (auto& [area_num, a] : areas)
					{
						ImGui::TableNextRow();

						// default selection
						if (!area_selection)
						{
							area_selection = &a;
							culltable_saved_row_y_pos_last_button = 0.0f; // reset
						}

						// save Y offset because a multiline cell messes with following cells
						const auto area_table_first_row_y_pos = ImGui::GetCursorScreenPos().y - ImGui::GetStyle().FramePadding.y + ImGui::GetStyle().CellPadding.y;

						// handle row background color for selected entry
						const bool is_area_selected = area_selection && area_selection == &a;
						const bool player_is_in_area = g_player_current_area_override && g_player_current_area_override == &a;

						if (is_area_selected && !player_is_in_area) {
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
						} else if (player_is_in_area) {
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::ColorConvertFloat4ToU32(ImVec4(0.05f, 0.1f, 0.25f, 0.15f))); // current
						}

						// -
						ImGui::TableNextColumn();

						// save row start of selector at the end of a row
						float start_y = ImGui::GetCursorPosY();

						// set background for first column
						ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));

						// - Area
						ImGui::Text("%d", area_num);

						if (is_area_selected) {
							area_selection_matches_any_entry = true; // check that the selection ptr is up to date
						}

						// - Leafs
						ImGui::TableNextColumn();
						{
							std::string arr_str;
							for (auto it = a.leafs.begin(); it != a.leafs.end(); ++it)
							{
								if (it != a.leafs.begin()) {
									arr_str += ", ";
								} arr_str += std::to_string(*it);
							}
							ImGui::TextWrapped(arr_str.c_str());

							// Leaf Input
							if (is_area_selected)
							{
								// if leafs are wrapping
								if (ImGui::GetItemRectMax().y > culltable_saved_row_y_pos_last_button) {
									culltable_saved_row_y_pos_last_button = ImGui::GetItemRectMax().y;
								}

								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, culltable_saved_row_y_pos_last_button + ImGui::GetStyle().ItemSpacing.y));
								if (ImGui::Button("-##Leafs"))
								{
									get_and_remove_integers_from_set(in_leafs_buf, area_selection->leafs);
									main_module::trigger_vis_logic();
								}

								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								const auto spos = ImGui::GetCursorScreenPos();
								ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
								if (ImGui::InputText("##LeafsCull", in_leafs_buf, IM_ARRAYSIZE(in_leafs_buf), input_text_flags)) {
									get_and_add_integers_to_set(in_leafs_buf, area_selection->leafs);
								}

								ImGui::SetCursorScreenPos(spos);
								if (!in_leafs_buf[0])
								{
									const auto min_content_area_width = ImGui::GetContentRegionAvail().x - 40.0f;
									ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(8.0f, ImGui::CalcTextSize("A").y * 0.45f);
									if (min_content_area_width > txt_input_full_width) {
										ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_full);
									}
									else if (min_content_area_width > txt_input_min_width) {
										ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_min);
									}
								}

								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								if (ImGui::Button("+##Leafs")) {
									get_and_add_integers_to_set(in_leafs_buf, area_selection->leafs);
								}
								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								if (ImGui::Button("P##Leafs")) 
								{
									auto c_leaf_str = utils::va("%d", g_current_leaf);
									get_and_add_integers_to_set((char*)c_leaf_str, area_selection->leafs, false);
								} TT("Pick Current Leaf");
							} // End Leaf Input
						}

						// - Areas
						ImGui::TableNextColumn();
						IMGUI_FIX_CELL_Y_OFFSET(is_area_selected, area_table_first_row_y_pos);
						{
							std::string arr_str;
							for (auto it = a.areas.begin(); it != a.areas.end(); ++it)
							{
								if (it != a.areas.begin()) {
									arr_str += ", ";
								} arr_str += std::to_string(*it);
							}
							ImGui::TextWrapped(arr_str.c_str());

							// Area Input
							if (is_area_selected)
							{
								// if areas are wrapping
								if (ImGui::GetItemRectMax().y > culltable_saved_row_y_pos_last_button) {
									culltable_saved_row_y_pos_last_button = ImGui::GetItemRectMax().y;
								}

								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, culltable_saved_row_y_pos_last_button + ImGui::GetStyle().ItemSpacing.y));
								if (ImGui::Button("-##AreasCull"))
								{
									get_and_remove_integers_from_set(in_areas_buf, area_selection->areas);
									main_module::trigger_vis_logic();
								}

								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								const auto spos = ImGui::GetCursorScreenPos();
								ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
								if (ImGui::InputText("##AreasCull", in_areas_buf, IM_ARRAYSIZE(in_areas_buf), input_text_flags)) {
									get_and_add_integers_to_set(in_areas_buf, area_selection->areas);
								}

								ImGui::SetCursorScreenPos(spos);
								if (!in_areas_buf[0])
								{
									const auto min_content_area_width = ImGui::GetContentRegionAvail().x - 40.0f;
									ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(8.0f, ImGui::CalcTextSize("A").y * 0.45f);
									if (min_content_area_width > txt_input_full_width) {
										ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_full);
									}
									else if (min_content_area_width > txt_input_min_width) {
										ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_min);
									}
								}

								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								if (ImGui::Button("+##AreasCull")) {
									get_and_add_integers_to_set(in_areas_buf, area_selection->areas);
								}
								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								if (ImGui::Button("P##AreasCull"))
								{
									auto c_leaf_str = utils::va("%d", g_current_leaf);
									get_and_add_integers_to_set((char*)c_leaf_str, area_selection->areas, false);
								} TT("Pick Current Area");
							} // End Area Input
						}

						// - Hide Leafs
						ImGui::TableNextColumn(); 
						IMGUI_FIX_CELL_Y_OFFSET(is_area_selected, area_table_first_row_y_pos);
						{
							std::string arr_str;
							for (auto it = a.hide_leafs.begin(); it != a.hide_leafs.end(); ++it)
							{
								if (it != a.hide_leafs.begin()) {
									arr_str += ", ";
								} arr_str += std::to_string(*it);
							}
							ImGui::TextWrapped(arr_str.c_str());

							// Hide Leafs Input
							if (is_area_selected)
							{
								// if leafs are wrapping
								if (ImGui::GetItemRectMax().y > culltable_saved_row_y_pos_last_button) {
									culltable_saved_row_y_pos_last_button = ImGui::GetItemRectMax().y;
								}

								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, culltable_saved_row_y_pos_last_button + ImGui::GetStyle().ItemSpacing.y));
								if (ImGui::Button("-##HLeafs"))
								{
									get_and_remove_integers_from_set(in_hide_leafs_buf, area_selection->hide_leafs);
									main_module::trigger_vis_logic();
								}

								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								const auto spos = ImGui::GetCursorScreenPos();
								ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
								if (ImGui::InputText("##HLeafsCull", in_hide_leafs_buf, IM_ARRAYSIZE(in_hide_leafs_buf), input_text_flags)) {
									get_and_add_integers_to_set(in_hide_leafs_buf, area_selection->hide_leafs);
								}

								ImGui::SetCursorScreenPos(spos);
								if (!in_hide_leafs_buf[0])
								{
									const auto min_content_area_width = ImGui::GetContentRegionAvail().x - 40.0f;
									ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(8.0f, ImGui::CalcTextSize("A").y * 0.45f);
									if (min_content_area_width > txt_input_full_width) {
										ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_full);
									}
									else if (min_content_area_width > txt_input_min_width) {
										ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_min);
									}
								}

								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								if (ImGui::Button("+##HLeafs")) {
									get_and_add_integers_to_set(in_hide_leafs_buf, area_selection->hide_leafs);
								}
								ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
								if (ImGui::Button("P##HLeafs"))
								{
									auto c_leaf_str = utils::va("%d", g_current_leaf);
									get_and_add_integers_to_set((char*)c_leaf_str, area_selection->hide_leafs, false);
								} TT("Pick Current Leaf");
							} // End Hide Leafs Input
						}

						// - tweak leafs + hide_areas
						ImGui::TableNextColumn();

						if (!a.leaf_tweaks.empty())
						{
							// inline table for leaf tweaks
							if (ImGui::BeginTable("tweak_leafs_nested_table", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
								ImGuiTableFlags_Reorderable | ImGuiTableFlags_ContextMenuInBody))
							{
								ImGui::TableSetupColumn("Tweak in Leafs", ImGuiTableColumnFlags_WidthStretch, 100.0f);
								ImGui::TableSetupColumn("Tweak Areas", ImGuiTableColumnFlags_WidthStretch, 60.0f);
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
									// Twk Leafs
									ImGui::TableNextColumn();

									// save row start of selector at the end of a row
									float twk_start_y = ImGui::GetCursorPosY();

									{
										std::string arr_str;
										for (auto it = lt.in_leafs.begin(); it != lt.in_leafs.end(); ++it)
										{
											if (it != lt.in_leafs.begin()) {
												arr_str += ", ";
											} arr_str += std::to_string(*it);
										}

										if (arr_str.empty()) {
											arr_str = "// Empty";
										}
										ImGui::TextUnformatted(arr_str.c_str());

										// Input
										if (is_tweak_selected)
										{
											if (ImGui::Button("-##TwkLeafs"))
											{
												get_and_remove_integers_from_set(in_twk_in_leafs_buf, tweak_selection->in_leafs);
												main_module::trigger_vis_logic();
											}

											ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
											const auto spos = ImGui::GetCursorScreenPos();
											ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
											if (ImGui::InputText("##TwkLeafsInput", in_twk_in_leafs_buf, IM_ARRAYSIZE(in_twk_in_leafs_buf), input_text_flags)) {
												get_and_add_integers_to_set(in_twk_in_leafs_buf, tweak_selection->in_leafs);
											}

											ImGui::SetCursorScreenPos(spos);
											if (!in_twk_in_leafs_buf[0])
											{
												const auto min_content_area_width = ImGui::GetContentRegionAvail().x - 40.0f;
												ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(8.0f, ImGui::CalcTextSize("A").y * 0.45f);
												if (min_content_area_width > txt_input_full_width) {
													ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_full);
												}
												else if (min_content_area_width > txt_input_min_width) {
													ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_min);
												}
											}

											ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
											if (ImGui::Button("+##TwkLeafs")) {
												get_and_add_integers_to_set(in_twk_in_leafs_buf, tweak_selection->in_leafs);
											}
											ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
											if (ImGui::Button("P##TwkLeafs"))
											{
												auto c_leaf_str = utils::va("%d", g_current_leaf);
												get_and_add_integers_to_set((char*)c_leaf_str, tweak_selection->in_leafs, false);
											} TT("Pick Current Leaf");
										} // End Input
									}

									// Twk Areas
									ImGui::TableNextColumn();
									IMGUI_FIX_CELL_Y_OFFSET(is_tweak_selected, tweak_table_first_row_y_pos);
									{
										std::string arr_str;
										for (auto it = lt.areas.begin(); it != lt.areas.end(); ++it)
										{
											if (it != lt.areas.begin()) {
												arr_str += ", ";
											} arr_str += std::to_string(*it);
										}

										if (arr_str.empty()) {
											arr_str = "// Empty";
										}
										ImGui::TextUnformatted(arr_str.c_str());

										// Input
										if (is_tweak_selected)
										{
											if (ImGui::Button("-##TwkAreas"))
											{
												get_and_remove_integers_from_set(in_twk_areas_buf, tweak_selection->areas);
												main_module::trigger_vis_logic();
											}

											ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
											const auto spos = ImGui::GetCursorScreenPos();
											ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
											if (ImGui::InputText("##TwkAreasInput", in_twk_areas_buf, IM_ARRAYSIZE(in_twk_areas_buf), input_text_flags)) {
												get_and_add_integers_to_set(in_twk_areas_buf, tweak_selection->areas);
											}

											ImGui::SetCursorScreenPos(spos);
											if (!in_twk_areas_buf[0])
											{
												const auto min_content_area_width = ImGui::GetContentRegionAvail().x - 40.0f;
												ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(8.0f, ImGui::CalcTextSize("A").y * 0.45f);
												if (min_content_area_width > txt_input_full_width) {
													ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_full);
												}
												else if (min_content_area_width > txt_input_min_width) {
													ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_min);
												}
											}

											ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
											if (ImGui::Button("+##TwkAreas")) {
												get_and_add_integers_to_set(in_twk_areas_buf, tweak_selection->areas);
											}
											ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
											if (ImGui::Button("P##TwkAreas"))
											{
												auto c_area_str = utils::va("%d", g_current_area);
												get_and_add_integers_to_set((char*)c_area_str, tweak_selection->areas, false);
											} TT("Pick Current Area");
										} // End Input
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

											const auto btn_size = ImVec2(16, is_tweak_selected ? (ImGui::GetItemRectMax().y - tweak_table_first_row_y_pos - 6.0f) : 16.0f);
											if (ImGui::Button("x##twkleaf", btn_size)) {
												marked_for_deletion = &lt;
											}
											ImGui::PopStyleVar(2);
											ImGui::PopStyleColor();
											ImGui::PopID();
										}

										if (is_area_selected && !is_tweak_selected)
										{
											float content_height = ImGui::GetCursorPosY() - twk_start_y - 4.0f;
											ImGui::SetCursorPosY(twk_start_y);

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
											culltable_saved_row_y_pos_last_button = 0.0f;
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
										std::string arr_str;
										for (auto it = hda.areas.begin(); it != hda.areas.end(); ++it)
										{
											if (it != hda.areas.begin()) {
												arr_str += ", ";
											} arr_str += std::to_string(*it);
										}

										if (arr_str.empty()) {
											arr_str = "// Empty";
										}
										ImGui::TextUnformatted(arr_str.c_str());

										// Input
										if (is_hda_selected)
										{
											if (ImGui::Button("-##HideArea"))
											{
												get_and_remove_integers_from_set(in_hide_areas_buf, hidearea_selection->areas);
												main_module::trigger_vis_logic();
											}

											ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
											const auto spos = ImGui::GetCursorScreenPos();
											ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
											if (ImGui::InputText("##HideAreaInput", in_hide_areas_buf, IM_ARRAYSIZE(in_hide_areas_buf), input_text_flags)) {
												get_and_add_integers_to_set(in_hide_areas_buf, hidearea_selection->areas);
											}

											ImGui::SetCursorScreenPos(spos);
											if (!in_hide_areas_buf[0])
											{
												const auto min_content_area_width = ImGui::GetContentRegionAvail().x - 40.0f;
												ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(8.0f, ImGui::CalcTextSize("A").y * 0.45f);
												if (min_content_area_width > txt_input_full_width) {
													ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_full);
												}
												else if (min_content_area_width > txt_input_min_width) {
													ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_min);
												}
											}

											ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
											if (ImGui::Button("+##HideArea")) {
												get_and_add_integers_to_set(in_hide_areas_buf, hidearea_selection->areas);
											}
											ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
											if (ImGui::Button("P##HideArea"))
											{
												auto c_area_str = utils::va("%d", g_current_area);
												get_and_add_integers_to_set((char*)c_area_str, hidearea_selection->areas, false);
											} TT("Pick Current Area");
										} // End Input
									}

									// Not in Leafs
									ImGui::TableNextColumn();
									IMGUI_FIX_CELL_Y_OFFSET(is_hda_selected, hide_table_first_row_y_pos);
									{
										std::string arr_str;
										for (auto it = hda.when_not_in_leafs.begin(); it != hda.when_not_in_leafs.end(); ++it)
										{
											if (it != hda.when_not_in_leafs.begin()) {
												arr_str += ", ";
											} arr_str += std::to_string(*it);
										}

										if (arr_str.empty()) {
											arr_str = "// Empty";
										}
										ImGui::TextUnformatted(arr_str.c_str());

										// Input
										if (is_hda_selected)
										{
											if (ImGui::Button("-##NLeafs")) {
												get_and_remove_integers_from_set(in_hide_nleafs_buf, hidearea_selection->when_not_in_leafs);
											}

											ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
											const auto spos = ImGui::GetCursorScreenPos();
											ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
											if (ImGui::InputText("##NLeafsInput", in_hide_nleafs_buf, IM_ARRAYSIZE(in_hide_nleafs_buf), input_text_flags)) {
												get_and_add_integers_to_set(in_hide_nleafs_buf, hidearea_selection->when_not_in_leafs);
											}

											ImGui::SetCursorScreenPos(spos);
											if (!in_hide_nleafs_buf[0])
											{
												const auto min_content_area_width = ImGui::GetContentRegionAvail().x - 40.0f;
												ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2(8.0f, ImGui::CalcTextSize("A").y * 0.45f);
												if (min_content_area_width > txt_input_full_width) {
													ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_full);
												}
												else if (min_content_area_width > txt_input_min_width) {
													ImGui::GetWindowDrawList()->AddText(pos, ImGui::GetColorU32(ImGuiCol_TextDisabled), txt_input_min);
												}
											}

											ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
											if (ImGui::Button("+##NLeafs")) {
												get_and_add_integers_to_set(in_hide_nleafs_buf, hidearea_selection->when_not_in_leafs);
											}
											ImGui::SetCursorScreenPos(ImVec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y));
											if (ImGui::Button("P##NLeafs"))
											{
												auto c_leaf_str = utils::va("%d", g_current_leaf);
												get_and_add_integers_to_set((char*)c_leaf_str, hidearea_selection->when_not_in_leafs, false);
											} TT("Pick Current Leaf");
										} // End Input
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

											const auto btn_size = ImVec2(16, is_hda_selected ? (ImGui::GetItemRectMax().y - hide_table_first_row_y_pos - 6.0f) : 16.0f);
											if (ImGui::Button("x##twkhidearea", btn_size)) {
												marked_for_deletion = &hda;
											}
											ImGui::PopStyleVar(2);
											ImGui::PopStyleColor();
											ImGui::PopID();
										}

										if (is_area_selected && !is_hda_selected)
										{
											float content_height = ImGui::GetCursorPosY() - hda_start_y - 4.0f;
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
											culltable_saved_row_y_pos_last_button = 0.0f;
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

						if (is_area_selected)
						{
							//if (ImGui::GetItemRectMax().y > culltable_saved_row_y_pos_last_button) { 
								culltable_saved_row_y_pos_last_button = ImGui::GetItemRectMax().y;
							//}
							//culltable_saved_row_y_pos_last_button = ImGui::GetItemRectMax().y;

							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.3f, 0.05f, 0.7f)); 
							if (ImGui::Button("++ Tweak Hide Area Entry", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
								area_selection->hide_areas.emplace_back();
							} ImGui::PopStyleColor();
						}
						else
						{
							float content_height = ImGui::GetCursorPosY() - start_y;
							ImGui::SetCursorPosY(start_y);

							// never show selection
							if (ImGui::Selectable(utils::va("##TEST%d", area_num), false, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, content_height))) {
								area_selection = &a;
							}
						}
					} // table end for loop

					if (!area_selection_matches_any_entry) 
					{
						// re-check for new selection (moving towards the start of the table)
						for (auto& [a_num, a] : areas)
						{
							if (area_selection && area_selection == &a) 
							{
								area_selection_matches_any_entry = true;
								culltable_saved_row_y_pos_last_button = 0.0f; 
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
								.cull_mode = map_settings::AREA_CULL_MODE::AREA_CULL_MODE_DEFAULT,
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
			ADD_TAB("Marker / Culling", tab_marker_culling)
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
		//ImGui::StyleColorsDark();

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
