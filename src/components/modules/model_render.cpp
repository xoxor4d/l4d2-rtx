#include "std_include.hpp"
#include "model_render.hpp"

#include "game_settings.hpp"
#include "imgui.hpp"
#include "main_module.hpp"
#include "map_settings.hpp"
#include "remix_lights.hpp"
#include "remix_markers.hpp"

namespace components
{
	namespace cmd
	{
		bool model_info_vis = false;
		bool unbake_model_info_vis = false;
		std::uint32_t ms_unbake_info = 0u;
		std::unordered_set<std::string> ms_unbake_info_logged_strings;
	}

	namespace tex_addons
	{
		LPDIRECT3DTEXTURE9 glass_shards;
		LPDIRECT3DTEXTURE9 rain_drop;
		LPDIRECT3DTEXTURE9 black;
		LPDIRECT3DTEXTURE9 white;
	}

	std::vector<Vector> g_sunoverlay_color = {};

	void model_render::init_texture_addons(bool release)
	{
		if (release)
		{
			if (tex_addons::glass_shards) tex_addons::glass_shards->Release();
			if (tex_addons::rain_drop) tex_addons::rain_drop->Release();
			if (tex_addons::black) tex_addons::black->Release();
			if (tex_addons::white) tex_addons::white->Release();
			return;
		}

		const auto dev = game::get_d3d_device();
		D3DXCreateTextureFromFileA(dev, COMPMOD_ASSET_DIR "textures\\glass_shards.png", &tex_addons::glass_shards);
		D3DXCreateTextureFromFileA(dev, COMPMOD_ASSET_DIR "textures\\raindrop.png", &tex_addons::rain_drop);
		D3DXCreateTextureFromFileA(dev, COMPMOD_ASSET_DIR "textures\\black.dds", &tex_addons::black);
		D3DXCreateTextureFromFileA(dev, COMPMOD_ASSET_DIR "textures\\white.dds", &tex_addons::white);
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


	// Uses unused Renderstate 149 to set per drawcall modifiers
	// ~ req. runtime changes
	void model_render::set_remix_modifier(IDirect3DDevice9* dev, RemixModifier mod, bool remove_mod)
	{
		primctx.save_rs(dev, RS_149_REMIX_MODIFIER);

		if (remove_mod) {
			primctx.modifiers.remix_modifier &= ~mod;
		} else {
			primctx.modifiers.remix_modifier |= mod;
		}

		dev->SetRenderState((D3DRENDERSTATETYPE)RS_149_REMIX_MODIFIER, static_cast<DWORD>(primctx.modifiers.remix_modifier));
	}

	// uses unused Renderstate 149 (mod) & 169 to tweak the emissive intensity of remix materials (legacy/opaque)
	// ~ currently req. runtime changes
	// ~ req. runtime changes --> remixTempFloat01FromD3D
	void model_render::set_remix_emissive_intensity(IDirect3DDevice9* dev, float intensity)
	{
		set_remix_modifier(dev, RemixModifier::EmissiveScalar);

		primctx.save_rs(dev, RS_169_EMISSIVE_SCALE);
		dev->SetRenderState((D3DRENDERSTATETYPE)RS_169_EMISSIVE_SCALE, *reinterpret_cast<DWORD*>(&intensity));
	}

	// Uses unused Renderstate 42 to set remix texture categories
	// ~ req. runtime changes
	void model_render::set_remix_texture_categories(IDirect3DDevice9* dev, const InstanceCategories& cat, bool remove_category)
	{
		primctx.save_rs(dev, RS_42_TEXTURE_CATEGORY);

		if (remove_category) {
			primctx.modifiers.remix_instance_categories &= ~cat;
		} else {
			primctx.modifiers.remix_instance_categories |= cat;
		}

		dev->SetRenderState((D3DRENDERSTATETYPE)RS_42_TEXTURE_CATEGORY, static_cast<DWORD>(primctx.modifiers.remix_instance_categories));
	}

	// Uses unused Renderstate 150 to set custom remix hash
	// ~ req. runtime changes
	void model_render::set_remix_texture_hash(IDirect3DDevice9* dev, const std::uint32_t& hash)
	{
		primctx.save_rs(dev, RS_150_TEXTURE_HASH);
		dev->SetRenderState((D3DRENDERSTATETYPE)RS_150_TEXTURE_HASH, hash);
	}

	// Uses unused Renderstate 220 re-hash the original hash with a given seed
	// ~ req. runtime changes
	void model_render::set_remix_texture_hash_modifier(IDirect3DDevice9* dev, const std::uint32_t& seed)
	{
		primctx.save_rs(dev, RS_220_HASH_MODIFIER_SEED);
		dev->SetRenderState((D3DRENDERSTATETYPE)RS_220_HASH_MODIFIER_SEED, seed);
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

	// detoured 'CModelRender::DrawModelExecute'
	void __fastcall tbl_hk::model_renderer::DrawModelExecute::Detour(void* ecx, void* edx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
	{
		// draw nocull markers before drawing the first model - no particular reason besides that we dont want to draw them before rendering the sky
		if (game::get_viewid() != VIEW_3DSKY && !model_render::get()->m_drew_model)
		{
			remix_markers::draw_nocull_markers();
			model_render::get()->m_drew_model = true;
		}

		bool ignore = false;
		const auto& ms = map_settings::get_map_settings();
		const auto& hmsettings = ms.hide_models;

		for (const auto& hide_mdl_with_radius : hmsettings.radii)
		{
			if (utils::float_equal(pInfo.pModel->radius, hide_mdl_with_radius))
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

		// check for attached lights
		remix_lights::on_draw_model_exec(pInfo);

		if (!ignore)
		{
			// draw the model
			tbl_hk::model_renderer::table.original<FN>(index)(ecx, edx, /*oo,*/ state, pInfo, pCustomBoneToWorld);

			if (cmd::model_info_vis)
			{
				const auto cutoff_dist = game_settings::get()->debug_info_distance.get_as<float>();
				if (game::get_current_view_origin()->DistToSqr(pInfo.origin) < cutoff_dist * cutoff_dist)
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
				const auto cutoff_dist = game_settings::get()->debug_info_distance.get_as<float>();
				if (game::get_current_view_origin()->DistToSqr(pInfo.origin) < cutoff_dist * cutoff_dist)
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

	void cmeshdx8_renderpass_pre_draw(CMeshDX8* mesh, [[maybe_unused]] /*CPrimList**/ std::uint32_t primlist)
	{
		const auto dev = game::get_d3d_device();
		IDirect3DVertexBuffer9* buffer9 = nullptr;
		UINT stride = 0;
		{
			UINT ofs = 0; dev->GetStreamSource(0, &buffer9, &ofs, &stride);
		}

		prim_fvf_context& ctx = model_render::primctx;
		const auto shaderapi = game::get_shaderapi();

		if (ctx.get_info_for_pass(shaderapi)) 
		{
			// added format check
			if (mesh->m_VertexFormat == 0x480033 || mesh->m_VertexFormat == 0x80033)
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

							if (has_bottom_mat)
							{
								const auto& ms = map_settings::get_map_settings();

								// we only need one surface
								ctx.modifiers.as_water = true;
								ctx.modifiers.og_mesh_z_offset = ms.water_offset_bottom;
								ctx.modifiers.dual_render_with_specified_texture = true;
								ctx.modifiers.dual_render_texture_z_offset = ms.water_offset_top; //0.5f;
								ctx.modifiers.dual_render_texture = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[2]);
								
								// assign flowmap
								IDirect3DBaseTexture9* tex = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[4]);
								if (tex)
								{
									ctx.save_texture(dev, 0);
									dev->SetTexture(0, tex);
								}

								// scale water uv
								D3DXMATRIX scaleMatrix; // create a scaling matrix
								D3DXMatrixScaling(&scaleMatrix, 1.5f * ms.water_uv_scale, 1.5f * ms.water_uv_scale, 1.0f);

								ctx.save_ss(dev, D3DSAMP_ADDRESSU);
								ctx.save_ss(dev, D3DSAMP_ADDRESSV);
								dev->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
								dev->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

								ctx.set_texture_transform(dev, &scaleMatrix);
								ctx.save_tss(dev, D3DTSS_TEXTURETRANSFORMFLAGS);
								dev->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
							}

							// ignore 'beneath'
							else
							{
								ctx.modifiers.do_not_render = true;
							}
						}
					}
				}
			}
		}

		// no longer set cam transforms in 'main_module::on_renderview'
		// setting them there causes meshes rendered with shaders to lag behind
		dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
		dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
		dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);

		// shader: VertexLitGeneric (infected - player model - viewmodel - dynamic props)
		// > models/weapons/melee/crowbar
		// > models/props_junk/wood_palletcrate001a
		// shader: Refract_DX90
		// > vgui/hud/scope_sniper_ul
		if (mesh->m_VertexFormat == 0xa0003)
		{
			//ctx.modifiers.do_not_render = true;
			bool use_shader = false;

			// viewmodel
			if (ctx.info.buffer_state.m_Transform[2].m[3][2] == -1.00003338f)
			{
				ctx.save_view_transform(dev);
				ctx.save_projection_transform(dev);
				dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
				dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);
			}
			// models/player/chell/gambler_eyeball_ l/r
			else if (ctx.info.material_name.contains("_eyeball_"))
			{
				if (const auto basemap2 = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[1]); basemap2) 
				{
					ctx.save_texture(dev, 0);
					dev->SetTexture(0, basemap2);
					ctx.modifiers.as_temp_unused = true;
				}
			}

			const auto im = imgui::get();

			if (ctx.info.shader_name == "Infected" && !im->m_dev_disable_infected_shader && (
					ctx.info.material_name.contains("/l4d2/") || 
					(ctx.info.material_name.contains("/l4d1/cim_") && !ctx.info.material_name.ends_with("pilot"))
				)) // ignore "cim_fallen_survivor_l4d1_pilot.vmt"
			{
				//ctx.modifiers.do_not_render = true;

				//IMaterialVar* var = nullptr;
				//if (has_materialvar(ctx.info.material, "$gradienttexture", &var))
				//{
				//	// if material has NO defined basetexture
				//	if (var && !var->vftable->IsDefined(var)) {
				//		goto NOT_INFECTED_SHADER;
				//	}

				//	auto asd = var->vftable->GetStringValue(var); 
				//	int break_me = 0;
				//}

				// References:
				// https://developer.valvesoftware.com/wiki/Infected_(shader)
				// https://steamcommunity.com/sharedfiles/filedetails/?id=1567031703&preview=true
				// https://cdn.fastly.steamstatic.com/apps/valve/2010/GDC10_ShaderTechniquesL4D2.pdf
				// https://steamcdn-a.akamaihd.net/apps/valve/2010/gdc2010_vlachos_l4d2wounds.pdf

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

				model_render::set_remix_modifier(dev, RemixModifier::InfectedShader);

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
				ctx.save_rs(dev, RS_196_INFECTED_SKIN_GRAD);
				dev->SetRenderState((D3DRENDERSTATETYPE)RS_196_INFECTED_SKIN_GRAD, skin_index + (cloth_index * 10));


				// g_vGradSelect - pack two floats into one RS
				ctx.save_rs(dev, RS_197_INFECTED_GRAD_SELECT);
				dev->SetRenderState((D3DRENDERSTATETYPE)RS_197_INFECTED_GRAD_SELECT, utils::pack_2f_in_dword(grad_select[0], grad_select[1]));


				// sprite index - pack two floats into one RS
				ctx.save_rs(dev, RS_177_INFECTED_SHEET_UV);
				dev->SetRenderState((D3DRENDERSTATETYPE)RS_177_INFECTED_SHEET_UV, utils::pack_2f_in_dword(uv_transform[0], uv_transform[1]));

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
				ctx.save_rs(dev, RS_211_INFECTED_NORMAL_ROUGH_BOOST);
				dev->SetRenderState((D3DRENDERSTATETYPE)RS_211_INFECTED_NORMAL_ROUGH_BOOST, utils::pack_2f_in_dword(normal_boost, roughness_boost));
				

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

			else if (ctx.info.material_name.starts_with("vgui/hud/scope_sniper")) {
				use_shader = true;
			}

			bool using_custom_transform = false;

			if (g_use_playershadow && g_is_rendering_our_thirdperson_mesh)
			{
				// backwards offset similar to whats found in remix but without the body mesh getting smeary
				const auto backward_offset = game_settings::get()->player_backwards_offset.get_as<float>();
				if (!utils::float_equal(backward_offset, 0.0f))
				{
					const Vector forward = *game::get_current_view_forward();
					Vector backward_offset_vector = forward;
					backward_offset_vector.z = 0.0f;

					backward_offset_vector.Normalize();
					backward_offset_vector *= -backward_offset;

					const D3DXMATRIX backward_offset_matrix
					{
						1.f, 0.f, 0.f, 0.f,
						0.f, 1.f, 0.f, 0.f,
						0.f, 0.f, 1.f, 0.f,
						backward_offset_vector.x, backward_offset_vector.y, backward_offset_vector.z, 1.f
					};

					D3DXMATRIX final_world_matrix;
					D3DXMatrixMultiply(&final_world_matrix, &backward_offset_matrix, &ctx.info.buffer_state.m_Transform[0]);

					ctx.save_world_transform(dev);
					dev->SetTransform(D3DTS_WORLD, &final_world_matrix);
					using_custom_transform = true;
				}

				//auto& playermodel_str = main_module::get()->m_playermodel_substr;
				//if (!playermodel_str.empty() && playermodel_str != "INVALID")
				//{
				//	if (ctx.info.material_name.starts_with(playermodel_str)) {
						model_render::set_remix_texture_categories(dev, InstanceCategories::ThirdPersonPlayerBody | InstanceCategories::ThirdPersonPlayerModel);
				//	}
				//}
			}

		//NOT_INFECTED_SHADER:
			if (!use_shader)
			{
				ctx.save_vs(dev);
				dev->SetVertexShader(nullptr);
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX6);

				if (!using_custom_transform) {
					dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
				}
			}
		}

		// shader: VertexLitGeneric
		// > models/props_downtown/pipes_rooftop
		// > models/props_foliage/urban_trees_branches03_small
		else if (mesh->m_VertexFormat == 0xa0103)
		{
			if (game_settings::get()->enable_3d_sky.get_as<bool>())
			{
				if (ctx.info.shader_name.starts_with("Black")) {
					ctx.modifiers.do_not_render = true; //skip = true;
				}
			}
			
			//lookat_vertex_decl(dev);
			//ctx.modifiers.do_not_render = true; 
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

		// UnlitGeneric -- stride 0x20
		// > skybox/urban_horizon_even
		else if (mesh->m_VertexFormat == 0x80103)
		{
			ctx.modifiers.do_not_render = false; 
		}

		// shader: screenspace_general_dx9, Engine_Post_dx9, Sky_HDR_DX9, DecalModulate_dx9 ..
		// > dev/lumcompare
		// > skybox/sky_l4d_c1_1_hdrrt
		// > decals/blood3_subrect
		else if (mesh->m_VertexFormat == 0x80001)
		{
			//ctx.modifiers.do_not_render = true;

			// causes some weird flickering artifacts from time to time?
			if (ctx.info.shader_name.starts_with("Black")) {
				ctx.modifiers.do_not_render = true;
			}

			if (ctx.info.shader_name.starts_with("DecalMod"))
			{
				//lookat_vertex_decl(dev);
				ctx.save_vs(dev);
				dev->SetVertexShader(nullptr);
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
				//dev->SetTransform(D3DTS_VIEW, &ctx.info.buffer_state.m_Transform[1]);
				//dev->SetTransform(D3DTS_PROJECTION, &ctx.info.buffer_state.m_Transform[2]);
				//set_remix_texture_categories(dev, ctx, WorldMatte);
			}

			// FIRST "UI/HUD" elem (remix injection triggers here)
			// -> fullscreen color transitions (damage etc.) and also "enables" the crosshair
			else if (ctx.info.shader_name.starts_with("Engine_")) // Engine_Post
			{
				// do not fog HUD elements :D
				dev->SetRenderState(D3DRS_FOGENABLE, FALSE);

				const auto s_viewFadeColor = l4d2::s_viewFadeColor; //reinterpret_cast<Vector4D*>(CLIENT_BASE + 0x7A3D68);

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
		}

		// shader: VertexLitGeneric, UnlitGeneric (stride 0x20)
		// > decals/wood/wood3
		// > decals/metal/metal04
		// > debug/debugtranslucentsinglecolor
		else if (mesh->m_VertexFormat == 0x80003)
		{
			//ctx.modifiers.do_not_render = true;
			//lookat_vertex_decl(dev);

			// TODO - HACK: see r_DispWalkable note in main_module
			if (ctx.info.material_name.starts_with("debug/")) {
				ctx.modifiers.do_not_render = true;
			}
	
			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
			dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
		}

		// shader: UnlitGeneric (stride 0x30)
		// > __fontpage_additive, vgui/hud/scalablepanel_bgmidgrey_outlinegreen_glow
		// > vgui/hud/scalablepanel_bgmidgrey_outlinegreen_glow
		// > detail/detailsprites_overgrown
		// > sun flare
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
					model_render::set_remix_texture_categories(dev, InstanceCategories::WorldUI);
				}
				else if (is_world_ui_text)
				{
					//lookat_vertex_decl(dev, primlist);
					float vcol_r = 0.0f;
					float vcol_g = 0.0f;
					float vcol_b = 0.0f;
					float vcol_a = 1.0f;

					// is this still needed?
					// cant get vertex color to work here -> grab vertex color and use tfactor instead
					
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
						vcol_r * scalar,
						vcol_g * scalar,
						vcol_b * scalar, scalar));
				}

				// some light sprites are rendered as ui through other geo 
				else if (ctx.info.material_name.ends_with("_noz")) {
					ctx.modifiers.do_not_render = true;
				}
			}
			else if (ctx.info.material_name.starts_with("detail/")) 
			{
				//lookat_vertex_decl(dev);

				ctx.save_vs(dev);
				dev->SetVertexShader(nullptr); 
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			}
			else if (ctx.info.material_name == "sprites/light_glow02_add_noz")
			{
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

				float r = 0.0f;
				float g = 0.0f;
				float b = 0.0f;

				if (!g_sunoverlay_color.empty())
				{
					r = g_sunoverlay_color.back().x;
					g = g_sunoverlay_color.back().y;
					b = g_sunoverlay_color.back().z;
					//g_sunoverlay_color.erase(g_sunoverlay_color.begin());
				}

				ctx.save_rs(dev, D3DRS_TEXTUREFACTOR);
				dev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_COLORVALUE(r, g, b, 1.0f));

				ctx.save_vs(dev);
				dev->SetVertexShader(nullptr);
				dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);

				// slightly offset surfaces so that we do not render sunlayers on the same plane (causes flickering)
				//ctx.info.buffer_state.m_Transform[0].m[3][3] *= (0.998f - (float)g_sunoverlay_color.size() * 0.001f);
				//dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
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
		}

		// shader: DecalModulate_dx9, Sprite_DX9, Bik
		// > decals/bloodstain_002
		// > sprites/glow_test02_rendermode_5
		// > videobikmaterial_background
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

			if (ctx.info.shader_name.starts_with("Spr")) {
				model_render::set_remix_texture_categories(dev, InstanceCategories::Particle);
			} else if (ctx.info.shader_name == "Bik") {
				mod_shader = false;
			}

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
			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		}

		// shader: Cable_DX9
		// > particle/smoker_tongue_beam
		else if (mesh->m_VertexFormat == 0x480035)
		{
			//ctx.modifiers.do_not_render = true;
			//ctx.save_texture(dev, 0);
			//dev->SetTexture(0, tex_addons::black);

			if (const auto basemap2 = shaderapi->vtbl->GetD3DTexture(shaderapi, nullptr, ctx.info.buffer_state.m_BoundTexture[1]); basemap2)
			{
				ctx.save_texture(dev, 0);
				dev->SetTexture(0, basemap2);
			}

			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetFVF(NULL); // using vertexdecl is fine
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
			else if (ctx.info.material_name.starts_with("particle/fire_")) {
				model_render::set_remix_emissive_intensity(dev, 10.0f);
			}
		}

		// shader: SpriteCard (spark) - stride 0x60
		// > particle/beam_flashlight (not used for surv. only the player) -> disable or blood overlay (hud) will raster this sprite
		else if (mesh->m_VertexFormat == 0x124900005)
		{
			ctx.modifiers.do_not_render = true;
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
			//lookat_vertex_decl(dev); 
			model_render::set_remix_emissive_intensity(dev, 0.05f);
			model_render::set_remix_texture_categories(dev, InstanceCategories::Particle);


			ctx.save_rs(dev, D3DRS_SRCBLEND);
			ctx.save_rs(dev, D3DRS_DESTBLEND);
			ctx.save_rs(dev, D3DRS_ALPHABLENDENABLE);

			dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);

			// we have to use the rain_drop texture somewhere else or remix does not load the texture
			// see: model_render::draw_nocull_markers #HACK
			ctx.save_texture(dev, 0);
			dev->SetTexture(0, tex_addons::rain_drop);

			/*ctx.save_tss(dev, D3DTSS_COLORARG1);
			ctx.save_tss(dev, D3DTSS_COLORARG2);
			ctx.save_tss(dev, D3DTSS_COLOROP);
			ctx.save_tss(dev, D3DTSS_ALPHAARG2);
			ctx.save_tss(dev, D3DTSS_ALPHAOP);

			dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
			dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

			float r = 1.0f;
			float g = 1.0f;
			float b = 1.0f;

			ctx.save_rs(dev, D3DRS_TEXTUREFACTOR);
			dev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_COLORVALUE(r, g, b, 0.0f));*/
		}

		// shader: Water_DX9_HDR (with bottommaterial)
		// > liquids/water_swamp_m2
		else if (mesh->m_VertexFormat == 0x480033)
		{
			//ctx.modifiers.do_not_render = true;
			//lookat_vertex_decl(dev);

			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
			dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
		}

		// shader: Water_DX9_HDR
		// > liquids/water_swamp_m2_beneath
		else if (mesh->m_VertexFormat == 0x80033)
		{
			//ctx.modifiers.do_not_render = true;
			//lookat_vertex_decl(dev);

			ctx.save_vs(dev);
			dev->SetVertexShader(nullptr);
			dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
			dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
		}

		// shader: Spritecard
		// > particle/string_light_beam (c2m3_coaster)
		else if (mesh->m_VertexFormat == 0x3724900005)
		{
			// cant fix for now
			ctx.modifiers.do_not_render = false;

			//lookat_vertex_decl(dev);

			//ctx.save_vs(dev);
			//dev->SetVertexShader(nullptr);
			//dev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX5);

			//ctx.save_texture(dev, 0);
			//dev->SetTexture(0, tex_addons::black);
		}

		else 
		{
			ctx.modifiers.do_not_render = false; 
			//int break_me = 1;  
		}

#if 0
		// Due to the very short renderdist on some maps, the game culls entire areas and that includes large structures.
		// To make culling of these structures less noticable, some 3d skyboxes include a very simplified world model that includes these large structures.
		// It is using the black material shader and gets colored by the map fog. So when the actual area gets culled and with it the large structure, a vista object with a single color will stay in its place.
		// We do not render that vista mesh as it's causing visual artifacts because we have no way to properly fade it using colormode ADD (with the fog color) as we A. have no fog color and B. no per pixel depth.
		// We cant use per object dist to the skycamera because it's a single mesh.

		// Idea: Split up largest and most noticable structures, make them slightly smaller so that they fit inside the original mesh (eg building) so that they are not visible with the original mesh is visible.
		// ^ via map marker + remix?

		if (game_settings::get()->enable_3d_sky.get_as<bool>())
		{
			// ignore for now (sometimes used for 3d skybox)
			if (mesh->m_VertexFormat == 0xa0103 && ctx.info.shader_name.starts_with("Black"))
			{
				// sdk: m_clrRender - nope
				// black shader dithers color to the fog color

				//ctx.modifiers.do_not_render = true;
				//int break_me = 0;

				if (const auto view = game::get_viewid(); 
					view == VIEW_3DSKY)
				{
					Vector meshpos = {
					ctx.info.buffer_state.m_Transform[0].m[3][0],
					ctx.info.buffer_state.m_Transform[0].m[3][1],
					ctx.info.buffer_state.m_Transform[0].m[3][2] };

					const auto main_module = main_module::get();
					auto dist = meshpos.DistTo(main_module->m_sky3d_camera_origin);
					dist *= (float)main_module->m_sky3d_scale;
					//if (dist < 3000.0f) {
						//ctx.modifiers.do_not_render = true;
					//}
					//else
					{
						dev->SetTexture(0, tex_addons::black);
						//set_remix_texture_categories(dev, ctx, REMIXAPI_INSTANCE_CATEGORY_BIT_BEAM);

						ctx.save_tss(dev, D3DTSS_COLORARG1);
						ctx.save_tss(dev, D3DTSS_COLORARG2);
						ctx.save_tss(dev, D3DTSS_COLOROP);
						ctx.save_tss(dev, D3DTSS_ALPHAARG2);
						ctx.save_tss(dev, D3DTSS_ALPHAOP);

						dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
						dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
						dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
						dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
						dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

						float r = 0.0f; //dist < 3000.0f ? 1.0f : 0.0f;
						float g = 0.0f; //dist < 3000.0f ? 0.0f : 1.0f;
						float b = 0.0f;

						ctx.save_rs(dev, D3DRS_TEXTUREFACTOR);
						dev->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_COLORVALUE(r, g, b, 1.0f));

						ctx.save_vs(dev);
						dev->SetVertexShader(nullptr);
						dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX3);

						float scale = 1.0f; 
						ctx.info.buffer_state.m_Transform[0].m[0][0] *= scale;
						ctx.info.buffer_state.m_Transform[0].m[1][1] *= scale;
						ctx.info.buffer_state.m_Transform[0].m[2][2] *= scale;
						dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
					}
				}
			}
		}
#endif

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
			if (ctx.modifiers.og_mesh_z_offset != 0.0f)
			{
				ctx.info.buffer_state.m_Transform[0].m[3][2] += ctx.modifiers.og_mesh_z_offset;
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			}

			/*if (ctx.modifiers.as_temp_unused)
			{
				const auto& im = imgui::get();
				ctx.save_world_transform(dev, &ctx.info.buffer_state.m_Transform[0]);

				ctx.info.buffer_state.m_Transform[0].m[2][0] *= im->m_debug_float_vec4[0];
				ctx.info.buffer_state.m_Transform[0].m[2][1] *= im->m_debug_float_vec4[1];
				ctx.info.buffer_state.m_Transform[0].m[2][2] *= im->m_debug_float_vec4[2];
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			}*/

			dev->DrawIndexedPrimitive(type, base_vert_index, min_vert_index, num_verts, start_index, prim_count);

			// restore transform
			if (ctx.modifiers.og_mesh_z_offset != 0.0f)
			{
				ctx.info.buffer_state.m_Transform[0].m[3][2] -= ctx.modifiers.og_mesh_z_offset;
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			}

			/*if (ctx.modifiers.as_temp_unused) {
				ctx.restore_world_transform(dev);
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

				model_render::set_remix_texture_categories(dev, InstanceCategories::WorldMatte | InstanceCategories::IgnoreOpacityMicromap);
			}

			if (ctx.modifiers.dual_render_texture_z_offset != 0.0f)
			{
				ctx.info.buffer_state.m_Transform[0].m[3][2] += ctx.modifiers.dual_render_texture_z_offset;
				dev->SetTransform(D3DTS_WORLD, &ctx.info.buffer_state.m_Transform[0]);
			}

			if (ctx.modifiers.as_water) {
				model_render::set_remix_texture_hash(dev, utils::string_hash32(ctx.info.material_name));
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
		// "particle/fire_burning_character/fire_molotov_crop_low"
		else if (mat_name.starts_with("particle/fire_burning_character") && mat_name.contains("crop")) {
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

	void RopeManager_DrawRenderCache_mid_hk(CMeshBuilder* builder)
	{
		const auto dev = game::get_d3d_device();

		auto CatmullRomSpline = [](const Vector4D& a, const Vector4D& b, const Vector4D& c, const Vector4D& d, const float t)
			{
				return b + 0.5f * t * (c - a + t * (2.0f * a - 5.0f * b + 4.0f * c - d + t * (-a + 3.0f * b - 3.0f * c + d)));
			};

		auto DCatmullRomSpline3 = [](const Vector& a, const Vector& b, const Vector& c, const Vector& d, const float t)
			{
				return 0.5f * (c - a + t * (2.0f * a - 5 * b + 4 * c - d + t * (3.0f * b - a - 3.0f * c + d))
					+ t * (2.0f * a - 5.0f * b + 4 * c - d + 2.0f * (t * (3 * b - a - 3.0f * c + d))));
			};

		Vector eyePos;
		{
			float v[4] = {}; dev->GetVertexShaderConstantF(2, v, 1);
			eyePos = Vector(v[0], v[1], v[2]);
		}

		// m_pCurr... is set to the next free, unused vert after the current rope so we start at (current - 1) - the total vert count
		for (auto v = 1; v <= builder->m_VertexBuilder.m_nVertexCount; v++)
		{
			const auto v_pos_in_src_buffer = v * builder->m_VertexBuilder.m_VertexSize_Position;

			const auto src_vParms = reinterpret_cast<Vector*>(((DWORD)builder->m_VertexBuilder.m_pCurrPosition - v_pos_in_src_buffer));
			const auto dest_pos = reinterpret_cast<Vector*>(src_vParms);

			const auto src_vTint = reinterpret_cast<D3DCOLOR*>(((DWORD)builder->m_VertexBuilder.m_pCurrColor - v_pos_in_src_buffer));

			const auto src_vSplinePt0 = reinterpret_cast<Vector4D*>(((DWORD)builder->m_VertexBuilder.m_pCurrTexCoord[0] - v_pos_in_src_buffer));
			const auto dest_tc = reinterpret_cast<Vector2D*>(src_vSplinePt0);

			const auto src_vSplinePt1 = reinterpret_cast<Vector4D*>(((DWORD)builder->m_VertexBuilder.m_pCurrTexCoord[1] - v_pos_in_src_buffer));
			const auto src_vSplinePt2 = reinterpret_cast<Vector4D*>(((DWORD)builder->m_VertexBuilder.m_pCurrTexCoord[2] - v_pos_in_src_buffer));
			const auto src_vSplinePt3 = reinterpret_cast<Vector4D*>(((DWORD)builder->m_VertexBuilder.m_pCurrTexCoord[3] - v_pos_in_src_buffer));

			// save vParms (because we will be overriding them when writing pos)
			const float parmsX = src_vParms->x;
			const float parmsY = src_vParms->y;
			const float parmsZ = src_vParms->z;

			const auto P0 = *src_vSplinePt0;
			const auto P1 = *src_vSplinePt1;
			const auto P2 = *src_vSplinePt2;
			const auto P3 = *src_vSplinePt3;

			auto posrad = CatmullRomSpline(P0, P1, P2, P3, parmsX);

			Vector v2p = { 0.0f, 0.0f, 1.0f };
			v2p.x = posrad.x - eyePos.x;	// screen aligned
			v2p.y = posrad.y - eyePos.y;
			v2p.z = posrad.z - eyePos.z;

			Vector tangent = DCatmullRomSpline3(P0, P1, P2, P3, parmsX);

			//float3 ofs = normalize(cross(v2p, normalize(tangent)));
			tangent.NormalizeChecked();
			Vector ofs = v2p.Cross(tangent); // maybe switch these - no difference
			ofs.NormalizeChecked();

			//posrad.xyz += ofs * (posrad.w * (v.vParms.z - .5));
			const auto add = ofs.Scale(posrad.w * (parmsZ - 0.5f));
			posrad.x += add.x;
			posrad.y += add.y;
			posrad.z += add.z;

			// pos
			dest_pos->x = posrad.x;
			dest_pos->y = posrad.y;
			dest_pos->z = posrad.z;

			// o.texCoord.xy = float2( 1.0f - v.vParms.z, v.vParms.y );
			dest_tc->x = 1.0f - parmsZ;
			dest_tc->y = parmsY;

			// unpack color
			Vector4D color;
			color.x = static_cast<float>((*src_vTint >> 16) & 0xFF) / 255.0f * 1.0f;
			color.y = static_cast<float>((*src_vTint >> 8) & 0xFF) / 255.0f * 1.0f;
			color.z = static_cast<float>((*src_vTint >> 0) & 0xFF) / 255.0f * 1.0f;
			color.w = static_cast<float>((*src_vTint >> 24) & 0xFF) / 255.0f * 0.1f; // ! 0.1

			// write color
			*src_vTint = D3DCOLOR_COLORVALUE(color.x, color.y, color.z, color.w);
		}
	}

	HOOK_RETN_PLACE_DEF(RopeManager_DrawRenderCache_retn_addr);
	void __declspec(naked) RopeManager_DrawRenderCache_stub()
	{
		__asm
		{
			pushad;
			lea     eax, [ebp - 0x1EC]; // meshbuilder
			push	eax;
			call	RopeManager_DrawRenderCache_mid_hk;
			add		esp, 4;
			popad;

			// og
			mov     ecx, [ebp - 0x138];
			jmp		RopeManager_DrawRenderCache_retn_addr;
		}
	}

	void grab_glowoverlay_color_hk(float* color)
	{
		if (color) {
			g_sunoverlay_color.push_back({ color[0], color[1], color[2] });
		}
	}

	HOOK_RETN_PLACE_DEF(grab_glowoverlay_color_retn_addr);
	void __declspec(naked) grab_glowoverlay_color_stub()
	{
		__asm
		{
			pushad;
			lea		eax, [ebp - 0x40C];
			push	eax;
			call	grab_glowoverlay_color_hk;
			add		esp, 4;
			popad;

			// og
			movss   xmm0, dword ptr[ebp - 0x40C];
			jmp		grab_glowoverlay_color_retn_addr;
		}
	}

	namespace unbake_transform
	{
		struct mstudio_modelvertexdata_t
		{
			const void* pVertexData;
			const void* pTangentData;
		};

		struct mstudiomodel_t
		{
			char name[64];
			int type;
			float boundingradius;
			int nummeshes;
			int meshindex;
			int numvertices;
			int vertexindex;
			int tangentsindex;
			int numattachments;
			int attachmentindex;
			int numeyeballs;
			int eyeballindex;
			mstudio_modelvertexdata_t vertexdata;
			int unused[8];
		};

		struct CStudioRender
		{
			char pad[0x6C];
			matrix3x4_t m_StaticPropRootToWorld;
			matrix3x4_t* m_pBoneToWorld;
			matrix3x4_t* m_PoseToWorld;
			int pad2[3];
			studiohdr_t* m_pStudioHdr;
			mstudiomodel_t* sub_model;
		}; STATIC_ASSERT_OFFSET(CStudioRender, m_pStudioHdr, 0xB0);

		int R_StudioDrawStaticMesh_hk(const CStudioRender* studio)
		{
			if (imgui::get()->m_debug_disable_unbake) {
				return 0;
			}

			if (imgui::get()->m_debug_unbake_all_single_bones && studio->m_pStudioHdr->numbones <= 1) {
				return 1;
			}

			const auto model_str = std::string_view(studio->sub_model->name);

			bool requires_unbake = false;
			const auto& unbake_model_names = map_settings::get_map_settings().unbake_models;

			// check for unbake checksums
			if (!unbake_model_names.checksums.empty())
			{
				for (const auto& unbake_mdl_checksum : unbake_model_names.checksums)
				{
					if (unbake_mdl_checksum == studio->m_pStudioHdr->checksum)
					{
						requires_unbake = true;
						break;
					}
				}
			}

			if (cmd::ms_unbake_info)
			{
				std::string str = utils::to_hex_string(studio->m_pStudioHdr->checksum) + ", # " + std::string(model_str);
				cmd::ms_unbake_info_logged_strings.insert(str);
			}

			if (cmd::unbake_model_info_vis)
			{
				const bool ends_with_dmx = std::string_view(studio->sub_model->name).ends_with("dmx");
				const auto name_hash = utils::string_hash32(studio->sub_model->name);
				const float rnd_z = utils::random_float_generator::get().random_float_from_hash(name_hash, -10.0f, ends_with_dmx ? 20 : 10.0f);

				const auto cutoff_dist = game_settings::get()->debug_info_distance.get_as<float>();
				const Vector org = { studio->m_PoseToWorld->m_flMatVal[0][3], studio->m_PoseToWorld->m_flMatVal[1][3], studio->m_PoseToWorld->m_flMatVal[2][3] + rnd_z };
				if (game::get_current_view_origin()->DistToSqr(org) < cutoff_dist * cutoff_dist)
				{
					if (requires_unbake) {
						game::debug_add_text_overlay(&org.x, "#UNBAKED#", 0, 1.0f, 0.6f, 0.6f, 0.6f);
					}

					game::debug_add_text_overlay(&org.x, studio->sub_model->name, 1, 1.0f, 1.0f, 1.0f, 1.0f);
					game::debug_add_text_overlay(&org.x, utils::va("Checksum: %s", utils::to_hex_string(studio->m_pStudioHdr->checksum).c_str()), 2, 1.0f, 1.0f, 1.0f, 1.0f);
					game::debug_add_text_overlay(&org.x, utils::va("NumBones: %d", studio->m_pStudioHdr->numbones), 3, 0.6f, 0.6f, 0.6f, 0.7f);
				}
			}

			/*if (studio->m_pStudioHdr->numbones <= 1)
			{
				return 0;
			}*/

			return requires_unbake;
		}

		HOOK_RETN_PLACE_DEF(R_StudioDrawStaticMesh_og_retn_addr);
		HOOK_RETN_PLACE_DEF(R_StudioDrawStaticMesh_nop_retn_addr);
		void __declspec(naked) R_StudioDrawStaticMesh_stub()
		{
			__asm
			{
				pushad;
				push	ebx; // CStudioRender
				call	R_StudioDrawStaticMesh_hk;
				add		esp, 4;

				cmp		eax, 1;
				je		SKIP_CHECK;	// jmp if eax = 1
				popad;

				// og
				mov     eax, [ebx + 4];
				test    byte ptr[eax + 0x24], 2;
				jmp		R_StudioDrawStaticMesh_og_retn_addr;

			SKIP_CHECK:
				popad;

				mov     eax, [ebx + 4]; // og
				jmp		R_StudioDrawStaticMesh_nop_retn_addr;
			}
		}
	}

	// called from remix_api::on_present_callback()
	void model_render::on_present()
	{
		// capture 1 frame
		if (cmd::ms_unbake_info == 2)
		{
			cmd::ms_unbake_info = 0u;
			std::filesystem::create_directories(game::root_path + "\\l4d2-rtx\\logs\\");

			std::ofstream file;
			file.open((game::root_path + "\\l4d2-rtx\\logs\\mapsettings_unbake_info.log").c_str());

			file << "# MapSettings [UNBAKE] : Logfile containing names of models that were drawn in the captured frame." << "\n";
			file << "# checksum, # model name" << "\n\n";

			for (const auto& str : cmd::ms_unbake_info_logged_strings) {
				file << str << "\n";
			}

			file.close();
			cmd::ms_unbake_info_logged_strings.clear();
		}

		// inc. when not 0
		if (cmd::ms_unbake_info) {
			cmd::ms_unbake_info++;
		}
	}

	// #
	// Commands

	ConCommand xo_debug_toggle_model_info_cmd{};
	void model_render::xo_debug_toggle_model_info_fn()
	{
		cmd::model_info_vis = !cmd::model_info_vis;
	}

	ConCommand xo_debug_toggle_unbake_model_info_cmd{};
	void xo_debug_toggle_unbake_model_info_fn()
	{
		cmd::unbake_model_info_vis = !cmd::unbake_model_info_vis;
	}

	ConCommand xo_mapsettings_get_unbake_info_cmd{};
	void xo_mapsettings_get_unbake_info_fn()
	{
		cmd::ms_unbake_info = 1;
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

		utils::hook(l4d2::kh_addr__cmeshdx8_renderpass_pre_draw, cmeshdx8_renderpass_pre_draw_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(cmeshdx8_renderpass_pre_draw_retn_addr, l4d2::kh_addr__cmeshdx8_renderpass_pre_draw + 5u);

		utils::hook(l4d2::kh_addr__cmeshdx8_renderpass_post_draw, cmeshdx8_renderpass_post_draw_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(cmeshdx8_renderpass_post_draw_retn_addr, l4d2::retn_addr__cmeshdx8_renderpass_post_draw);

		// C_OP_RenderSprites::Render :: fix SpriteCard UV's
		utils::hook::nop(l4d2::hk_addr__render_spritecard_new, 6);
		utils::hook(l4d2::hk_addr__render_spritecard_new, RenderSpriteCardNew_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(RenderSpriteCardNew_retn_addr, l4d2::hk_addr__render_spritecard_new + 6u);

		// Fix actual ropes
		utils::hook::nop(l4d2::hk_addr__rope_mgr_draw_render_cache, 6);
		utils::hook(l4d2::hk_addr__rope_mgr_draw_render_cache, RopeManager_DrawRenderCache_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(RopeManager_DrawRenderCache_retn_addr, l4d2::hk_addr__rope_mgr_draw_render_cache + 6u);

		// CGlowOverlay::Draw :: grab sun overlay color to apply color via TFACTOR instead of vertex colors (as that fails - search for "sprites/light_glow02_add_noz")
		utils::hook::nop(l4d2::hk_addr__glow_overlay_draw, 8); // offs changed 0726
		utils::hook(l4d2::hk_addr__glow_overlay_draw, grab_glowoverlay_color_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(grab_glowoverlay_color_retn_addr, l4d2::hk_addr__glow_overlay_draw + 8u);

		// C_FuncAreaPortalWindow::DrawModel :: disable drawing Area Portal Brushmodels
		utils::hook::nop(l4d2::nop_addr__func_area_portal_window_draw_mdl, 2);

		// --
		// Remove transforms from prop vertices (UNBAKE)
		utils::hook::nop(l4d2::hk_addr__studio_draw_static_mesh, 7);
		utils::hook(l4d2::hk_addr__studio_draw_static_mesh, unbake_transform::R_StudioDrawStaticMesh_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(unbake_transform::R_StudioDrawStaticMesh_og_retn_addr, l4d2::hk_addr__studio_draw_static_mesh + 7u); // 0xEF82
		HOOK_RETN_PLACE(unbake_transform::R_StudioDrawStaticMesh_nop_retn_addr, l4d2::hk_addr__studio_draw_static_mesh + 9u); // 0xEF84

		// #
		// commands

		game::con_add_command(&xo_debug_toggle_model_info_cmd, "xo_debug_toggle_model_info", xo_debug_toggle_model_info_fn, "Toggle model name and radius visualizations");

		game::con_add_command(&xo_debug_toggle_unbake_model_info_cmd, "xo_debug_toggle_unbake_model_info", xo_debug_toggle_unbake_model_info_fn, "Draw model name checksums for [UNBAKE] (mapsettings)");
		game::con_add_command(&xo_mapsettings_get_unbake_info_cmd, "xo_mapsettings_get_unbake_info", xo_mapsettings_get_unbake_info_fn, "This log names of drawn models in the current frame to a logfile in portal2-rtx/logs/. Useful for MapSettings : [UNBAKE]");
	
		log("ModelRender", "Module initialized.", utils::LOG_TYPE::LOG_TYPE_DEFAULT, false);
	}
}

