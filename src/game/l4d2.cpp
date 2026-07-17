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

	


	// --------------
	// game functions


	// --------------
	// game asm offsets

	// - server
	uint32_t hk_addr__scene_ent_on_start_event = 0u;
	uint32_t hk_addr__scene_ent_on_finish_event = 0u;

	// - engine
	uint32_t hk_addr__on_map_load = 0u;
	uint32_t hk_addr__on_host_disconnect = 0u;
	uint32_t hk_addr__on_host_change_level = 0u;
	uint32_t nop_addr__cdispinfo_render = 0u;
	uint32_t hk_addr__pre_recursive_world_node = 0u;
	uint32_t jmp_addr__cullnode01 = 0u;
	uint32_t retn_addr__cullnode_cull = 0u;
	uint32_t retn_addr__cullnode_skip = 0u;
	uint32_t nop_addr_cullnode_backface_check01 = 0u;
	uint32_t nop_addr_cullnode_backface_check02 = 0u;
	uint32_t nop_addr_drawleaf_backface_check = 0u;
	uint32_t nop_addr_draw_opaque_bmodel_backface_check = 0u;

	// - client
	uint32_t hk_addr__cviewrenderer_renderview = 0u;
	uint32_t hk_addr__skyboxview_draw_internal = 0u;

	
	uint32_t hk_addr__01 = 0u;

	uint32_t retn_addr__01 = 0u;

	uint32_t jmp_addr__01 = 0u;

	uint32_t nop_addr__01 = 0u;


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

		// - server
		PATTERN_OFFSET_SIMPLE(SERVER_MOD, hk_addr__scene_ent_on_start_event, "8B 7D ? 8B F1 68 ? ? ? ? 8B CF 89 45", 0, 0x1C0765);
		PATTERN_OFFSET_SIMPLE(SERVER_MOD, hk_addr__scene_ent_on_finish_event, "8B 86 ?? ?? ?? ?? 85 C0 75 ?? B8 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 53", 0, 0x1C7813);
		
		// - engine
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, hk_addr__on_map_load, "8D 4F ? 33 D2", 0, 0xEE05C);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, hk_addr__on_host_disconnect, "8B EC 83 EC ? FF 15 ? ? ? ? 84 C0", 0, 0x192F11);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, hk_addr__on_host_change_level, "6A ? 8D 4D ? 56 51 E8 ? ? ? ? 83 C4 ? 3B DF", 0, 0x18D048);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr__cdispinfo_render, "75 ? 53 38 45", 0, 0xB13E5);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, hk_addr__pre_recursive_world_node, "8B 50 ? 8B CB E8", 0, 0xD1648);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, jmp_addr__cullnode01, "7C ? 53 E8", 0, 0xCD7E5);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, retn_addr__cullnode_cull, "5F 8B C6 5E 5B 8B E5 5D C3 53", 0, 0xCD935);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, retn_addr__cullnode_skip, "? ? ? 0F 8D ? ? ? ? 8B 43 ? 8A 48 ? 80 F9 ? 77 ? 0F B6 D1", 0, 0xCD7F8);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr_cullnode_backface_check01, "74 ? ? ? F7 C1", 0, 0xCD8C1);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr_cullnode_backface_check02, "75 ? 8B D1 C1 EA", 0, 0xCD8CB);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr_drawleaf_backface_check, "0F 87 ? ? ? ? 8B DA", 0, 0xCD4E7);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr_draw_opaque_bmodel_backface_check, "74 ? 0F BF 4B", 0, 0xD2250);


		// - client
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__cviewrenderer_renderview, "? ? 8B 50 ? FF D2 8B 0D ? ? ? ? E8 ? ? ? ? E8", 0, 0x1D7113);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__skyboxview_draw_internal, "83 C4 ? 80 7D ? ? 74 ? E8", 0, 0x1D3F1D);


		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__skyboxview_draw_internal, "", 0, 0x0);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__skyboxview_draw_internal, "", 0, 0x0);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__skyboxview_draw_internal, "", 0, 0x0);

		
		

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
		init_game_addresses();

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
