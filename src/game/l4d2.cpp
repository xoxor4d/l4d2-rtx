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
	CRender* engine_renderer = nullptr;
	DWORD* hoststate_worldbrush_data_ptr = nullptr;
	Vector* current_view_origin = nullptr;
	Vector* current_view_forward = nullptr;

	// - client
	Vector* g_vecCurrentRenderOrigin = nullptr;
	Vector4D* s_viewFadeColor = nullptr;
	DWORD* material_system_ptr = nullptr;
	DWORD* modelinfo_ptr = nullptr;
	Vector* camera_forward_vector = nullptr;

	// - shaderapidx9
	DWORD* d3d_device_ptr = nullptr;
	DWORD* shaderapi_ptr = nullptr;


	// -------------------------------------------
	// game functions

	// - server
	GetCurrentSkyCamera_t GetCurrentSkyCamera = nullptr;

	// - engine
	R_CullNode_t R_CullNode = nullptr;
	CM_LeafArea_t CM_LeafArea = nullptr;

	// - client
	//

	// - shaderapidx9
	//


	// -------------------------------------------
	// game asm offsets

	// - server
	uint32_t hk_addr__scene_ent_on_start_event = 0u;
	uint32_t hk_addr__scene_ent_on_finish_event = 0u;
	uint32_t fn_addr__util_remove = 0u;

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
	uint32_t hk_addr__start_sound = 0u;

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
	uint32_t hk_addr__render_spritecard_new = 0u;
	uint32_t hk_addr__rope_mgr_draw_render_cache = 0u;
	uint32_t hk_addr__glow_overlay_draw = 0u;
	uint32_t nop_addr__func_area_portal_window_draw_mdl = 0u;
	uint32_t fn_addr__add_console_cmd = 0u;
	uint32_t fn_addr__debug_overlay_add_text = 0u;
	uint32_t fn_addr__debug_overlay_add_text_colored = 0u;

	// - shaderapidx9
	uint32_t kh_addr__cmeshdx8_renderpass_pre_draw = 0u;
	uint32_t kh_addr__cmeshdx8_renderpass_post_draw = 0u;
	uint32_t retn_addr__cmeshdx8_renderpass_post_draw = 0u;

	// - studiorender
	uint32_t hk_addr__studio_draw_static_mesh = 0u;

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

		// --------------------------
		// - server - variables
		PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(SERVER_MOD, mdl_cache, void*, "8B 0D ?? ?? ?? ?? ?? ?? 50 8B 82 ?? ?? ?? ?? FF D0 8B F8 8B 83", 2, 0x31251);

		// - server - functions
		PATTERN_OFFSET_SIMPLE_CAST(SERVER_MOD, GetCurrentSkyCamera, GetCurrentSkyCamera_t, "A1 ? ? ? ? 83 F8 ? 74 ? 8B 15 ? ? ? ? 8B C8 81 E1 ? ? ? ? 03 C9 8D 4C CA ? 85 C9 74 ? 8B D0", 0, 0x1D0D10);

		// - server - asm
		PATTERN_OFFSET_SIMPLE(SERVER_MOD, hk_addr__scene_ent_on_start_event, "8B 7D ? 8B F1 68 ? ? ? ? 8B CF 89 45", 0, 0x1C0765);
		PATTERN_OFFSET_SIMPLE(SERVER_MOD, hk_addr__scene_ent_on_finish_event, "8B 86 ?? ?? ?? ?? 85 C0 75 ?? B8 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 53", 0, 0x1C7813);
		PATTERN_OFFSET_SIMPLE(SERVER_MOD, fn_addr__util_remove, "55 8B EC 8B 45 ? 85 C0 74 ? 83 C0", 0, 0x2071E0);


		// --------------------------
		// - engine - variables
		if (const auto offset = utils::mem::find_pattern(ENGINE_MOD, "8B 0D ? ? ? ? 8B 75 ? ? ? 8B 55", 2, "engine_renderer", use_pattern, 0xAA3B2); offset) {
			engine_renderer = (CRender*)*(DWORD*)*(DWORD*)offset; found_pattern_count++;
		} total_pattern_count++;

		PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(ENGINE_MOD, hoststate_worldbrush_data_ptr, DWORD*, "A1 ? ? ? ? 8B 50 ? 53 56", 1, 0x5DB53);
		PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(ENGINE_MOD, current_view_origin, Vector*, "F3 0F 10 05 ? ? ? ? F3 0F 10 0D ? ? ? ? F3 0F 10 15 ? ? ? ? 53", 4, 0xB9843);
		PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(ENGINE_MOD, current_view_forward, Vector*, "68 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 ? 56", 1, 0xC3DE9);


		// - engine - functions
		PATTERN_OFFSET_SIMPLE_CAST(ENGINE_MOD, R_CullNode, R_CullNode_t, "55 8B EC 80 3D ? ? ? ? ? 8B 4D", 0, 0xFC490);
		PATTERN_OFFSET_SIMPLE_CAST(ENGINE_MOD, CM_LeafArea, CM_LeafArea_t, "55 8B EC 56 8B 75 ? 85 F6 78 ? 3B 35 ? ? ? ? 7C ? 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 ? A1 ? ? ? ? 03 F6 66 8B 4C F0", 0, 0x14C2C0);

		// - engine - asm
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, hk_addr__on_map_load, "8D 4F ? 33 D2", 0, 0xEE05C);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, hk_addr__on_host_disconnect, "8B EC 83 EC ? FF 15 ? ? ? ? 84 C0", 0, 0x192F11);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, hk_addr__on_host_change_level, "6A ? 8D 4D ? 56 51 E8 ? ? ? ? 83 C4 ? 3B DF", 0, 0x18D048);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr__cdispinfo_render, "75 ? 53 38 45", 0, 0xB13E5);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, hk_addr__pre_recursive_world_node, "8B 50 ? 8B CB E8", 0, 0xD1648);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, jmp_addr__cullnode01, "7C ? 53 E8 ? ? ? ? 83 C4 04", 0, 0xCD7E5);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, retn_addr__cullnode_cull, "5F 8B C6 5E 5B 8B E5 5D C3 53", 0, 0xCD935);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, retn_addr__cullnode_skip, "? ? ? 0F 8D ? ? ? ? 8B 43 ? 8A 48 ? 80 F9 ? 77 ? 0F B6 D1", 0, 0xCD7F8);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr__cullnode_backface_check01, "74 ? ? ? F7 C1", 0, 0xCD8C1);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr__cullnode_backface_check02, "75 ? 8B D1 C1 EA", 0, 0xCD8CB);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr__drawleaf_backface_check, "0F 87 ? ? ? ? 8B DA", 0, 0xCD4E7);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, nop_addr__draw_opaque_bmodel_backface_check, "74 ? 0F BF 4B", 0, 0xD2250);
		PATTERN_OFFSET_SIMPLE(ENGINE_MOD, hk_addr__start_sound, "53 56 33 C9", 0, 0x1C0B6);

		if (const auto offset = utils::mem::find_pattern(ENGINE_MOD, "E8 ? ? ? ? 83 C4 ? 8B 0D ? ? ? ? ? ? 8B 40 ? 8D 95", 0, "fn_addr__debug_overlay_add_text", use_pattern, 0x1C004); offset) {
			fn_addr__debug_overlay_add_text = utils::mem::resolve_relative_call_address(offset); found_pattern_count++;
		} total_pattern_count++;

		if (const auto offset = utils::mem::find_pattern(ENGINE_MOD, "E8 ? ? ? ? 83 C4 ? 5F 5E 8B 4D ? 33 CD E8 ? ? ? ? 8B E5 5D 8B E3 5B C3 ? ? 68", 0, "fn_addr__debug_overlay_add_text_colored", use_pattern, 0xDB33F); offset) {
			fn_addr__debug_overlay_add_text_colored = utils::mem::resolve_relative_call_address(offset); found_pattern_count++;
		} total_pattern_count++;


		// --------------------------
		// - client - variables
		PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(CLIENT_MOD, g_vecCurrentRenderOrigin, Vector*, "F3 0F 11 05 ? ? ? ? F3 0F 10 40 ? 68 ? ? ? ? F3 0F 11 05", 4, 0x1CCF5A);
		PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(CLIENT_MOD, s_viewFadeColor, Vector4D*, "81 C1 ? ? ? ? ? ? ? ? 0F 57 C9", 2, 0x1C44F3);
		PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(CLIENT_MOD, material_system_ptr, DWORD*, "A1 ? ? ? ? 53 56 57 89 4D ? 89 45", 1, 0x11B76);
		PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(CLIENT_MOD, modelinfo_ptr, DWORD*, "8B 0D ? ? ? ? ? ? 50 8B 42 ? FF D0 85 C0 74 ? 8B 0D ? ? ? ? ? ? 50 8B 82", 2, 0x19236);
		PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(CLIENT_MOD, camera_forward_vector, Vector*, "8D 93 ? ? ? ? 89 4D", 2, 0x1BAB58);

		

		// - client - functions

		// - client - asm
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

		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__render_spritecard_new, "8B 8D ? ? ? ? ? ? 8B 40 ? 8D 95 ? ? ? ? 52 8B 95 ? ? ? ? 52 8B 95 ? ? ? ? 52 FF D0 8B 8D", 0, 0x3C9F1F); PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__render_spritecard_new, "8B 8D ? ? ? ? ? ? 8B 40 ? 8D 95 ? ? ? ? 52 8B 95 ? ? ? ? 52 8B 95 ? ? ? ? 52 FF D0 8B 8D", 0, 0x3C9F1F);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__rope_mgr_draw_render_cache, "8B 8D ? ? ? ? ? ? 8B 52 ? 8D 85 ? ? ? ? 50 8B 85 ? ? ? ? 50 8B 45 ? 50 FF D2 8B 8D ? ? ? ? 89 9D ? ? ? ? 89 9D ? ? ? ? 89 5D ? 89 5D ? C7 85 ? ? ? ? ? ? ? ? 89 9D", 0, 0x93A3D);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, hk_addr__glow_overlay_draw, "F3 0F 10 85 ? ? ? ? F3 0F 59 C0 F3 0F 11 85 ? ? ? ? F3 0F 10 85 ? ? ? ? F3 0F 10 8D ? ? ? ? F3 0F 59 C0 F3 0F 58 C8 F3 0F 10 85 ? ? ? ? F3 0F 59 C0 F3 0F 58 C8 F3 0F 10 05", 0, 0x1080C0);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, nop_addr__func_area_portal_window_draw_mdl, "75 ? 33 C0 5F 8B E5 5D C2 ? ? ? ? 8B 50", 0, 0x7690E);
		PATTERN_OFFSET_SIMPLE(CLIENT_MOD, fn_addr__add_console_cmd, "55 8B EC 8B 45 ? 53 33 DB 56 8B F1 8B 4D ? 80 66", 0, 0x3D36F0);


		// --------------------------
		// - shaderapidx9 - variables
		PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(RENDERER_MOD, d3d_device_ptr, DWORD*, "A1 ? ? ? ? 50 8B CE E8 ? ? ? ? 84 DB 75", 1, 0x5E5C);
		PATTERN_OFFSET_DWORD_PTR_CAST_TYPE(RENDERER_MOD, shaderapi_ptr, DWORD*, "8B 0D ? ? ? ? ? ? 8B 90 ? ? ? ? FF D2 ? ? 8B C8 8B 82 ? ? ? ? FF E0", 2, 0x6870);

		// - shaderapidx9 - functions


		// - shaderapidx9 - asm
		PATTERN_OFFSET_SIMPLE(RENDERER_MOD, kh_addr__cmeshdx8_renderpass_pre_draw, "8B 43 ? 3B C7", 0, 0xBEFA);
		PATTERN_OFFSET_SIMPLE(RENDERER_MOD, kh_addr__cmeshdx8_renderpass_post_draw, "56 51 50 FF D2", 0, 0xC05A);
		if (kh_addr__cmeshdx8_renderpass_post_draw) {
			retn_addr__cmeshdx8_renderpass_post_draw = utils::mem::resolve_relative_jump_address(l4d2::kh_addr__cmeshdx8_renderpass_post_draw + 5u, 5u, 1u); found_pattern_count++;
		} total_pattern_count++;


		// --------------------------
		// - studiorender - variables


		// - studiorender - functions


		// - studiorender - asm
		PATTERN_OFFSET_SIMPLE(STUDIORENDER_MOD, hk_addr__studio_draw_static_mesh, "8B 43 ? F6 40 ? ? 75", 0, 0xEF7B);


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
