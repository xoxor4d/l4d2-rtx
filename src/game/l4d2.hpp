#pragma once

namespace l4d2
{
	// -------------------------------------------
	// game variables

	// - server
	extern void* mdl_cache;

	// - engine
	extern CRender* engine_renderer;
	extern DWORD* hoststate_worldbrush_data_ptr;

	// - client
	extern Vector* g_vecCurrentRenderOrigin;
	extern Vector4D* s_viewFadeColor;
	extern DWORD* material_system_ptr;

	// - shaderapidx9
	extern DWORD* d3d_device_ptr;
	extern DWORD* shaderapi_ptr;

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
	extern uint32_t fn_addr__util_remove;

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
	extern uint32_t hk_addr__start_sound;

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
	extern uint32_t hk_addr__render_spritecard_new;
	extern uint32_t hk_addr__rope_mgr_draw_render_cache;
	extern uint32_t hk_addr__glow_overlay_draw;
	extern uint32_t nop_addr__func_area_portal_window_draw_mdl;
	extern uint32_t fn_addr__add_console_cmd;
	extern uint32_t fn_addr__debug_overlay_add_text;
	extern uint32_t fn_addr__debug_overlay_add_text_colored;

	// - shaderapidx9
	extern uint32_t kh_addr__cmeshdx8_renderpass_pre_draw;
	extern uint32_t kh_addr__cmeshdx8_renderpass_post_draw;
	extern uint32_t retn_addr__cmeshdx8_renderpass_post_draw;

	// - studiorender
	extern uint32_t hk_addr__studio_draw_static_mesh;

	// -------------------------------------------

	void init_game_addresses();
	void main();
}

namespace game
{
	inline CRender* get_engine_renderer() { return l4d2::engine_renderer; /*reinterpret_cast<CRender*>(ENGINE_BASE + 0x601F00);*/ }
	inline IDirect3DDevice9* get_d3d_device() { return reinterpret_cast<IDirect3DDevice9*>(*l4d2::d3d_device_ptr /*(RENDERER_BASE + 0xD3EE8)*/); }
	inline IShaderAPIDX8* get_shaderapi() { return reinterpret_cast<IShaderAPIDX8*>(*l4d2::shaderapi_ptr /*(RENDERER_BASE + 0xC9C50)*/); }
	inline IMaterialSystem* get_material_system() { return reinterpret_cast<IMaterialSystem*>(*l4d2::material_system_ptr /*(CLIENT_BASE + 0x88B7F0)*/); }
	inline worldbrushdata_t* get_hoststate_worldbrush_data() { return reinterpret_cast<worldbrushdata_t*>(*l4d2::hoststate_worldbrush_data_ptr /*(DWORD*)(ENGINE_BASE + 0x42FFB8)*/); }
}
