#include "std_include.hpp"

// surface dual render (displacement with blending) gets invisible when skinned objects are moved?
// eg terrain gets invisible if cube is picked up?

namespace components
{
	namespace tex_addons
	{
		LPDIRECT3DTEXTURE9 glass_shards;
		LPDIRECT3DTEXTURE9 black_shader;
		LPDIRECT3DTEXTURE9 white;
	}

	void model_render::init_texture_addons(bool release)
	{
		if (release)
		{
			if (tex_addons::glass_shards) tex_addons::glass_shards->Release();
			if (tex_addons::black_shader) tex_addons::black_shader->Release();
			if (tex_addons::white) tex_addons::white->Release();
			return;
		}

		const auto dev = game::get_d3d_device();
		D3DXCreateTextureFromFileA(dev, "portal2-rtx\\textures\\glass_shards.png", &tex_addons::glass_shards);
		D3DXCreateTextureFromFileA(dev, "portal2-rtx\\textures\\black_shader.png", &tex_addons::black_shader);
		D3DXCreateTextureFromFileA(dev, "portal2-rtx\\textures\\white.dds", &tex_addons::white);
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

	// uses unused Renderstate 149 to tweak the emissive intensity of remix legacy materials
	// ~ currently req. runtime changes
	void set_remix_emissive_intensity(IDirect3DDevice9* dev, prim_fvf_context& ctx, float intensity)
	{
		ctx.save_rs(dev, (D3DRENDERSTATETYPE)149);
		dev->SetRenderState((D3DRENDERSTATETYPE)149, *reinterpret_cast<DWORD*>(&intensity));
	}

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

		DWORD bufferedstateaddr = RENDERER_BASE + 0x19530;
		auto x = reinterpret_cast<components::IShaderAPIDX8*>(*(DWORD*)(RENDERER_BASE + 0xC9C50));
		auto y = reinterpret_cast<components::IShaderAPIDX8*>((RENDERER_BASE + 0xC9C54));

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


		// hack for runtime hack: https://github.com/xoxor4d/dxvk-remix/commit/3867843a68db7ec8a5ab603a250689cca1505970
		// TODO: fix this asap
		if (static bool runtime_hack_once = false; !runtime_hack_once)
		{
			runtime_hack_once = true;
			set_remix_emissive_intensity(dev, ctx, 0.0f);
		}

		// player model - gun - grabable stuff
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
			else if (ctx.info.material_name.contains("models/props_destruction/glass_")) 
			{
				//ctx.modifiers.do_not_render = true;
				if (tex_addons::glass_shards)
				{
					ctx.save_texture(dev, 0);
					dev->SetTexture(0, tex_addons::glass_shards);
				}
			}
			// models/player/chell/gambler_eyeball_ l/r
			else if (ctx.info.material_name.contains("_eyeball_"))
			{
				if (const auto basemap2 = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[1]); basemap2) 
				{
					ctx.save_texture(dev, 0);
					dev->SetTexture(0, basemap2);
				}
			}

			//if (ctx.info.material_name.contains("models/infected/"))
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

				//if (ctx.info.shader_name == "Infected")
				{
					float enable = 1.0f;
					ctx.save_rs(dev, (D3DRENDERSTATETYPE)149);
					dev->SetRenderState((D3DRENDERSTATETYPE)149, *reinterpret_cast<DWORD*>(&enable));

					float uv_transform[4] = {}; // xy = sprite, zw = gradient z:skin - w:cloth
					dev->GetPixelShaderConstantF(10, uv_transform, 1); // g_vGradSelect

					float grad_select[4] = {};
					dev->GetPixelShaderConstantF(3, grad_select, 1); // g_vGradSelect

					float blood_color[4] = {};
					dev->GetPixelShaderConstantF(4, blood_color, 1); // g_cBloodColor_WaterFogOORange


					// skin tint
					int skin_index = (int)(16.0f * uv_transform[2]); // 0-7
					//ctx.save_rs(dev, (D3DRENDERSTATETYPE)196);
					//dev->SetRenderState((D3DRENDERSTATETYPE)196, skin_index);

					// cloth tint
					int cloth_index = (int)(16.0f * uv_transform[3]);

					// 8-15 -> bring to 0-7 range
					if (cloth_index >= 8) {
						cloth_index -= 8;
					}

					//ctx.save_rs(dev, (D3DRENDERSTATETYPE)197);
					//dev->SetRenderState((D3DRENDERSTATETYPE)197, cloth_index);

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

				}

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

				//float enable = 1.0f;

				//ctx.save_rs(dev, (D3DRENDERSTATETYPE)149);
				//dev->SetRenderState((D3DRENDERSTATETYPE)149, *reinterpret_cast<DWORD*>(&enable));


				//IMaterialVar* var_out = nullptr;
				//if (has_materialvar(ctx.info.material, "$skintintgradient", &var_out))
				//{
				//	if (var_out) 
				//	{
				//		const auto varstr = var_out->vftable->GetName(var_out);
				//		int var = var_out->vftable->GetIntValueInternal(var_out);
				//		//var = 6;
				//		ctx.save_rs(dev, (D3DRENDERSTATETYPE)196);
				//		dev->SetRenderState((D3DRENDERSTATETYPE)196, var);  
				//	}

				//	

				//}

				//if (has_materialvar(ctx.info.material, "$colortintgradient", &var_out))
				//{
				//	if (var_out)
				//	{
				//		int var = var_out->vftable->GetIntValueInternal(var_out);

				//		if (var >= 8) {
				//			var -= 8;
				//		}

				//		if (var)
				//		{
				//			int x = 1;
				//		}

				//		//var = 2;
				//		ctx.save_rs(dev, (D3DRENDERSTATETYPE)197);
				//		dev->SetRenderState((D3DRENDERSTATETYPE)197, var);
				//	}
				//}

				//if (has_materialvar(ctx.info.material, "$sheetindex", &var_out))
				//{
				//	if (var_out)
				//	{
				//		const int var = var_out->vftable->GetIntValueInternal(var_out);
				//		ctx.save_rs(dev, (D3DRENDERSTATETYPE)177);
				//		dev->SetRenderState((D3DRENDERSTATETYPE)177, var);
				//	}
				//}
			}

			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX6);
		}

		else if (mesh->m_VertexFormat == 0xa0103)
		{
			lookat_vertex_decl(dev);

			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX3);
		}

		// world geo - floor / walls --- "LightmappedGeneric"
		else if (mesh->m_VertexFormat == 0x480003)
		{
			//ctx.modifiers.do_not_render = true;
			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2);
			dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
		}

		else if (mesh->m_VertexFormat == 0x80001)
		{
			//ctx.modifiers.do_not_render = true;

			// FIRST "UI/HUD" elem (remix injection triggers here)
			// -> fullscreen color transitions (damage etc.) and also "enables" the crosshair
			// -> takes ~ 0.8ms on a debug build

			if (ctx.info.shader_name.starts_with("Engine_")) // Engine_Post
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

			// sky ff "works" but visuals are messed up
			// setting view/proj here would render 3d sky?
			else
			{
				int x = 1;
				//ctx.save_vs(dev);
				//dev->SetVertexShader(nullptr);
				//dev->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
				//dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
				//dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
				//dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);
			}

			//lookat_vertex_decl(dev);
		}

		// UnlitGeneric
		else if (mesh->m_VertexFormat == 0x80003 || mesh->m_VertexFormat == 0x80007)
		{
			//ctx.modifiers.do_not_render = true;

			// always render UI and world ui with high gamma
			ctx.modifiers.with_high_gamma = true; 

			// early out if vgui_white
			if (ctx.info.material_name != "vgui_white" && ctx.info.buffer_state.m_Transform[0].m[3][0] != 0.0f)
			{
				bool is_world_ui_text = ctx.info.buffer_state.m_Transform[0].m[3][0] != 0.0f && ctx.info.material_name == "vgui__fontpage";

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

		// Terrain Decals:
		// WorldVertexTransition_DX9
		else if (mesh->m_VertexFormat == 0x480007)
		{
			//ctx.modifiers.do_not_render = true;

			if (ctx.info.shader_name == "WorldVertexTransition_DX9") {
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

		// simpleshadow
		else if (mesh->m_VertexFormat == 0x6c0005)
		{
			ctx.modifiers.do_not_render = true;
			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetFVF(D3DFVF_XYZB1 | D3DFVF_TEX5); // stride 48
		}

		// hanging cables - requires vertex shader - verts not modified on the cpu
		else if (mesh->m_VertexFormat == 0x24900005)
		{
			//ctx.modifiers.do_not_render = true;
			ctx.save_texture(dev, 0);
			dev->SetTexture(0, tex_addons::black_shader);
		}

		// Cable_DX9 - particle/smoker_tongue_beam
		else if (mesh->m_VertexFormat == 0x480035)
		{
			//ctx.modifiers.do_not_render = true;
			int x = 1;
		}


		// SpriteCard shader
		else if (mesh->m_VertexFormat == 0x114900005) // stride 96 
		{
			int x = 1;
		}

		// SpriteCard - spark
		else if (mesh->m_VertexFormat == 0x124900005)
		{
			int x = 1;
		}

		// SpriteCard vista smoke
		// Client::C_OP_RenderSprites::Render - UV's handled in 'fix_sprite_card_texcoords_mid_hk'
		else if (mesh->m_VertexFormat == 0x24914900005)
		{
			int x = 1;
		}

		// Sprite shader
		else if (mesh->m_VertexFormat == 0x914900005)
		{
			int x = 1;
		}

		else
		{
			//ctx.modifiers.do_not_render = true;
			int break_me = 1;  
		}

		int break_me = 1;
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
				//dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				//dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

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


	// #
	// #

	model_render::model_render()
	{
		/* tbl_hk::model_renderer::_interface = utils::module_interface.get<tbl_hk::model_renderer::IVModelRender*>("engine.dll", "VEngineModel016");

		XASSERT(tbl_hk::model_renderer::table.init(tbl_hk::model_renderer::_interface) == false);
		XASSERT(tbl_hk::model_renderer::table.hook(&tbl_hk::model_renderer::DrawModelExecute::Detour, tbl_hk::model_renderer::DrawModelExecute::Index) == false);
 		*/

		utils::hook(RENDERER_BASE + 0xBEFA, cmeshdx8_renderpass_pre_draw_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(cmeshdx8_renderpass_pre_draw_retn_addr, RENDERER_BASE + 0xBEFF);

		utils::hook(RENDERER_BASE + 0xC05A, cmeshdx8_renderpass_post_draw_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(cmeshdx8_renderpass_post_draw_retn_addr, RENDERER_BASE + 0xC0E1);
	}
}

