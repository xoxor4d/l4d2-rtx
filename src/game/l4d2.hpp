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
	extern Vector* current_view_origin;
	extern Vector* current_view_forward;
	extern int* visframecount;

	// - client
	extern Vector* g_vecCurrentRenderOrigin;
	extern Vector4D* s_viewFadeColor;
	extern DWORD* material_system_ptr;
	extern DWORD* modelinfo_ptr;
	extern Vector* camera_forward_vector;
	extern view_id* viewid;

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
	extern uint32_t fn_addr__debug_overlay_add_text;
	extern uint32_t fn_addr__debug_overlay_add_text_colored;
	extern uint32_t fn_addr__point_leafnum;

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
	extern uint32_t fn_addr__get_bone_transform;
	extern uint32_t fn_addr__lookup_bone;
	extern uint32_t fn_addr__get_model_ptr;

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
	inline IVModelInfo* get_modelinfo() { return reinterpret_cast<IVModelInfo*>(*l4d2::modelinfo_ptr /*(DWORD*)(CLIENT_BASE + 0x735E58)*/); }

	inline Vector* get_current_view_origin() { return l4d2::current_view_origin; /*reinterpret_cast<Vector*>(ENGINE_BASE + 0x501344);*/ }
	
	// we read past the vector size since this is actually a 3x3 axis
	inline Vector* get_current_view_forward() { return l4d2::current_view_forward; /*reinterpret_cast<Vector*>(ENGINE_BASE + 0x427A30);*/ }
	inline Vector* get_current_view_right() { return &l4d2::current_view_forward[3]; /*return reinterpret_cast<Vector*>(ENGINE_BASE + 0x427A3C);*/ }
	inline Vector* get_current_view_up() { return &l4d2::current_view_forward[6]; /*return reinterpret_cast<Vector*>(ENGINE_BASE + 0x427A48);*/ }
	inline Vector* get_camera_forward_vector() { return l4d2::camera_forward_vector; /*return reinterpret_cast<Vector*>(CLIENT_BASE + 0x7A2638);*/ } // same as engine one above?

	inline int get_visframecount() { return *l4d2::visframecount; /*return *reinterpret_cast<int*>(ENGINE_BASE + 0x6AFDD8);*/ }
	inline view_id get_viewid() { return *l4d2::viewid; /*return *reinterpret_cast<view_id*>(CLIENT_BASE + 0x6DF6CC);*/ }

	// CM_PointLeafnum
	inline int get_leaf_from_position(const Vector& pos) { return utils::hook::call<int(__cdecl)(const float*)>(l4d2::fn_addr__point_leafnum /*ENGINE_BASE + 0x14B130*/)(&pos.x); }

	namespace namespaces::C_BaseAnimating
	{
		// returns bone matrix for given bone index
		/// @param this_ptr			C_BaseAnimating ptr
		/// @param bone				bone index
		/// @param boneToWorld		out bone matrix
		inline void GetBoneTransform(void* this_ptr, const int bone, matrix3x4_t* boneToWorld)
		{
			utils::hook::call<void(__fastcall)(void* this_ptr, void* null, int bone, matrix3x4_t* boneToWorld)>(l4d2::fn_addr__get_bone_transform)
				(this_ptr, nullptr, bone, boneToWorld);
		}

		// returns bone index for given bone name
		/// @param this_ptr			C_BaseAnimating ptr
		/// @param bone_name		bone name
		/// @return					bone index
		inline int LookupBone(void* this_ptr, const char* bone_name)
		{
			//xref "doorhandlebone"
			return utils::hook::call<int(__fastcall)(void* this_ptr, void* null, const char* bone_name)>(l4d2::fn_addr__lookup_bone)
				(this_ptr, nullptr, bone_name);
		}

		// returns CStudioHdr pointer for given C_BaseAnimating pointer
		/// @param this_ptr			C_BaseAnimating ptr
		/// @return					CStudioHdr ptr
		inline CStudioHdr* GetModelPtr(void* this_ptr)
		{
			return utils::hook::call<CStudioHdr * (__fastcall)(void* this_ptr, void* null)>(l4d2::fn_addr__get_model_ptr)
				(this_ptr, nullptr);
		}
	}
}
