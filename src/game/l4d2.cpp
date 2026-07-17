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
	// -------------------------------------------
	// game variables

	// - server
	void* mdl_cache = nullptr;

	// - engine

	// - client
	Vector* g_vecCurrentRenderOrigin = nullptr;


	// -------------------------------------------
	// game functions

	// - server
	GetCurrentSkyCamera_t GetCurrentSkyCamera = nullptr;

	// - engine
	R_CullNode_t R_CullNode = nullptr;
	CM_LeafArea_t CM_LeafArea = nullptr;

	// - client


	// -------------------------------------------
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
	uint32_t nop_addr__cullnode_backface_check01 = 0u;
	uint32_t nop_addr__cullnode_backface_check02 = 0u;
	uint32_t nop_addr__drawleaf_backface_check = 0u;
	uint32_t nop_addr__draw_opaque_bmodel_backface_check = 0u;

	// - client
	uint32_t hk_addr__cviewrenderer_renderview = 0u;
	uint32_t hk_addr__skyboxview_draw_internal = 0u;
	uint32_t jmp_addr__extract_culled_renderables = 0u;
	uint32_t nop_addr__simple_world_view_intersect_water_check = 0u;
	uint32_t hk_addr__draw_player_thirdperson_mesh_check01 = 0u;
	uint32_t hk_addr__draw_player_thirdperson_mesh_check02 = 0u;
	uint32_t hk_addr__draw_player_thirdperson_mesh_check03 = 0u;
	uint32_t retn_addr__draw_player_thirdperson_mesh = 0u;
	uint32_t hk_addr__impact_marks_pshadow = 0u;
	uint32_t retn_addr__impact_marks_pshadow_skip = 0u;

	// -------------------------------------------


#define PATTERN_OFFSET_SIMPLE(mod, var, pattern, byte_offset, static_addr) \
		if (const auto offset = utils::mem::find_pattern(mod, ##pattern, byte_offset, #var, use_pattern, static_addr); offset) { \
			(var) = offset; found_pattern_count++; \
		} total_pattern_count++;

#define PATTERN_OFFSET_SIMPLE_CAST(mod, var, type, pattern, byte_offset, static_addr) \
		if (const auto offset = utils::mem::find_pattern(mod, ##pattern, byte_offset, #var, use_pattern, static_addr); offset) { \
			(var) = (type)offset; found_pattern_count++; \
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

		// - server
		PATTERN_OFFSET_SIMPLE(SERVER_MOD, hk_addr__scene_ent_on_start_event, "8B 7D ? 8B F1 68 ? ? ? ? 8B CF 89 45", 0, 0x1C0765);
		PATTERN_OFFSET_SIMPLE(SERVER_MOD, hk_addr__scene_ent_on_finish_event, "8B 86 ?? ?? ?? ?? 85 C0 75 ?? B8 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 53", 0, 0x1C7813);
		PATTERN_OFFSET_SIMPLE_CAST(SERVER_MOD, GetCurrentSkyCamera, GetCurrentSkyCamera_t, "A1 ? ? ? ? 83 F8 ? 74 ? 8B 15 ? ? ? ? 8B C8 81 E1 ? ? ? ? 03 C9 8D 4C CA ? 85 C9 74 ? 8B D0", 0, 0x1D0D10);
		PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(SERVER_MOD, mdl_cache, void*, "8B 0D ?? ?? ?? ?? ?? ?? 50 8B 82 ?? ?? ?? ?? FF D0 8B F8 8B 83", 2, 0x31251);

		

		// - engine
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, hk_addr__on_map_load, "8D 4F ? 33 D2", 0, 0xEE05C);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, hk_addr__on_host_disconnect, "8B EC 83 EC ? FF 15 ? ? ? ? 84 C0", 0, 0x192F11);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, hk_addr__on_host_change_level, "6A ? 8D 4D ? 56 51 E8 ? ? ? ? 83 C4 ? 3B DF", 0, 0x18D048);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr__cdispinfo_render, "75 ? 53 38 45", 0, 0xB13E5);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, hk_addr__pre_recursive_world_node, "8B 50 ? 8B CB E8", 0, 0xD1648);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, jmp_addr__cullnode01, "7C ? 53 E8", 0, 0xCD7E5);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, retn_addr__cullnode_cull, "5F 8B C6 5E 5B 8B E5 5D C3 53", 0, 0xCD935);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, retn_addr__cullnode_skip, "? ? ? 0F 8D ? ? ? ? 8B 43 ? 8A 48 ? 80 F9 ? 77 ? 0F B6 D1", 0, 0xCD7F8);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr__cullnode_backface_check01, "74 ? ? ? F7 C1", 0, 0xCD8C1);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr__cullnode_backface_check02, "75 ? 8B D1 C1 EA", 0, 0xCD8CB);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr__drawleaf_backface_check, "0F 87 ? ? ? ? 8B DA", 0, 0xCD4E7);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr__draw_opaque_bmodel_backface_check, "74 ? 0F BF 4B", 0, 0xD2250);
		PATTERN_OFFSET_SIMPLE_CAST(ENGINE_MOD, R_CullNode, R_CullNode_t, "55 8B EC 80 3D ? ? ? ? ? 8B 4D", 0, 0xFC490);
		PATTERN_OFFSET_SIMPLE_CAST(ENGINE_MOD, CM_LeafArea, CM_LeafArea_t, "55 8B EC 56 8B 75 ? 85 F6 78 ? 3B 35 ? ? ? ? 7C ? 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 ? A1 ? ? ? ? 03 F6 66 8B 4C F0", 0, 0x14C2C0);

		// - client
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__cviewrenderer_renderview, "? ? 8B 50 ? FF D2 8B 0D ? ? ? ? E8 ? ? ? ? E8", 0, 0x1D7113);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__skyboxview_draw_internal, "83 C4 ? 80 7D ? ? 74 ? E8", 0, 0x1D3F1D);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, jmp_addr__extract_culled_renderables, "74 ? 8B 45 ? FF 48 ? EB ? 0F 57 C0", 0, 0xBDA76);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, nop_addr__simple_world_view_intersect_water_check, "74 ? 83 8E ? ? ? ? ? EB ? 80 7F", 0, 0x1CF46F);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__draw_player_thirdperson_mesh_check01, "83 C4 ? 3B C1 0F 85", 0, 0x223371);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__draw_player_thirdperson_mesh_check02, "6A ? FF D0 85 C0 0F 85 ? ? ? ? 8B 56", 0, 0x223387);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__draw_player_thirdperson_mesh_check03, "8B CF FF D0 84 C0 75 ? 53", 0, 0x22339F);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, retn_addr__draw_player_thirdperson_mesh, "8B 45 ? 8B 4D ? 50 51 8B CE E8 ? ? ? ? 5F 5E 5D", 0, 0x22341F);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__impact_marks_pshadow, "B1 ? 84 4B", 0, 0xF894E);
		if (hk_addr__impact_marks_pshadow) {
			retn_addr__impact_marks_pshadow_skip = utils::mem::resolve_relative_jump_address(l4d2::hk_addr__impact_marks_pshadow + 5u, 6u, 2u); found_pattern_count++;
		} total_pattern_count++;

		PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(CLIENT_MOD, g_vecCurrentRenderOrigin, Vector*, "F3 0F 11 05 ? ? ? ? F3 0F 10 40 ? 68 ? ? ? ? F3 0F 11 05", 4, 0x1CCF5A);




		
		

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
