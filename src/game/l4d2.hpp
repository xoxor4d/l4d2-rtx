#pragma once

namespace l4d2
{
	// -------------------------------------------
	// game variables

	// - server
	extern void* mdl_cache;

	// - engine
	// 

	// - client
	extern Vector* g_vecCurrentRenderOrigin;
	extern Vector4D* s_viewFadeColor;

	// - shaderapidx9
	// 

	// -------------------------------------------
	// game functions

	// - server
	typedef	CSkyCamera*(__cdecl* GetCurrentSkyCamera_t)();
	extern GetCurrentSkyCamera_t GetCurrentSkyCamera;

	// - engine
	typedef	bool (__cdecl* R_CullNode_t)(mnode_t*);
	extern R_CullNode_t R_CullNode;

	typedef	bool(__cdecl* CM_LeafArea_t)(int leaf_num);
	extern CM_LeafArea_t CM_LeafArea;

	// - shaderapidx9

	// -------------------------------------------
	// game asm offsets

	// - server
	extern uint32_t hk_addr__scene_ent_on_start_event;
	extern uint32_t hk_addr__scene_ent_on_finish_event;

	// - engine
	extern uint32_t hk_addr__on_map_load;
	extern uint32_t hk_addr__on_host_disconnect;
	extern uint32_t hk_addr__on_host_change_level;
	extern uint32_t nop_addr__cdispinfo_render;
	extern uint32_t hk_addr__pre_recursive_world_node;
	extern uint32_t jmp_addr__cullnode01;
	extern uint32_t retn_addr__cullnode_cull;
	extern uint32_t retn_addr__cullnode_skip;
	extern uint32_t nop_addr__cullnode_backface_check01;
	extern uint32_t nop_addr__cullnode_backface_check02;
	extern uint32_t nop_addr__drawleaf_backface_check;
	extern uint32_t nop_addr__draw_opaque_bmodel_backface_check;

	// - client
	extern uint32_t hk_addr__cviewrenderer_renderview;
	extern uint32_t hk_addr__skyboxview_draw_internal;
	extern uint32_t jmp_addr__extract_culled_renderables;
	extern uint32_t nop_addr__simple_world_view_intersect_water_check;
	extern uint32_t hk_addr__draw_player_thirdperson_mesh_check01;
	extern uint32_t hk_addr__draw_player_thirdperson_mesh_check02;
	extern uint32_t hk_addr__draw_player_thirdperson_mesh_check03;
	extern uint32_t retn_addr__draw_player_thirdperson_mesh;
	extern uint32_t hk_addr__impact_marks_pshadow;
	extern uint32_t retn_addr__impact_marks_pshadow_skip;

	// - shaderapidx9
	extern uint32_t kh_addr__cmeshdx8_renderpass_pre_draw;
	extern uint32_t kh_addr__cmeshdx8_renderpass_post_draw;
	extern uint32_t retn_addr__cmeshdx8_renderpass_post_draw;

	// -------------------------------------------

	void init_game_addresses();
	void main();
}
