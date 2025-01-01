#include "std_include.hpp"

namespace components
{
	int g_current_leaf = -1;
	int g_current_area = -1;

	namespace api
	{
		bool m_initialized = false;
		remixapi_Interface bridge = {};

		remixapi_MaterialHandle remix_debug_line_materials[3];
		remixapi_MeshHandle remix_debug_line_list[128] = {};
		std::uint32_t remix_debug_line_amount = 0u;
		std::uint64_t remix_debug_last_line_hash = 0u;
		bool remix_debug_node_vis = false; // show/hide debug vis of bsp nodes/leafs
		std::unordered_map<std::string, flashlight_s> m_flashlights;

		void begin_scene_callback()
		{
			if (api::remix_debug_line_amount)
			{
				for (auto l = 1u; l < api::remix_debug_line_amount + 1; l++)
				{
					if (api::remix_debug_line_list[l])
					{
						remixapi_Transform t0 = {};
						t0.matrix[0][0] = 1.0f;
						t0.matrix[1][1] = 1.0f;
						t0.matrix[2][2] = 1.0f;

						const remixapi_InstanceInfo inst =
						{
							.sType = REMIXAPI_STRUCT_TYPE_INSTANCE_INFO,
							.pNext = nullptr,
							.categoryFlags = 0,
							.mesh = api::remix_debug_line_list[l],
							.transform = t0,
							.doubleSided = true
						};

						api::bridge.DrawInstance(&inst);
					}
				}
			}

			for (const auto& [n, fl] : m_flashlights)
			{
				if (fl.handle) {
					bridge.DrawLightInstance(fl.handle);
				}
			}
		}

		void end_scene_callback()
		{
			imgui::endscene_stub();
		}

		// called on device->Present
		void on_present_callback()
		{
			main_module::iterate_entities();

			// Draw current node/leaf as HUD
			if (api::remix_debug_node_vis && main_module::d3d_font)
			{
				RECT rect;

				//if (!map_settings::get_map_name().empty())
				//{
				//	SetRect(&rect, 20, 100, 512, 512);
				//	main_module::d3d_font->DrawTextA
				//	(
				//		nullptr,
				//		map_settings::get_map_name().c_str(),
				//		-1,       // text length (-1 = null-terminated)
				//		&rect,
				//		DT_NOCLIP,
				//		D3DCOLOR_XRGB(255, 255, 255)
				//	);
				//}

				if (g_current_area != -1)
				{
					SetRect(&rect, 20, 125, 512, 512);
					auto text = utils::va("Area: %d", g_current_area);
					main_module::d3d_font->DrawTextA
					(
						nullptr,
						text,
						-1,       // text length (-1 = null-terminated)
						&rect,
						DT_NOCLIP,
						D3DCOLOR_XRGB(255, 255, 255)
					);
				}

				if (g_current_leaf != -1)
				{
					SetRect(&rect, 20, 145, 512, 512);
					auto text = utils::va("Leaf: %d", g_current_leaf);
					main_module::d3d_font->DrawTextA
					(
						nullptr,
						text,
						-1,       // text length (-1 = null-terminated)
						&rect,
						DT_NOCLIP,
						D3DCOLOR_XRGB(50, 255, 20)
					);
				}
			}
		}

		// called once from the main_module constructor
		void init()
		{
			const auto status = remixapi::bridge_initRemixApi(&api::bridge);
			if (status == REMIXAPI_ERROR_CODE_SUCCESS)
			{
				m_initialized = true;
				remixapi::bridge_setRemixApiCallbacks(begin_scene_callback, end_scene_callback, on_present_callback);
			}
		}

		void create_quad(remixapi_HardcodedVertex* v_out, uint32_t* i_out, const float scale)
		{
			if (!v_out || !i_out)
			{
				return;
			}

			auto make_vertex = [&](float x, float y, float z, float u, float v)
				{
					const remixapi_HardcodedVertex vert =
					{
					  .position = { x, y, z },
					  .normal = { 0.0f, 0.0f, -1.0f },
					  .texcoord = { u, v },
					  .color = 0xFFFFFFFF,
					};
					return vert;
				};

			v_out[0] = make_vertex(-1.0f * scale, 1, -1.0f * scale, 0.0f, 0.0f); // bottom left
			v_out[1] = make_vertex(-1.0f * scale, 1, 1.0f * scale, 0.0f, 1.0f); // top left
			v_out[2] = make_vertex(1.0f * scale, 1, -1.0f * scale, 1.0f, 0.0f); // bottom right
			v_out[3] = make_vertex(1.0f * scale, 1, 1.0f * scale, 1.0f, 1.0f); // top right

			i_out[0] = 0; i_out[1] = 1; i_out[2] = 2;
			i_out[3] = 3; i_out[4] = 2; i_out[5] = 1;
		}

		void create_line_quad(remixapi_HardcodedVertex* v_out, uint32_t* i_out, const Vector& p1, const Vector& p2, const float width)
		{
			if (!v_out || !i_out)
			{
				return;
			}

			auto make_vertex = [&](const Vector& pos, float u, float v)
				{
					const remixapi_HardcodedVertex vert =
					{
					  .position = { pos.x, pos.y, pos.z },
					  .normal = { 0.0f, 0.0f, -1.0f },
					  .texcoord = { u, v },
					  .color = 0xFFFFFFFF,
					};
					return vert;
				};

			Vector up = { 0.0f, 0.0f, 1.0f };

			// dir of the line
			Vector dir = p2 - p1;
			dir.Normalize();

			// check if dir is parallel or very close to the up vector
			if (fabs(DotProduct(dir, up)) > 0.999f)
			{
				// if parallel, choose a different up vector
				up = { 1.0f, 0.0f, 0.0f };
			}

			Vector perp = dir.Cross(up); // perpendicular vector to line
			perp.Normalize(); // unit length

			// scale by half width to offset vertices
			const Vector offset = perp * (width * 0.5f);

			v_out[0] = make_vertex(p1 - offset, 0.0f, 0.0f); // bottom left
			v_out[1] = make_vertex(p1 + offset, 0.0f, 1.0f); // top left
			v_out[2] = make_vertex(p2 - offset, 1.0f, 0.0f); // bottom right
			v_out[3] = make_vertex(p2 + offset, 1.0f, 1.0f); // top right

			i_out[0] = 0; i_out[1] = 1; i_out[2] = 2;
			i_out[3] = 3; i_out[4] = 2; i_out[5] = 1;
		}

		void add_debug_line(const Vector& p1, const Vector& p2, const float width, DEBUG_REMIX_LINE_COLOR color)
		{
			if (remix_debug_line_materials[color])
			{
				remix_debug_line_amount++;
				remixapi_HardcodedVertex verts[4] = {};
				uint32_t indices[6] = {};
				api::create_line_quad(verts, indices, p1, p2, width);

				remixapi_MeshInfoSurfaceTriangles triangles =
				{
				  .vertices_values = verts,
				  .vertices_count = ARRAYSIZE(verts),
				  .indices_values = indices,
				  .indices_count = 6,
				  .skinning_hasvalue = FALSE,
				  .skinning_value = {},
				  .material = remix_debug_line_materials[color],
				};

				remixapi_MeshInfo info
				{
					.sType = REMIXAPI_STRUCT_TYPE_MESH_INFO,
					.hash = utils::string_hash64(utils::va("line%d", remix_debug_last_line_hash ? remix_debug_last_line_hash : 1)),
					.surfaces_values = &triangles,
					.surfaces_count = 1,
				}; 

				api::bridge.CreateMesh(&info, &remix_debug_line_list[remix_debug_line_amount]);
				remix_debug_last_line_hash = reinterpret_cast<std::uint64_t>(remix_debug_line_list[remix_debug_line_amount]);
			}
		}
	}

	// ------------------------------------------------------

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

							auto update_flashlight = [&](const Vector& pos, const Vector& fwd, const Vector& rt, const Vector& up, bool is_enabled, bool is_player_flag)
								{
									auto it = api::m_flashlights.find(info.name);
									if (it == api::m_flashlights.end())
									{
										// insert new flashlight data
										api::m_flashlights[info.name] =
										{
											.def = {.pos = pos, .fwd = fwd, .rt = rt, .up = up },
											.is_player = is_player_flag,
											.is_enabled = is_enabled
										};
									}
									else
									{
										// update existing flashlight data
										it->second.def.pos = pos;
										it->second.def.fwd = fwd;
										it->second.def.rt = rt;
										it->second.def.up = up;
										it->second.is_player = is_player_flag;
										it->second.is_enabled = is_enabled;
									}
								};


							const auto is_player = i == intf->m_engine->get_local_player();
							if (!is_player)
							{
								const auto& m_fEffects = entity->read<int>(0xE0);
								const bool flashlight_enabled = m_fEffects & 4;

								const auto& eyepos = entity->get_eye_pos();
								const auto& angles = entity->read<Vector>(0x196C); // m_angEyeAngles[0] - DT_CSPlayer 

								Vector fwd, rt, up;
								utils::vector::AngleVectors(angles, &fwd, &rt, &up);

								update_flashlight(eyepos, fwd, rt, up, flashlight_enabled, false);
							}
							else
							{
								// player
								const auto& flashlight_enabled = entity->read<bool>(0x14D8);
								const auto& eyepos = entity->read<Vector>(0x1110);
								const auto& fwd = entity->read<Vector>(0x111C);
								const auto& rt = entity->read<Vector>(0x1134);
								const auto& up = entity->read<Vector>(0x1128);

								update_flashlight(eyepos, fwd, rt, up, flashlight_enabled, true);
							}
							break;
						}
					}
				}
			}
		}
	}

	void flashlight_update_init()
	{
		if (api::m_initialized)
		{
			for (auto& [name, fl] : api::m_flashlights)
			{
				if (fl.handle)
				{
					api::bridge.DestroyLight(fl.handle);
					fl.handle = nullptr;
				}

				if (fl.is_enabled)
				{
					const auto* im = imgui::get();

					auto& info = fl.info;
					auto& ext = fl.ext;

					ext.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO_SPHERE_EXT;
					ext.pNext = nullptr;

					const Vector light_org = fl.def.pos
						+ (fl.def.fwd * (fl.is_player ? im->m_flashlight_fwd_offset  : im->m_flashlight_bot_fwd_offset))
						+ (fl.def.up  * (fl.is_player ? im->m_flashlight_vert_offset : im->m_flashlight_bot_vert_offset))
						+ (fl.def.rt  * (fl.is_player ? im->m_flashlight_horz_offset : im->m_flashlight_bot_horz_offset));

					ext.position = light_org.ToRemixFloat3D();

					ext.radius = im->m_flashlight_radius;
					ext.shaping_hasvalue = TRUE;
					ext.shaping_value = {};

					if (im->m_flashlight_use_custom_dir)
					{
						auto nrm_dir = im->m_flashlight_direction; nrm_dir.Normalize();
						ext.shaping_value.direction = nrm_dir.ToRemixFloat3D();
					}
					else
					{
						ext.shaping_value.direction = fl.def.fwd.ToRemixFloat3D();
					}

					ext.shaping_value.coneAngleDegrees = im->m_flashlight_angle;
					ext.shaping_value.coneSoftness = im->m_flashlight_softness;
					ext.shaping_value.focusExponent = im->m_flashlight_exp;

					info.sType = REMIXAPI_STRUCT_TYPE_LIGHT_INFO;
					info.pNext = &fl.ext;
					info.hash = utils::string_hash64(utils::va("fl%s", name.c_str()));
					info.radiance = remixapi_Float3D{ 20.0f * im->m_flashlight_intensity, 20.0f * im->m_flashlight_intensity, 20.0f * im->m_flashlight_intensity };

					api::bridge.CreateLight(&fl.info, &fl.handle);
				}
			}
		}
	}

	void on_renderview()
	{
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


		// api debug lines
		if (!api::remix_debug_line_materials[0])
		{
			remixapi_MaterialInfo info
			{
				.sType = REMIXAPI_STRUCT_TYPE_MATERIAL_INFO,
				.hash = utils::string_hash64("linemat0"),
				.albedoTexture = L"",
				.normalTexture = L"",
				.tangentTexture = L"",
				.emissiveTexture = L"",
				.emissiveIntensity = 1.0f,
				.emissiveColorConstant = { 1.0f, 0.0f, 0.0f },
			};

			info.albedoTexture = L"";
			info.normalTexture = L"";
			info.tangentTexture = L"";
			info.emissiveTexture = L"";

			remixapi_MaterialInfoOpaqueEXT ext = {};
			{
				ext.sType = REMIXAPI_STRUCT_TYPE_MATERIAL_INFO_OPAQUE_EXT;
				ext.useDrawCallAlphaState = 1;
				ext.opacityConstant = 1.0f;
				ext.roughnessTexture = L"";
				ext.metallicTexture = L"";
				ext.heightTexture = L"";
			}
			info.pNext = &ext;

			api::bridge.CreateMaterial(&info, &api::remix_debug_line_materials[0]);

			info.hash = utils::string_hash64("linemat1");
			info.emissiveColorConstant = { 0.0f, 1.0f, 0.0f };
			api::bridge.CreateMaterial(&info, &api::remix_debug_line_materials[1]);

			info.hash = utils::string_hash64("linemat3");
			info.emissiveColorConstant = { 0.0f, 1.0f, 1.0f };
			api::bridge.CreateMaterial(&info, &api::remix_debug_line_materials[2]);
		}

		// destroy all lines added the prev. frame
		if (api::remix_debug_line_amount)
		{
			for (auto& line : api::remix_debug_line_list)
			{
				if (line)
				{
					api::bridge.DestroyMesh(line);
					line = nullptr;
				}
			}

			api::remix_debug_line_amount = 0;
		}

		flashlight_update_init();
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
		// disable frustum culling for now
		return 0;

		// R_CullNode - uses area frustums if avail. and not in a solid - uses player frustum otherwise
		if (!utils::hook::call<bool(__cdecl)(mnode_t*)>(ENGINE_BASE + 0xFC490)(node)) {
			return 0;
		}

		// ^ R_CullNode would cull the node if we reach this point
		// MODE: force all leafs/nodes in CURRENT area
		//if (!g_player_current_area_override || cmode == map_settings::AREA_CULL_MODE_FRUSTUM_FORCE_AREA)
		{
			// force draw this node/leaf if it's within the forced area
			if ((int)node->area == g_current_area) {
				return 0;
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
			push	ebx;
			call	r_cullnode_wrapper; // return 0 to not jump
			add		esp, 4;
			test	eax, eax;
			jz		SKIP; // jump if eax = 0
			popad;

			add     esp, 4; // og
			jmp		r_cullnode_cull_retn;

		SKIP:
			popad;
			add     esp, 4; // og
			jmp		r_cullnode_skip_retn;
		}
	}


	void pre_recursive_world_node()
	{
		const auto world = game::get_hoststate_worldbrush_data();

		// show leaf index as 3D text
		if (g_current_leaf < world->numleafs)
		{
			if (api::remix_debug_node_vis)
			{
				const auto curr_leaf = &world->leafs[g_current_leaf];
				//game::debug_add_text_overlay(&curr_leaf->m_vecCenter.x, 0.0f, utils::va("Leaf: %i", g_current_leaf));
				main_module::debug_draw_box(curr_leaf->m_vecCenter, curr_leaf->m_vecHalfDiagonal, 2.0f, api::DEBUG_REMIX_LINE_COLOR::GREEN);
			}
		}

		if (game::get_viewid() != VIEW_3DSKY)
		//if (!g_player_current_area_override || g_player_current_area_override->cull_mode == map_settings::AREA_CULL_MODE_FRUSTUM_FORCE_AREA)
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
	}

	HOOK_RETN_PLACE_DEF(pre_recursive_world_node_retn);
	__declspec(naked) void pre_recursive_world_node_stub()
	{
		__asm
		{
			pushad;
			call	pre_recursive_world_node;
			popad;

			// og
			mov     edx, [eax + 0x50];
			mov     ecx, ebx;
			jmp		pre_recursive_world_node_retn;
		}
	}

	/**
	 * Draw a wireframe box using the remix api
	 * @param center		Center of the cube
	 * @param half_diagonal Half diagonal distance of the box
	 * @param width			Line width
	 * @param color			Line color
	 */
	void main_module::debug_draw_box(const VectorAligned& center, const VectorAligned& half_diagonal, const float width, const api::DEBUG_REMIX_LINE_COLOR& color)
	{
		Vector min, max;
		Vector corners[8];

		// calculate min and max positions based on the center and half diagonal
		min = center - half_diagonal;
		max = center + half_diagonal;

		// get the corners of the cube
		corners[0] = Vector(min.x, min.y, min.z);
		corners[1] = Vector(min.x, min.y, max.z);
		corners[2] = Vector(min.x, max.y, min.z);
		corners[3] = Vector(min.x, max.y, max.z);
		corners[4] = Vector(max.x, min.y, min.z);
		corners[5] = Vector(max.x, min.y, max.z);
		corners[6] = Vector(max.x, max.y, min.z);
		corners[7] = Vector(max.x, max.y, max.z);

		// define the edges
		Vector lines[12][2];
		lines[0][0] = corners[0];	lines[0][1] = corners[1]; // Edge 1
		lines[1][0] = corners[0];	lines[1][1] = corners[2]; // Edge 2
		lines[2][0] = corners[0];	lines[2][1] = corners[4]; // Edge 3
		lines[3][0] = corners[1];	lines[3][1] = corners[3]; // Edge 4
		lines[4][0] = corners[1];	lines[4][1] = corners[5]; // Edge 5
		lines[5][0] = corners[2];	lines[5][1] = corners[3]; // Edge 6
		lines[6][0] = corners[2];	lines[6][1] = corners[6]; // Edge 7
		lines[7][0] = corners[3];	lines[7][1] = corners[7]; // Edge 8
		lines[8][0] = corners[4];	lines[8][1] = corners[5]; // Edge 9
		lines[9][0] = corners[4];	lines[9][1] = corners[6]; // Edge 10
		lines[10][0] = corners[5];	lines[10][1] = corners[7]; // Edge 11
		lines[11][0] = corners[6];	lines[11][1] = corners[7]; // Edge 12

		for (auto e = 0u; e < 12; e++) {
			api::add_debug_line(lines[e][0], lines[e][1], width, color);
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

	ConCommand xo_debug_toggle_node_vis_cmd{};
	void xo_debug_toggle_node_vis_fn()
	{
		api::remix_debug_node_vis = !api::remix_debug_node_vis;
	}

	main_module::main_module()
	{
		{ // init filepath var
			char path[MAX_PATH]; GetModuleFileNameA(nullptr, path, MAX_PATH);
			game::root_path = path; utils::erase_substring(game::root_path, "left4dead2.exe");
		}

		// init remixAPI
		api::init();

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
		// commands

		game::con_add_command(&xo_debug_toggle_node_vis_cmd, "xo_debug_toggle_node_vis", xo_debug_toggle_node_vis_fn, "Toggle bsp node/leaf debug visualization using the remix api");


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

		// stub before calling 'R_RecursiveWorldNode' to override node/leaf vis
		utils::hook(ENGINE_BASE + 0xD1648, pre_recursive_world_node_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(pre_recursive_world_node_retn, ENGINE_BASE + 0xD164D);

		// ^ :: while( ... node->contents < -1 .. ) -> jl to jle
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

		// CBrushBatchRender::DrawOpaqueBrushModel :: :: backface check - nop 'if ( bShadowDepth )' to disable culling
		utils::hook::nop(ENGINE_BASE + 0xD2250, 2);

		// CClientLeafSystem::ExtractCulledRenderables :: disable 'engine->CullBox' check to disable entity culling in leafs
		// needs r_PortalTestEnts to be 0 -> je to jmp (0xEB)
		utils::hook::set<BYTE>(CLIENT_BASE + 0xBDA76, 0xEB);
	}

	main_module::~main_module()
	{ }
}
