#include "std_include.hpp"
#define USE_BUILD_WORLD_LIST_NOCULL 1

namespace components
{
	int g_current_leaf = -1;
	int g_current_area = -1;

	void on_renderview()
	{
		//model_render::get()->m_drew_hud = false;

		auto enginerender = game::get_engine_renderer();
		const auto dev = game::get_d3d_device();

		// setup main camera
		{
			float colView[4][4] = {};
			utils::row_major_to_column_major(enginerender->m_matrixView.m[0], colView[0]);

			float colProj[4][4] = {};
			utils::row_major_to_column_major(enginerender->m_matrixProjection.m[0], colProj[0]);

			/*auto pos = game::get_current_view_origin();
			D3DXMATRIX world =
			{
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				pos->x, pos->y, pos->z, 1.0f
			};*/

			dev->SetTransform(D3DTS_WORLD, &game::IDENTITY);
			dev->SetTransform(D3DTS_VIEW, reinterpret_cast<const D3DMATRIX*>(colView));
			dev->SetTransform(D3DTS_PROJECTION, reinterpret_cast<const D3DMATRIX*>(colProj));

			// set a default material with diffuse set to a warm white
			// so that add light to texture works and does not require rtx.effectLightPlasmaBall (animated)
			D3DMATERIAL9 dmat = {};
			dmat.Diffuse.r = 1.0f;
			dmat.Diffuse.g = 0.8f;
			dmat.Diffuse.b = 0.8f;
			dev->SetMaterial(&dmat);
		}

		main_module::force_cvars();

		// CM_PointLeafnum :: get current leaf
		g_current_leaf = game::get_leaf_from_position(*game::get_current_view_origin());

		// CM_LeafArea :: get current area the camera is in
		g_current_area = utils::hook::call<int(__cdecl)(int leafnum)>(ENGINE_BASE + 0x14C2C0)(g_current_leaf); 

		remix_api::get()->on_renderview();
	}

	HOOK_RETN_PLACE_DEF(cviewrenderer_renderview_retn);
	__declspec(naked) void cviewrenderer_renderview_stub()
	{
		__asm
		{
			pushad;
			call	on_renderview;
			popad;

			// og
			mov     eax, [ecx];
			mov     edx, [eax + 0x20];
			jmp		cviewrenderer_renderview_retn;
		}
	}

	// ##
	// ##

#if 0
	void on_skyboxdraw()
	{
		auto enginerender = game::get_engine_renderer();
		const auto dev = game::get_d3d_device();

		auto mat = game::get_material_system();
		auto ctx = mat->vtbl->GetRenderContext(mat);

		VMatrix viewm = {};
		ctx->vtbl->GetMatrix2(ctx, MATERIAL_VIEW, &viewm);

		// setup main camera
		{
			float colView[4][4] = {};
			utils::row_major_to_column_major(enginerender->m_matrixView.m[0], colView[0]);

			float colProj[4][4] = {};
			utils::row_major_to_column_major(enginerender->m_matrixProjection.m[0], colProj[0]);

			/*auto pos = game::get_current_view_origin();
			auto render_pos = reinterpret_cast<Vector*>(CLIENT_BASE + 0x7A52A0);

			D3DXMATRIX world =
			{
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				pos->x, pos->y, pos->z, 1.0f
			};*/

			dev->SetTransform(D3DTS_WORLD, &game::IDENTITY);
			dev->SetTransform(D3DTS_VIEW, reinterpret_cast<const D3DMATRIX*>(colView));
			dev->SetTransform(D3DTS_PROJECTION, reinterpret_cast<const D3DMATRIX*>(colProj));
		}

		//g_current_leaf = game::get_leaf_from_position(*game::get_current_view_origin());

		//// CM_LeafArea :: get current area the camera is in
		//g_current_area = utils::hook::call<int(__cdecl)(int leafnum)>(ENGINE_BASE + 0x14C2C0)(g_current_leaf);
	}

	HOOK_RETN_PLACE_DEF(skyboxview_draw_internal_retn);
	__declspec(naked) void skyboxview_draw_internal_stub()
	{
		__asm
		{
			pushad;
			call	on_skyboxdraw;
			popad;

			// og
			mov     eax, [edx + 0xC4];
			jmp		skyboxview_draw_internal_retn;
		}
	}
#endif

	// called on EndScene - remix_api::end_scene_callback()
	void main_module::iterate_entities()
	{
		const auto intf = interfaces::get();
		const auto max_ent = intf->m_entity_list->get_max_entity();

		for (auto i = 0; i < max_ent; i++)
		{
			if (const auto	entity = reinterpret_cast<sdk::c_base_player*>(intf->m_entity_list->get_client_entity(i));
				entity)
			{
				if (const auto* m_classes = entity->client_class();
					m_classes)
				{
					switch (m_classes->class_id)
					{
					default:
						continue;

					case sdk::ET_CTERRORPLAYER:
					case sdk::ET_SURVIVORBOT:
					{
						sdk::player_info_t info;
						if (!intf->m_engine->get_player_info(i, &info)) {
							continue;
						}

						if (const auto is_player = i == intf->m_engine->get_local_player();
							is_player)
						{
							const auto& flashlight_enabled = entity->read<bool>(0x14D8);
							const auto& eyepos = entity->read<Vector>(0x1110);
							const auto& fwd = entity->read<Vector>(0x111C);
							const auto& rt = entity->read<Vector>(0x1134);
							const auto& up = entity->read<Vector>(0x1128);
							remix_api::get()->flashlight_create_or_update(info.name, eyepos, fwd, rt, up, flashlight_enabled, true);
						}

						else // SurvivorBot
						{
							const auto& m_fEffects = entity->read<int>(0xE0);
							const bool flashlight_enabled = m_fEffects & 4;

							const auto& eyepos = entity->get_eye_pos();
							const auto& angles = entity->read<Vector>(0x196C); // m_angEyeAngles[0] - DT_CSPlayer 

							Vector fwd, rt, up;
							utils::vector::AngleVectors(angles, &fwd, &rt, &up);

							remix_api::get()->flashlight_create_or_update(info.name, eyepos, fwd, rt, up, flashlight_enabled);
						}
						break;
					}
					}
				}
			}
		}
	}


	/**
	 * Force visibility of a specific node
	 * @param node_index	The node to force vis for
	 * @param player_node	The node the player is currently in
	 */
	void force_node_vis(int node_index, bool hide = false)
	{
		const auto world = game::get_hoststate_worldbrush_data();
		const auto root_node = &world->nodes[0];

		int next_node_index = node_index;
		while (next_node_index >= 0)
		{
			if (node_index == g_current_leaf) {
				break;
			}

			const auto node = &world->nodes[next_node_index];

			if (!hide)
			{
				// node was already set to current visframe, do not continue
				if (node->visframe == game::get_visframecount()) {
					break;
				}

				// force node vis
				node->visframe = game::get_visframecount();
			}
			else
			{
				// nodes already hidden
				if (node->visframe == 0) {
					break;
				}

				node->visframe = 0;
			}

			// we only need to traverse to the root node
			if (node == root_node) {
				break;
			}

			next_node_index = &node->parent[0] - root_node;
		}
	}

	/**
	 * Force visibility of a specific leaf
	 * @param leaf_index   The leaf to force vis for
	 * @param player_node  The node the player is currently in
	 */
	void force_leaf_vis(int leaf_index, bool hide = false)
	{
		const auto world = game::get_hoststate_worldbrush_data();
		auto leaf_node = &world->leafs[leaf_index];
		auto parent_node_index = &leaf_node->parent[0] - &world->nodes[0];

		if (!hide)
		{
			// force leaf vis
			leaf_node->visframe = game::get_visframecount();
		}
		else
		{
			leaf_node->visframe = 0;
		}

		// force nodes
		force_node_vis(parent_node_index, hide);
	}


	// Stub before calling 'R_CullNode' in 'R_RecursiveWorldNode'
	// Return 0 to NOT cull the node
	int r_cullnode_wrapper(mnode_t* node)
	{
		if (game::get_viewid() == VIEW_3DSKY) {
			return utils::hook::call<bool(__cdecl)(mnode_t*)>(ENGINE_BASE + 0xFC490)(node);
		}

		if (imgui::get()->m_disable_cullnode) {
			return 0;
		}


		bool is_leaf = node->contents >= 0;
		const auto world = game::get_hoststate_worldbrush_data();
		int idx = is_leaf ? ((mleaf_t*)node - &world->leafs[0]) : (node - &world->nodes[0]);

		if (idx == 3174)
		{
			int x = 1;
		}

		if (idx == 3152 || idx == 3136)
		{
			int x = 1;
			return 0;
		}



		// R_CullNode - uses area frustums if avail. and not in a solid - uses player frustum otherwise
		if (!utils::hook::call<bool(__cdecl)(mnode_t*)>(ENGINE_BASE + 0xFC490)(node)) {
			return 0;
		}

		// ^ R_CullNode would cull the node if we reach this point
		// MODE: force all leafs/nodes in CURRENT area
		//if (!g_player_current_area_override || cmode == map_settings::AREA_CULL_MODE_FRUSTUM_FORCE_AREA)
		{
			
			if (imgui::get()->m_enable_area_forcing) 
			{
				// force draw this node/leaf if it's within the forced area
				if ((int)node->area == g_current_area) {
					return 0;
				}
			}
		}

		// cull node
		return 1;
	}

	HOOK_RETN_PLACE_DEF(r_cullnode_cull_retn);
	HOOK_RETN_PLACE_DEF(r_cullnode_skip_retn);
	__declspec(naked) void r_cullnode_stub()
	{
		__asm
		{
			pushad;
#if USE_BUILD_WORLD_LIST_NOCULL
			push	esi;
#else
			push	ebx;
#endif
			call	r_cullnode_wrapper; // return 0 to not jump
			add		esp, 4;
			test	eax, eax;
			jz		SKIP; // jump if eax = 0
			popad;

#if USE_BUILD_WORLD_LIST_NOCULL
			mov     ecx, [ebp - 4];
#endif
			add     esp, 4; // og
			jmp		r_cullnode_cull_retn;

		SKIP:
			popad;

#if USE_BUILD_WORLD_LIST_NOCULL
			mov     ecx, [ebp - 4];
#endif

			add     esp, 4; // og
			jmp		r_cullnode_skip_retn;
		}
	}


	void pre_recursive_world_node()
	{
		if (game::get_viewid() != VIEW_3DSKY)
		{
			const auto world = game::get_hoststate_worldbrush_data();

			// show leaf index as 3D text
			if (g_current_leaf < world->numleafs)
			{
				if (remix_api::is_node_debug_enabled())
				{
					const auto curr_leaf = &world->leafs[g_current_leaf];
					remix_api::get()->debug_draw_box(curr_leaf->m_vecCenter, curr_leaf->m_vecHalfDiagonal, 2.0f, remix_api::DEBUG_REMIX_LINE_COLOR::GREEN);
				}
			}


			//if (!g_player_current_area_override || g_player_current_area_override->cull_mode == map_settings::AREA_CULL_MODE_FRUSTUM_FORCE_AREA)
			{
				/*for (auto i = 0; i < world->numnodes; i++)
				{
					world->nodes[i].visframe = game::get_visframecount();
				}*/

				for (auto i = 0; i < world->numleafs; i++) 
				{
					// leaf forcing test of disp
					if (i == 3152 || i == 3136) 
					{
						force_leaf_vis(i);
					} 

					if (i == 3174) 
					{
						int x = 1;
					}

					if (imgui::get()->m_enable_area_forcing)
					{
						if (auto& l = world->leafs[i];
							(int)l.area == g_current_area)
						{
							force_leaf_vis(i);
						}
					}
				}
			}
		}
	}

#if USE_BUILD_WORLD_LIST_NOCULL
	HOOK_RETN_PLACE_DEF(p_build_world_list_no_cull_func);
#endif
	HOOK_RETN_PLACE_DEF(pre_recursive_world_node_retn);
	__declspec(naked) void pre_recursive_world_node_stub()
	{
		__asm
		{
			pushad;
			call	pre_recursive_world_node;
			popad;

#if USE_BUILD_WORLD_LIST_NOCULL
			// og
			mov     ecx, [edx + 0x50];
			push	ebx;
			call	p_build_world_list_no_cull_func;
#else
			// og
			mov     edx, [eax + 0x50];
			mov     ecx, ebx;
#endif
			jmp		pre_recursive_world_node_retn;
		}
	}

	void main_module::force_cvars()
	{
		// z_wound_client_disabled
		// z_skip_wounds
		// z_randomskins
		// z_randombodygroups
		// z_infected_tinting
		// z_infected_decals
		// z_infected_damage_cutouts
		// z_gibs_per_frame
		// z_forcezombiemodel --- z_forcezombiemodelname

		//game::cvar_uncheat_and_set_int("z_randomskins", 0);
		//game::cvar_uncheat_and_set_int("z_randombodygroups", 0);
		//game::cvar_uncheat_and_set_int("z_infected_tinting", 0);

		//game::cvar_uncheat_and_set_int("z_infected_damage_cutouts", 0);
		//game::cvar_uncheat_and_set_int("z_skip_wounds", 1);
		//game::cvar_uncheat_and_set_int("z_wound_client_disabled", 1);

		game::cvar_uncheat_and_set_int("r_staticprop_lod", 0);
		game::cvar_uncheat_and_set_int("r_lod", 0);
		game::cvar_uncheat_and_set_int("r_lod_switch_scale", 1); // hidden cvar
		

		game::cvar_uncheat_and_set_int("r_dopixelvisibility", 0); // hopefully fix random crash (dxvk cmdBindPipeline) on map load

		game::cvar_uncheat_and_set_int("r_WaterDrawRefraction", 0); // fix weird culling behaviour near water surfaces
		game::cvar_uncheat_and_set_int("r_WaterDrawReflection", 0); // perf?

		game::cvar_uncheat_and_set_int("r_threaded_particles", 0);
		game::cvar_uncheat_and_set_int("r_entityclips", 0);
		game::cvar_uncheat_and_set_int("r_PortalTestEnts", 0);

		game::cvar_uncheat_and_set_int("cl_fastdetailsprites", 0);
		game::cvar_uncheat_and_set_int("cl_brushfastpath", 0);
		game::cvar_uncheat_and_set_int("cl_tlucfastpath", 0); // 
		game::cvar_uncheat_and_set_int("cl_modelfastpath", 0); // gain 4-5 fps on some act 4 maps but FF rendering not implemented
		game::cvar_uncheat_and_set_int("mat_queue_mode", 0); // does improve performance but breaks rendering
		game::cvar_uncheat_and_set_int("r_queued_ropes", 0);
		game::cvar_uncheat_and_set_int("mat_softwarelighting", 0);
		game::cvar_uncheat_and_set_int("mat_parallaxmap", 0);
		game::cvar_uncheat_and_set_int("mat_frame_sync_enable", 0);
		game::cvar_uncheat_and_set_int("mat_dof_enabled", 0);
		game::cvar_uncheat_and_set_int("mat_displacementmap", 0);
		game::cvar_uncheat_and_set_int("mat_drawflat", 0);
		game::cvar_uncheat_and_set_int("mat_normalmaps", 0);
		game::cvar_uncheat_and_set_int("r_flashlightrender", 0); // messes up terrain blending otherwise
		game::cvar_uncheat_and_set_int("r_flashlightrendermodels", 0);
		game::cvar_uncheat_and_set_int("r_flashlightrenderworld", 0);
		game::cvar_uncheat_and_set_int("r_FlashlightDetailProps", 0);

		game::cvar_uncheat_and_set_int("r_3dsky", 1);
		game::cvar_uncheat_and_set_int("mat_fullbright", 1);
		game::cvar_uncheat_and_set_int("mat_softwareskin", 1);
		game::cvar_uncheat_and_set_int("mat_phong", 1);
		game::cvar_uncheat_and_set_int("mat_fastnobump", 1);
		game::cvar_uncheat_and_set_int("mat_disable_bloom", 1);


		// TODO: HACK
		// remove when displacement-backface culling check is found - currently in use so that
		// displacements are rendered when leaf is forced
		game::cvar_uncheat_and_set_int("r_DispWalkable", 1); 


		// graphic settings

		// lvl 0
		game::cvar_uncheat_and_set_int("cl_particle_fallback_base", 0);//3); // 0 = render portalgun viewmodel effects
		game::cvar_uncheat_and_set_int("cl_particle_fallback_multiplier", 1); //2);
		
		game::cvar_uncheat_and_set_int("cl_impacteffects_limit_general", 10);
		game::cvar_uncheat_and_set_int("cl_impacteffects_limit_exit", 3);
		game::cvar_uncheat_and_set_int("cl_impacteffects_limit_water", 2);
		game::cvar_uncheat_and_set_int("mat_depthfeather_enable", 0);

		game::cvar_uncheat_and_set_int("r_shadowrendertotexture", 0);
		game::cvar_uncheat_and_set_int("r_shadowfromworldlights", 0);

		game::cvar_uncheat_and_set_int("mat_force_vertexfog", 1);
		game::cvar_uncheat_and_set_int("cl_footstep_fx", 1);
		game::cvar_uncheat_and_set_int("cl_ragdoll_self_collision", 1);
		game::cvar_uncheat_and_set_int("cl_ragdoll_maxcount", 24);

		game::cvar_uncheat_and_set_int("cl_detaildist", 1024);
		game::cvar_uncheat_and_set_int("cl_detailfade", 400);
		game::cvar_uncheat_and_set_int("r_drawmodeldecals", 1);
		game::cvar_uncheat_and_set_int("r_decalstaticprops", 1);
		game::cvar_uncheat_and_set_int("cl_player_max_decal_count", 32);

		// lvl 3
		game::cvar_uncheat_and_set_int("r_decals", 2048);
		game::cvar_uncheat_and_set_int("r_decal_overlap_count", 3);

		// disable fog
		game::cvar_uncheat_and_set_int("fog_override", 1);
		game::cvar_uncheat_and_set_int("fog_enable", 0);
	}

	main_module::main_module()
	{
		{ // init filepath var
			char path[MAX_PATH]; GetModuleFileNameA(nullptr, path, MAX_PATH);
			game::root_path = path; utils::erase_substring(game::root_path, "left4dead2.exe");
		}

		{ // init d3d font
			D3DXFONT_DESC desc =
			{
				18,                  // Height
				0,                   // Width (0 = default)
				FW_NORMAL,           // Weight (FW_BOLD, FW_LIGHT, etc.)
				1,                   // Mip levels
				FALSE,               // Italic
				DEFAULT_CHARSET,     // Charset
				OUT_DEFAULT_PRECIS,  // Output Precision
				CLIP_DEFAULT_PRECIS, // Clipping Precision
				DEFAULT_PITCH,       // Pitch and Family
				TEXT("Arial")        // Typeface
			};

			D3DXCreateFontIndirect(game::get_d3d_device(), &desc, &d3d_font);
		}


		// #
		// events

		// CViewRender::RenderView :: "start" of current frame (after CViewRender::DrawMonitors)
		utils::hook(CLIENT_BASE + 0x1D7113, cviewrenderer_renderview_stub).install()->quick();
		HOOK_RETN_PLACE(cviewrenderer_renderview_retn, CLIENT_BASE + 0x1D7118);

		// not needed
		//utils::hook::nop(CLIENT_BASE + 0x1D3F33, 6);
		//utils::hook(CLIENT_BASE + 0x1D3F33, skyboxview_draw_internal_stub).install()->quick();
		//HOOK_RETN_PLACE(skyboxview_draw_internal_retn, CLIENT_BASE + 0x1D3F39);

		// #
		// culling

		// CDispInfo::Render :: disable 'Frustum_t::CullBox' check
		utils::hook::nop(ENGINE_BASE + 0xB13E5, 2);

#if USE_BUILD_WORLD_LIST_NOCULL
		// R_RecursiveWorldNodeNoCull:: use 'R_BuildWorldListNoCull' instead of 'R_RecursiveWorldNode'
		utils::hook::nop(ENGINE_BASE + 0xD162D, 2);

		// stub before calling 'R_RecursiveWorldNode' to override node/leaf vis
		utils::hook::nop(ENGINE_BASE + 0xD1635, 9);
		utils::hook(ENGINE_BASE + 0xD1635, pre_recursive_world_node_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(p_build_world_list_no_cull_func, ENGINE_BASE + 0xCD630);
		HOOK_RETN_PLACE(pre_recursive_world_node_retn, ENGINE_BASE + 0xD163E);

		// ^ :: while( ... node->contents < -1 .. ) -> jl to jle .. to jmp to cull less (same as returning 0 in cullnode)
		utils::hook::set<BYTE>(ENGINE_BASE + 0xCD665, 0x7E); // needed?

		// ^ :: while( ... !R_CullNode) - wrapper function to impl. additional culling control (force areas/leafs + use frustum culling when needed)
		utils::hook(ENGINE_BASE + 0xCD668, r_cullnode_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(r_cullnode_cull_retn, ENGINE_BASE + 0xCD68F);
		HOOK_RETN_PLACE(r_cullnode_skip_retn, ENGINE_BASE + 0xCD677);

#else
		// stub before calling 'R_RecursiveWorldNode' to override node/leaf vis
		utils::hook(ENGINE_BASE + 0xD1648, pre_recursive_world_node_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(pre_recursive_world_node_retn, ENGINE_BASE + 0xD164D);

		// ^ :: while( ... node->contents < -1 .. ) -> jl to jle .. to jmp to cull less
		utils::hook::set<BYTE>(ENGINE_BASE + 0xCD7E5, 0x7E);

		// ^ :: while( ... !R_CullNode) - wrapper function to impl. additional culling control (force areas/leafs + use frustum culling when needed)
		utils::hook(ENGINE_BASE + 0xCD7E8, r_cullnode_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(r_cullnode_cull_retn, ENGINE_BASE + 0xCD935);
		HOOK_RETN_PLACE(r_cullnode_skip_retn, ENGINE_BASE + 0xCD7F8);

		// ^ :: backface check -> je to jl
		utils::hook::nop(ENGINE_BASE + 0xCD8C1, 2); // okay - draws a little more but not so heavy on perf.

		// ^ :: backface check -> jnz to je
		utils::hook::set<BYTE>(ENGINE_BASE + 0xCD8CB, 0x74); // ^

		// R_DrawLeaf :: backface check (emissive lamps) plane normal >= -0.00999f
		utils::hook::nop(ENGINE_BASE + 0xCD4E7, 6); // ^ 
#endif

		// CBrushBatchRender::DrawOpaqueBrushModel :: :: backface check - nop 'if ( bShadowDepth )' to disable culling
		utils::hook::nop(ENGINE_BASE + 0xD2250, 2);

		// CClientLeafSystem::ExtractCulledRenderables :: disable 'engine->CullBox' check to disable entity culling in leafs
		// needs r_PortalTestEnts to be 0 -> je to jmp (0xEB)
		utils::hook::set<BYTE>(CLIENT_BASE + 0xBDA76, 0xEB);

		// not req. rn
		// CSimpleWorldView::Setup :: nop 'DoesViewPlaneIntersectWater' check
		//utils::hook::nop(CLIENT_BASE + 0x1CF46F, 2);
		// ^ next instruction :: OR m_DrawFlags with 0x60 instead of 0x30
		//utils::hook::set<BYTE>(CLIENT_BASE + 0x1CF471 + 6, 0x60);

		// engine 0xB3383 - no cull overlay decals (no need)
	}

	main_module::~main_module()
	{ }
}
