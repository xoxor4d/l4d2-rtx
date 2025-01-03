#include "std_include.hpp"

namespace components
{
	// called on device->BeginScene
	void remix_api::begin_scene_callback()
	{
		const auto api = get();
		if (api->m_debug_line_amount)
		{
			for (auto l = 1u; l < api->m_debug_line_amount + 1; l++)
			{
				if (api->m_debug_line_list[l])
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
						.mesh = api->m_debug_line_list[l],
						.transform = t0,
						.doubleSided = true
					};

					api->m_bridge.DrawInstance(&inst);
				}
			}
		}

		for (const auto& [n, fl] : api->m_flashlights)
		{
			if (fl.handle) {
				api->m_bridge.DrawLightInstance(fl.handle);
			}
		}
	}

	// called on device->EndScene
	void remix_api::end_scene_callback()
	{
		imgui::endscene_stub(); 

#if 0
		if (!model_render::get()->m_drew_hud)
		{
			const auto dev = game::get_d3d_device();
			IDirect3DVertexShader9* og_vs = nullptr;
			dev->GetVertexShader(&og_vs);
			dev->SetVertexShader(nullptr);

			IDirect3DPixelShader9* og_ps = nullptr;
			dev->GetPixelShader(&og_ps);
			dev->SetPixelShader(nullptr);

			IDirect3DBaseTexture9* og_tex = nullptr;
			dev->GetTexture(0, &og_tex);
			dev->SetTexture(0, nullptr);

			DWORD og_zwrite;
			dev->GetRenderState(D3DRS_ZWRITEENABLE, &og_zwrite);
			dev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

			D3DMATRIX* og_view = nullptr;
			dev->GetTransform(D3DTS_VIEW, og_view);

			D3DMATRIX* og_proj = nullptr;
			dev->GetTransform(D3DTS_PROJECTION, og_proj);

			dev->SetTransform(D3DTS_WORLD, &game::IDENTITY);
			dev->SetTransform(D3DTS_VIEW, &game::IDENTITY);
			dev->SetTransform(D3DTS_PROJECTION, &game::IDENTITY);

			struct CUSTOMVERTEX
			{
				float x, y, z, rhw;
				D3DCOLOR color = D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, 0.0f);
			};

			CUSTOMVERTEX vertices[] =
			{
				{ -0.5f,  -0.5f,  0.0f, 1.0f }, // tl
				{ -0.49f, -0.5f,  0.0f, 1.0f }, // tr
				{ -0.5f,  -0.49f, 0.0f, 1.0f }, // bl
				{ -0.49f, -0.49f, 0.0f, 1.0f }  // br
			};

			DWORD og_unused;
			dev->GetRenderState((D3DRENDERSTATETYPE)42, &og_unused);
			dev->SetRenderState((D3DRENDERSTATETYPE)42, REMIXAPI_INSTANCE_CATEGORY_BIT_WORLD_MATTE);

			dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
			dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(CUSTOMVERTEX));

			dev->SetVertexShader(og_vs);
			dev->SetPixelShader(og_ps);
			dev->SetTexture(0, og_tex);
			dev->SetRenderState(D3DRS_ZWRITEENABLE, og_zwrite);
			dev->SetTransform(D3DTS_VIEW, og_view);
			dev->SetTransform(D3DTS_PROJECTION, og_proj);

			model_render::get()->m_drew_hud = true;
		}
#endif
	}

	// called on device->Present
	void remix_api::on_present_callback()
	{
		main_module::iterate_entities();

		// Draw current node/leaf as HUD
		if (is_node_debug_enabled() && main_module::d3d_font)
		{
			RECT rect;
			if (g_current_area != -1)
			{
				SetRect(&rect, 60, 125, 512, 512);
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
				SetRect(&rect, 60, 145, 512, 512);
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

	// #
	// #

	void remix_api::init_debug_lines()
	{
		if (!m_debug_lines_initialized)
		{
			remixapi_MaterialInfoOpaqueEXT ext = {};
			{
				ext.sType = REMIXAPI_STRUCT_TYPE_MATERIAL_INFO_OPAQUE_EXT;
				ext.useDrawCallAlphaState = 1;
				ext.opacityConstant = 1.0f;
				ext.roughnessConstant = 1.0f;
				ext.metallicConstant = 1.0f;
				ext.roughnessTexture = L"";
				ext.metallicTexture = L"";
				ext.heightTexture = L"";
			}

			remixapi_MaterialInfo info
			{
				.sType = REMIXAPI_STRUCT_TYPE_MATERIAL_INFO,
				.pNext = &ext,
				.hash = utils::string_hash64("linemat0"),
				.albedoTexture = L"",
				.normalTexture = L"",
				.tangentTexture = L"",
				.emissiveTexture = L"",
				.emissiveIntensity = 0.1f,
				.emissiveColorConstant = { 1.0f, 0.0f, 0.0f },
			};
			m_bridge.CreateMaterial(&info, &m_debug_line_materials[0]);

			info.hash = utils::string_hash64("linemat1");
			info.emissiveColorConstant = { 0.0f, 1.0f, 0.0f };
			m_bridge.CreateMaterial(&info, &m_debug_line_materials[1]);

			info.hash = utils::string_hash64("linemat3");
			info.emissiveColorConstant = { 0.0f, 1.0f, 1.0f };
			m_bridge.CreateMaterial(&info, &m_debug_line_materials[2]);

			m_debug_lines_initialized = true;
		}
	}

	void remix_api::create_quad(remixapi_HardcodedVertex* v_out, uint32_t* i_out, const float scale)
	{
		if (!v_out || !i_out) {
			return;
		}

		auto make_vertex = [&](const float x, const float y, const float z, const float u, const float v)
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

	void remix_api::create_line_quad(remixapi_HardcodedVertex* v_out, uint32_t* i_out, const Vector& p1, const Vector& p2, const float width)
	{
		if (!v_out || !i_out) {
			return;
		}

		auto make_vertex = [&](const Vector& pos, const float u, const float v)
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
		Vector dir = p2 - p1;
		dir.Normalize();

		// check if dir is parallel or very close to the up vector
		if (fabs(DotProduct(dir, up)) > 0.999f) {
			up = { 1.0f, 0.0f, 0.0f }; // if parallel, choose a different up vector
		}

		// perpendicular vector to line
		Vector perp = dir.Cross(up); 
		perp.Normalize();

		// scale by half width to offset vertices
		const Vector offset = perp * (width * 0.5f);

		v_out[0] = make_vertex(p1 - offset, 0.0f, 0.0f); // bottom left
		v_out[1] = make_vertex(p1 + offset, 0.0f, 1.0f); // top left
		v_out[2] = make_vertex(p2 - offset, 1.0f, 0.0f); // bottom right
		v_out[3] = make_vertex(p2 + offset, 1.0f, 1.0f); // top right

		i_out[0] = 0; i_out[1] = 1; i_out[2] = 2;
		i_out[3] = 3; i_out[4] = 2; i_out[5] = 1;
	}

	void remix_api::add_debug_line(const Vector& p1, const Vector& p2, const float width, DEBUG_REMIX_LINE_COLOR color)
	{
		if (m_debug_line_materials[color])
		{
			if (can_add_debug_lines())
			{
				m_debug_line_amount++;
				remixapi_HardcodedVertex verts[4] = {};
				uint32_t indices[6] = {};
				create_line_quad(verts, indices, p1, p2, width);

				remixapi_MeshInfoSurfaceTriangles triangles =
				{
				  .vertices_values = verts,
				  .vertices_count = ARRAYSIZE(verts),
				  .indices_values = indices,
				  .indices_count = 6,
				  .skinning_hasvalue = FALSE,
				  .skinning_value = {},
				  .material = m_debug_line_materials[color],
				};

				remixapi_MeshInfo info
				{
					.sType = REMIXAPI_STRUCT_TYPE_MESH_INFO,
					.hash = utils::string_hash64(utils::va("line%d", m_debug_last_line_hash ? m_debug_last_line_hash : 1)),
					.surfaces_values = &triangles,
					.surfaces_count = 1,
				};

				m_bridge.CreateMesh(&info, &m_debug_line_list[m_debug_line_amount]);
				m_debug_last_line_hash = reinterpret_cast<std::uint64_t>(m_debug_line_list[m_debug_line_amount]);
			}
		}
	}

	/**
	 * Draw a wireframe box using the remix api
	 * @param center		Center of the cube
	 * @param half_diagonal Half diagonal distance of the box
	 * @param width			Line width
	 * @param color			Line color
	 */
	void remix_api::debug_draw_box(const VectorAligned& center, const VectorAligned& half_diagonal, const float width, const DEBUG_REMIX_LINE_COLOR& color)
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
			add_debug_line(lines[e][0], lines[e][1], width, color);
		}
	}

	void remix_api::flashlight_create_or_update(const char* player_name, const Vector& pos, const Vector& fwd, const Vector& rt, const Vector& up, bool is_enabled, bool is_player)
	{
		if (const auto it = m_flashlights.find(player_name);
			it == m_flashlights.end())
		{
			// insert new flashlight data
			m_flashlights[player_name] =
			{
				.def = {.pos = pos, .fwd = fwd, .rt = rt, .up = up },
				.is_player = is_player,
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
			it->second.is_player = is_player;
			it->second.is_enabled = is_enabled;
		}
	}

	void flashlight_frame()
	{
		if (const auto api = remix_api::get();
			remix_api::is_initialized())
		{
			for (auto& [name, fl] : api->m_flashlights)
			{
				if (fl.handle)
				{
					api->m_bridge.DestroyLight(fl.handle);
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
						+ (fl.def.fwd * (fl.is_player ? im->m_flashlight_fwd_offset : im->m_flashlight_bot_fwd_offset))
						+ (fl.def.up * (fl.is_player ? im->m_flashlight_vert_offset : im->m_flashlight_bot_vert_offset))
						+ (fl.def.rt * (fl.is_player ? im->m_flashlight_horz_offset : im->m_flashlight_bot_horz_offset));

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

					api->m_bridge.CreateLight(&fl.info, &fl.handle);
				}
			}
		}
	}

	// called from main_module::on_renderview()
	void remix_api::on_renderview()
	{
		if (is_initialized()) 
		{
			flashlight_frame();

			init_debug_lines();

			// destroy all lines added the prev. frame
			if (m_debug_line_amount)
			{
				for (auto& line : m_debug_line_list)
				{
					if (line)
					{
						m_bridge.DestroyMesh(line);
						line = nullptr;
					}
				}
				m_debug_line_amount = 0;
			}
		}
	}

	// #
	// #

	ConCommand xo_debug_toggle_node_vis_cmd{};
	void remix_api::xo_debug_toggle_node_vis_fn()
	{
		get()->m_cmd_debug_node_vis = !get()->m_cmd_debug_node_vis;
	}

	// #
	// #

	remix_api::remix_api()
	{
		p_this = this;

		if (const auto status = remixapi::bridge_initRemixApi(&m_bridge); 
			status == REMIXAPI_ERROR_CODE_SUCCESS)
		{
			get()->m_initialized = true;
			remixapi::bridge_setRemixApiCallbacks(begin_scene_callback, end_scene_callback, on_present_callback);
		}
		else { game::console(); printf("[!][RemixApi] Failed to initialize the remixApi - Code: %d\n", status); }


		// #
		// commands
		game::con_add_command(&xo_debug_toggle_node_vis_cmd, "xo_debug_toggle_node_vis", xo_debug_toggle_node_vis_fn, "Toggle bsp node/leaf debug visualization using the remix api");

	}
}