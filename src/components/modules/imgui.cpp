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

	bool imgui::input_message(const UINT message_type, const WPARAM wparam, const LPARAM lparam)
	{
		if (message_type == WM_KEYUP && wparam == VK_F5) 
		{
			m_menu_active = !m_menu_active;

			// reset cursor to center when closing the menu to not affect player angles
			if (interfaces::get()->m_surface->is_cursor_visible() && !m_menu_active) {
				center_cursor(); 
			}

			interfaces::get()->m_surface->set_cursor_always_visible(m_menu_active);
		}

		if (m_menu_active)
		{
			auto& io = ImGui::GetIO();

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
			ImGui::Checkbox("Enable 3D Sky", &m_enable_3dsky);

			bool im_area_debug_state = main_module::is_node_debug_enabled();
			if (ImGui::Checkbox("Toggle Area Debug Info", &im_area_debug_state)) {
				main_module::set_node_vis_info(im_area_debug_state);
			}

			ImGui::SliderInt2("HUD: Area Debug Position", &main_module::get()->m_hud_debug_node_vis_pos[0], 0, 512);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			//if (ImGui::TreeNodeEx("Marker Manipulation", ImGuiTreeNodeFlags_FramePadding))
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

				if (ImGui::BeginTable("MarkerTable", 8, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoSavedSettings /*| ImGuiTableFlags_ScrollY, ImVec2(0, 200)*/))
				{
					ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoResize, 12.0f);
					ImGui::TableSetupColumn("Num", ImGuiTableColumnFlags_NoResize, 24.0f);
					ImGui::TableSetupColumn("NC", ImGuiTableColumnFlags_NoResize, 24.0f);
					ImGui::TableSetupColumn("Areas", ImGuiTableColumnFlags_WidthStretch, 80.0f);
					ImGui::TableSetupColumn("NLeafs", ImGuiTableColumnFlags_WidthStretch, 80.0f);
					ImGui::TableSetupColumn("Pos", ImGuiTableColumnFlags_WidthFixed, 200.0f);
					ImGui::TableSetupColumn("Rot", ImGuiTableColumnFlags_WidthFixed, 180.0f);
					ImGui::TableSetupColumn("Scale", ImGuiTableColumnFlags_WidthFixed, 130.0f);
					ImGui::TableHeadersRow();

					bool selection_matches_any_entry = false;

					for (auto i = 0u; i < markers.size(); i++)
					{
						auto& m = markers[i];
						//if (m.no_cull)
						{
							// default selection
							if (!selection) {
								selection = &m;
							}

							ImGui::TableNextRow();
							ImGui::TableNextColumn();

							if (ImGui::Selectable(utils::va("%d", i), selection && selection == &m, ImGuiSelectableFlags_SpanAllColumns))
							{
								selection = &m;
								m.imgui_is_selected = true;
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

							// -
							ImGui::TableNextColumn();
							std::string nleaf_str;
							for (auto it = m.when_not_in_leafs.begin(); it != m.when_not_in_leafs.end(); ++it)
							{
								if (it != m.when_not_in_leafs.begin()) {
									nleaf_str += ", ";
								} nleaf_str += std::to_string(*it);
							}
							ImGui::TextUnformatted(nleaf_str.c_str());

							// -
							ImGui::TableNextColumn();
							ImGui::Text("%.2f, %.2f, %.2f", m.origin.x, m.origin.y, m.origin.z);

							// -
							ImGui::TableNextColumn();
							ImGui::Text("%.2f, %.2f, %.2f", m.rotation.x, m.rotation.y, m.rotation.z);

							// -
							ImGui::TableNextColumn();
							ImGui::Text("%.2f, %.2f, %.2f", m.scale.x, m.scale.y, m.scale.z);
						}
					}

					if (!selection_matches_any_entry) {
						selection = nullptr;
					}
					else if (selection) {
						game::debug_add_text_overlay(&selection->origin.x, "[ImGui] Selected Marker", 0, 0.8f, 1.0f, 0.3f, 0.8f);
					}
					ImGui::EndTable();


					ImGui::SeparatorText("Modify Marker");
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

					auto get_and_add_integers_to_marker = [](const char* str, std::unordered_set<std::uint32_t>& set)
						{
							std::vector<int> temp_area;
							utils::extract_integer_words(str, temp_area, true);
							set.insert(temp_area.begin(), temp_area.end());
						};

					auto get_and_remove_integers_from_marker = [](const char* str, std::unordered_set<std::uint32_t>& set)
						{
							std::vector<int> temp_area;
							utils::extract_integer_words(str, temp_area, true);

							for (const auto& v : temp_area) {
								set.erase(v);
							}
						};

					static char in_area_buf[256], in_nleaf_buf[256];
					ImGuiInputTextFlags input_text_flags = /*ImGuiInputTextFlags_CharsScientific |*/ ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll;

					{
						if (ImGui::Button("+##Area")) {
							get_and_add_integers_to_marker(in_area_buf, selection->areas);
						}

						ImGui::SameLine();
						if (ImGui::Button("-##Area"))
						{
							get_and_remove_integers_from_marker(in_area_buf, selection->areas);
							main_module::trigger_vis_logic();
						}

						ImGui::SameLine(0, 8);
						ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - ((ImGui::GetItemRectSize().x + ImGui::GetStyle().ItemSpacing.x) * 2.0f));
						ImGui::InputText("Areas (sep. by spaces)##Marker", in_area_buf, IM_ARRAYSIZE(in_area_buf), input_text_flags);
					}

					{
						if (ImGui::Button("+##NLeafs")) {
							get_and_add_integers_to_marker(in_nleaf_buf, selection->when_not_in_leafs);
						}
						ImGui::SameLine();
						if (ImGui::Button("-##NLeafs"))
						{
							get_and_remove_integers_from_marker(in_nleaf_buf, selection->when_not_in_leafs);
							main_module::trigger_vis_logic();
						}

						ImGui::SameLine();
						ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - ((ImGui::GetItemRectSize().x + ImGui::GetStyle().ItemSpacing.x) * 2.0f));
						ImGui::InputText("NLeafs (sep. by spaces)", in_nleaf_buf, IM_ARRAYSIZE(in_nleaf_buf), input_text_flags);
					}

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();
					ImGui::Spacing();

					ImGui::BeginGroup();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.1f, 0.8f));
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

						ImGui::SameLine();
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.05f, 0.05f, 0.8f));
						if (ImGui::Button("X Current Marker"))
						{
							for (auto it = markers.begin(); it != markers.end(); ++it)
							{
								if (&*it == selection)
								{
									markers.erase(it);
									selection = nullptr;
									break;
								}
							}
						}
						ImGui::PopStyleColor();
					}
					//auto group_width = ImGui::GetItemRectSize().x;
					ImGui::EndGroup();

					ImGui::SameLine();
					ImGui::BeginDisabled(!selection);
					{
						if (ImGui::Button("Teleport to Marker", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
							interfaces::get()->m_engine->execute_client_cmd_unrestricted(utils::va("noclip; setpos %.2f %.2f %.2f", selection->origin.x, selection->origin.y, selection->origin.z - 40.0f));
						}
						ImGui::EndDisabled();
					}
				}

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Separator();
				SPACING_INDENT_END;
				ImGui::ItemSize(ImVec2(0, 10));
			}

			ImGui::Spacing();
			ImGui::Spacing();

			//if (ImGui::TreeNodeEx("Culling Manipulation", ImGuiTreeNodeFlags_FramePadding))
			if (ImGui::CollapsingHeader("Culling Manipulation"))
			{
				SPACING_INDENT_BEGIN;

				auto& areas = map_settings::get_map_settings().area_settings;
				if (ImGui::Button("Copy Settings to Clipboard", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0)))
				{
					std::string toml_str = "    "s + map_settings::get_map_settings().mapname + " = [\n"s;
					for (auto& [ar_num, a] : areas)
					{
						// {
						toml_str += "        { in_area = " + std::to_string(ar_num);

						if (!a.areas.empty())
						{
							toml_str += ", areas = [";
							for (auto it = a.areas.begin(); it != a.areas.end(); ++it)
							{
								if (it != a.areas.begin()) {
									toml_str += ", ";
								}
								toml_str += std::to_string(*it);
							}
							toml_str += "]";
						}

						if (!a.leafs.empty())
						{
							toml_str += ", leafs = [";
							for (auto it = a.leafs.begin(); it != a.leafs.end(); ++it)
							{
								if (it != a.leafs.begin()) {
									toml_str += ", ";
								}
								toml_str += std::to_string(*it);
							}
							toml_str += "]";
						}

						if (!a.leaf_tweaks.empty())
						{
							toml_str += ", leaf_tweak = [\n";
							for (auto& lf : a.leaf_tweaks)
							{
								if (!lf.areas.empty() && !lf.in_leafs.empty())
								{
									// {
									toml_str += "            { in_leafs = [";
									for (auto it = lf.in_leafs.begin(); it != lf.in_leafs.end(); ++it)
									{
										if (it != lf.in_leafs.begin()) {
											toml_str += ", ";
										}
										toml_str += std::to_string(*it);
									}

									toml_str += "], areas = [";
									for (auto it = lf.areas.begin(); it != lf.areas.end(); ++it)
									{
										if (it != lf.areas.begin()) {
											toml_str += ", ";
										}
										toml_str += std::to_string(*it);
									}
									// }
									toml_str += "]},\n";
								}
							}
							toml_str += "        ]";
						}

						if (!a.hide_leafs.empty())
						{
							toml_str += ", hide_leafs = [";
							for (auto it = a.hide_leafs.begin(); it != a.hide_leafs.end(); ++it)
							{
								if (it != a.hide_leafs.begin()) {
									toml_str += ", ";
								}
								toml_str += std::to_string(*it);
							}
							toml_str += "]";
						}
						
						if (!a.hide_areas.empty())
						{
							toml_str += ", hide_areas = [\n";
							for (auto& lf : a.hide_areas)
							{
								if (!lf.areas.empty())
								{
									// { 
									toml_str += "            { areas = [";
									for (auto it = lf.areas.begin(); it != lf.areas.end(); ++it)
									{
										if (it != lf.areas.begin()) {
											toml_str += ", ";
										}
										toml_str += std::to_string(*it);
									}
									toml_str += "]";

									if (!lf.when_not_in_leafs.empty())
									{
										toml_str += ", N_leafs = [";
										for (auto it = lf.when_not_in_leafs.begin(); it != lf.when_not_in_leafs.end(); ++it)
										{
											if (it != lf.when_not_in_leafs.begin()) {
												toml_str += ", ";
											}
											toml_str += std::to_string(*it);
										}
										toml_str += "]";
									}
									// }
									toml_str += " }";
								}
							}
						}
						// }
						toml_str += " },\n";
					}
					toml_str += "    ]";

					ImGui::LogToClipboard();
					ImGui::LogText(toml_str.c_str());
					ImGui::LogFinish();
				}

				ImGui::SameLine();
				if (ImGui::Button("Reload MapSettings##Cull", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
					map_settings::reload();
				}

				ImGui::TextDisabled("No auto-save or file writing. You need to export to clipboard and override the settings yourself!");
				ImGui::Spacing();
				ImGui::Spacing();
				
				static map_settings::leaf_tweak_s* tweak_selection = nullptr;
				static map_settings::hide_area_s* hidearea_selection = nullptr;
				if (ImGui::BeginTable("CullTable", 5, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_ContextMenuInBody /*| ImGuiTableFlags_ScrollY, ImVec2(0, 200)*/))
				{
					ImGui::TableSetupScrollFreeze(0, 1); // make top row always visible
					ImGui::TableSetupColumn("Ar", ImGuiTableColumnFlags_NoResize, 18.0f);
					ImGui::TableSetupColumn("Leafs", ImGuiTableColumnFlags_WidthStretch, 120.0f);
					ImGui::TableSetupColumn("Areas", ImGuiTableColumnFlags_WidthStretch, 60.0f);

					//ImGui::TableSetupColumn("Twk-Leafs", ImGuiTableColumnFlags_WidthStretch, 40.0f);
					//ImGui::TableSetupColumn("Twk-Areas", ImGuiTableColumnFlags_WidthStretch, 40.0f);

					ImGui::TableSetupColumn("Hide-Leafs", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultHide, 40.0f);
					ImGui::TableSetupColumn("LeafTweaks & HideAreas", ImGuiTableColumnFlags_WidthStretch, 140.0f);
					//ImGui::TableSetupColumn("Hide-Areas", ImGuiTableColumnFlags_WidthStretch, 40.0f);
					//ImGui::TableSetupColumn("Hide-NLeafs", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultHide, 40.0f);

					ImGui::TableHeadersRow();

					for (auto& [area_num, a] : areas)
					{
						const bool is_current_area = (int)a.area_index == g_current_area;

						ImGui::TableNextRow();
						const bool is_selected = g_player_current_area_override && g_player_current_area_override == &a;
						if (is_selected) {
							ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
						}

						ImGui::TableNextColumn();
						ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));

						// - Area
						/*if (g_player_current_area_override && g_player_current_area_override == &a) {
							ImGui::Selectable(utils::va("%d", area_num), g_player_current_area_override&& g_player_current_area_override == &a, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
						}
						else*/ {
							ImGui::Text("%d", area_num);
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
						}

						// - Areas
						ImGui::TableNextColumn();
						{
							std::string arr_str;
							for (auto it = a.areas.begin(); it != a.areas.end(); ++it)
							{
								if (it != a.areas.begin()) {
									arr_str += ", ";
								} arr_str += std::to_string(*it);
							}
							ImGui::TextWrapped(arr_str.c_str());
						}

#if 0
						// - Tweak In Leafs
						ImGui::TableNextColumn();
						{
							std::string arr_str;
							for (const auto& lt : a.leaf_tweaks)
							{


								for (auto it = lt.in_leafs.begin(); it != lt.in_leafs.end(); ++it)
								{
									if (it != lt.in_leafs.begin()) {
										arr_str += ", ";
									} arr_str += std::to_string(*it);
								}
							}
							ImGui::TextUnformatted(arr_str.c_str());
						}

						// - Tweak Areas
						ImGui::TableNextColumn();
						{
							std::string arr_str;
							for (const auto& lt : a.leaf_tweaks)
							{
								for (auto it = lt.areas.begin(); it != lt.areas.end(); ++it)
								{
									if (it != lt.areas.begin()) {
										arr_str += ", ";
									} arr_str += std::to_string(*it);
								}
							}
							ImGui::TextUnformatted(arr_str.c_str());
						}
#endif

						// - Hide Leafs
						ImGui::TableNextColumn();
						{
							std::string arr_str;
							for (auto it = a.hide_leafs.begin(); it != a.hide_leafs.end(); ++it)
							{
								if (it != a.hide_leafs.begin()) {
									arr_str += ", ";
								} arr_str += std::to_string(*it);
							}
							ImGui::TextWrapped(arr_str.c_str());
						}

#if 0
						// - Hide Areas
						ImGui::TableNextColumn();
						{
							std::string arr_str;
							for (const auto& ha : a.hide_areas)
							{
								for (auto it = ha.areas.begin(); it != ha.areas.end(); ++it)
								{
									if (it != ha.areas.begin()) {
										arr_str += ", ";
									} arr_str += std::to_string(*it);
								}
							}
							ImGui::TextUnformatted(arr_str.c_str());
						}

						// - Hide Not in Leafs
						ImGui::TableNextColumn();
						{
							std::string arr_str;
							for (const auto& ha : a.hide_areas)
							{
								for (auto it = ha.when_not_in_leafs.begin(); it != ha.when_not_in_leafs.end(); ++it)
								{
									if (it != ha.when_not_in_leafs.begin()) {
										arr_str += ", ";
									} arr_str += std::to_string(*it);
								}
							}
							ImGui::TextUnformatted(arr_str.c_str());
						}
#endif
						// - tweak leafs + hide_areas
						ImGui::TableNextColumn();

						// inline table for leaf tweaks
						if (!a.leaf_tweaks.empty())
						{
							if (ImGui::BeginTable("tweak_leafs_nested_table", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_ContextMenuInBody))
							{
								ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoResize, 12.0f);
								ImGui::TableSetupColumn("Tweak in Leafs", ImGuiTableColumnFlags_WidthStretch, 100.0f);
								ImGui::TableSetupColumn("Tweak Areas", ImGuiTableColumnFlags_WidthStretch, 60.0f);
								ImGui::TableSetupColumn("##Delete", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoClip, 16.0f);
								ImGui::TableHeadersRow();

								std::uint32_t cur_row = 0;
								bool selection_matches_any_entry = false;
								map_settings::leaf_tweak_s* marked_for_deletion = nullptr;

								for (auto& lt : a.leaf_tweaks)
								{
									if (is_current_area && !tweak_selection) {
										tweak_selection = &lt;
									}

									ImGui::TableNextRow();
									ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBg));
									ImGui::TableNextColumn();
									ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
									{
										if (is_current_area)
										{
											if (ImGui::Selectable(utils::va("%d", cur_row), tweak_selection && tweak_selection == &lt, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
												tweak_selection = &lt;
											}

											if (tweak_selection && tweak_selection == &lt) {
												selection_matches_any_entry = true; // check that the selection ptr is up to date
											}
										}
										else {
											ImGui::Text("%d", cur_row);
										}
									}

									// Twk Leafs
									ImGui::TableNextColumn();
									{
										std::string arr_str;
										for (auto it = lt.in_leafs.begin(); it != lt.in_leafs.end(); ++it)
										{
											if (it != lt.in_leafs.begin()) {
												arr_str += ", ";
											} arr_str += std::to_string(*it);
										}
										ImGui::TextUnformatted(arr_str.c_str());
									}

									// Twk Areas
									ImGui::TableNextColumn();
									{
										std::string arr_str;
										for (auto it = lt.areas.begin(); it != lt.areas.end(); ++it)
										{
											if (it != lt.areas.begin()) {
												arr_str += ", ";
											} arr_str += std::to_string(*it);
										}
										ImGui::TextUnformatted(arr_str.c_str());
									}

									// Delete Button
									ImGui::TableNextColumn();
									{
										if (is_current_area)
										{
											ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.05f, 0.05f, 0.8f));
											ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
											ImGui::PushID((int)cur_row);
											if (ImGui::Button("x", ImVec2(16, 16))) {
												marked_for_deletion = &lt;
											}
											ImGui::PopStyleVar();
											ImGui::PopStyleColor();
											ImGui::PopID();
										}
									}

									cur_row++;
								}

								if (is_current_area && !selection_matches_any_entry) {
									tweak_selection = nullptr;
								}

								// remove entry
								if (marked_for_deletion)
								{
									for (auto it = g_player_current_area_override->leaf_tweaks.begin(); it != g_player_current_area_override->leaf_tweaks.end(); ++it)
									{
										if (&*it == marked_for_deletion) 
										{
											g_player_current_area_override->leaf_tweaks.erase(it);
											tweak_selection = nullptr;
											break;
										}
									}
								}

								ImGui::EndTable();
							}
						}

						if (is_current_area)
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.3f, 0.05f, 0.7f));
							if (ImGui::Button("++ Tweak Leaf Entry", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
								g_player_current_area_override->leaf_tweaks.emplace_back();
							} ImGui::PopStyleColor();
						}

						// ---
						// inline table for leaf tweaks
						if (!a.hide_areas.empty())
						{
							if (ImGui::BeginTable("hide_areas_nested_table", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_ContextMenuInBody))
							{
								ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoResize, 12.0f);
								ImGui::TableSetupColumn("Areas", ImGuiTableColumnFlags_WidthStretch, 100.0f);
								ImGui::TableSetupColumn("NLeafs", ImGuiTableColumnFlags_WidthStretch, 60.0f);
								ImGui::TableSetupColumn("##Delete", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoClip, 16.0f);
								ImGui::TableHeadersRow();

								std::uint32_t cur_row = 0;
								bool selection_matches_any_entry = false;
								map_settings::hide_area_s* marked_for_deletion = nullptr;

								for (auto& hda : a.hide_areas)
								{
									if (is_current_area && !hidearea_selection) {
										hidearea_selection = &hda;
									}

									ImGui::TableNextRow();
									ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBg));
									ImGui::TableNextColumn();
									ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));

									if (is_current_area)
									{
										if (ImGui::Selectable(utils::va("%d", cur_row), hidearea_selection && hidearea_selection == &hda, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
											hidearea_selection = &hda;
										}

										if (hidearea_selection && hidearea_selection == &hda) {
											selection_matches_any_entry = true; // check that the selection ptr is up to date
										}
									}
									else {
										ImGui::Text("%d", cur_row);
									}

									// Areas
									ImGui::TableNextColumn();
									{
										std::string arr_str;
										for (auto it = hda.areas.begin(); it != hda.areas.end(); ++it)
										{
											if (it != hda.areas.begin()) {
												arr_str += ", ";
											} arr_str += std::to_string(*it);
										}
										ImGui::TextUnformatted(arr_str.c_str());
									}

									// Not in Leafs
									ImGui::TableNextColumn();
									{
										std::string arr_str;
										for (auto it = hda.when_not_in_leafs.begin(); it != hda.when_not_in_leafs.end(); ++it)
										{
											if (it != hda.when_not_in_leafs.begin()) {
												arr_str += ", ";
											} arr_str += std::to_string(*it);
										}
										ImGui::TextUnformatted(arr_str.c_str());
									}

									// Delete Button
									ImGui::TableNextColumn();
									{
										if (is_current_area)
										{
											ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.05f, 0.05f, 0.8f));
											ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
											ImGui::PushID((int)cur_row);
											if (ImGui::Button("x", ImVec2(16, 16))) {
												marked_for_deletion = &hda;
											}
											ImGui::PopStyleVar();
											ImGui::PopStyleColor();
											ImGui::PopID();
										}
									}

									cur_row++;
								}

								if (is_current_area && !selection_matches_any_entry) {
									hidearea_selection = nullptr;
								}

								// remove entry
								if (marked_for_deletion)
								{
									for (auto it = g_player_current_area_override->hide_areas.begin(); it != g_player_current_area_override->hide_areas.end(); ++it)
									{
										if (&*it == marked_for_deletion)
										{
											g_player_current_area_override->hide_areas.erase(it);
											hidearea_selection = nullptr;
											break;
										}
									}
								}

								ImGui::EndTable();
							}
						}

						if (is_current_area)
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.3f, 0.05f, 0.7f));
							if (ImGui::Button("++ Tweak Hide Area Entry", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
								g_player_current_area_override->hide_areas.emplace_back();
							} ImGui::PopStyleColor();
						}
					}
					ImGui::EndTable();

					ImGui::SeparatorText("Modify Culling");

					if (g_player_current_area_override)
					{
						auto get_and_add_integers_to_set = [](const char* str, std::unordered_set<std::uint32_t>& set)
							{
								std::vector<int> temp_vec;
								utils::extract_integer_words(str, temp_vec, true);
								set.insert(temp_vec.begin(), temp_vec.end());
							};

						auto get_and_remove_integers_from_set = [](const char* str, std::unordered_set<std::uint32_t>& set)
							{
								std::vector<int> temp_vec;
								utils::extract_integer_words(str, temp_vec, true);

								for (const auto& v : temp_vec) {
									set.erase(v);
								}
							};

						static char in_leafs_buf[256], in_areas_buf[256], in_twk_in_leafs_buf[256], in_twk_areas_buf[256], in_hide_leafs_buf[256], in_hide_areas_buf[256], in_hide_nleafs_buf[256];
						ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll;


#define IMGUI_SET_INPUT_HANDLER(IDNAME, TXTBUF, SET) {																				\
		if (ImGui::Button("+##" #IDNAME)) {																							\
			get_and_add_integers_to_set((TXTBUF), (SET));																			\
			main_module::trigger_vis_logic();																						\
		} ImGui::SameLine();																										\
		if (ImGui::Button("-##" #IDNAME)) {																							\
			get_and_remove_integers_from_set((TXTBUF), (SET));																		\
			main_module::trigger_vis_logic();																						\
		} ImGui::SameLine(0, 8);																									\
		ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - ((ImGui::GetItemRectSize().x + ImGui::GetStyle().ItemSpacing.x) * 2.0f));	\
		ImGui::InputText(IDNAME " (sep. by spaces)##Culling", (TXTBUF), IM_ARRAYSIZE((TXTBUF)), input_text_flags);					\
	}
						// -----------------------

						IMGUI_SET_INPUT_HANDLER("Leafs", in_leafs_buf, g_player_current_area_override->leafs);
						IMGUI_SET_INPUT_HANDLER("Areas", in_areas_buf, g_player_current_area_override->areas);

						if (tweak_selection)
						{
							ImGui::Spacing();
							ImGui::SeparatorText("Tweak Leafs");
							ImGui::Spacing();
							IMGUI_SET_INPUT_HANDLER("Tweak Leafs", in_twk_in_leafs_buf, tweak_selection->in_leafs);
							IMGUI_SET_INPUT_HANDLER("Tweak Areas", in_twk_areas_buf, tweak_selection->areas);
							ImGui::Spacing();
						}

						if (hidearea_selection)
						{
							ImGui::Spacing();
							ImGui::SeparatorText("Hide Area");
							ImGui::Spacing();
							IMGUI_SET_INPUT_HANDLER("Hide Areas", in_hide_areas_buf, hidearea_selection->areas);
							IMGUI_SET_INPUT_HANDLER("Not in Leafs", in_hide_nleafs_buf, hidearea_selection->when_not_in_leafs);
							ImGui::Spacing();
						}

						ImGui::Spacing();
						ImGui::SeparatorText("Hide Leafs");
						IMGUI_SET_INPUT_HANDLER("Hide Leafs", in_hide_leafs_buf, g_player_current_area_override->hide_leafs);
#undef IMGUI_SET_INPUT_HANDLER
					}
					else if (g_current_area >= 0)
					{
						// Add culling tweak to area button
						if (ImGui::Button("Add Entry for current Area"))
						{
							map_settings::area_overrides_s new_area = {};
							new_area.cull_mode = map_settings::AREA_CULL_MODE_DEFAULT; // we need to add some "custom" setting so that it gets assigned to 'g_player_current_area_override'
							areas.emplace(g_current_area, std::move(new_area));
						}
					}
				}

				//ImGui::TreePop();
				SPACING_INDENT_END;
			}

			SPACING_INDENT_END;
		}
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
		m_im_window_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

		static bool im_demo_menu = false;
		if (ImGui::Button("Demo Menu")) {
			im_demo_menu = !im_demo_menu;
		}

		if (im_demo_menu) {
			ImGui::ShowDemoWindow(&im_demo_menu);
		}

#define ADD_TAB(NAME, FUNC) \
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0)));			\
	if (ImGui::BeginTabItem(NAME)) {																		\
		if (ImGui::BeginChild("##child_" NAME, ImVec2(0, 0), ImGuiChildFlags_AlwaysUseWindowPadding )) {	\
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
			style->FrameRounding = 0.0f;
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
