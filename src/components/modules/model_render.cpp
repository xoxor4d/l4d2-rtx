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

		/*if (ctx.info.material_name.contains("props_foliage"))
		{
			int break_me = 1;    
		}*/

		// hack for runtime hack: https://github.com/xoxor4d/dxvk-remix/commit/3867843a68db7ec8a5ab603a250689cca1505970
		// TODO: fix this asap
		if (static bool runtime_hack_once = false; !runtime_hack_once)
		{
			runtime_hack_once = true;
			set_remix_emissive_intensity(dev, ctx, 0.0f);
		}

		//if (ff_bmodel::s_shader && mesh->m_VertexFormat == 0x2480033)
		//{
		//	//ctx.modifiers.do_not_render = true;
		//	dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
		//	dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX7);
		//	dev->SetVertexShader(nullptr);

		//	if (is_rendering_bmodel_paint) {
		//		render_painted_surface(ctx, primlist);
		//	}
		//}

		// player model - gun - grabable stuff
		if (mesh->m_VertexFormat == 0xa0003)
		{
			//ctx.modifiers.do_not_render = true;

			// viewmodel
			if (ctx.info.buffer_state.m_Transform[2].m[3][2] == -1.00003529f)
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
			/*else if (ctx.info.material_name.contains("models/player/chell/gambler_eyeball"))
			{
				if (const auto basemap2 = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[1]); basemap2) 
				{
					ctx.save_texture(dev, 0);
					dev->SetTexture(0, basemap2);
				}
			}*/

			dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX6);
			dev->SetVertexShader(nullptr); // vertexformat 0x00000000000a0003 
		}

		// this also renders the glass infront of white panel lamps
		// also renders some foliage (2nd level - emissive) - treeleaf // 0xa0103

		// general models (eg. furniture in first lvl - container)
		else if (mesh->m_VertexFormat == 0xa0103)
		{
			// replace all refract shaders with wireframe (ex. glass/containerwindow_)
			if (ctx.info.shader_name.starts_with("Re") && ctx.info.shader_name.contains("Refract_DX90") && !ctx.info.material_name.starts_with("glass/contain"))
			{
				// I think we are simply missing basetex0 here
				ctx.info.material->vftable->SetShader(ctx.info.material, "Wireframe");
			}

			// make sure that alpha testing is set
			else if (ctx.info.material_name.contains("props_foliage"))
			{
				//ctx.modifiers.do_not_render = true;
				ctx.save_rs(dev, D3DRS_ALPHABLENDENABLE);
				ctx.save_rs(dev, D3DRS_SRCBLEND);
				ctx.save_rs(dev, D3DRS_DESTBLEND);
				ctx.save_rs(dev, D3DRS_ALPHATESTENABLE);
				ctx.save_rs(dev, D3DRS_ALPHAFUNC);
				ctx.save_rs(dev, D3DRS_ALPHAREF);

				dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				dev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
				dev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
				dev->SetRenderState(D3DRS_ALPHAREF, 128);
			}

			// glass
			else if (ctx.info.material_name.starts_with("gla"))
			{
				// glass/glasswindow_ ...
				//if (ctx.info.material_name.starts_with("glass/glassw"))
				//{
				//	if (tex_addons::glass_window_lamps)
				//	{
				//		//dev->GetTexture(0, &ff_model::s_texture);
				//		ctx.save_texture(dev, 0);
				//		dev->SetTexture(0, tex_addons::glass_window_lamps);
				//	}
				//}
				// glass/containerwindow_
				//else 
				if (ctx.info.material_name.starts_with("glass/contain"))
				{
					if (const auto basemap2 = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[2]);
						basemap2)
					{
						ctx.save_texture(dev, 0);
						dev->SetTexture(0, basemap2);
					}

					// create a scaling matrix
					D3DXMATRIX scaleMatrix;
					D3DXMatrixScaling(&scaleMatrix, 1.0f, 29.0f, 1.0f);

					ctx.set_texture_transform(dev, &scaleMatrix);
					ctx.save_tss(dev, D3DTSS_TEXTURETRANSFORMFLAGS);
					dev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
				}
			}

			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX3);
			dev->SetVertexShader(nullptr); // vertexformat 0x00000000000a0003
		}

		else
		{
			// world geo - floor / walls --- "LightmappedGeneric"
			// this renders water but not the $bottommaterial
			if (mesh->m_VertexFormat == 0x2480033)
			{
				//ctx.modifiers.do_not_render = true;
				ctx.save_vs(dev);
				dev->SetVertexShader(nullptr);
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX3); // tc @ 24
				dev->SetTransform(D3DTS_WORLD, &game::IDENTITY); 

				// scale water uv's
				//if (ctx.modifiers.as_water)  
				//{
				//	const auto& scale_setting = map_settings::get_map_settings().water_uv_scale;

				//	// create a scaling matrix
				//	D3DXMATRIX scaleMatrix;
				//	D3DXMatrixScaling(&scaleMatrix, 1.5f * scale_setting, 1.5f * scale_setting, 1.0f);

				//	ctx.save_ss(dev, D3DSAMP_ADDRESSU);
				//	ctx.save_ss(dev, D3DSAMP_ADDRESSV);
				//	dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
				//	dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

				//	ctx.set_texture_transform(dev, &scaleMatrix); 
				//	ctx.save_tss(dev, D3DTSS_TEXTURETRANSFORMFLAGS);
				//	dev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
				//}
			} 

			// transport tubes
			// billboard spites and decals
			// main menu bik background
			else if (mesh->m_VertexFormat == 0x80005) // stride 0x20
			{
				int x = 1;
			}

			// laser platforms + DebugTextureView
			// renders a small quad at 0 0 0 ?
			// also rendering transporting beams now?
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
			}

			// lasers - indicator dots - some of the white light stripes
			// portals
			// area portal windows
			// treeleaf shader
			// skybox
			else if (mesh->m_VertexFormat == 0x4a0003 || mesh->m_VertexFormat == 0x80003)
			{
				{
					// unique textures for the white sky so they can be marked
					//if (ctx.info.material_name.contains("sky"))
					//{
					//	if (ctx.info.material_name.contains("_white"))
					//	{
					//		ctx.save_texture(dev, 0);

					//		// sky_whiteft
					//		if (ctx.info.material_name.contains("eft")) {
					//			dev->SetTexture(0, tex_addons::sky_gray_ft);
					//		}
					//		else if (ctx.info.material_name.contains("ebk")) {
					//			dev->SetTexture(0, tex_addons::sky_gray_bk);
					//		}
					//		else if (ctx.info.material_name.contains("elf")) {
					//			dev->SetTexture(0, tex_addons::sky_gray_lf);
					//		}
					//		else if (ctx.info.material_name.contains("ert")) {
					//			dev->SetTexture(0, tex_addons::sky_gray_rt);
					//		}
					//		else if (ctx.info.material_name.contains("eup")) {
					//			dev->SetTexture(0, tex_addons::sky_gray_up);
					//		}
					//		else if (ctx.info.material_name.contains("edn")) {
					//			dev->SetTexture(0, tex_addons::sky_gray_dn);
					//		}

					//		ctx.modifiers.as_sky = true;
					//	}
					//}

					ctx.save_vs(dev);
					dev->SetVertexShader(nullptr);

					// noticed some normal issues on vgui_indicator's .. disable normals for now?
					dev->SetFVF(D3DFVF_XYZB3 /*| D3DFVF_NORMAL*/ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0));
					dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
				}
			}

			// portal_draw_ghosting 0 disables this
			// this normally shows portals through walls
			else if (mesh->m_VertexFormat == 0xa0007) // portal fx  
			{
				//ctx.modifiers.do_not_render = true;
				ctx.save_vs(dev); 
				dev->SetVertexShader(nullptr);
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL | D3DFVF_TEX5 | D3DFVF_TEXCOORDSIZE1(4)); // tc at 16 + 12 :: 68 - 4 as last tc is one float
				dev->SetTransform(D3DTS_WORLD, &game::IDENTITY);
			}

			// related to props like portalgun, pickup-ables
			// does not have a visibile texture set? (setting one manually makes it show up)
			// renders decals/simpleshadow
			else if (mesh->m_VertexFormat == 0x6c0005)
			{
				ctx.modifiers.do_not_render = true;
				ctx.save_vs(dev);
				dev->SetVertexShader(nullptr);
				dev->SetFVF(D3DFVF_XYZB1 | D3DFVF_TEX5); // stride 48
			}

			// engine/shadowbuild
			else if (mesh->m_VertexFormat == 0xa0003)
			{
				//ctx.modifiers.do_not_render = true; // not used anywhere else besides in puzzlemaker?
				//if (game::is_puzzlemaker_active())

				//lookat_vertex_decl(dev, primlist);
				ctx.save_vs(dev);
				dev->SetVertexShader(nullptr); // def. render using FF as the shader is causing heavy frametime drops
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE1(4)); // stride 0x30
			}

			// HUD
			// Video decals
			// rain
			else if (mesh->m_VertexFormat == 0x80007) 
			{
				//ctx.modifiers.do_not_render = true; 
				
				// always render UI and world ui with high gamma
				ctx.modifiers.with_high_gamma = true; 

				// early out if vgui_white
				if (ctx.info.material_name != "vgui_white" && ctx.info.buffer_state.m_Transform[0].m[3][0] != 0.0f)
				{
					// should be fine now that engine_post is the first hud elem
					//if (ctx.info.material_name == "vgui__fontpage")
					//{
					//	// get rid of all world-rendered text as its using the same glyph as HUD elements?!
					//	if (ctx.info.buffer_state.m_Transform[0].m[3][0] != 0.0f) {
					//		ctx.modifiers.do_not_render = true;
					//	}
					//}
					//else

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

					else if (ctx.info.material_name.contains("elevator_video_")) 
					{
						//ctx.modifiers.do_not_render = true;
						ctx.save_vs(dev);
						dev->SetVertexShader(nullptr);
						dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
						//dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1); // no need to set fvf here
					}

					// fix rain by making it slightly emissive
					else if (ctx.info.material_name.ends_with("/rain"))
					{
						ctx.modifiers.with_high_gamma = false;
						ctx.save_rs(dev, D3DRS_DESTBLEND);
						dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

						ctx.save_rs(dev, D3DRS_TEXTUREFACTOR);
						ctx.save_tss(dev, D3DTSS_COLOROP);
						ctx.save_tss(dev, D3DTSS_COLORARG2);

						dev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(100, 100, 100, 255));
						dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
						dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
					}

					// some light sprites are rendered as ui through other geo 
					else if (ctx.info.material_name.ends_with("_noz")) {
						ctx.modifiers.do_not_render = true;
					}

					// fix particles on intro1 after breaking the wall
					else if (ctx.info.material_name.starts_with("particle/"))
					{
						//lookat_vertex_decl(dev, primlist);
						ctx.save_vs(dev);
						dev->SetVertexShader(nullptr);
						dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);
						dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]); 
						dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
						dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);

						ctx.save_tss(dev, D3DTSS_COLOROP);
						ctx.save_tss(dev, D3DTSS_COLORARG2);
						ctx.save_tss(dev, D3DTSS_ALPHAOP);
						ctx.save_tss(dev, D3DTSS_ALPHAARG2);

						dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
						dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
						dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
						dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X);
					}
				}
			}

			// on portal open - spark fx (center)
			// + portal clearing gate (blue sweeping beam)
			// + portal gun pickup effect (beam)
			// + laser emitter swirrl
			// + portal cleanser
			// can be rendered but requires vertexshader + position
#if 1
			else if (mesh->m_VertexFormat == 0x924900005) // stride 0x70 - 112 
			{
				// Spritecard -> Splinecard
				bool is_spline = false; 
			}
#endif

			// terrain - "WorldVertexTransition"
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

				ctx.save_vs(dev);
				dev->SetVertexShader(nullptr);
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX3 | D3DFVF_TEXCOORDSIZE1(2)); // tc @ 28

				// not doing this and picking up a skinned model (eg. cube) will break displacement rendering???
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			}

			// hanging cables - requires vertex shader - verts not modified on the cpu
			else if (mesh->m_VertexFormat == 0x24900005)
			{
				//ctx.modifiers.do_not_render = true; // they can freak out sometimes so just ignore them for now
				ctx.save_texture(dev, 0);
				dev->SetTexture(0, tex_addons::black_shader);
			}

			// SpriteCard shader
			// on portal open
			// portal gun pickup effect (pusling lights (not the beam))
			// other particles like smoke - wakeup scene ring - water splash
#if 1
			else if (mesh->m_VertexFormat == 0x114900005) // stride 96 
			{
				//const auto is_spritecard = ctx.info.material->vftable->IsSpriteCard(ctx.info.material);
				//ctx.modifiers.do_not_render = true;
				bool disable_vertex_color_modulation = false;
			}
#endif
			// on portal open - blob in the middle (impact)
			// water drops on glados wakeup
			else if (mesh->m_VertexFormat == 0x80037) // TODO - test with buffer_state transforms  
			{
				//ctx.modifiers.do_not_render = true; // this needs a position as it spawns on 0 0 0 // stride 0x40
				int break_me = 0;
			}

			// on portal open - outer ring effect
			// transport beam emitter effect
			else if (mesh->m_VertexFormat == 0x1b924900005) // stride 0x90 - 144
			{
				//ctx.modifiers.do_not_render = true;
				int break_me = 0;
				//lookat_vertex_decl(dev, primlist);
			}

			// emancipation grill
			// renders water $bottommaterial
			else if (mesh->m_VertexFormat == 0x80033) //stride = 0x40   
			{
				/*
				float flPowerUp = params[ info.m_nPowerUp ]->GetFloatValue();
				float flIntensity = params[info.m_nFlowColorIntensity]->GetFloatValue();

				bool bActive = (flIntensity > 0.0f);
				if ((bHasFlowmap) && (flPowerUp <= 0.0f))
				{
					bActive = false;
				}*/

				//ctx.modifiers.do_not_render = true;

				//lookat_vertex_decl(dev, primlist);

				if (ctx.info.material_name.starts_with("nature/"))
				{
					ctx.save_vs(dev);
					dev->SetVertexShader(nullptr);
					dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX5); // 64 :: tc @ 24
					dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
				}
			}

			// decals
			// moon surface
			else if (mesh->m_VertexFormat == 0x2480037)  // stride 0x50 - 80 
			{
				//ctx.modifiers.do_not_render = true;
				ctx.save_vs(dev);
				dev->SetVertexShader(nullptr); 
				//dev->SetFVF(D3DFVF_XYZB4 | D3DFVF_TEX7 | D3DFVF_TEXCOORDSIZE1(4)); // 84 - 4 as last tc is one float :: tc at 28
				//lookat_vertex_decl(dev, primlist);
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX7 | D3DFVF_TEXCOORDSIZE1(4));
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			}

			// Sprite shader
			else if (mesh->m_VertexFormat == 0x914900005)
			{
				//ctx.modifiers.do_not_render = true;
				int x = 1;
				//ctx.save_vs(dev);
				//dev->SetVertexShader(nullptr);
				//dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX3); // vertex fmt looks like pos normal 3xtc (float2)

				//dev->SetTransform(D3DTS_WORLD, &buffer_state.m_Transform[0]);
				//dev->SetTransform(D3DTS_VIEW, &buffer_state.m_Transform[1]);
				//dev->SetTransform(D3DTS_PROJECTION, &buffer_state.m_Transform[2]);
			}

			// SpriteCard vista smoke
			// Client::C_OP_RenderSprites::Render - UV's handled in 'fix_sprite_card_texcoords_mid_hk'
			else if (mesh->m_VertexFormat == 0x24914900005)
			{
				ctx.save_tss(dev, D3DTSS_ALPHAARG2);
				ctx.save_tss(dev, D3DTSS_ALPHAOP);
				dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
				dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

				if (ctx.info.material_name == "particle/vistasmokev1_add_nearcull")
				{
					set_remix_emissive_intensity(dev, ctx, 0.01f);
				}
			}
			

			// paint blobs (blob only, not the painted surfs)
			// maybe use r_paintblob_wireframe to force wireframe mode later
			else if (mesh->m_VertexFormat == 0x100003)
			{
				ctx.save_vs(dev);
				dev->SetVertexShader(nullptr);
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX3);
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			}
#ifdef DEBUG
			else
			{
				//ctx.modifiers.do_not_render = true;
				int break_me = 1;  
			}

			int break_me = 1;
#endif
		}
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
		//if (ctx.modifiers.dual_render_with_basetexture2)
		//{
		//	// check if basemap2 is assigned
		//	if (ctx.info.buffer_state.m_BoundTexture[7])
		//	{
		//		// save texture, renderstates and texturestates

		//		IDirect3DBaseTexture9* og_tex0 = nullptr;
		//		dev->GetTexture(0, &og_tex0);

		//		DWORD og_alphablend = {};
		//		dev->GetRenderState(D3DRS_ALPHABLENDENABLE, &og_alphablend);

		//		DWORD og_alphaop = {}, og_alphaarg1 = {}, og_alphaarg2 = {};
		//		dev->GetTextureStageState(0, D3DTSS_ALPHAOP, &og_alphaop);
		//		dev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &og_alphaarg1);
		//		dev->GetTextureStageState(0, D3DTSS_ALPHAARG2, &og_alphaarg2);

		//		DWORD og_colorop = {}, og_colorarg1 = {}, og_colorarg2 = {};
		//		dev->GetTextureStageState(0, D3DTSS_COLOROP, &og_colorop);
		//		dev->GetTextureStageState(0, D3DTSS_COLORARG1, &og_colorarg1);
		//		dev->GetTextureStageState(0, D3DTSS_COLORARG2, &og_colorarg2);

		//		DWORD og_srcblend = {}, og_destblend = {};
		//		dev->GetRenderState(D3DRS_SRCBLEND, &og_srcblend);
		//		dev->GetRenderState(D3DRS_DESTBLEND, &og_destblend);


		//		// assign basemap2 to textureslot 0
		//		if (const auto basemap2 = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[7]);
		//			basemap2)
		//		{
		//			dev->SetTexture(0, basemap2);
		//		}

		//		// enable blending
		//		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);

		//		// picking up / moving a cube affects this and causes flickering on the blended surface
		//		dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		//		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		//		// can be used to lighten up the albedo and add a little more alpha
		//		dev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(0, 0, 0, 30));
		//		dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		//		dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
		//		dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);

		//		// add a little more alpha
		//		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
		//		dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
		//		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_ADD);

		//		//state.m_Transform[0].m[3][2] += 40.0f;
		//		dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);

		//		// draw second surface 
		//		dev->DrawIndexedPrimitive(type, base_vert_index, min_vert_index, num_verts, start_index, prim_count);

		//		// restore texture, renderstates and texturestates
		//		dev->SetTexture(0, og_tex0);
		//		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, og_alphablend);
		//		dev->SetRenderState(D3DRS_SRCBLEND, og_srcblend);
		//		dev->SetRenderState(D3DRS_DESTBLEND, og_destblend);
		//		dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, og_alphaarg1);
		//		dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, og_alphaarg2);
		//		dev->SetTextureStageState(0, D3DTSS_ALPHAOP, og_alphaop);
		//		dev->SetTextureStageState(0, D3DTSS_COLORARG1, og_colorarg1);
		//		dev->SetTextureStageState(0, D3DTSS_COLORARG2, og_colorarg2);
		//		dev->SetTextureStageState(0, D3DTSS_COLOROP, og_colorop);
		//	}
		//}

		//if (ctx.modifiers.dual_render_with_specified_texture)
		//{
		//	// save og texture
		//	IDirect3DBaseTexture9* og_tex0 = nullptr;
		//	dev->GetTexture(0, &og_tex0);

		//	// set new texture
		//	dev->SetTexture(0, ctx.modifiers.dual_render_texture);

		//	// BLEND ADD mode
		//	if (ctx.modifiers.dual_render_with_specified_texture_blend_add)
		//	{
		//		ctx.save_rs(dev, D3DRS_ALPHABLENDENABLE);
		//		dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

		//		ctx.save_rs(dev, D3DRS_BLENDOP);
		//		dev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

		//		ctx.save_rs(dev, D3DRS_SRCBLEND);
		//		dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);

		//		ctx.save_rs(dev, D3DRS_DESTBLEND);
		//		dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

		//		ctx.save_rs(dev, D3DRS_ZWRITEENABLE);
		//		dev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

		//		ctx.save_rs(dev, D3DRS_ZENABLE);
		//		dev->SetRenderState(D3DRS_ZENABLE, FALSE);

		//		set_remix_texture_categories(dev, ctx, WorldMatte | IgnoreOpacityMicromap);
		//	}

		//	// re-draw surface
		//	dev->DrawIndexedPrimitive(type, base_vert_index, min_vert_index, num_verts, start_index, prim_count);

		//	// restore texture
		//	dev->SetTexture(0, og_tex0);
		//}

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

