#include "std_include.hpp"

//#define ENABLE_3D_SKY 1

namespace components
{
	namespace cmd
	{
		bool model_info_vis = false;
	}

	namespace tex_addons
	{
		LPDIRECT3DTEXTURE9 glass_shards;
		LPDIRECT3DTEXTURE9 black;
		LPDIRECT3DTEXTURE9 white;
	}

	void model_render::init_texture_addons(bool release)
	{
		if (release)
		{
			if (tex_addons::glass_shards) tex_addons::glass_shards->Release();
			if (tex_addons::black) tex_addons::black->Release();
			if (tex_addons::white) tex_addons::white->Release();
			return;
		}

		const auto dev = game::get_d3d_device();
		D3DXCreateTextureFromFileA(dev, "l4d2-rtx\\textures\\glass_shards.png", &tex_addons::glass_shards);
		D3DXCreateTextureFromFileA(dev, "l4d2-rtx\\textures\\black.dds", &tex_addons::black);
		D3DXCreateTextureFromFileA(dev, "l4d2-rtx\\textures\\white.dds", &tex_addons::white);
	}

	// check for specific material var and return it in 'out_var'
	bool has_materialvar(IMaterialInternal* cmat, const char* var_name, IMaterialVar** out_var = nullptr)
	{
		bool found = false;
		const auto var = cmat->vftable->FindVar(cmat, nullptr, var_name, &found, false);

		if (out_var) {
			*out_var = var;
		}

		return found;
	}

	D3DCOLORVALUE g_old_light_to_texture_color = {};
	bool g_light_to_texture_modified = false;

	// To be used before rendering a surface with a texture that is marked with the 'add light to tex' category in remix
	// > will change the color and intensity of the light created by remix
	// > supports 1 saved state for now (should be enough)
	// > values also influence radiance (can be larger than 1)
	void add_light_to_texture_color_edit(const float& r, const float& g, const float& b, const float scalar = 1.0f)
	{
		const auto dev = game::get_d3d_device();

		D3DMATERIAL9 temp_mat = {};
		dev->GetMaterial(&temp_mat);

		// save prev. color
		g_old_light_to_texture_color = temp_mat.Diffuse;

		temp_mat.Diffuse = { .r = r * scalar, .g = g * scalar, .b = b * scalar };
		dev->SetMaterial(&temp_mat);

		g_light_to_texture_modified = true;
	}

	// restore color
	void add_light_to_texture_color_restore()
	{
		if (g_light_to_texture_modified)
		{
			const D3DMATERIAL9 temp_mat = {
				.Diffuse = { .r = g_old_light_to_texture_color.r, .g = g_old_light_to_texture_color.g, .b = g_old_light_to_texture_color.b }
			};

			game::get_d3d_device()->SetMaterial(&temp_mat);
			g_light_to_texture_modified = false;
		}
	}

	enum REMIX_MODIFIER : std::uint32_t
	{
		NONE = 0,
		INFECTED = 1 << 0,
		//EMISSIVE_TWEAK = 1 << 1,
	};

	// set remix texture categories - RemixInstanceCategories
	// ~ currently req. runtime changes
	void set_remix_texture_categories(IDirect3DDevice9* dev, prim_fvf_context& ctx, const std::uint32_t& cat)
	{
		ctx.save_rs(dev, (D3DRENDERSTATETYPE)42);
		dev->SetRenderState((D3DRENDERSTATETYPE)42, cat);
	}

	// set custom remix hash
	// ~ currently req. runtime changes
	void set_remix_texture_hash(IDirect3DDevice9* dev, prim_fvf_context& ctx, const std::uint32_t& hash)
	{
		ctx.save_rs(dev, (D3DRENDERSTATETYPE)150);
		dev->SetRenderState((D3DRENDERSTATETYPE)150, hash);
	}


	// can be used to figure out the layout of the vertex buffer
	void lookat_vertex_decl([[maybe_unused]] IDirect3DDevice9* dev, [[maybe_unused]] CPrimList* primlist = nullptr)
	{
#ifdef DEBUG
		IDirect3DVertexDeclaration9* vertex_decl = nullptr;
		dev->GetVertexDeclaration(&vertex_decl);

		enum d3ddecltype : BYTE
		{
			D3DDECLTYPE_FLOAT1 = 0,		// 1D float expanded to (value, 0., 0., 1.)
			D3DDECLTYPE_FLOAT2 = 1,		// 2D float expanded to (value, value, 0., 1.)
			D3DDECLTYPE_FLOAT3 = 2,		// 3D float expanded to (value, value, value, 1.)
			D3DDECLTYPE_FLOAT4 = 3,		// 4D float
			D3DDECLTYPE_D3DCOLOR = 4,	// 4D packed unsigned bytes mapped to 0. to 1. range

			// Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
			D3DDECLTYPE_UBYTE4 = 5,		// 4D unsigned byte
			D3DDECLTYPE_SHORT2 = 6,		// 2D signed short expanded to (value, value, 0., 1.)
			D3DDECLTYPE_SHORT4 = 7,		// 4D signed short

			// The following types are valid only with vertex shaders >= 2.0
			D3DDECLTYPE_UBYTE4N = 8,	// Each of 4 bytes is normalized by dividing to 255.0
			D3DDECLTYPE_SHORT2N = 9,	// 2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
			D3DDECLTYPE_SHORT4N = 10,	// 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
			D3DDECLTYPE_USHORT2N = 11,  // 2D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,0,1)
			D3DDECLTYPE_USHORT4N = 12,  // 4D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,v[2]/65535.0,v[3]/65535.0)
			D3DDECLTYPE_UDEC3 = 13,		// 3D unsigned 10 10 10 format expanded to (value, value, value, 1)
			D3DDECLTYPE_DEC3N = 14,		// 3D signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1)
			D3DDECLTYPE_FLOAT16_2 = 15,	// Two 16-bit floating point values, expanded to (value, value, 0, 1)
			D3DDECLTYPE_FLOAT16_4 = 16,	// Four 16-bit floating point values
			D3DDECLTYPE_UNUSED = 17,	// When the type field in a decl is unused.
		};

		enum d3ddecluse : BYTE
		{
			D3DDECLUSAGE_POSITION = 0,
			D3DDECLUSAGE_BLENDWEIGHT,   // 1
			D3DDECLUSAGE_BLENDINDICES,  // 2
			D3DDECLUSAGE_NORMAL,        // 3
			D3DDECLUSAGE_PSIZE,         // 4
			D3DDECLUSAGE_TEXCOORD,      // 5
			D3DDECLUSAGE_TANGENT,       // 6
			D3DDECLUSAGE_BINORMAL,      // 7
			D3DDECLUSAGE_TESSFACTOR,    // 8
			D3DDECLUSAGE_POSITIONT,     // 9
			D3DDECLUSAGE_COLOR,         // 10
			D3DDECLUSAGE_FOG,           // 11
			D3DDECLUSAGE_DEPTH,         // 12
			D3DDECLUSAGE_SAMPLE,        // 13
		};

		struct d3dvertelem
		{
			WORD Stream;		// Stream index
			WORD Offset;		// Offset in the stream in bytes
			d3ddecltype Type;	// Data type
			BYTE Method;		// Processing method
			d3ddecluse Usage;	// Semantics
			BYTE UsageIndex;	// Semantic index
		};

		d3dvertelem decl[MAX_FVF_DECL_SIZE]; UINT numElements = 0;
		vertex_decl->GetDeclaration((D3DVERTEXELEMENT9*)decl, &numElements);
		int break_me = 1; // look into decl
#endif
	}

	// draw 'nocull' map_setting marker meshes
	void model_render::draw_nocull_markers()
	{
		const auto& msettings = map_settings::get_map_settings();

		// early out
		if (msettings.map_markers.empty()) {
			return;
		}

		const auto dev = game::get_d3d_device();

		struct vertex {
			D3DXVECTOR3 position; D3DCOLOR color; float tu, tv;
		};

		// save & restore after drawing
		IDirect3DVertexShader9* og_vs = nullptr;
		dev->GetVertexShader(&og_vs);
		dev->SetVertexShader(nullptr);

		IDirect3DBaseTexture9* og_tex = nullptr;
		dev->GetTexture(0, &og_tex);
		dev->SetTexture(0, tex_addons::white);

		DWORD og_rs;
		dev->GetRenderState((D3DRENDERSTATETYPE)150, &og_rs);

		dev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		//D3DXMATRIX mtx = game::IDENTITY;

		for (auto& m : msettings.map_markers)
		{
			// ignore normal markers
			if (!m.no_cull) {
				continue;
			}

			if (m.is_hidden) {
				continue;
			}

			const float f_index = static_cast<float>(m.index);
			const vertex mesh_verts[4] =
			{
				D3DXVECTOR3(-1.337f - (f_index * 0.01f), -1.337f - (f_index * 0.01f), 0), D3DCOLOR_XRGB(m.index, 0, 0), 0.0f, f_index / 100.0f,
				D3DXVECTOR3( 1.337f + (f_index * 0.01f), -1.337f - (f_index * 0.01f), 0), D3DCOLOR_XRGB(0, m.index, 0), f_index / 100.0f, 0.0,
				D3DXVECTOR3( 1.337f + (f_index * 0.01f),  1.337f + (f_index * 0.01f), 0), D3DCOLOR_XRGB(0, 0, m.index), 0.0f, f_index / 100.0f,
				D3DXVECTOR3(-1.337f - (f_index * 0.01f),  1.337f + (f_index * 0.01f), 0), D3DCOLOR_XRGB(m.index, 0, m.index), 0.0f, f_index / 100.0f,
			};

			D3DXMATRIX scale_matrix, rotation_x, rotation_y, rotation_z, mat_rotation, mat_translation, world;

			D3DXMatrixScaling(&scale_matrix, m.scale.x, m.scale.y, m.scale.z);
			D3DXMatrixRotationX(&rotation_x, m.rotation.x); // pitch
			D3DXMatrixRotationY(&rotation_y, m.rotation.y); // yaw
			D3DXMatrixRotationZ(&rotation_z, m.rotation.z); // roll
			mat_rotation = rotation_z * rotation_y * rotation_x; // combine rotations (order: Z * Y * X)

			D3DXMatrixTranslation(&mat_translation, m.origin.x, m.origin.y, m.origin.z);
			world = scale_matrix * mat_rotation * mat_translation;

			// set remix texture hash ~req. dxvk-runtime changes - not really needed
			dev->SetRenderState((D3DRENDERSTATETYPE)150, 100 + m.index);

			dev->SetTransform(D3DTS_WORLD, &world);
			dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, mesh_verts, sizeof(vertex));
		}

		// restore
		dev->SetVertexShader(og_vs);
		dev->SetTexture(0, og_tex);
		dev->SetRenderState((D3DRENDERSTATETYPE)150, og_rs);
		dev->SetFVF(NULL);
		dev->SetTransform(D3DTS_WORLD, &game::IDENTITY);
	}

	// detoured 'CModelRender::DrawModelExecute'
	void __fastcall tbl_hk::model_renderer::DrawModelExecute::Detour(void* ecx, void* edx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
	{
		// draw nocull markers before drawing the first model - no particular reason besides that we dont want to draw them before rendering the sky
		if (!model_render::get()->m_drew_model)
		{
			model_render::draw_nocull_markers();
			model_render::get()->m_drew_model = true;
		}

		bool ignore = false;
		const auto& hmsettings = map_settings::get_map_settings().hide_models;

		for (const auto& hide_mdl_with_radius : hmsettings.radii)
		{
			if (pInfo.pModel->radius == hide_mdl_with_radius)
			{
				ignore = true;
				break;
			}
		}

		if (!ignore && !hmsettings.substrings.empty())
		{
			const auto mdl_string = std::string_view(pInfo.pModel->szPathName);
			for (const auto& hide_mdl_with_substr : hmsettings.substrings)
			{
				if (mdl_string.contains(hide_mdl_with_substr))
				{
					ignore = true;
					break;
				}
			}
		}

		if (!ignore)
		{
			// draw the model
			tbl_hk::model_renderer::table.original<FN>(index)(ecx, edx, /*oo,*/ state, pInfo, pCustomBoneToWorld);

			if (cmd::model_info_vis)
			{
				if (game::get_current_view_origin()->DistToSqr(pInfo.origin) < 1000.0f * 1000.0f)
				{
					game::debug_add_text_overlay(&pInfo.origin.x, pInfo.pModel->szPathName, 0);
					game::debug_add_text_overlay(&pInfo.origin.x, utils::va("Radius: %.7f", pInfo.pModel->radius), 1);
					game::debug_add_text_overlay(&pInfo.origin.x, utils::va("Origin: %.7f %.7f %.7f", pInfo.origin.x, pInfo.origin.y, pInfo.origin.z), 2);
				}
			}
		}
		else
		{
			if (cmd::model_info_vis)
			{
				if (game::get_current_view_origin()->DistToSqr(pInfo.origin) < 1000.0f * 1000.0f)
				{
					game::debug_add_text_overlay(&pInfo.origin.x, "#IGNORED#", 0, 1.0f, 0.6f, 0.6f, 0.6f);
					game::debug_add_text_overlay(&pInfo.origin.x, pInfo.pModel->szPathName, 1, 1.0f, 0.6f, 0.6f, 0.6f);
					game::debug_add_text_overlay(&pInfo.origin.x, utils::va("Radius: %.7f", pInfo.pModel->radius), 2, 1.0f, 0.6f, 0.6f, 0.6f);
				}
			}
		}
	}

	// 
	// main render path for every surface

	void cmeshdx8_renderpass_pre_draw(CMeshDX8* mesh, [[maybe_unused]] CPrimList* primlist)
	{
		const auto dev = game::get_d3d_device();

		IDirect3DVertexBuffer9* b = nullptr;
		UINT stride = 0;
		{
			UINT ofs = 0; dev->GetStreamSource(0, &b, &ofs, &stride);
		}

		//DWORD bufferedstateaddr = RENDERER_BASE + 0x19530;
		//auto x = reinterpret_cast<components::IShaderAPIDX8*>(*(DWORD*)(RENDERER_BASE + 0xC9C50));
		//auto y = reinterpret_cast<components::IShaderAPIDX8*>((RENDERER_BASE + 0xC9C54));

		auto& ctx = model_render::primctx;
		const auto shaderapi = game::get_shaderapi();

		if (ctx.get_info_for_pass(shaderapi)) 
		{
			// added format check
			if (mesh->m_VertexFormat == 0x2480033 || mesh->m_VertexFormat == 0x80033) 
			{
				if (ctx.info.shader_name.starts_with("Wa") && ctx.info.shader_name.contains("Water"))
				{
					IMaterialVar* var = nullptr;
					if (has_materialvar(ctx.info.material, "$basetexture", &var))
					{
						// if material has NO defined basetexture
						if (var && !var->vftable->IsDefined(var))
						{
							// check if it has a defined bottommaterial
							var = nullptr;
							const auto has_bottom_mat = has_materialvar(ctx.info.material, "$bottommaterial", &var);

							if (!has_bottom_mat)
							{
								// do not render water surfaces that have no bottom material (this is the surface below the water)
								// could just check $abovewater I guess? lmao

								// we only need one surface
								ctx.modifiers.do_not_render = true;
							}

							// put the normalmap into texture slot 0
							else
							{
								//  BindTexture( SHADER_SAMPLER2, TEXTURE_BINDFLAGS_NONE, NORMALMAP, BUMPFRAME );
								IDirect3DBaseTexture9* tex = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[2]);
								if (tex)
								{
									// save og texture
									ctx.modifiers.as_water = true;
									ctx.save_texture(dev, 0);
									dev->SetTexture(0, tex);
								}
							}
						}

						// material has defined a $basetexture
						else
						{
							//  sampler 10
							IDirect3DBaseTexture9* tex = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[10]);
							if (tex)
							{
								// save og texture
								ctx.modifiers.as_water = true;
								ctx.save_texture(dev, 0);
								dev->SetTexture(0, tex);
							}
						}
					}
				}
			}
		}

//#if ENABLE_3D_SKY
		if (imgui::get()->m_enable_3dsky)
		{
			// needed for 3d skybox
			dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
			dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);
		}
//#endif


		// hack for runtime hack: https://github.com/xoxor4d/dxvk-remix/commit/3867843a68db7ec8a5ab603a250689cca1505970
		/*if (static bool runtime_hack_once = false; !runtime_hack_once)
		{
			runtime_hack_once = true;
			set_remix_emissive_intensity(dev, ctx, 0.0f);
		}*/

		// shader: VertexLitGeneric (infected - player model - viewmodel - dynamic props)
		// > models/weapons/melee/crowbar
		// > models/props_junk/wood_palletcrate001a
		if (mesh->m_VertexFormat == 0xa0003)
		{
			//ctx.modifiers.do_not_render = true;

			// viewmodel
			if (ctx.info.buffer_state.m_Transform[2].m[3][2] == -1.00003338f)
			{
				ctx.save_view_transform(dev);
				ctx.save_projection_transform(dev);
				dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
				dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);
			}
			//else if (ctx.info.material_name.contains("models/props_destruction/glass_")) 
			//{
			//	//ctx.modifiers.do_not_render = true;
			//	if (tex_addons::glass_shards)
			//	{
			//		ctx.save_texture(dev, 0);
			//		dev->SetTexture(0, tex_addons::glass_shards);
			//	}
			//}
			// models/player/chell/gambler_eyeball_ l/r
			else if (ctx.info.material_name.contains("_eyeball_"))
			{
				if (const auto basemap2 = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[1]); basemap2) 
				{
					ctx.save_texture(dev, 0);
					dev->SetTexture(0, basemap2);
				}
			}

			if (ctx.info.shader_name == "Infected")
			{
				//ctx.modifiers.do_not_render = true;

				// gradient
				if (const auto tex = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[5]); tex)
				{
					ctx.save_texture(dev, 1);
					dev->SetTexture(1, tex);
				}

				// detail
				if (const auto tex = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[4]); tex)
				{
					//ctx.save_texture(dev, 0);
					dev->SetTexture(2, tex);
				}

				

				ctx.save_rs(dev, (D3DRENDERSTATETYPE)149);
				dev->SetRenderState((D3DRENDERSTATETYPE)149, INFECTED);

				float uv_transform[4] = {}; // xy = sprite, zw = gradient z:skin - w:cloth
				dev->GetPixelShaderConstantF(10, uv_transform, 1); // g_vGradSelect

				float grad_select[4] = {};
				dev->GetPixelShaderConstantF(3, grad_select, 1); // g_vGradSelect

				float blood_color[4] = {};
				dev->GetPixelShaderConstantF(4, blood_color, 1); // g_cBloodColor_WaterFogOORange


				// skin tint
				int skin_index = (int)(16.0f * uv_transform[2]); // 0-7

				// cloth tint
				int cloth_index = (int)(16.0f * uv_transform[3]);

				// 8-15 -> bring to 0-7 range
				if (cloth_index >= 8) {
					cloth_index -= 8;
				}

				// pack into single RS
				// 0/1/2..7: skin --- 00/10/20...70: cloth
				ctx.save_rs(dev, (D3DRENDERSTATETYPE)196);
				dev->SetRenderState((D3DRENDERSTATETYPE)196, skin_index + (cloth_index * 10));


				// g_vGradSelect - pack two floats into one RS
				ctx.save_rs(dev, (D3DRENDERSTATETYPE)197);
				dev->SetRenderState((D3DRENDERSTATETYPE)197, utils::pack_2f_in_dword(grad_select[0], grad_select[1]));


				// sprite index - pack two floats into one RS
				ctx.save_rs(dev, (D3DRENDERSTATETYPE)177);
				dev->SetRenderState((D3DRENDERSTATETYPE)177, utils::pack_2f_in_dword(uv_transform[0], uv_transform[1]));

				float roughness_boost = 1.0f; // less = more reflections
				float normal_boost = 3.0f;
				if (ctx.info.material_name.contains("head")) 
				{
					normal_boost = 6.0f;
					roughness_boost = 0.9f;
				}

				
				if (ctx.info.material_name.contains("wet")) {
					roughness_boost = 0.2f;
				} else if (ctx.info.material_name.contains("swamp")) {
					roughness_boost = 0.6f;
				}

				// normal boost & roughness boost - pack two floats into one RS
				ctx.save_rs(dev, (D3DRENDERSTATETYPE)169);
				dev->SetRenderState((D3DRENDERSTATETYPE)169, utils::pack_2f_in_dword(normal_boost, roughness_boost));
				

				// $skintintgradient - $colortintgradient
				/*auto parms = ctx.info.material->vftable->GetShaderParams(ctx.info.material);
				const auto count = ctx.info.material->vftable->ShaderParamCount(ctx.info.material);

				for (auto i = 0u; i < count; i++)
				{
					auto p = parms[i];
					const auto str = p->vftable->GetName(p);
					const auto vint = p->vftable->GetIntValueInternal(p);
					const auto vfloat = p->vftable->GetFloatValueInternal(p);
					const auto vvec = p->vftable->GetVecValueInternal1(p);
					int xxx = 1;
				}*/
			}

			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX6);
		}

		// shader: VertexLitGeneric
		// > models/props_downtown/pipes_rooftop
		// > models/props_foliage/urban_trees_branches03_small
		else if (mesh->m_VertexFormat == 0xa0103)
		{
			//lookat_vertex_decl(dev);

			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX3);
		}

		// shader: LightmappedGeneric (world geo)
		// > plaster/plaster_ext_20c
		// > concrete/concrete_floor_10
		else if (mesh->m_VertexFormat == 0x480003)
		{
			//ctx.modifiers.do_not_render = true;
			ctx.save_vs(dev); 
			dev->SetVertexShader(nullptr);
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2);
			dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
		}

		// shader: screenspace_general_dx9, Engine_Post_dx9, Sky_HDR_DX9, DecalModulate_dx9 ..
		// > dev/lumcompare
		// > skybox/sky_l4d_c1_1_hdrrt
		// > decals/blood3_subrect
		else if (mesh->m_VertexFormat == 0x80001)
		{
			//ctx.modifiers.do_not_render = true;

			// FIRST "UI/HUD" elem (remix injection triggers here)
			// -> fullscreen color transitions (damage etc.) and also "enables" the crosshair
			// -> takes ~ 0.8ms on a debug build

			//ctx.save_view_transform(dev);
			//ctx.save_projection_transform(dev);
			//
			/*dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
			dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);*/

			/*if (!ctx.info.shader_name.contains("Sky"))
			{
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
				dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
				dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);
			}*/

			if (ctx.info.shader_name.starts_with("DecalMod"))
			{
				//lookat_vertex_decl(dev);
				ctx.save_vs(dev);
				dev->SetVertexShader(nullptr);
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			}

			else if (ctx.info.shader_name.starts_with("Engine_")) // Engine_Post
			{
				// do not fog HUD elements :D
				dev->SetRenderState(D3DRS_FOGENABLE, FALSE);

				const auto s_viewFadeColor = reinterpret_cast<Vector4D*>(CLIENT_BASE + 0x7A3D68);

				ctx.save_vs(dev);
				dev->SetVertexShader(nullptr);
				dev->SetPixelShader(nullptr); // needed

				ctx.save_texture(dev, 0);
				dev->SetTexture(0, nullptr); // disable bound texture

				ctx.save_rs(dev, D3DRS_ALPHABLENDENABLE);
				dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

				ctx.save_rs(dev, D3DRS_BLENDOP);
				dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

				ctx.save_rs(dev, D3DRS_SRCBLEND);
				dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);

				ctx.save_rs(dev, D3DRS_DESTBLEND);
				dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

				ctx.save_rs(dev, D3DRS_ZWRITEENABLE);
				dev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

				ctx.save_rs(dev, D3DRS_ZENABLE);
				dev->SetRenderState(D3DRS_ZENABLE, FALSE);

				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
				dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
				dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);

				struct CUSTOMVERTEX
				{
					float x, y, z, rhw;
					D3DCOLOR color;
				};

				auto color = D3DCOLOR_COLORVALUE(s_viewFadeColor->x, s_viewFadeColor->y, s_viewFadeColor->z, s_viewFadeColor->w);
				const auto w = (float)ctx.info.buffer_state.m_Viewport.Width + 0.5f;
				const auto h = (float)ctx.info.buffer_state.m_Viewport.Height + 0.5f;

				CUSTOMVERTEX vertices[] =
				{
					{ -0.5f, -0.5f, 0.0f, 1.0f, color }, // tl
					{     w, -0.5f, 0.0f, 1.0f, color }, // tr
					{ -0.5f,     h, 0.0f, 1.0f, color }, // bl
					{     w,     h, 0.0f, 1.0f, color }  // br
				};

				dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
				dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(CUSTOMVERTEX));

				// do not render the original mesh
				ctx.modifiers.do_not_render = true;

				//model_render::get()->m_drew_hud = true;
			}
			else if (  ctx.info.material_name.starts_with("engine/occl")
					|| ctx.info.material_name.starts_with("dev/lumc")) // dev/lumcompare 
			{
				ctx.modifiers.do_not_render = false;
			}

			// outline related
			else if (ctx.info.material_name.starts_with("dev/glow_"))
			{
				ctx.modifiers.do_not_render = true;

				/*ctx.save_vs(dev);
				dev->SetVertexShader(nullptr);
				lookat_vertex_decl(dev);
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
				dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
				dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);*/
			}

			// adds object outlines
			else if (ctx.info.material_name == "dev/halo_add_to_screen")
			{
				//lookat_vertex_decl(dev);
				ctx.modifiers.do_not_render = true;

				/*ctx.save_vs(dev);
				dev->SetVertexShader(nullptr);
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
				dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
				dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);*/
			}
			else if (ctx.info.shader_name.contains("Sky"))
			{
				ctx.save_rs(dev, D3DRS_FOGENABLE);
				dev->SetRenderState(D3DRS_FOGENABLE, FALSE);

				// this fixes the sky on intros or when no vgui is being drawn
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
				ctx.modifiers.do_not_render = false;
			}
			//ctx.modifiers.do_not_render = true;

			// sky ff "works" but visuals are messed up
			// setting view/proj here would render 3d sky?
			//else
			//{
			//	//int x = 1;
			//	//ctx.save_vs(dev);
			//	//dev->SetVertexShader(nullptr);
			//	//dev->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
			//	dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			//	dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
			//	dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);
			//}

			//lookat_vertex_decl(dev);
		}

		// shader: VertexLitGeneric, UnlitGeneric (stride 0x20)
		// > decals/wood/wood3
		// > decals/metal/metal04
		// > debug/debugtranslucentsinglecolor
		else if (mesh->m_VertexFormat == 0x80003)
		{
			// TODO - HACK: see r_DispWalkable note in main_module
			if (ctx.info.material_name.starts_with("debug/")) {
				ctx.modifiers.do_not_render = true;
			}
			//ctx.modifiers.do_not_render = true;
			lookat_vertex_decl(dev); 
		}

		// shader: UnlitGeneric (stride 0x30)
		// > __fontpage_additive, vgui/hud/scalablepanel_bgmidgrey_outlinegreen_glow
		// > vgui/hud/scalablepanel_bgmidgrey_outlinegreen_glow
		else if (mesh->m_VertexFormat == 0x80007)
		{
			//ctx.modifiers.do_not_render = true;

			// always render UI and world ui with high gamma
			ctx.modifiers.with_high_gamma = true;

			/*if (ctx.info.buffer_state.m_Transform[2].m[3][3] == 1.0f && 
				(ctx.info.material_name == "vgui_white" || ctx.info.material_name == "__fontpage"))
			{
				set_remix_texture_categories(dev, ctx, REMIXAPI_INSTANCE_CATEGORY_BIT_WORLD_MATTE);
				model_render::get()->m_drew_hud = true;
			}*/

			// early out if vgui_white
			if (ctx.info.buffer_state.m_Transform[0].m[3][0] != 0.0f && ctx.info.material_name != "vgui_white")
			{
				bool is_world_ui_text = ctx.info.buffer_state.m_Transform[0].m[3][0] != 0.0f && ctx.info.material_name == "__fontpage";

				if (is_world_ui_text) {
					set_remix_texture_categories(dev, ctx, WorldUI);
				}
				else if (is_world_ui_text)
				{
					//lookat_vertex_decl(dev, primlist);
					float vcol_r = 0.0f;
					float vcol_g = 0.0f;
					float vcol_b = 0.0f;
					float vcol_a = 1.0f;

					// cant get vertex color to work here? -> grab vertex color and use tfactor instead
					//{
					//	IDirect3DVertexBuffer9* vb = nullptr; UINT t_stride = 0u, t_offset = 0u;
					//	dev->GetStreamSource(0, &vb, &t_offset, &t_stride);

					//	IDirect3DIndexBuffer9* ib = nullptr;
					//	if (SUCCEEDED(dev->GetIndices(&ib)))
					//	{
					//		void* ib_data; // retrieve a single vertex index (*2 because WORD)
					//		if (SUCCEEDED(ib->Lock(primlist->m_FirstIndex * 2, 2, &ib_data, D3DLOCK_READONLY)))
					//		{
					//			const auto first_index = *static_cast<std::uint16_t*>(ib_data);
					//			ib->Unlock();

					//			void* src_buffer_data; // retrieve single indexed vertex
					//			if (SUCCEEDED(vb->Lock(first_index * t_stride, t_stride, &src_buffer_data, D3DLOCK_READONLY)))
					//			{
					//				struct src_vert { Vector pos; Vector normal;  D3DCOLOR color; Vector2D tc0; };
					//				const auto src = reinterpret_cast<src_vert*>(((DWORD)src_buffer_data));

					//				// unpack color
					//				vcol_r = static_cast<float>((src->color >> 16) & 0xFF) / 255.0f * 1.0f;
					//				vcol_g = static_cast<float>((src->color >> 8) & 0xFF) / 255.0f * 1.0f;
					//				vcol_b = static_cast<float>((src->color >> 0) & 0xFF) / 255.0f * 1.0f;
					//				vcol_a = static_cast<float>((src->color >> 24) & 0xFF) / 255.0f * 1.0f;
					//				vb->Unlock();
					//			}
					//		}
					//	}
					//}

					ctx.save_vs(dev);
					dev->SetVertexShader(nullptr);
					dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);
					dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);

					ctx.save_tss(dev, D3DTSS_COLORARG1);
					ctx.save_tss(dev, D3DTSS_COLORARG2);
					ctx.save_tss(dev, D3DTSS_COLOROP);
					ctx.save_tss(dev, D3DTSS_ALPHAARG2);
					ctx.save_tss(dev, D3DTSS_ALPHAOP);

					dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
					dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
					dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
					dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

					float scalar = vcol_a;
					if (vcol_a <= 0.5f) {
						scalar = std::powf(vcol_a / 0.5f, 2.0f) * 0.5f; // crush values closer to 0
					}

					ctx.save_rs(dev, D3DRS_TEXTUREFACTOR);
					dev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_COLORVALUE(
						vcol_r* scalar,
						vcol_g* scalar,
						vcol_b* scalar, scalar));
				}

				// some light sprites are rendered as ui through other geo 
				else if (ctx.info.material_name.ends_with("_noz")) {
					ctx.modifiers.do_not_render = true;
				}
			}
		}

		// shader: LightmappedGeneric, WorldVertexTransition_DX9 (terrain decals)
		// > buildings/blend_roof_01
		// > decals/burn02a
		else if (mesh->m_VertexFormat == 0x480007) 
		{
			//ctx.modifiers.do_not_render = true;

			if (ctx.info.shader_name == "WorldVertexTransition_DX9") 
			{
				ctx.save_texture(dev, 0); // helps with culling issue
				ctx.modifiers.dual_render_with_basetexture2 = true;  
			}

			// m_BoundTexture[7]  = first blend colormap
			// m_BoundTexture[12] = second blend colormap

			// if envmap		:: VERTEX_TANGENT_S | VERTEX_TANGENT_T | VERTEX_NORMAL is set
			// if basetex2		:: vertex color is set
			// if bumpmap		:: tc count = 3 ... else 2

			// texcoord0 : base texcoord
			// texcoord1 : lightmap texcoord
			// texcoord2 : lightmap texcoord offset

			//lookat_vertex_decl(dev); 

			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2);
			dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			//dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
			//dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);
		}

		// shader: DecalModulate_dx9
		// > decals/bloodstain_002
		else if (mesh->m_VertexFormat == 0x80005) // stride 0x20
		{
			//ctx.modifiers.do_not_render = true;
			bool mod_shader = true; 

			// render bik using shaders
			/*if (ctx.info.material_name.starts_with("videobik") || ctx.info.material_name.starts_with("media/"))
			{
				set_remix_texture_categories(dev, ctx, DecalStatic);
				set_remix_texture_hash(dev, ctx, utils::string_hash32(ctx.info.material_name));
				mod_shader = false;
			}*/

			if (mod_shader)
			{
				ctx.save_vs(dev);
				dev->SetVertexShader(nullptr);
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2);
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			}
		}

		// shader: Shadow
		// > decals/simpleshadow
		else if (mesh->m_VertexFormat == 0x6c0005)
		{
			ctx.modifiers.do_not_render = true;
			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetFVF(D3DFVF_XYZB1 | D3DFVF_TEX5); // stride 48
		}

		// shader: SplineRope (hanging cables - requires vertex shader - verts not modified on the cpu)
		// > cable/cable
		else if (mesh->m_VertexFormat == 0x24900005)
		{
			//ctx.modifiers.do_not_render = true;
			ctx.save_texture(dev, 0);
			dev->SetTexture(0, tex_addons::black); 
		}

		// shader: Cable_DX9
		// > particle/smoker_tongue_beam
		else if (mesh->m_VertexFormat == 0x480035)
		{
			//ctx.modifiers.do_not_render = true;
			ctx.save_texture(dev, 0);
			dev->SetTexture(0, tex_addons::black);
		}


		// shader: SpriteCard - stride 0x60
		// > particle/smoke1/smoke1
		// > particle/fire_burning_character/fire_burning_character
		// > particle/blood_splatter/bloodsplatter (hud)
		else if (mesh->m_VertexFormat == 0x114900005)
		{
			//ctx.modifiers.do_not_render = true;
			ctx.save_tss(dev, D3DTSS_COLOROP);
			ctx.save_tss(dev, D3DTSS_COLORARG2);
			dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			ctx.save_tss(dev, D3DTSS_ALPHAARG2);
			ctx.save_tss(dev, D3DTSS_ALPHAOP);
			dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE4X);

			dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);

			if (ctx.info.material_name.starts_with("particle/blood_s"))
			{
				dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
				dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);
			}
		}

		// shader: SpriteCard (spark) - stride 0x60
		// > particle/beam_flashlight
		else if (mesh->m_VertexFormat == 0x124900005)
		{
			ctx.modifiers.do_not_render = false;
			//int x = 1;
		}

		// shader: Spritecard (vista smoke) - stride 0x90
		// Client::C_OP_RenderSprites::Render - UV's handled in 'fix_sprite_card_texcoords_mid_hk'
		// > particle/vistasmokev1/vistasmokev4_nearcull
		else if (mesh->m_VertexFormat == 0x24914900005) 
		{
			//ctx.modifiers.do_not_render = true;

			ctx.save_tss(dev, D3DTSS_ALPHAARG2); 
			ctx.save_tss(dev, D3DTSS_ALPHAOP); 
			dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		}

		// Sprite shader
		else if (mesh->m_VertexFormat == 0x914900005) 
		{
			ctx.modifiers.do_not_render = false;
			//int x = 1; 
		}

		// shader: Refract_DX90
		// particle/warp_rain
		else if (mesh->m_VertexFormat == 0x80037)
		{
			ctx.modifiers.do_not_render = false;
		}

		else 
		{
			ctx.modifiers.do_not_render = false;
			//int break_me = 1;  
		}

		//ctx.modifiers.do_not_render = false;
		//int break_me = 1;
	}

	HOOK_RETN_PLACE_DEF(cmeshdx8_renderpass_pre_draw_retn_addr);
	void __declspec(naked) cmeshdx8_renderpass_pre_draw_stub()
	{
		__asm
		{
			pushad;
			push	esi; // CPrimList
			push	ebx; // CMeshDX8
			call	cmeshdx8_renderpass_pre_draw;
			add		esp, 8;
			popad;

			// og code
			mov     eax, [ebx + 0x48];
			cmp     eax, edi;
			jmp		cmeshdx8_renderpass_pre_draw_retn_addr;
		}
	}


	// #

	//void cmeshdx8_renderpass_post_draw(std::uint32_t num_prims_rendered)
	void cmeshdx8_renderpass_post_draw([[maybe_unused]] void* device_ptr, D3DPRIMITIVETYPE type, std::int32_t base_vert_index, std::uint32_t min_vert_index, std::uint32_t num_verts, std::uint32_t start_index, std::uint32_t prim_count)
	{
		const auto dev = game::get_d3d_device();
		const auto shaderapi = game::get_shaderapi();
		auto& ctx = model_render::primctx;

		// 0 = Gamma 1.0 (fixes dark albedo) :: 1 = Gamma 2.2
		dev->SetSamplerState(0, D3DSAMP_SRGBTEXTURE, ctx.modifiers.with_high_gamma ? 1u : 0u);

		// do not render next surface if set
		if (!ctx.modifiers.do_not_render)
		{
			// dirty hack to fix 1 oob point
			//if (ctx.modifiers.as_portalgun_pickup_beam) {
			//	prim_count -= 1;
			//}

			//DWORD og_texfactor = {}, og_colorarg2 = {}, og_colorop = {};
			//if (ctx.modifiers.as_sky)
			//{
			//	// dev->SetRenderState(D3DRS_FOGENABLE, FALSE);

			//	// HACK - as long as sky marking is broken, use WORLD SPACE UI (emissive)
			//	// -> means that we can not use a distant light
			//	// -> this reduces the emissive intensity
			//	dev->GetRenderState(D3DRS_TEXTUREFACTOR, &og_texfactor);
			//	dev->GetTextureStageState(0, D3DTSS_COLORARG2, &og_colorarg2);
			//	dev->GetTextureStageState(0, D3DTSS_COLOROP, &og_colorop);
			//	
			//	dev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(25, 25, 25, 255));
			//	dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			//	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			//}
			//else if (ctx.modifiers.as_transport_beam)
			//{
			//	dev->GetRenderState(D3DRS_TEXTUREFACTOR, &og_texfactor);
			//	dev->GetTextureStageState(0, D3DTSS_ALPHAARG2, &og_colorarg2);
			//	dev->GetTextureStageState(0, D3DTSS_ALPHAOP, &og_colorop);

			//	// slightly increase the alpha so that the 'fog' becomes visible
			//	dev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(0, 0, 0, 40));
			//	dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
			//	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_ADD);
			//}

			dev->DrawIndexedPrimitive(type, base_vert_index, min_vert_index, num_verts, start_index, prim_count);

			// restore emissive sky settings
			/*if (ctx.modifiers.as_sky)
			{
				dev->SetRenderState(D3DRS_TEXTUREFACTOR, og_texfactor);
				dev->SetTextureStageState(0, D3DTSS_COLORARG2, og_colorarg2);
				dev->SetTextureStageState(0, D3DTSS_COLOROP, og_colorop);
			}
			else if (ctx.modifiers.as_transport_beam)
			{
				dev->SetRenderState(D3DRS_TEXTUREFACTOR, og_texfactor);
				dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, og_colorarg2);
				dev->SetTextureStageState(0, D3DTSS_ALPHAOP, og_colorop);
			}*/
		}

		// render the current surface a second time (alpha blended) if set
		// only works with shaders using basemap2 in sampler7
		if (ctx.modifiers.dual_render_with_basetexture2)
		{
			// check if basemap2 is assigned
			if (ctx.info.buffer_state.m_BoundTexture[7])
			{
				// save texture, renderstates and texturestates

				IDirect3DBaseTexture9* og_tex0 = nullptr;
				dev->GetTexture(0, &og_tex0);

				DWORD og_alphablend = {};
				dev->GetRenderState(D3DRS_ALPHABLENDENABLE, &og_alphablend);

				DWORD og_alphaop = {}, og_alphaarg1 = {}, og_alphaarg2 = {};
				dev->GetTextureStageState(0, D3DTSS_ALPHAOP, &og_alphaop);
				dev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &og_alphaarg1);
				dev->GetTextureStageState(0, D3DTSS_ALPHAARG2, &og_alphaarg2);

				DWORD og_colorop = {}, og_colorarg1 = {}, og_colorarg2 = {};
				dev->GetTextureStageState(0, D3DTSS_COLOROP, &og_colorop);
				dev->GetTextureStageState(0, D3DTSS_COLORARG1, &og_colorarg1);
				dev->GetTextureStageState(0, D3DTSS_COLORARG2, &og_colorarg2);

				DWORD og_srcblend = {}, og_destblend = {};
				dev->GetRenderState(D3DRS_SRCBLEND, &og_srcblend);
				dev->GetRenderState(D3DRS_DESTBLEND, &og_destblend);


				// assign basemap2 to textureslot 0
				if (const auto basemap2 = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[7]);
					basemap2)
				{
					dev->SetTexture(0, basemap2);
				}

				// enable blending
				dev->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);

				// picking up / moving a cube affects this and causes flickering on the blended surface
				dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

				// can be used to lighten up the albedo and add a little more alpha
				dev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(0, 0, 0, 30));
				dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
				dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);

				// add a little more alpha
				dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
				dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_ADD);

				//ctx.info.buffer_state.m_Transform[0].m[3][2] += 0.05f;
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);

				// draw second surface 
				dev->DrawIndexedPrimitive(type, base_vert_index, min_vert_index, num_verts, start_index, prim_count);

				// restore texture, renderstates and texturestates
				dev->SetTexture(0, og_tex0);
				dev->SetRenderState(D3DRS_ALPHABLENDENABLE, og_alphablend);
				dev->SetRenderState(D3DRS_SRCBLEND, og_srcblend);
				dev->SetRenderState(D3DRS_DESTBLEND, og_destblend);
				dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, og_alphaarg1);
				dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, og_alphaarg2);
				dev->SetTextureStageState(0, D3DTSS_ALPHAOP, og_alphaop);
				dev->SetTextureStageState(0, D3DTSS_COLORARG1, og_colorarg1);
				dev->SetTextureStageState(0, D3DTSS_COLORARG2, og_colorarg2);
				dev->SetTextureStageState(0, D3DTSS_COLOROP, og_colorop);
			}
		}

		if (ctx.modifiers.dual_render_with_specified_texture)
		{
			// save og texture
			IDirect3DBaseTexture9* og_tex0 = nullptr;
			dev->GetTexture(0, &og_tex0);

			// set new texture
			dev->SetTexture(0, ctx.modifiers.dual_render_texture);

			// BLEND ADD mode
			if (ctx.modifiers.dual_render_with_specified_texture_blend_add)
			{
				ctx.save_rs(dev, D3DRS_ALPHABLENDENABLE);
				dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

				ctx.save_rs(dev, D3DRS_BLENDOP);
				dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

				ctx.save_rs(dev, D3DRS_SRCBLEND);
				dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);

				ctx.save_rs(dev, D3DRS_DESTBLEND);
				dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

				ctx.save_rs(dev, D3DRS_ZWRITEENABLE);
				dev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

				ctx.save_rs(dev, D3DRS_ZENABLE);
				dev->SetRenderState(D3DRS_ZENABLE, FALSE);

				set_remix_texture_categories(dev, ctx, WorldMatte | IgnoreOpacityMicromap);
			}

			// re-draw surface
			dev->DrawIndexedPrimitive(type, base_vert_index, min_vert_index, num_verts, start_index, prim_count);

			// restore texture
			dev->SetTexture(0, og_tex0);
		}

		add_light_to_texture_color_restore();

		// reset prim/pass modifications
		model_render::primctx.restore_all(dev); 
		model_render::primctx.reset_context();
		dev->SetFVF(NULL);
	}

	HOOK_RETN_PLACE_DEF(cmeshdx8_renderpass_post_draw_retn_addr);
	void __declspec(naked) cmeshdx8_renderpass_post_draw_stub()
	{
		__asm
		{
			// og code
			//mov     ecx, [ecx + 0x4C];
			push    esi;
			push    ecx;
			push    eax;
			call	cmeshdx8_renderpass_post_draw; // instead of 'edx' (DrawIndexedPrimitive)
			add		esp, 0x1C;

			jmp		cmeshdx8_renderpass_post_draw_retn_addr;
		}
	}




	/**
	 * Called right before unlocking the sprite mesh. m_nCurrentVertex should match m_nVertexCount
	 */
	void fix_sprite_card_texcoords_mid_hk(CMeshBuilder* builder, [[maybe_unused]] int type)
	{
		const auto dev = game::get_d3d_device();
		auto renderer = game::get_engine_renderer();

		/*const auto mat = game::get_material_system();
		if (mat)
		{
			auto ctx = mat->vtbl->GetRenderContext(mat);

			VMatrix m2w;
			ctx->vtbl->GetMatrix2(ctx, MATERIAL_VIEW, &m2w); 
			auto x = 1; 
		}*/

		const auto eye = renderer->vftable->ViewOrigin(renderer);

		bool use_crop = false;
		bool use_dualsequence = false;

		float startfadesize = 0.0f;
		float endfadesize = 0.0f;


		BufferedState_t buffer_state;
		std::string mat_name;

		
		if (const auto shaderapi = game::get_shaderapi(); shaderapi)
		{
			shaderapi->vtbl->GetBufferedState(shaderapi, nullptr, &buffer_state);

			if (const auto m = shaderapi->vtbl->GetBoundMaterial(shaderapi, nullptr); m) {
				mat_name = m->vftable->GetName(m);
			}

			if (const auto material = shaderapi->vtbl->GetBoundMaterial(shaderapi, nullptr);
				material)
			{
				IMaterialVar* var_out = nullptr;
				if (has_materialvar(material, "$CROPFACTOR", &var_out))
				{
					if (var_out) {
						use_crop = var_out->vftable->GetVecValueInternal1(var_out)[0] != 1.0f || var_out->vftable->GetVecValueInternal1(var_out)[1] != 1.0f;
					}
				}

				if (has_materialvar(material, "$DUALSEQUENCE", &var_out))
				{
					if (var_out) {
						use_dualsequence = var_out->vftable->GetIntValueInternal(var_out) != 0;
					}
				}

				if (has_materialvar(material, "$STARTFADESIZE", &var_out))
				{
					if (var_out) {
						startfadesize = var_out->vftable->GetFloatValueInternal(var_out);
					}
				}

				if (has_materialvar(material, "$ENDFADESIZE", &var_out))
				{
					if (var_out) {
						endfadesize = var_out->vftable->GetFloatValueInternal(var_out);
					}
				}
			}
		}


		bool has_fade = startfadesize <= 1.0f || endfadesize <= 1.0f;
		bool no_near_fade = false;
		bool low_crop = false;
		bool reduce_emissiveness = false;

		bool is_emissive = false;
		{
			DWORD dest_blend;
			dev->GetRenderState(D3DRS_DESTBLEND, &dest_blend);

			if ((D3DBLEND)dest_blend == D3DBLEND_ONE) {
				is_emissive = true;
			}
		}

		/*bool vista_test = false;
		if (mat_name == "particle/vistasmokev1/vistasmokev4_nearcull") {
			vista_test = true;
		}
		else*/
		if (mat_name == "particle/smoke1/smoke1")
		{
			if (!has_fade)
			{
				startfadesize = 3.0f;
				//endfadesize = 5.5f;
				has_fade = true;
				no_near_fade = true;
			}
		}
		else if (mat_name == "particle/fire_burning_character/fire_molotov_crop_low") {
			low_crop = true;
		}
		else if (is_emissive && mat_name.starts_with("particle/vistasmo") || mat_name.starts_with("particle/smoke1/") || mat_name.starts_with("particle/spray1/spray1")) {
			reduce_emissiveness = true;
		}

		float g_vCropFactor[4] = {};
		dev->GetVertexShaderConstantF(15, g_vCropFactor, 1);

		float SizeParms[4] = {};
		dev->GetVertexShaderConstantF(56, SizeParms, 1);

		float SizeParms2[4] = {};
		dev->GetVertexShaderConstantF(57, SizeParms2, 1);

		// meshBuilder positions are set to the last vertex it created
		for (auto v = builder->m_VertexBuilder.m_nVertexCount; v > 0; v--)
		{
			const auto v_pos_in_src_buffer = v * builder->m_VertexBuilder.m_VertexSize_Position; 

			const auto src_vPos = reinterpret_cast<Vector*>(((DWORD)builder->m_VertexBuilder.m_pCurrPosition - v_pos_in_src_buffer));
			const auto src_vTint = reinterpret_cast<D3DCOLOR*>(((DWORD)builder->m_VertexBuilder.m_pCurrColor - v_pos_in_src_buffer));

			const auto src_tc0 = reinterpret_cast<Vector4D*>(((DWORD)builder->m_VertexBuilder.m_pCurrTexCoord[0] - v_pos_in_src_buffer));
			const auto dest_tc = reinterpret_cast<Vector2D*>(src_tc0);

			const auto src_tc1 = reinterpret_cast<Vector4D*>(((DWORD)builder->m_VertexBuilder.m_pCurrTexCoord[1] - v_pos_in_src_buffer));
			const auto src_tc2 = reinterpret_cast<Vector4D*>(((DWORD)builder->m_VertexBuilder.m_pCurrTexCoord[2] - v_pos_in_src_buffer));
			const auto src_tc3 = reinterpret_cast<Vector4D*>(((DWORD)builder->m_VertexBuilder.m_pCurrTexCoord[3] - v_pos_in_src_buffer));
			//const auto src_tc4 = reinterpret_cast<Vector4D*>(((DWORD)builder->m_VertexBuilder.m_pCurrTexCoord[4] - v_pos_in_src_buffer));
			const auto src_tc5 = reinterpret_cast<Vector4D*>(((DWORD)builder->m_VertexBuilder.m_pCurrTexCoord[5] - v_pos_in_src_buffer));
			const auto src_tc6 = reinterpret_cast<Vector4D*>(((DWORD)builder->m_VertexBuilder.m_pCurrTexCoord[6] - v_pos_in_src_buffer));
			const auto src_tc7 = reinterpret_cast<Vector4D*>(((DWORD)builder->m_VertexBuilder.m_pCurrTexCoord[7] - v_pos_in_src_buffer));

			if (use_crop)
			{
				if (low_crop) // g_vCropFactor is failing sometimes? (c1m1_hotel)
				{
					dest_tc->x = std::lerp(src_tc1->z, src_tc1->x, src_tc3->x * 0.5f + 0.25f /*g_vCropFactor[0] + g_vCropFactor[2]*/);
					dest_tc->y = std::lerp(src_tc1->w, src_tc1->y, src_tc3->y * 1.0f /*g_vCropFactor[1] + g_vCropFactor[3]*/);
				}
				else
				{
					dest_tc->x = std::lerp(src_tc0->z, src_tc0->x, src_tc3->x * g_vCropFactor[0] + g_vCropFactor[2]);
					dest_tc->y = std::lerp(src_tc0->w, src_tc0->y, src_tc3->y * g_vCropFactor[1] + g_vCropFactor[3]);
				}
			}
			else
			{
				dest_tc->x = std::lerp(src_tc0->z, src_tc0->x, src_tc3->x);
				dest_tc->y = std::lerp(src_tc0->w, src_tc0->y, src_tc3->y);
			}

			if (use_dualsequence)
			{
#if 0
				Vector2D lerpold = { src_tc3->x, src_tc3->y };
				Vector2D lerpnew = { src_tc3->x, src_tc3->y };

				if (bZoomSeq2)
				{
					lerpold.x = getlerpscaled(src_tc3->x, OLDFRM_SCALE_START, OLDFRM_SCALE_END, src_tc7->x);
					lerpold.y = getlerpscaled(src_tc3->y, OLDFRM_SCALE_START, OLDFRM_SCALE_END, src_tc7->x);
					lerpnew.x = getlerpscaled(src_tc3->x, 1.0f, OLDFRM_SCALE_START, src_tc7->x);
					lerpnew.y = getlerpscaled(src_tc3->y, 1.0f, OLDFRM_SCALE_START, src_tc7->x);
				}

				// src->tc7.x = blendfactor between tc5 lerpold and tc6 lerpnew
				if (src_tc7->x < 0.5f)
				{
					dest_tc->x = std::lerp(src_tc5->z, src_tc5->x, lerpold.x);
					dest_tc->y = std::lerp(src_tc5->w, src_tc5->y, lerpold.y);
				}
				else
				{
					dest_tc->x = std::lerp(src_tc6->z, src_tc6->x, lerpnew.x);
					dest_tc->y = std::lerp(src_tc6->w, src_tc6->y, lerpnew.y);
				}
#else
				if (src_tc7->x < 1.0f)
				{
					dest_tc->x = std::lerp(src_tc5->z, src_tc5->x, src_tc3->x);
					dest_tc->y = std::lerp(src_tc5->w, src_tc5->y, src_tc3->y);
				}
				else
				{
					dest_tc->x = std::lerp(src_tc6->z, src_tc6->x, src_tc3->x);
					dest_tc->y = std::lerp(src_tc6->w, src_tc6->y, src_tc3->y);
				}
#endif
			}

			if (has_fade)
			{
				float r = static_cast<float>((*src_vTint >> 16) & 0xFF) / 255.0f * 1.0f;
				float g = static_cast<float>((*src_vTint >> 8) & 0xFF) / 255.0f * 1.0f;
				float b = static_cast<float>((*src_vTint >> 0) & 0xFF) / 255.0f * 1.0f;
				float a = static_cast<float>((*src_vTint >> 24) & 0xFF) / 255.0f * 1.0f;

				auto l = (*src_vPos - *eye).Length();
				const float normalized_size = src_tc2->z / l;
				float near_fade_factor = no_near_fade ? 0.0f : std::clamp<float>((normalized_size - startfadesize) / (endfadesize - startfadesize), 0.0f, 1.0f);

				float far_fade_factor = std::clamp<float>(((startfadesize - normalized_size) / startfadesize), 0.0f, 1.0f);
				float fade_factor = std::max<float>(near_fade_factor, far_fade_factor);

				Vector4D tint = { r, g, b, a };
				tint = tint * (1.0f - fade_factor); // Fades from 1 (visible) to 0 (invisible)

				if (reduce_emissiveness) {
					tint.w *= 0.25f;
				}

#if 0
				const float MINIMUM_SIZE_FACTOR = SizeParms[0];
				const float MAXIMUM_SIZE_FACTOR = SizeParms[1];
				const float START_FADE_SIZE_FACTOR = SizeParms[2];
				const float END_FADE_SIZE_FACTOR = SizeParms[3];

				const float START_FAR_FADE = SizeParms2[0];
				const float FAR_FADE_FACTOR = SizeParms2[1];

				const float RADIUS = /*use_dualsequence ? src_tc7->z :*/ src_tc2->z;
				//const float RADIUS = 30000.0f;

				float rad = RADIUS; // RADIUS
				auto l = (*src_vPos - *eye).Length();
				rad = std::max<float>(rad, MINIMUM_SIZE_FACTOR * l);

				Vector4D tint = { r, g, b, a }; 

				// now, perform fade out
				if (rad > START_FADE_SIZE_FACTOR * l)
				{
					if (rad > END_FADE_SIZE_FACTOR * l)
					{
						tint = { 0.0f, 0.0f, 0.0f, 0.0f };
						rad = 0;											// cull so we emit 0-sized sprite
					}
					else
					{
						const float t = 1.0f - (rad - START_FADE_SIZE_FACTOR * l) / (END_FADE_SIZE_FACTOR * l - START_FADE_SIZE_FACTOR * l);
						tint = tint * t;
					}
				}

				// perform far fade
				float ttt = (l - START_FAR_FADE) * FAR_FADE_FACTOR;
				float tscale = 1.0f - std::min<float>(1.0f, std::max<float>(0.0f, ttt));
				tint = tint * tscale;

				if (tscale <= 0) {
					rad = 0; // cull so we emit 0-sized sprite
				}

				rad = std::min<float>(rad, MAXIMUM_SIZE_FACTOR * l); 

				/*if (l > rad / 2)
				{
					if (l < rad * 2)
					{
						tint = tint * std::lerp(rad / 2.0f, rad, l);
					}
				}*/
#endif

				*src_vTint = D3DCOLOR_COLORVALUE(tint.x, tint.y, tint.z, tint.w);
			}

			
		}
	}

	HOOK_RETN_PLACE_DEF(RenderSpriteCardNew_retn_addr);
	void __declspec(naked) RenderSpriteCardNew_stub()
	{
		__asm
		{
			pushad;
			push	0;
			lea     eax, [ebp - 0x238];
			push	eax; // builder
			call	fix_sprite_card_texcoords_mid_hk;
			add		esp, 8;
			popad;

			// og
			mov     ecx, [ebp - 0x184];
			jmp		RenderSpriteCardNew_retn_addr;
		}
	}

	// #
	// Commands

	ConCommand xo_debug_toggle_model_info_cmd{};
	void xo_debug_toggle_model_info_fn()
	{
		cmd::model_info_vis = !cmd::model_info_vis;
	}

	// #
	// #

	model_render::model_render()
	{
		p_this = this;

		tbl_hk::model_renderer::_interface = utils::module_interface.get<tbl_hk::model_renderer::IVModelRender*>("engine.dll", "VEngineModel016");
		XASSERT(tbl_hk::model_renderer::table.init(tbl_hk::model_renderer::_interface) == false);
		XASSERT(tbl_hk::model_renderer::table.hook(&tbl_hk::model_renderer::DrawModelExecute::Detour, tbl_hk::model_renderer::DrawModelExecute::index) == false);

		// init addon textures
		init_texture_addons();

		utils::hook(RENDERER_BASE + 0xBEFA, cmeshdx8_renderpass_pre_draw_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(cmeshdx8_renderpass_pre_draw_retn_addr, RENDERER_BASE + 0xBEFF);

		utils::hook(RENDERER_BASE + 0xC05A, cmeshdx8_renderpass_post_draw_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(cmeshdx8_renderpass_post_draw_retn_addr, RENDERER_BASE + 0xC0E1);

		// C_OP_RenderSprites::Render :: fix SpriteCard UV's
		utils::hook::nop(CLIENT_BASE + 0x3C9F1F, 6);
		utils::hook(CLIENT_BASE + 0x3C9F1F, RenderSpriteCardNew_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(RenderSpriteCardNew_retn_addr, CLIENT_BASE + 0x3C9F25);

		// #
		// commands

		game::con_add_command(&xo_debug_toggle_model_info_cmd, "xo_debug_toggle_model_info", xo_debug_toggle_model_info_fn, "Toggle model name and radius visualizations");
	}
}

