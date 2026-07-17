#pragma once
#include "components/modules/map_settings.hpp"

#define TOML_ERROR(TITLE, ENTRY, MSG, ...) \
	utils::log("MapSettings", toml::format_error(toml::make_error_info(#TITLE, (ENTRY), utils::va(#MSG, __VA_ARGS__))) + "\n", utils::LOG_TYPE::LOG_TYPE_ERROR, true); \

namespace common::toml
{
	std::string build_light_string_for_single_light(const components::map_settings::remix_light_settings_s& def);
	std::string build_map_marker_string_for_current_map(const std::vector<components::map_settings::marker_settings_s>& markers);
	std::string build_culling_overrides_string_for_current_map(const std::unordered_map<std::uint32_t, components::map_settings::area_overrides_s>& areas);
}
