#include "std_include.hpp"
#define USE_BUILD_WORLD_LIST_NOCULL 1

namespace components
{
	namespace cmd
	{
		bool debug_node_vis = false;
	}

	int g_current_leaf = -1;
	int g_current_area = -1;

	bool g_player_leaf_update = false;
	map_settings::area_overrides_s* g_player_current_area_override = nullptr; // contains overrides for the current area, nullptr if no overrides exist

	void on_renderview()
	{
		const auto dev = game::get_d3d_device();

		// helper for nocull markers
		model_render::get()->m_drew_model = false;

		// set a default material with diffuse set to a warm white
		// so that add light to texture works and does not require rtx.effectLightPlasmaBall (animated)
		D3DMATERIAL9 dmat = {};
		dmat.Diffuse.r = 1.0f;
		dmat.Diffuse.g = 0.8f;
		dmat.Diffuse.b = 0.8f;
		dev->SetMaterial(&dmat);

		// ----
		// ----

		//choreo_events::on_client_frame();
		remix_vars::on_client_frame();
		remix_lights::on_client_frame();

		main_module::force_cvars();

		// TODO - find better spot to call this
		map_settings::spawn_markers_once();
		// nocull markers handled in 'model_renderer::DrawModelExecute::Detour'

		// CM_PointLeafnum :: get current leaf
		const auto current_leaf = game::get_leaf_from_position(*game::get_current_view_origin());
		g_player_leaf_update = g_current_leaf != current_leaf;
		g_current_leaf = current_leaf;

		// CM_LeafArea :: get current area the camera is in
		g_current_area = utils::hook::call<int(__cdecl)(int leafnum)>(ENGINE_BASE + 0x14C2C0)(g_current_leaf); // #OFFS 2501

		remix_api::get()->on_renderview();

		// fog
		if (static bool allow_fog = !flags::has_flag("no_fog"); allow_fog)
		{
			const auto& s = map_settings::get_map_settings();
			if (s.fog_dist > 0.0f)
			{
				const float fog_start = 1.0f; // not useful
				dev->SetRenderState(D3DRS_FOGENABLE, TRUE);
				dev->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
				dev->SetRenderState(D3DRS_FOGSTART, *(DWORD*)&fog_start);
				dev->SetRenderState(D3DRS_FOGEND, *(DWORD*)&s.fog_dist);
				dev->SetRenderState(D3DRS_FOGCOLOR, s.fog_color);
			}
			else
			{
				dev->SetRenderState(D3DRS_FOGENABLE, FALSE);
			}
		}
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

	//void on_skyboxdraw()
	//{
	//	auto enginerender = game::get_engine_renderer();
	//	const auto dev = game::get_d3d_device();

	//	auto mat = game::get_material_system();
	//	auto ctx = mat->vtbl->GetRenderContext(mat);

	//	VMatrix viewm = {};
	//	ctx->vtbl->GetMatrix2(ctx, MATERIAL_VIEW, &viewm);

	//	// setup main camera
	//	{
	//		float colView[4][4] = {};
	//		utils::row_major_to_column_major(enginerender->m_matrixView.m[0], colView[0]);

	//		float colProj[4][4] = {};
	//		utils::row_major_to_column_major(enginerender->m_matrixProjection.m[0], colProj[0]);

	//		auto pos = game::get_current_view_origin();
	//		auto render_pos = reinterpret_cast<Vector*>(CLIENT_BASE + 0x7A52A0);

	//		D3DXMATRIX world =
	//		{
	//			1.0f, 0.0f, 0.0f, 0.0f,
	//			0.0f, 1.0f, 0.0f, 0.0f,
	//			0.0f, 0.0f, 1.0f, 0.0f,
	//			render_pos->x, render_pos->y, render_pos->z, 1.0f
	//		};

	//		dev->SetTransform(D3DTS_WORLD, &world);
	//		dev->SetTransform(D3DTS_VIEW, reinterpret_cast<const D3DMATRIX*>(colView));
	//		dev->SetTransform(D3DTS_PROJECTION, reinterpret_cast<const D3DMATRIX*>(colProj));
	//	}
	//}

	//HOOK_RETN_PLACE_DEF(skyboxview_draw_internal_retn);
	//__declspec(naked) void skyboxview_draw_internal_stub()
	//{
	//	__asm
	//	{
	//		add     esp, 0x14;

	//		pushad;
	//		call	on_skyboxdraw;
	//		popad;

	//		// og
	//		cmp     byte ptr[ebp + 0xC], 0;
	//		jmp		skyboxview_draw_internal_retn;
	//	}
	//}

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

	bool is_aabb_within_distance(const VectorAligned& center, const VectorAligned& half_diagonal, const Vector& player_origin, const float radius)
	{
		const Vector min_bounds = center - half_diagonal;
		const Vector max_bounds = center + half_diagonal;

		auto sq_dist = 0.0f;
		for (auto i = 0; i < 3; ++i)
		{
			if (player_origin[i] < min_bounds[i])
			{
				const auto d = min_bounds[i] - player_origin[i];
				sq_dist += d * d;
			}
			else if (player_origin[i] > max_bounds[i])
			{
				const auto d = player_origin[i] - max_bounds[i];
				sq_dist += d * d;
			}

			// return false if distance exceeds radius sqr
			if (sq_dist > radius * radius) {
				return false;
			}
		}

		return true;
	}

	// Stub before calling 'R_CullNode' in 'R_RecursiveWorldNode'
	// Return 0 to NOT cull the node
	int r_cullnode_wrapper(mnode_t* node)
	{
		if (game::get_viewid() == VIEW_3DSKY || game::get_viewid() == VIEW_MONITOR) {
			return utils::hook::call<bool(__cdecl)(mnode_t*)>(ENGINE_BASE + 0xFC490)(node); // #OFFS 2501
		}

		// default culling mode or no culling if cmd was used
		map_settings::AREA_CULL_MODE cmode = imgui::get()->m_disable_cullnode ? map_settings::AREA_CULL_MODE_NO_FRUSTUM : map_settings::AREA_CULL_INFO_DEFAULT;
		int node_index = 0;

		/*if (imgui::get()->m_disable_cullnode) {
			return 0;
		}*/

		// check if we have area overrides
		if (g_player_current_area_override)
		{
			// set area cull mode
			cmode = g_player_current_area_override->cull_mode;

			// calculate index of leaf/node
			if (node->contents >= 0) { // this is a leaf
				node_index = (mleaf_t*)node - &game::get_hoststate_worldbrush_data()->leafs[0];
			}
			else { // this is a node
				node_index = node - &game::get_hoststate_worldbrush_data()->nodes[0];
			}

			// HIDE
			// check if this node was forced visible
			if (!g_player_current_area_override->leafs.contains(node_index))
			{
				// if node not forced, iterate all hidden area entries
				for (const auto& hidden_area : g_player_current_area_override->hide_areas)
				{
					// check if node is part of a hidden area but only cull if the player is not in a specified leaf
					if (hidden_area.areas.contains((std::uint32_t)node->area)
						&& !hidden_area.when_not_in_leafs.contains(g_current_leaf))
					{
						return 1;
					}
				}

				// check if this leaf is set to be hidden
				if (g_player_current_area_override->hide_leafs.contains(node_index)) {
					return 1;
				}
			}
		}

		// draw node if culling mode is set to 'no culling'
		if (cmode == map_settings::AREA_CULL_MODE_NO_FRUSTUM) {
			return 0;
		}


		/*bool is_leaf = node->contents >= 0;
		const auto world = game::get_hoststate_worldbrush_data();
		int idx = is_leaf ? ((mleaf_t*)node - &world->leafs[0]) : (node - &world->nodes[0]);
		if (idx == 3152 || idx == 3136)
		{
			int x = 1;
			return 0;
		}*/

		// "global" nocull distance if area has no overrides
		float nocull_dist = map_settings::get_map_settings().default_nocull_dist;
		const bool using_distance_based_mode = cmode >= map_settings::AREA_CULL_INFO_NOCULLDIST_START && cmode <= map_settings::AREA_CULL_INFO_NOCULLDIST_END;

		if (using_distance_based_mode && g_player_current_area_override)
		{
			// nocull distance if area has override
			nocull_dist = g_player_current_area_override->nocull_distance;

			// if any leaf tweak has a nocull override
			if (g_player_current_area_override->nocull_distance_overrides_in_leaf_twk)
			{
				for (const auto& lt : g_player_current_area_override->leaf_tweaks)
				{
					// check if node the player is currently in has any overrides
					if (lt.in_leafs.contains(g_current_leaf))
					{
						nocull_dist = lt.nocull_dist;
						break;
					}
				}
			}
		}

		// if no area override or if cull mode is distance based
		if (   !g_player_current_area_override 
			|| using_distance_based_mode)
		{
			if (is_aabb_within_distance(node->m_vecCenter, node->m_vecHalfDiagonal, *game::get_current_view_origin(), nocull_dist)) {
				return 0;
			}

			// if forcing current area + distance
			if (cmode == map_settings::AREA_CULL_MODE_FORCE_AREA_DISTANCE)
			{
				if ((int)node->area == g_current_area) {
					return 0;
				}
			}
		}

		// MODE: force all leafs/nodes in CURRENT area
		else if (cmode == map_settings::AREA_CULL_MODE_NO_FRUSTUM_IN_CURRENT_AREA
			  || cmode == map_settings::AREA_CULL_MODE_FORCE_AREA)
		{
			// force draw this node/leaf if it's within the forced area
			if ((int)node->area == g_current_area) {
				return 0;
			}
		}

		// R_CullNode - uses area frustums if avail. and not in a solid - uses player frustum otherwise
		if (!utils::hook::call<bool(__cdecl)(mnode_t*)>(ENGINE_BASE + 0xFC490)(node)) { // #OFFS
			return 0;
		}

		// check if we have area overrides
		if (g_player_current_area_override)
		{
			// check if this leaf/node is part of a forced area
			if (g_player_current_area_override->areas.contains((std::uint32_t)node->area)) {
				return 0;
			}

			// check if this leaf/node is force enabled
			if (g_player_current_area_override->leafs.contains(node_index)) {
				return 0;
			}

			// check if there are leaf specific tweaks
			if (!g_player_current_area_override->leaf_tweaks.empty())
			{
				for (const auto& lt : g_player_current_area_override->leaf_tweaks)
				{
					// check if node the player is currently in has any overrides
					if (lt.in_leafs.contains(g_current_leaf)) 
					{
						// if so, check if the current node to be culled is part of a forced area
						// note: areas are not vis forced - this only disables frustum culling and relies on PVS
						if (lt.areas.contains((std::uint32_t)node->area)) {
							return 0;
						}

						// force individual leafs
						if (lt.leafs.contains(node_index)) {
							return 0;
						}
					}
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


	// Trigger leaf/node forcing logic and updates 'g_player_current_area_override' when 'pre_recursive_world_node()' gets called
	void main_module::trigger_vis_logic()
	{
		g_player_leaf_update = true;
		g_player_current_area_override = nullptr;
	}

	// called from remix_api::on_present_callback()
	void main_module::hud_draw_area_info()
	{
		// Draw current node/leaf as HUD
		if (cmd::debug_node_vis && d3d_font)
		{
			RECT rect;
			if (g_current_area != -1)
			{
				SetRect(&rect, get()->m_hud_debug_node_vis_pos[0], get()->m_hud_debug_node_vis_pos[1], 512, 512);
				d3d_font->DrawTextA(nullptr, utils::va("Area: %d", g_current_area), -1, &rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 255, 255)); // text length (-1 = null-terminated)
			}

			if (g_current_leaf != -1)
			{
				SetRect(&rect, get()->m_hud_debug_node_vis_pos[0], get()->m_hud_debug_node_vis_pos[1] + 15, 512, 512);
				d3d_font->DrawTextA(nullptr, utils::va("Leaf: %d", g_current_leaf), -1, &rect, DT_NOCLIP, D3DCOLOR_XRGB(50, 255, 20));
			}

			if (get()->m_hud_debug_node_vis_has_forced_leafs)
			{
				SetRect(&rect, get()->m_hud_debug_node_vis_pos[0], get()->m_hud_debug_node_vis_pos[1] + 40, 512, 512);
				d3d_font->DrawTextA(nullptr, "Individual forced leafs", -1, &rect, DT_NOCLIP, D3DCOLOR_XRGB(0, 255, 255));
			}

			if (get()->m_hud_debug_node_vis_has_forced_arealeafs)
			{
				SetRect(&rect, get()->m_hud_debug_node_vis_pos[0], get()->m_hud_debug_node_vis_pos[1] + 55, 512, 512);
				d3d_font->DrawTextA(nullptr, "Leafs of a forced area", -1, &rect, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 0));
			}
		}
	}

	void pre_recursive_world_node()
	{
		if (game::get_viewid() == VIEW_3DSKY || game::get_viewid() == VIEW_MONITOR) {
			return;
		}

		// reset
		main_module::get()->m_hud_debug_node_vis_has_forced_leafs = false;
		main_module::get()->m_hud_debug_node_vis_has_forced_arealeafs = false;

		const auto world = game::get_hoststate_worldbrush_data();
		auto& map_settings = map_settings::get_map_settings();

		// visualize current leaf + forced leafs (map_settings)
		if (g_current_leaf < world->numleafs)
		{
			if (cmd::debug_node_vis)
			{
				const auto curr_leaf = &world->leafs[g_current_leaf];
				remix_api::get()->debug_draw_box(curr_leaf->m_vecCenter, curr_leaf->m_vecHalfDiagonal, 2.0f, remix_api::DEBUG_REMIX_LINE_COLOR::GREEN); // current leaf

				// does the area the player is currently in have any overrides?
				if (g_player_current_area_override)
				{
					// visualize forced leafs
					for (const auto& l : g_player_current_area_override->leafs)
					{
						if (!remix_api::can_add_debug_lines()) {
							break;
						}

						if (const auto	forced_leaf = &world->leafs[l]; 
										forced_leaf != curr_leaf)
						{
							// visualize near-by leaf overrides (TEAL)
							if (game::get_current_view_origin()->DistToSqr(forced_leaf->m_vecCenter) < 2000.0f * 2000.0f) 
							{
								remix_api::get()->debug_draw_box(forced_leaf->m_vecCenter, forced_leaf->m_vecHalfDiagonal, 3.5f, remix_api::DEBUG_REMIX_LINE_COLOR::TEAL);
								main_module::get()->m_hud_debug_node_vis_has_forced_leafs = true;
							}
						}
					}

					// visualize leafs of forced areas
					for (const auto& a : g_player_current_area_override->areas)
					{
						for (auto i = 0u; i < (std::uint32_t)world->numleafs; i++)
						{
							if (!remix_api::can_add_debug_lines()) {
								break;
							}

							// visualize near-by leafs that are part of area overrides (RED)
							if (const auto	forced_leaf = &world->leafs[i]; 
											forced_leaf != curr_leaf && a == (std::uint32_t)forced_leaf->area)
							{
								if (game::get_current_view_origin()->DistToSqr(forced_leaf->m_vecCenter) < 350.0f * 350.0f) 
								{
									remix_api::get()->debug_draw_box(forced_leaf->m_vecCenter, forced_leaf->m_vecHalfDiagonal, 3.5f, remix_api::DEBUG_REMIX_LINE_COLOR::RED);
									main_module::get()->m_hud_debug_node_vis_has_forced_arealeafs = true;
								}
							}
						}
					}

					// visualize leafs of forced areas defined in leaf_tweaks
					for (const auto& lt : g_player_current_area_override->leaf_tweaks)
					{
						if (lt.in_leafs.contains(g_current_leaf))
						{
							for (auto i = 0u; i < (std::uint32_t)world->numleafs; i++)
							{
								if (!remix_api::can_add_debug_lines()) {
									break;
								}

								// visualize near-by leafs that are part of area overrides (RED)
								if (const auto	forced_leaf = &world->leafs[i];
									forced_leaf != curr_leaf && lt.areas.contains((std::uint32_t)forced_leaf->area))
								{
									if (game::get_current_view_origin()->DistToSqr(forced_leaf->m_vecCenter) < 350.0f * 350.0f) 
									{
										remix_api::get()->debug_draw_box(forced_leaf->m_vecCenter, forced_leaf->m_vecHalfDiagonal, 3.5f, remix_api::DEBUG_REMIX_LINE_COLOR::RED);
										main_module::get()->m_hud_debug_node_vis_has_forced_arealeafs = true;
									}
								}
							}
						}
					}
				}
			}
		}

		// #
		// leaf/node forcing

		// We have to set all nodes from the target leaf to the root node or the node the the player is in to the current visframe
		// Otherwise, 'R_RecursiveWorldNode' will never reach the target leaf

		if (!map_settings.area_settings.empty())
		{
			//if (leaf_update)
			//if (g_player_leaf_update)
			{
				g_player_current_area_override = nullptr;

				if (const auto& t = map_settings.area_settings.find(g_current_area);
					t != map_settings.area_settings.end())
				{
					g_player_current_area_override = &t->second; // cache

					// force all specified leafs/nodes
					for (const auto& l : g_player_current_area_override->leafs)
					{
						if (l < static_cast<std::uint32_t>(world->numleafs)) {
							force_leaf_vis(l); // force leaf to be visible
						}
					}

					// force all leafs/nodes in specified areas
					if (!g_player_current_area_override->areas.empty())
					{
						for (auto i = 0; i < world->numleafs; i++)
						{
							if (const auto& l = world->leafs[i];
								g_player_current_area_override->areas.contains((std::uint32_t)l.area))
							{
								force_leaf_vis(i);
							}
						}
					}
				}
			}
		}
		else {
			g_player_current_area_override = nullptr;
		}

		const map_settings::AREA_CULL_MODE cmode = !g_player_current_area_override ? map_settings::AREA_CULL_INFO_DEFAULT : g_player_current_area_override->cull_mode;
		const float nocull_dist = !g_player_current_area_override ? map_settings.default_nocull_dist : g_player_current_area_override->nocull_distance;

		const bool check_area = cmode == map_settings::AREA_CULL_MODE_FORCE_AREA_DISTANCE;

		// MODE: force all leafs/nodes within a certain dist to the player (+ only in current area modifier)
		if ((  check_area
			|| cmode == map_settings::AREA_CULL_MODE_DISTANCE)
			&& nocull_dist > 0.0f)
		{
			for (auto i = 0; i < world->numleafs; i++)
			{
				if (auto& l = world->leafs[i];
					!check_area || (int)l.area == g_current_area) // ignore area check if distance mode
				{
					if (is_aabb_within_distance(l.m_vecCenter, l.m_vecHalfDiagonal, *game::get_current_view_origin(), nocull_dist)) {
						force_leaf_vis(i);
					}
				}
			}
		}

		// MODE: force all leafs/nodes in current area
		else if (cmode == map_settings::AREA_CULL_MODE_FORCE_AREA)
		{
			for (auto i = 0; i < world->numleafs; i++)
			{
				if (auto& l = world->leafs[i];
					(int)l.area == g_current_area)
				{
					force_leaf_vis(i);
				}
			}
		}

		if (g_player_current_area_override)
		{
			// leaf tweaks: this forces all leafs of an area that is forced per leaf
			if (!g_player_current_area_override->leaf_tweaks.empty())
			{
				for (const auto& lt : g_player_current_area_override->leaf_tweaks)
				{
					if (lt.in_leafs.contains(g_current_leaf))
					{
						for (auto i = 0u; i < (std::uint32_t)world->numleafs; i++)
						{
							// visualize near-by leafs that are part of area overrides (RED)
							if (const auto	forced_leaf = &world->leafs[i];
								lt.areas.contains((std::uint32_t)forced_leaf->area))
							{
								force_leaf_vis(i);
							}
						}
					}
				}
			}
		}

		// update visibility of nocull markers
		if (g_player_leaf_update)
		{
			for (auto& m : map_settings.map_markers)
			{
				// ignore normal markers
				if (!m.no_cull || m.areas.empty()) {
					continue;
				}

				// hide marker
				m.is_hidden = true;

				// check if player is in specified area & not in specified leaf
				if (m.areas.contains(g_current_area) && !m.when_not_in_leafs.contains(g_current_leaf)) {
					m.is_hidden = false; // show marker
				}
			}
		}

		// leaf transitions
		if (g_player_leaf_update && !map_settings.remix_transitions.empty())
		{
			for (auto t = map_settings.remix_transitions.begin(); t != map_settings.remix_transitions.end();)
			{
				// only handle leaf transitions
				if (t->trigger_type != map_settings::TRANSITION_TRIGGER_TYPE::LEAF) {
					++t; continue;
				}

				bool iterpp = false;
				bool trigger_transition = false;

				const bool keep_transition = t->mode >= map_settings::ALWAYS_ON_ENTER;
				const bool trigger_on_enter = t->mode == map_settings::ONCE_ON_ENTER || t->mode == map_settings::ALWAYS_ON_ENTER;
				const bool trigger_on_leave = t->mode == map_settings::ONCE_ON_LEAVE || t->mode == map_settings::ALWAYS_ON_LEAVE;

				if (t->leafs.contains(g_current_leaf))
				{
					if (!t->_state_enter) // first time we enter the leafset
					{
						if (trigger_on_enter) {
							trigger_transition = true;
						}

						t->_state_enter = true;
					}
				}

				// no longer touching any leaf in this set
				else
				{
					if (t->_state_enter) // player just moved out of the leafset
					{
						if (trigger_on_leave) {
							trigger_transition = true;
						}
					}

					t->_state_enter = false;
				}

				if (trigger_transition)
				{
					bool can_add_transition = true;

					// do not allow the same transition twice
					for (const auto& ip : remix_vars::interpolate_stack)
					{
						if (ip.identifier == t->hash)
						{
							can_add_transition = false;
							break;
						}
					}

					if (can_add_transition)
					{
						remix_vars::parse_and_apply_conf_with_lerp(
							t->config_name,
							t->hash,
							t->interpolate_type,
							t->duration,
							t->delay_in,
							t->delay_out);

						if (!keep_transition)
						{
							t = map_settings.remix_transitions.erase(t);
							iterpp = true; // erase returns the next iterator
						}
					}
				}

				if (!iterpp) {
					++t;
				}
			}
		}
#if 0
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
#endif
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

	/**
	 * Called from CModelLoader::Map_LoadModel
	 * @param map_name  Name of loading map
	 */
	void on_map_load_hk(const char* map_name)
	{
		remix_vars::on_map_load();
		remix_lights::on_map_load();
		map_settings::on_map_load(map_name);
		main_module::force_cvars();

		game::cvar_uncheat("r_propsmaxdist");
		game::cvar_uncheat("cl_detaildist");
		game::cvar_uncheat("cl_detailfade");
		game::cvar_uncheat("cl_footstep_fx");
		game::cvar_uncheat("cl_fov");
		game::cvar_uncheat("cl_impacteffects");
		game::cvar_uncheat("cl_interpolate");
		game::cvar_uncheat("cl_particle_batch_mode");
		game::cvar_uncheat("cl_particle_fallback_base");
		game::cvar_uncheat("cl_particle_fallback_multiplier");
		game::cvar_uncheat("cl_smoke_alpha");
		game::cvar_uncheat("cl_smoke_far");
		game::cvar_uncheat("cl_viewbob");
		game::cvar_uncheat("cpu_level");
		game::cvar_uncheat("gpu_level");
		game::cvar_uncheat("gpu_mem_level");
		game::cvar_uncheat("fx_drawimpactdebris");
		game::cvar_uncheat("fx_drawimpactdust");
		game::cvar_uncheat("fx_drawmetalspark");
		game::cvar_uncheat("r_decals");
		game::cvar_uncheat("r_decalstaticprops");
		game::cvar_uncheat("r_draw_flashlight_3rd_person");
		game::cvar_uncheat("r_draw_lasersight_1st_person");
		game::cvar_uncheat("r_draw_lasersight_3rd_person");
		game::cvar_uncheat("r_drawbatchdecals");
		game::cvar_uncheat("r_drawflecks");
		game::cvar_uncheat("r_drawmodeldecals");
		game::cvar_uncheat("r_drawunderwaterfogblocker");
		game::cvar_uncheat("r_fade360style");
		game::cvar_uncheat("r_flashlight_3rd_person_range");
		game::cvar_uncheat("r_frustumcullworld");
		game::cvar_uncheat("r_impactparticles");
		game::cvar_uncheat("r_maxmodeldecal");
		game::cvar_uncheat("r_occlusion");
		game::cvar_uncheat("r_particle_timescale");
		game::cvar_uncheat("r_queued_decals");
		game::cvar_uncheat("r_queued_ropes");
		game::cvar_uncheat("r_RainParticleDensity");
		game::cvar_uncheat("r_ropetranslucent");
		game::cvar_uncheat("r_ShowViewerArea");
		game::cvar_uncheat("r_snapportal");
		game::cvar_uncheat("r_staticlight_streams");
		game::cvar_uncheat("r_staticpropinfo");
		game::cvar_uncheat("r_teeth");
		game::cvar_uncheat("r_3dsky");
		game::cvar_uncheat("scene_print");
	}

	HOOK_RETN_PLACE_DEF(on_map_load_stub_retn);
	__declspec(naked) void on_map_load_stub()
	{
		__asm
		{
			pushad;
			push    eax;
			call	on_map_load_hk;
			add		esp, 4;
			popad;

			// og
			lea     ecx, [edi + 0x68];
			xor		edx, edx;
			jmp		on_map_load_stub_retn;

		}
	}

	/**
	 * Called from Host_Disconnect
	 * on: disconnect, restart, killserver, stopdemo ...
	 */
	void on_host_disconnect_hk()
	{
		//choreo_events::reset_all();
		main_module::trigger_vis_logic();

		// ----------

		map_settings::on_map_unload();
	}

	HOOK_RETN_PLACE_DEF(on_host_disconnect_retn);
	__declspec(naked) void on_host_disconnect_stub()
	{
		__asm
		{
			pushad;
			call	on_host_disconnect_hk;
			popad;

			// og
			mov     ebp, esp;
			sub     esp, 0x10;
			jmp		on_host_disconnect_retn;
		}
	}

	/**
	 * Called from Host_Changelevel
	 * Host_Disconnect is not called when this triggers
	 */
	void on_host_change_level_hk()
	{
		on_host_disconnect_hk();
	}

	HOOK_RETN_PLACE_DEF(on_host_change_level_retn);
	__declspec(naked) void on_host_change_level_stub()
	{
		__asm
		{
			pushad;
			call	on_host_change_level_hk;
			popad;

			// og
			push    0x60;
			lea     ecx, [ebp - 0x64];
			jmp		on_host_change_level_retn;
		}
	}

	// #
	// #

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

		if (!game_settings::get()->lod_forcing.get_as<bool>())
		{
			game::cvar_uncheat("r_staticprop_lod");
			game::cvar_uncheat("r_lod");
			game::cvar_uncheat("r_lod_switch_scale"); // hidden cvar
		}
		else
		{
			game::cvar_uncheat_and_set_int("r_staticprop_lod", 0);
			game::cvar_uncheat_and_set_int("r_lod", 0);
			game::cvar_uncheat_and_set_int("r_lod_switch_scale", 1); // hidden cvar
		}

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

		// yes the user could just set it via the console but .. people
		game::cvar_uncheat_and_set_int("r_3dsky", game_settings::get()->enable_3d_sky.get_as<bool>());
		game::cvar_uncheat_and_set_int("mat_fullbright", 1);
		game::cvar_uncheat_and_set_int("mat_softwareskin", 1);
		game::cvar_uncheat_and_set_int("mat_phong", 1);
		game::cvar_uncheat_and_set_int("mat_fastnobump", 1);
		game::cvar_uncheat_and_set_int("mat_disable_bloom", 1);

		game::cvar_uncheat_and_set_int("r_threadeddetailprops", 0);
		game::cvar_uncheat_and_set_int("r_DrawDetailProps", 0); // disables grass (detail) sprites on displacements (unstable and blurry)


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

	// logic after loading either map or game settings
	void main_module::cross_handle_map_and_game_settings()
	{
		if (remix_api::is_initialized())
		{
			// rtx.skyAutoDetect
			const auto is_3d_sky_enabled = game_settings::get()->enable_3d_sky.get_as<bool>();
			remix_vars::set_option(remix_vars::get_option("rtx.skyAutoDetect"), remix_vars::string_to_option_value(remix_vars::OPTION_TYPE_FLOAT, is_3d_sky_enabled ? "1" : "0"));
		}
	}

	// #
	// #

	ConCommand xo_debug_toggle_node_vis_cmd {};
	void main_module::xo_debug_toggle_node_vis_fn()
	{
		cmd::debug_node_vis = !cmd::debug_node_vis;
	}

	main_module::main_module()
	{
		p_this = this;

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

		// CModelLoader::Map_LoadModel :: called on map load
		utils::hook(ENGINE_BASE + 0xEE05C, on_map_load_stub).install()->quick();
		HOOK_RETN_PLACE(on_map_load_stub_retn, ENGINE_BASE + 0xEE061);

		// Host_Disconnect :: called on map unload
		utils::hook(ENGINE_BASE + 0x192F11, on_host_disconnect_stub).install()->quick();
		HOOK_RETN_PLACE(on_host_disconnect_retn, ENGINE_BASE + 0x192F16);

		utils::hook(ENGINE_BASE + 0x18D048, on_host_change_level_stub).install()->quick();
		HOOK_RETN_PLACE(on_host_change_level_retn, ENGINE_BASE + 0x18D04D);

		// --

		// CViewRender::RenderView :: "start" of current frame (after CViewRender::DrawMonitors)
		utils::hook(CLIENT_BASE + 0x1D7113, cviewrenderer_renderview_stub).install()->quick(); // 2501
		HOOK_RETN_PLACE(cviewrenderer_renderview_retn, CLIENT_BASE + 0x1D7118);

		// not needed
		//utils::hook::nop(CLIENT_BASE + 0x1D3F1D, 7);
		//utils::hook(CLIENT_BASE + 0x1D3F1D, skyboxview_draw_internal_stub).install()->quick();
		//HOOK_RETN_PLACE(skyboxview_draw_internal_retn, CLIENT_BASE + 0x1D3F24);

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

		// ~ always show geometry below water surface
		// CSimpleWorldView::Setup :: nop 'DoesViewPlaneIntersectWater' check
		utils::hook::nop(CLIENT_BASE + 0x1CF46F, 2);
		// ^ next instruction :: OR m_DrawFlags with 0x60 instead of 0x30
		//utils::hook::set<BYTE>(CLIENT_BASE + 0x1CF471 + 6, 0x60); ...... not needed in l4d2?

		// engine 0xB3383 - no cull overlay decals (no need)

		// #
		// commands

		game::con_add_command(&xo_debug_toggle_node_vis_cmd, "xo_debug_toggle_node_vis", xo_debug_toggle_node_vis_fn, "Toggle bsp node/leaf debug visualization using the remix api");
	}

	main_module::~main_module()
	{ }
}
