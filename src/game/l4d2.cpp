#include "std_include.hpp"

#include "components/modules/choreo_events.hpp"
#include "components/modules/game_settings.hpp"
#include "components/modules/imgui.hpp"
#include "components/modules/interfaces.hpp"
#include "components/modules/main_module.hpp"
#include "components/modules/model_render.hpp"
#include "components/modules/remix_api.hpp"
#include "components/modules/remix_lights.hpp"
#include "components/modules/remix_markers.hpp"
#include "components/modules/sound_events.hpp"

namespace l4d2
{
	// --------------
	// game variables

	uint32_t hk_addr__scene_ent_on_start_event = 0u;
	


	// --------------
	// game functions



	// --------------
	// game asm offsets

#define PATTERN_OFFSET_SIMPLE(mod, var, pattern, byte_offset, static_addr) \
		if (const auto offset = utils::mem::find_pattern(mod, ##pattern, byte_offset, #var, use_pattern, static_addr); offset) { \
			(var) = offset; found_pattern_count++; \
		} total_pattern_count++;

#define PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(mod, var, type, pattern, byte_offset, static_addr) \
		if (const auto offset = utils::mem::find_pattern(mod, ##pattern, byte_offset, #var, use_pattern, static_addr); offset) { \
			(var) = (type)*(DWORD*)offset; found_pattern_count++; \
		} total_pattern_count++;

	// init any adresses here
	void init_game_addresses()
	{
		const bool use_pattern = !utils::flags::has_flag("no_pattern");
		if (use_pattern) {
			utils::log("L4D2", "Getting offsets ...", utils::LOG_TYPE::LOG_TYPE_DEFAULT, false);
		}

		std::uint32_t total_pattern_count = 0u;
		std::uint32_t found_pattern_count = 0u;

#pragma region GAME_VARIABLES

		//if (const auto offset = utils::mem::find_pattern("FF 35 ? ? ? ? C7 44 24 ? ? ? ? ? E8 ? ? ? ? 83 C4 ? 85 C0", 2, "d3d_dev_addr", use_pattern, 0x4208FD); offset) {
		//	d3d_dev_addr = (DWORD*)*(DWORD*)offset; found_pattern_count++;
		//} total_pattern_count++;
		
		//PATTERN_OFFSET_SIMPLE(retn_addr__special_SetupVsPsPass_handling, "C7 05 ? ? ? ? ? ? ? ? B0 ? 5F C3", 0, 0x6091F1);
		
		PATTERN_OFFSET_SIMPLE(SERVER_MOD, hk_addr__scene_ent_on_start_event, "8B 7D ? 8B F1 68 ? ? ? ? 8B CF 89 45", 0, 0x1C0765);

		// end GAME_ASM_OFFSETS
#pragma endregion

		if (use_pattern)
		{
			if (found_pattern_count == total_pattern_count) {
				utils::log("L4D2", std::format("Found all '{:d}' Patterns.", total_pattern_count), utils::LOG_TYPE::LOG_TYPE_GREEN, true);
			}
			else
			{
				utils::log("L4D2", std::format("Only found '{:d}' out of '{:d}' Patterns.", found_pattern_count, total_pattern_count), utils::LOG_TYPE::LOG_TYPE_ERROR, true);
				utils::log("L4D2", ">> Please create an issue on GitHub and attach this console log and information about your game (version, platform etc.)", utils::LOG_TYPE::LOG_TYPE_STATUS, true);
			}
		}
	}

#undef PATTERN_OFFSET_SIMPLE

	void main()
	{
		loader::module_loader::register_module(std::make_unique<interfaces>());
		loader::module_loader::register_module(std::make_unique<game_settings>());
		loader::module_loader::register_module(std::make_unique<remix_api>());
		loader::module_loader::register_module(std::make_unique<choreo_events>());
		loader::module_loader::register_module(std::make_unique<sound_events>());
		loader::module_loader::register_module(std::make_unique<main_module>());
		loader::module_loader::register_module(std::make_unique<model_render>());
		loader::module_loader::register_module(std::make_unique<imgui>());
		loader::module_loader::register_module(std::make_unique<remix_vars>());
		loader::module_loader::register_module(std::make_unique<remix_markers>());
		loader::module_loader::register_module(std::make_unique<map_settings>());
		loader::module_loader::register_module(std::make_unique<remix_lights>());

		MH_EnableHook(MH_ALL_HOOKS);
	}
}
