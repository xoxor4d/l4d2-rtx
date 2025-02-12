#pragma once

#define TOML_ERROR(TITLE, ENTRY, MSG, ...) \
	game::console(); std::cout << toml::format_error(toml::make_error_info(#TITLE, (ENTRY), utils::va(#MSG, __VA_ARGS__))) << std::endl; \

namespace common::toml
{
	std::string build_light_string_for_single_light(const map_settings::remix_light_settings_s& def);
	std::string build_map_marker_string_for_current_map(const std::vector<map_settings::marker_settings_s>& markers);
	std::string build_culling_overrides_string_for_current_map(const std::unordered_map<std::uint32_t, map_settings::area_overrides_s>& areas);
}