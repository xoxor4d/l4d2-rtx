#include "std_include.hpp"

namespace common::toml
{
	// format 2 decimals
	inline std::string format_float(float value)
	{
		return std::format("{:.2f}", value);
	}

	/// Builds a toml string for the provided light definition
	/// @param def	ref to the light def
	/// @return		the final string in toml format
	std::string build_light_string_for_single_light(const map_settings::remix_light_settings_s& def)
	{
		std::string toml_str = "# " + def.comment + "\n";

		toml_str += "        {";

		// choreo trigger
		if (!def.trigger_choreo_name.empty())
		{
			toml_str += " trigger = { choreo = \"" + def.trigger_choreo_name + "\"";

			if (!def.trigger_choreo_actor.empty()) {
				toml_str += ", actor = \"" + def.trigger_choreo_actor + "\"";
			}

			if (!def.trigger_choreo_event.empty()) {
				toml_str += ", event = \"" + def.trigger_choreo_event + "\"";
			}

			if (!def.trigger_choreo_param1.empty()) {
				toml_str += ", param1 = \"" + def.trigger_choreo_param1 + "\"";
			}

			if (!utils::float_equal(def.trigger_delay, 0.0f)) {
				toml_str += ", delay = " + format_float(def.trigger_delay);
			}

			if (def.trigger_always) {
				toml_str += ", always = " + (def.trigger_always ? "true"s : "false"s);
			}

			toml_str += " },"; // end table
		}
		// sound trigger
		else if (def.trigger_sound_hash)
		{
			toml_str += " trigger = { sound = " + std::format("0x{:X}", def.trigger_sound_hash);

			if (!utils::float_equal(def.trigger_delay, 0.0f)) {
				toml_str += ", delay = " + format_float(def.trigger_delay);
			}

			if (def.trigger_always) {
				toml_str += ", always = " + (def.trigger_always ? "true"s : "false"s);
			}

			toml_str += " },"; // end table
		}

		// kill
		if (const auto has_kill_choreo = !def.kill_choreo_name.empty();
			has_kill_choreo || def.kill_sound_hash != 0u)
		{
			if (has_kill_choreo) {
				toml_str += " kill = { choreo = \"" + def.kill_choreo_name + "\"";
			}
			else { // sound
				toml_str += " kill = { sound = " + std::format("0x{:X}", def.kill_sound_hash);
			}

			if (!utils::float_equal(def.kill_delay, 0.0f)) {
				toml_str += ", delay = " + format_float(def.kill_delay);
			}

			toml_str += " },"; // end table
		}

		// attach
		if (const auto has_attach_radius = def.attach_prop_radius != 0.0f;
			has_attach_radius || !def.attach_prop_name.empty())
		{
			if (has_attach_radius) {
				toml_str += " attach = { radius = " + format_float(def.attach_prop_radius);
			}
			else { // name str
				toml_str += " attach = { name = \"" + def.attach_prop_name + "\"";
			}

			if (!(def.attach_prop_mins.IsZero() && def.attach_prop_maxs.IsZero())) 
			{
				toml_str += ", bounds = ["
					+ format_float(def.attach_prop_mins.x) + ", " + format_float(def.attach_prop_mins.y) + ", " + format_float(def.attach_prop_mins.z) + ", "
					+ format_float(def.attach_prop_maxs.x) + ", " + format_float(def.attach_prop_maxs.y) + ", " + format_float(def.attach_prop_maxs.z) + "]";
			}

			toml_str += " },"; // end table
		}

		// points
		toml_str += " points = [\n";

		for (const auto& p : def.points)
		{
			toml_str += "            { ";

			toml_str += "position = [" + format_float(p.position.x) + ", " + format_float(p.position.y) + ", " + format_float(p.position.z) + "]";
			toml_str += ", radiance = [" + format_float(p.radiance.x) + ", " + format_float(p.radiance.y) + ", " + format_float(p.radiance.z) + "]";
			toml_str += ", scalar = " + format_float(p.radiance_scalar);
			toml_str += ", radius = " + format_float(p.radius);
			toml_str += ", smoothness = " + format_float(p.smoothness);

			// only write shaping if deg != 180
			if (!utils::float_equal(p.degrees, 180.0f))
			{
				toml_str += ", direction = [" + format_float(p.direction.x) + ", " + format_float(p.direction.y) + ", " + format_float(p.direction.z) + "]";
				toml_str += ", degrees = " + format_float(p.degrees);
				toml_str += ", softness = " + format_float(p.softness);
				toml_str += ", exponent = " + format_float(p.exponent);
			}

			// ignore t0
			if (!utils::float_equal(p.timepoint, 0.0f)) {
				toml_str += ", timepoint = " + format_float(p.timepoint);
			}

			toml_str += " },\n";
		}

		// end points
		toml_str += "        ]";

		if (def.run_once) {
			toml_str += ", run_once = " + (def.run_once ? "true"s : "false"s);
		}

		if (def.loop) {
			toml_str += ", loop = " + (def.loop ? "true"s : "false"s);
		}

		if (def.loop_smoothing) {
			toml_str += ", loop_smoothing = " + (def.loop_smoothing ? "true"s : "false"s);
		}

		toml_str += " },";
		return toml_str;
	}

	/// Builds a string containing all map markers for the current map
	/// @param areas	map_settings -> area overrides
	/// @return			the final string in toml format
	std::string build_map_marker_string_for_current_map(const std::vector<map_settings::marker_settings_s>& markers)
	{
		std::string toml_str = map_settings::get_map_settings().mapname + " = [\n"s;
		for (auto& m : markers)
		{
			if (!m.comment.empty()) {
				toml_str += "\n        # " + m.comment + "\n";
			}
			
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

			toml_str += ", position = [" + format_float(m.origin.x) + ", " + format_float(m.origin.y) + ", " + format_float(m.origin.z) + "]";
			toml_str += ", rotation = [" + format_float(RAD2DEG(m.rotation.x)) + ", " + format_float(RAD2DEG(m.rotation.y)) + ", " + format_float(RAD2DEG(m.rotation.z)) + "]";

			if (m.no_cull) {
				toml_str += ", scale = [" + format_float(m.scale.x) + ", " + format_float(m.scale.y) + ", " + format_float(m.scale.z) + "]";
			}

			toml_str += " },\n";
		}
		toml_str += "    ]";

		return toml_str;
	}

	/// Builds a string containing all culling overrides for the current map
	/// @param areas	map_settings -> area overrides
	/// @return			the final string in toml format
	std::string build_culling_overrides_string_for_current_map(const std::unordered_map<std::uint32_t, map_settings::area_overrides_s>& areas)
	{
		// temp map to sort areas by area number
		std::map<int, map_settings::area_overrides_s> sorted_area_settings(areas.begin(), areas.end());

		std::string toml_str = map_settings::get_map_settings().mapname + " = [\n"s;
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

			if (a.cull_mode != map_settings::AREA_CULL_MODE::AREA_CULL_INFO_DEFAULT) {
				toml_str += ", cull = " + std::to_string(a.cull_mode);
			}

			if (a.cull_mode >= map_settings::AREA_CULL_INFO_NOCULLDIST_START
				&& a.cull_mode <= map_settings::AREA_CULL_INFO_NOCULLDIST_END)
			{
				toml_str += ", nocull_dist = " + format_float(a.nocull_distance);
			}

			if (!a.leaf_tweaks.empty())
			{
				is_multiline = true;
				auto leaf_count = 0u;
				toml_str += ", leaf_tweak = [\n";
				for (auto& lf : a.leaf_tweaks)
				{
					if (!lf.in_leafs.empty() && !(lf.areas.empty() && lf.leafs.empty()))
					{
						// temp set to sort in_leafs
						std::set<int> sorted_twk_inleafs(lf.in_leafs.begin(), lf.in_leafs.end());

						// {
						toml_str += "                { in_leafs = [\n                    ";
						for (auto it = sorted_twk_inleafs.begin(); it != sorted_twk_inleafs.end(); ++it)
						{
							if (it != sorted_twk_inleafs.begin()) 
							{
								toml_str += ", ";

								// new line every X leafs
								if (!(leaf_count % 8))
								{
									toml_str += "\n                    ";
									is_multiline = true;
								}
							}
							toml_str += std::to_string(*it);
							leaf_count++;
						}

						leaf_count = 0u;
						if (!lf.areas.empty())
						{
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
						}

						if (!lf.leafs.empty())
						{
							// temp set to sort leafs
							std::set<int> sorted_twk_leafs(lf.leafs.begin(), lf.leafs.end());

							toml_str += "], leafs = [";
							for (auto it = sorted_twk_leafs.begin(); it != sorted_twk_leafs.end(); ++it)
							{
								if (it != sorted_twk_leafs.begin()) {
									toml_str += ", ";
								}
								toml_str += std::to_string(*it);
							}
						}

						toml_str += "]";

						if (lf.nocull_dist > 0.0f) {
							toml_str += ", nocull_dist = " + format_float(lf.nocull_dist);
						}

						// }
						toml_str += " },\n";
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

			/*if (is_multiline && ar_num != (int)sorted_area_settings.size() - 1) {
				toml_str += "\n";
			}*/

		} // end loop
		toml_str += "    ]";

		return toml_str;
	}
}