#pragma once
#include "components/modules/map_settings.hpp"
#include "utils/console.hpp"
#include "toml.hpp"

#define TOML_ERROR(TITLE, ENTRY, MSG, ...) \
	utils::log("MapSettings", toml::format_error(toml::make_error_info(#TITLE, (ENTRY), utils::va(#MSG, __VA_ARGS__))) + "\n", utils::LOG_TYPE::LOG_TYPE_ERROR, true); \

#define TOML_CATCH_ERROR_WHAT	{ utils::log("Toml", std::format("{}", err.what()), utils::LOG_TYPE::LOG_TYPE_ERROR, true); }

#define TOML_CATCH_SYNTAX_ERROR	catch (toml::syntax_error& err) TOML_CATCH_ERROR_WHAT
#define TOML_CATCH_TYPE_ERROR	catch (toml::type_error& err) TOML_CATCH_ERROR_WHAT

namespace common::toml_ext
{
	std::string build_light_string_for_single_light(const components::map_settings::remix_light_settings_s& def);
	std::string build_map_marker_string_for_current_map(const std::vector<components::map_settings::marker_settings_s>& markers);
	std::string build_culling_overrides_string_for_current_map(const std::unordered_map<std::uint32_t, components::map_settings::area_overrides_s>& areas);

	// format 2 decimals
	inline std::string format_float(float value)
	{
		return std::format("{:.2f}", value);
	}

	inline bool to_bool(const toml::value& entry, const bool default_setting = false)
	{
		if (entry.is_boolean()) {
			return static_cast<bool>(entry.as_boolean());
		}

		if (entry.is_integer()) {
			return static_cast<bool>(entry.as_integer());
		}

		try { // this will fail and let the user know whats wrong
			return static_cast<bool>(entry.as_boolean());
		} TOML_CATCH_TYPE_ERROR;

		//toml::format_error(toml::make_error_info

		return default_setting;
	}

	inline int to_int(const toml::value& entry, const int default_setting = 0)
	{
		if (entry.is_boolean()) {
			return static_cast<int>(entry.as_boolean());
		}

		if (entry.is_integer()) {
			return static_cast<int>(entry.as_integer());
		}

		if (entry.is_floating()) {
			return static_cast<int>(entry.as_floating());
		}

		try { // this will fail and let the user know whats wrong
			return static_cast<int>(entry.as_integer());
		} TOML_CATCH_TYPE_ERROR;

		return default_setting;
	}

	inline std::uint32_t to_uint(const toml::value& entry, const std::uint32_t default_val = 0u)
	{
		if (entry.is_floating()) {
			return static_cast<std::uint32_t>(entry.as_floating());
		}

		if (entry.is_integer()) {
			return static_cast<std::uint32_t>(entry.as_integer());
		}

		try { // this will fail and let the user know whats wrong
			return static_cast<std::uint32_t>(entry.as_integer());
		} TOML_CATCH_TYPE_ERROR;

		return default_val;
	}

	inline float to_float(const toml::value& entry, const float default_setting = 0.0f)
	{
		if (entry.is_integer()) {
			return static_cast<float>(entry.as_integer());
		}

		if (entry.is_floating()) {
			return static_cast<float>(entry.as_floating());
		}

		try { // this will fail and let the user know whats wrong
			return static_cast<float>(entry.as_floating());
		} TOML_CATCH_TYPE_ERROR;

		return default_setting;
	}
}
