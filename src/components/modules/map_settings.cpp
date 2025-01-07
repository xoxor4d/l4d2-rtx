#include "std_include.hpp"

namespace components
{
	void map_settings::set_settings_for_map(const std::string& map_name)
	{
		m_map_settings.mapname = !map_name.empty() ? map_name : interfaces::get()->m_engine->get_level_name();
		utils::replace_all(m_map_settings.mapname, std::string("maps/"), "");		// if sp map
		utils::replace_all(m_map_settings.mapname, std::string(".bsp"), "");

		parse_toml();

		static bool disable_map_configs = flags::has_flag("xo_disable_map_conf");
		if (remix_api::is_initialized())
		{
			if (!disable_map_configs)
			{
				// resets all modified variables back to rtx.conf level
				remix_vars::reset_all_modified();

				// auto apply {map_name}.conf (if it exists)
				open_and_set_var_config(m_map_settings.mapname + ".conf", true);

				// apply other manually defined configs
				for (const auto& f : m_map_settings.api_var_configs) {
					open_and_set_var_config(f);
				}
			}

			remix_lights::get()->add_all_map_setting_lights_without_creation_trigger();
		}

		m_loaded = true;
	}

	// cannot be called in the current on_map_load stub (too early)
	// called from 'once_per_frame_cb()' instead
	void map_settings::spawn_markers_once()
	{
		// only spawn markers once
		if (m_spawned_markers) {
			return;
		}

		m_spawned_markers = true;

		// spawn map markers
		for (auto& m : m_map_settings.map_markers)
		{
			// ignore nocull markers (main_module::draw_nocull_markers)
			if (m.no_cull) {
				continue;
			}

			const auto mdl_num = m.index / 10u;
			const auto skin_num = m.index % 10u;
			const auto model_name = utils::va("models/props_xo/mapmarker%03d.mdl", mdl_num * 10);

			//const auto model_name = "models/items/l4d_gift.mdl";
			//const auto skin_num = 1; 
			//const auto model_name = "models/extras/info_speech.mdl";

			void* mdlcache = reinterpret_cast<void*>(*(DWORD*)(SERVER_BASE + 0x897AE0));

			// mdlcache->BeginLock
			utils::hook::call_virtual<26, void>(mdlcache);

			// mdlcache->FindMDL
			const auto mdl_handle = utils::hook::call_virtual<6, std::uint16_t>(mdlcache, model_name);
			if (mdl_handle != 0xFFFF)
			{
				// save precache state - CBaseEntity::m_bAllowPrecache
				const bool old_precache_state = *reinterpret_cast<bool*>(SERVER_BASE + 0x7D8FC0);

				// allow precaching - CBaseEntity::m_bAllowPrecache
				*reinterpret_cast<bool*>(SERVER_BASE + 0x7D8FC0) = true;

				// CreateEntityByName - CBaseEntity *__cdecl CreateEntityByName(const char *className, int iForceEdictIndex, bool bNotify)
				m.handle = utils::hook::call<void* (__cdecl)(const char* className, int iForceEdictIndex, bool bNotify)>(SERVER_BASE + 0x1196B0)
					("dynamic_prop", -1, true);

				if (m.handle)
				{
					// ent->KeyValue
					utils::hook::call_virtual<34, void>(m.handle, "origin", utils::va("%.10f %.10f %.10f", m.origin[0], m.origin[1], m.origin[2]));
					utils::hook::call_virtual<34, void>(m.handle, "angles", utils::va("%.10f %.10f %.10f", m.rotation[0], m.rotation[1], m.rotation[2]));
					utils::hook::call_virtual<34, void>(m.handle, "model", model_name);
					utils::hook::call_virtual<34, void>(m.handle, "solid", "0");

					struct skin_offset
					{
						char pad[1092];
						int m_nSkin;
					}; STATIC_ASSERT_OFFSET(skin_offset, m_nSkin, 1092);

					auto skin_val = static_cast<skin_offset*>(m.handle);
					skin_val->m_nSkin = skin_num;

					// ent->Precache
					utils::hook::call_virtual<26, void>(m.handle);

					// DispatchSpawn
					utils::hook::call<void(__cdecl)(void* pEntity, bool bRunVScripts)>(SERVER_BASE + 0x209580)
						(m.handle, true);

					// ent->Activate
					utils::hook::call_virtual<36, void>(m.handle);
				}

				// restore precaching state - CBaseEntity::m_bAllowPrecache
				*reinterpret_cast<bool*>(SERVER_BASE + 0x7D8FC0) = old_precache_state;
			}

			utils::hook::call_virtual<27, void>(mdlcache); // mdlcache->EndLock
		}
	}

	void map_settings::destroy_markers()
	{
		// destroy active markers
		for (auto& m : m_map_settings.map_markers)
		{
			if (m.handle)
			{
				game::cbaseentity_remove(m.handle);
				m.handle = nullptr;
			}
		}

		m_map_settings.map_markers.clear();
		m_spawned_markers = false;
	}

#define TOML_ERROR(TITLE, ENTRY, MSG, ...) \
	game::console(); std::cout << toml::format_error(toml::make_error_info(#TITLE, (ENTRY), utils::va(#MSG, __VA_ARGS__))) << std::endl; \

	bool map_settings::parse_toml()
	{
		try 
		{
			auto config = toml::parse("l4d2-rtx\\map_settings.toml", toml::spec::v(1, 1, 0));

			// #
			auto to_float = [](const toml::value& entry, const float default_val = 0.0f)
				{
					if (entry.is_floating()) {
						return static_cast<float>(entry.as_floating());
					}

					if (entry.is_integer()) {
						return static_cast<float>(entry.as_integer());
					}

					try { // this will fail and let the user know whats wrong
						return static_cast<float>(entry.as_floating());
					}
					catch (toml::type_error& err) {
						game::console(); printf("%s\n", err.what());
					}

					return default_val;
				};

			// #
			auto to_int = [](const toml::value& entry, const int default_val = 0)
				{
					if (entry.is_floating())  {
						return static_cast<int>(entry.as_floating());
					}

					if (entry.is_integer()) {
						return static_cast<int>(entry.as_integer());
					}

					try { // this will fail and let the user know whats wrong
						return static_cast<int>(entry.as_integer());
					}
					catch (toml::type_error& err) {
						game::console(); printf("%s\n", err.what());
					}

					return default_val;
				};

			auto to_uint = [](const toml::value& entry, const std::uint32_t default_val = 0u)
				{
					if (entry.is_floating()) {
						return static_cast<std::uint32_t>(entry.as_floating());
					}

					if (entry.is_integer()) {
						return static_cast<std::uint32_t>(entry.as_integer());
					}

					try { // this will fail and let the user know whats wrong
						return static_cast<std::uint32_t>(entry.as_integer());
					}
					catch (toml::type_error& err) {
						game::console(); printf("%s\n", err.what());
					}

					return default_val;
				};

			// ####################
			// parse 'FOG' table
			if (config.contains("FOG"))
			{
				auto& fog_table = config["FOG"];

				// try to find the loaded map
				if (fog_table.contains(m_map_settings.mapname))
				{
					if (const auto map = fog_table[m_map_settings.mapname];
						!map.is_empty())
					{
						if (map.contains("distance") && map.contains("color"))
						{
							const auto dist = map.at("distance");
							m_map_settings.fog_dist = to_float(dist);

							if (const auto& color = map.at("color").as_array(); 
								color.size() == 3)
							{
								const auto r = static_cast<std::uint8_t>(to_int(color[0]));
								const auto g = static_cast<std::uint8_t>(to_int(color[1]));
								const auto b = static_cast<std::uint8_t>(to_int(color[2]));
								m_map_settings.fog_color = D3DCOLOR_XRGB(r, g, b);
							}
						}
					}
				}
			} // end 'FOG'


			// ####################
			// parse 'WATER' table
			if (config.contains("WATER"))
			{
				auto& water_table = config["WATER"];

				// try to find the loaded map
				if (water_table.contains(m_map_settings.mapname))
				{
					if (const auto map = water_table[m_map_settings.mapname];
						!map.is_empty())
					{
						m_map_settings.water_uv_scale = to_float(map, 1.0f);
					}
				}
			} // end 'WATER'


			// ####################
			// parse 'CULL' table
			if (config.contains("CULL"))
			{
				auto& cull_table = config["CULL"];

				// #
				auto process_cull_entry = [to_uint](const toml::value& entry)
					{
						const auto contains_leafs = entry.contains("leafs");
						const auto contains_areas = entry.contains("areas");
						const auto contains_leaf_tweak = entry.contains("leaf_tweak");
						const auto contains_hidden_leafs = entry.contains("hide_leafs");
						const auto contains_hidden_areas = entry.contains("hide_areas");
						const auto contains_cull = entry.contains("cull");

						if (entry.contains("in_area") && (contains_leafs || contains_areas || contains_hidden_leafs || contains_hidden_areas || contains_leaf_tweak || contains_cull))
						{
							const auto area = to_uint(entry.at("in_area"));

							// forced leafs
							std::unordered_set<std::uint32_t> leaf_set;
							if (contains_leafs)
							{
								auto& leafs = entry.at("leafs").as_array();

								for (const auto& leaf : leafs) {
									leaf_set.insert(to_uint(leaf));
								}
							}

							// forced areas
							std::unordered_set<std::uint32_t> area_set;
							if (contains_areas)
							{
								auto& areas = entry.at("areas").as_array();

								for (const auto& a : areas) {
									area_set.insert(to_uint(a));
								}
							}

							// culling mode
							AREA_CULL_MODE cmode = imgui::get()->m_disable_cullnode ? map_settings::AREA_CULL_MODE_NO_FRUSTUM : map_settings::AREA_CULL_MODE_DEFAULT;
							if (contains_cull)
							{
								auto m = to_uint(entry.at("cull"));
								if (m >= AREA_CULL_COUNT) 
								{
									game::console(); printf("MapSettings: param 'cull' was out-of-range (%d)\n", m);
									m = 0u;
								}
								cmode = (AREA_CULL_MODE)(std::uint8_t)m;
							}

							// hidden leafs
							std::unordered_set<std::uint32_t> hidden_leaf_set;
							if (contains_hidden_leafs)
							{
								auto& leafs = entry.at("hide_leafs").as_array();

								for (const auto& leaf : leafs) {
									hidden_leaf_set.insert(to_uint(leaf));
								}
							}

							// hidden areas
							std::vector<hide_area_s> temp_hidden_areas_set;
							if (contains_hidden_areas)
							{
								auto& hide_areas = entry.at("hide_areas").as_array();
								for (const auto& elem : hide_areas)
								{
									if (elem.contains("areas"))
									{
										std::unordered_set<std::uint32_t> temp_area_set;
										const auto& areas = elem.at("areas").as_array();

										for (const auto& a : areas) {
											temp_area_set.insert(to_uint(a));
										}

										std::unordered_set<std::uint32_t> temp_not_in_leaf_set;
										if (elem.contains("N_leafs"))
										{
											const auto& nleafs = elem.at("N_leafs").as_array();
											for (const auto& nl : nleafs) {
												temp_not_in_leaf_set.insert(to_uint(nl));
											}
										}

										temp_hidden_areas_set.emplace_back(std::move(temp_area_set), std::move(temp_not_in_leaf_set));
									}
								}
							}

							// leaf tweaks
							std::vector<leaf_tweak_s> temp_leaf_tweak_set;
							if (contains_leaf_tweak)
							{
								auto& leaf_tweak = entry.at("leaf_tweak").as_array();
								for (const auto& elem : leaf_tweak)
								{
									if (elem.contains("in_leafs"))
									{
										std::unordered_set<std::uint32_t> temp_in_leafs_set;
										const auto& in_leafs = elem.at("in_leafs").as_array();

										for (const auto& l : in_leafs) {
											temp_in_leafs_set.insert(to_uint(l));
										}

										std::unordered_set<std::uint32_t> temp_areas;
										if (elem.contains("areas"))
										{
											const auto& areas = elem.at("areas").as_array();
											for (const auto& a : areas) {
												temp_areas.insert(to_uint(a));
											}
										}

										temp_leaf_tweak_set.emplace_back(std::move(temp_in_leafs_set), std::move(temp_areas));
									}
								}
							}

							m_map_settings.area_settings.emplace(area,
								area_overrides_s 
								{
									std::move(leaf_set),
									std::move(area_set),
									std::move(hidden_leaf_set),
									std::move(temp_hidden_areas_set),
									std::move(temp_leaf_tweak_set),
									cmode,
									area
								});
						}
					};

				// try to find the loaded map
				if (cull_table.contains(m_map_settings.mapname))
				{
					if (const auto map = cull_table[m_map_settings.mapname]; 
						!map.is_empty() && !map.as_array().empty())
					{
						for (const auto& entry : map.as_array()) {
							process_cull_entry(entry);
						}
					}
				}
			} // end 'CULL'


			// ####################
			// parse 'HIDEMODEL' table
			if (config.contains("HIDEMODEL"))
			{
				// try to find the loaded map
				if (auto& hidemdl_table = config["HIDEMODEL"]; 
					hidemdl_table.contains(m_map_settings.mapname))
				{
					if (const auto map = hidemdl_table[m_map_settings.mapname];
						!map.is_empty())
					{
						if (map.contains("name"))
						{
							if (auto& names = map.at("name");
								!names.is_empty())
							{
								if (const auto& narray = map.at("name").as_array();
									!narray.empty())
								{
									for (auto& str : narray) {
										m_map_settings.hide_models.substrings.insert(str.as_string());
									}
								}
							}
						}

						if (map.contains("radius"))
						{
							if (auto& radii = map.at("radius");
								!radii.is_empty())
							{
								if (const auto& rarray = map.at("radius").as_array();
									!rarray.empty())
								{
									for (auto& r : rarray) {
										m_map_settings.hide_models.radii.insert(to_float(r, -1.0f));
									}
								}
							}
						}
						
					}
				}
			} // end 'HIDEMODEL'


			// ####################
			// parse 'MARKER' table
			if (config.contains("MARKER"))
			{
				auto& marker_table = config["MARKER"];

				// #
				auto process_marker_entry = [to_int, to_float](const toml::value& entry)
					{
						bool temp_is_nocull_marker = false;
						std::uint32_t temp_marker_index = 0u;

						if (entry.contains("marker")) {
							temp_marker_index = static_cast<std::uint32_t>(to_int(entry.at("marker"), 0u));
						}
						else if (entry.contains("nocull")) 
						{
							temp_marker_index = static_cast<std::uint32_t>(to_int(entry.at("nocull"), 0u));
							temp_is_nocull_marker = true;
						}
						else
						{
							TOML_ERROR("[MARKER] #index", entry, "Marker did not define an index via 'marker' or 'nocull' -> skipping");
							return;
						}

						if (entry.contains("position"))
						{
							if (const auto& pos = entry.at("position").as_array();
								pos.size() == 3)
							{
								Vector temp_rotation;
								Vector temp_scale = { 1.0, 1.0f, 1.0f };

								// optional
								if (entry.contains("rotation"))
								{
									if (const auto& rot = entry.at("rotation").as_array(); rot.size() == 3) {
										temp_rotation = { DEG2RAD(to_float(rot[0])), DEG2RAD(to_float(rot[1])), DEG2RAD(to_float(rot[2])) };
									} else { TOML_ERROR("[MARKER] #rotation", entry.at("rotation"), "expected a 3D vector but got => %d ", entry.at("rotation").as_array().size()); }
								}

								// optional
								if (entry.contains("scale"))
								{
									if (const auto& scale = entry.at("scale").as_array(); scale.size() == 3) {
										temp_scale = { to_float(scale[0]), to_float(scale[1]), to_float(scale[2]) };
									} else { TOML_ERROR("[MARKER] #scale", entry.at("scale"), "expected a 3D vector but got => %d ", entry.at("scale").as_array().size()); }
								}

								// optional
								std::unordered_set<std::uint32_t> temp_area_set;
								if (entry.contains("areas"))
								{
									if (const auto& areas = entry.at("areas").as_array(); !areas.empty()) 
									{
										for (const auto& a : areas) {
											temp_area_set.insert(to_int(a));
										}
									}
								}

								// optional
								std::unordered_set<std::uint32_t> temp_not_in_leaf_set;
								if (entry.contains("N_leafs"))
								{
									if (const auto& nleafs = entry.at("N_leafs").as_array(); !nleafs.empty())
									{
										for (const auto& nl : nleafs) {
											temp_not_in_leaf_set.insert(to_int(nl));
										}
									}
								}

								m_map_settings.map_markers.emplace_back(
									marker_settings_s
									{
										.index = temp_marker_index,
										.origin = { to_float(pos[0]), to_float(pos[1]), to_float(pos[2]) },
										.no_cull = temp_is_nocull_marker,
										.rotation = temp_rotation,
										.scale = temp_scale,
										.areas = std::move(temp_area_set),
										.when_not_in_leafs = std::move(temp_not_in_leaf_set),
									});
							}
							else { TOML_ERROR("[MARKER] #position", entry.at("position"), "expected a 3D vector but got => %d ", entry.at("position").as_array().size()); }
						}
					};

				// try to find the loaded map
				if (marker_table.contains(m_map_settings.mapname))
				{
					if (const auto map = marker_table[m_map_settings.mapname];
						!map.is_empty() && !map.as_array().empty())
					{
						for (const auto& entry : map.as_array()) {
							process_marker_entry(entry);
						}
					}
				}
			} // end 'MARKER'


			// ####################
			// parse 'CONFIGVARS' table
			{
				auto& configvar_table = config["CONFIGVARS"];

				// #TODO
				auto process_transition_entry = [to_int, to_float](const toml::value& entry)
					{
						// we NEED conf, leafs and duration or speed
						if (entry.contains("conf") && (entry.contains("leafs") || entry.contains("choreo")) && (entry.contains("duration") || entry.contains("speed")))
						{
							std::string config_name;

							try { config_name = entry.at("conf").as_string(); }
							catch (toml::type_error& err) 
							{
								game::console(); printf("%s\n", err.what());
								return;
							}

							if (!config_name.empty()) 
							{
								std::uint8_t mode = 0u;
								remix_vars::EASE_TYPE ease = remix_vars::EASE_TYPE_LINEAR;
								float delay_in = 0.0f, delay_out = 0.0f, duration = 0.0f;

								if (entry.contains("mode")) {
									mode = (std::uint8_t)to_int(entry.at("mode"));
								}

								if (entry.contains("ease")) {
									ease = (remix_vars::EASE_TYPE)to_int(entry.at("ease"));
								}

								if (entry.contains("delay_in")) {
									delay_in = to_float(entry.at("delay_in"));
								}

								if (entry.contains("delay_out")) {
									delay_out = to_float(entry.at("delay_out"));
								}

								if (entry.contains("duration")) {
									duration = to_float(entry.at("duration"));
								}

								const bool choreo_mode = entry.contains("choreo");
								if (choreo_mode)
								{
									std::string choreo_name;
									try { choreo_name = entry.at("choreo").as_string(); }
									catch (toml::type_error& err)
									{
										game::console(); printf("%s\n", err.what());
										return;
									}

									if (!choreo_name.empty())
									{
										const auto hash = utils::string_hash64(utils::va("%s%s%.2f", choreo_name.c_str(), config_name.c_str(), duration));
										m_map_settings.choreo_transitions.emplace_back(
											std::move(choreo_name),
											config_name,
											(CHOREO_TRANS_MODE)mode,
											ease,
											delay_in,
											delay_out,
											duration,
											hash);
									}
								}
								else
								{
									const bool leaf_mode = !choreo_mode && entry.contains("leafs");
									if (leaf_mode)
									{
										std::unordered_set<std::uint32_t> leaf_set;
										const auto& leafs = entry.at("leafs").as_array();
										if (!leafs.empty())
										{
											for (const auto& leaf : leafs) {
												leaf_set.insert(to_int(leaf));
											}

											// create a unique hash for this transition
											int leaf_sum = 0;
											for (const auto& leaf : leaf_set) {
												leaf_sum += leaf;
											}

											const auto hash = utils::string_hash64(utils::va("%d%s%.2f", leaf_sum, config_name.c_str(), duration));
											m_map_settings.leaf_transitions.emplace_back(
												std::move(leaf_set),
												config_name,
												(LEAF_TRANS_MODE)mode,
												ease,
												delay_in,
												delay_out,
												duration,
												hash);
										}
									}
								}
							}
						}
					};

				// try to find the loaded map
				if (configvar_table.contains(m_map_settings.mapname))
				{
					if (const auto map = configvar_table[m_map_settings.mapname];
						!map.is_empty())
					{
						if (map.contains("startup"))
						{
							if (auto& startup = map.at("startup").as_array(); 
								!startup.empty())
							{
								for (const auto& conf : startup)
								{
									try {
										m_map_settings.api_var_configs.emplace_back(conf.as_string());
									}
									catch (toml::type_error& err) {
										game::console(); printf("%s\n", err.what());
									}
								}
							}
						}

						if (map.contains("transitions"))
						{
							if (auto& transitions = map.at("transitions").as_array();
								!transitions.empty())
							{
								for (const auto& entry : transitions) {
									process_transition_entry(entry);
								}
							}
						}
					}
				}
			} // end 'CONFIGVARS'

			// ####################
			// parse 'LIGHTS' table
			if (config.contains("LIGHTS"))
			{
				auto& light_table = config["LIGHTS"];

				// #
				auto process_light_entry = [to_int, to_uint, to_float](const toml::value& entry)
					{
						if (entry.contains("points") && !entry.at("points").as_array().empty())
						{
							// - parse trigger

							std::string temp_trigger_choreo_name;
							std::uint32_t temp_trigger_sound = 0u;
							float temp_trigger_delay = 0.0f;
							bool temp_trigger_always = false;

							if (entry.contains("trigger"))
							{
								bool has_valid_trigger = false;
								const auto& trigger = entry.at("trigger");

								// choreo trigger
								if (trigger.contains("choreo"))
								{
									try { temp_trigger_choreo_name = trigger.at("choreo").as_string(); }
									catch (toml::type_error& err)
									{
										game::console(); printf("%s\n", err.what());
										return;
									}

									has_valid_trigger = true;
								}
								// sound trigger
								else if (trigger.contains("sound")) 
								{
									temp_trigger_sound = to_uint(trigger.at("sound"), 0u);
									has_valid_trigger = true;
								}

								if (has_valid_trigger) 
								{
									if (trigger.contains("delay")) {
										temp_trigger_delay = to_float(trigger.at("delay"), 0.0f);
									}

									if (trigger.contains("always")) {
										temp_trigger_always = to_int(trigger.at("always"), 0);
									}
								} else { TOML_ERROR("[LIGHTS] #trigger", trigger, "defined trigger with no choreo / sound hash"); }
							}

							// - parse kill

							std::string temp_kill_choreo_name;
							std::uint32_t temp_kill_sound = 0u;
							float temp_kill_delay = 0.0f;

							if (entry.contains("kill"))
							{
								bool has_valid_kill_trigger = false;
								const auto& kill = entry.at("kill");

								// choreo
								if (kill.contains("choreo"))
								{
									try { temp_kill_choreo_name = kill.at("choreo").as_string(); }
									catch (toml::type_error& err)
									{
										game::console(); printf("%s\n", err.what());
										return;
									}

									has_valid_kill_trigger = true;
								}
								// sound
								else if (kill.contains("sound"))
								{
									temp_kill_sound = to_uint(kill.at("sound"), 0u);
									has_valid_kill_trigger = true;
								}

								if (has_valid_kill_trigger)
								{
									if (kill.contains("delay")) {
										temp_kill_delay = to_float(kill.at("delay"), 0.0f);
									}
								} else { TOML_ERROR("[LIGHTS] #trigger", kill, "defined kill trigger with no choreo / sound hash"); }
							}

							// - parse points

							const auto& parray = entry.at("points").as_array();
							std::vector<remix_light_settings_s::point_s> temp_points;

							bool temp_loop_smoothing = false;
							if (entry.contains("loop_smoothing")) {
								temp_loop_smoothing = to_int(entry.at("loop_smoothing"), 0);
							}

							// for each point
							for (auto i = 0u; i < parray.size(); i++)
							{
								bool point_has_valid_position = false;

								const auto& p = parray[i];
								if (p.contains("position"))
								{
									if (const auto& positions = p.at("position").as_array(); positions.size() == 3) {
										point_has_valid_position = true;
									}
									else { TOML_ERROR("[LIGHTS] #position", p.at("position"), "expected a 3D vector but got => %d ", p.at("position").as_array().size()); }
								}

								if (!i && !point_has_valid_position) // first point needs to define a position
								{	
									TOML_ERROR("[LIGHTS] #position", p, "first point needs to define a position! Ignoring light");
									break;
								}

								Vector temp_radiance = { 10.0f, 10.0f, 10.0f };
								if (p.contains("radiance"))
								{
									if (const auto& radiance = p.at("radiance").as_array(); radiance.size() == 3) 
									{
										temp_radiance = Vector(to_float(radiance[0], 10.0f), to_float(radiance[1], 10.0f), to_float(radiance[2], 10.0f));
									} else { TOML_ERROR("[LIGHTS] #radiance", p.at("radiance"), "expected a 3D vector but got => %d ", p.at("radiance").as_array().size()); }
								}

								float temp_radiance_scalar = 1.0f;
								if (p.contains("scalar")) {
									temp_radiance_scalar = to_float(p.at("scalar"), 1.0f);
								}

								float temp_radius = 1.0f;
								if (p.contains("radius")) {
									temp_radius = to_float(p.at("radius"), 1.0f);
								}

								float temp_timepoint = 0.0f;
								if (i && p.contains("timepoint")) { // do not set timepoint for first point
									temp_timepoint = to_float(p.at("timepoint"), 0.0f);
								}

								float temp_smoothness = 0.5f;
								if (p.contains("smoothness")) 
								{
									temp_smoothness = to_float(p.at("smoothness"), 0.5f);
									temp_smoothness = std::clamp<float>(temp_smoothness, 0.0f, 10.0f);
								}


								// shaping

								Vector temp_direction = { 0.0f, 0.0f, 1.0f };
								if (p.contains("direction"))
								{
									if (const auto& direction = p.at("direction").as_array(); direction.size() == 3)
									{
										temp_direction = Vector(to_float(direction[0], 0.0f), to_float(direction[1], 0.0f), to_float(direction[2], 1.0f));
										temp_direction.Normalize();
									} else { TOML_ERROR("[LIGHTS] #direction", p.at("direction"), "expected a 3D vector but got => %d ", p.at("direction").as_array().size()); }
								}

								bool temp_shaping_enabled = false;
								float temp_degrees = 180.0f;
								if (p.contains("degrees")) 
								{
									temp_degrees = to_float(p.at("degrees"), 180.0f);
									temp_degrees = std::clamp<float>(temp_degrees, 0.0f, 180.0f);
									temp_shaping_enabled = temp_degrees != 180.0f;
								}

								float temp_softness = 0.0f;
								if (p.contains("softness"))
								{
									temp_softness = to_float(p.at("softness"), 0.0f);
									temp_softness = std::clamp<float>(temp_softness, 0.0f, M_PI);
								}

								float temp_exponent = 0.0f;
								if (p.contains("exponent")) {
									temp_exponent = to_float(p.at("exponent"), 0.0f);
								}

								// to avoid code duplication
								Vector pt;

								// using either position defined in current point or previous position
								if (point_has_valid_position)
								{
									const auto& positions = p.at("position").as_array();
									pt = Vector(to_float(positions[0]), to_float(positions[1]), to_float(positions[2]));
								}
								else {
									pt = temp_points.back().position; // pos of previous point
								}

								temp_points.emplace_back(
									remix_light_settings_s::point_s(
										pt, 
										temp_radiance,
										temp_radiance_scalar,
										temp_radius,
										temp_timepoint,
										temp_smoothness,
										temp_shaping_enabled,
										temp_direction,
										temp_degrees,
										temp_softness,
										temp_exponent)
								);
							}

							// - parse general settings

							if (!temp_points.empty())
							{
								bool temp_run_once = false;
								if (entry.contains("run_once")) {
									temp_run_once = to_int(entry.at("run_once"), 0);
								}

								bool temp_loop = false;
								if (entry.contains("loop")) {
									temp_loop = to_int(entry.at("loop"), 0);
								}

								m_map_settings.remix_lights.push_back(
									remix_light_settings_s(
										std::move(temp_points),
										temp_run_once,
										temp_loop,
										temp_loop_smoothing,
										temp_trigger_always,
										std::move(temp_trigger_choreo_name),
										temp_trigger_sound,
										temp_trigger_delay,
										std::move(temp_kill_choreo_name),
										temp_kill_sound,
										temp_kill_delay)
								);
							}
						}
						else { TOML_ERROR("[LIGHTS] #points", entry, "needs at least one point to define a light"); }
					};

				// try to find the loaded map
				if (light_table.contains(m_map_settings.mapname))
				{
					if (const auto& map = light_table[m_map_settings.mapname];
						!map.is_empty() && !map.as_array().empty())
					{
						for (const auto& entry : map.as_array()) {
							process_light_entry(entry);
						}
					}
				}
			} // end 'LIGHTS'
		}

		catch (const toml::syntax_error& err)
		{
			game::console();
			printf("%s\n", err.what());
			return false;
		}

		return true;
	}

	bool map_settings::matches_map_name()
	{
		return utils::str_to_lower(m_args[0]) == m_map_settings.mapname;
	}

	void map_settings::open_and_set_var_config(const std::string& config, const bool no_error, const bool ignore_hashes, const char* custom_path)
	{
		std::string path = "l4d2-rtx\\map_configs";
		if (custom_path)
		{
			path = custom_path;
		}

		std::ifstream file;
		if (utils::open_file_homepath(path, config, file))
		{
			std::string input;
			while (std::getline(file, input))
			{
				if (utils::starts_with(input, "#")) {
					continue;
				}

				if (auto pair = utils::split(input, '=');
					pair.size() == 2u)
				{
					utils::trim(pair[0]);
					utils::trim(pair[1]);

					if (ignore_hashes && pair[1].starts_with("0x")) {
						continue;
					}

					if (pair[1].empty()) {
						continue;
					}

					if (const auto o = remix_vars::get_option(pair[0].c_str()); o)
					{
						const auto& v = remix_vars::string_to_option_value(o->second.type, pair[1]);
						remix_vars::set_option(o, v, true);
					}
				}
			}

			file.close();
		}
		else if (!no_error)
		{
			game::console();
			printf("[MapSettings] Failed to find config: \"%s\" in %s \n", config.c_str(), custom_path ? custom_path : "\"l4d2-rtx\\map_configs\"");
		}
	}

	void map_settings::on_map_load(const std::string& map_name)
	{
		if (m_loaded) {
			get()->clear_map_settings();
		}

		get()->set_settings_for_map(map_name);

		is_level.reset();
		is_level.update(get_map_name());
	}

	void map_settings::on_map_unload()
	{
		get()->clear_map_settings();
	}

	void map_settings::clear_map_settings()
	{
		remix_lights::get()->destroy_and_clear_all_map_lights();
		m_map_settings.remix_lights.clear();

		m_map_settings.area_settings.clear();
		m_map_settings.leaf_transitions.clear();
		m_map_settings.choreo_transitions.clear();

		destroy_markers();
		m_map_settings.map_markers.clear();

		m_map_settings.api_var_configs.clear();
		m_map_settings = {};
		m_loaded = false;

		main_module::trigger_vis_logic();
	}

	ConCommand xo_mapsettings_update {};
	void map_settings::reload()
	{
		get()->clear_map_settings();
		get()->set_settings_for_map("");
	}

	map_settings::map_settings()
	{
		p_this = this;
		game::con_add_command(&xo_mapsettings_update, "xo_mapsettings_update", map_settings::reload, "Reloads the map_settings.toml file + map.conf");
	}
}
