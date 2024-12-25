#include "std_include.hpp"

namespace components
{
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

	void main_module::force_cvars()
	{
		game::cvar_uncheat_and_set_int("r_staticprop_lod", 0);
		game::cvar_uncheat_and_set_int("r_lod", 0);
		game::cvar_uncheat_and_set_int("r_lod_switch_scale", 1); // hidden cvar
		

		game::cvar_uncheat_and_set_int("r_dopixelvisibility", 0); // hopefully fix random crash (dxvk cmdBindPipeline) on map load

		game::cvar_uncheat_and_set_int("r_WaterDrawRefraction", 0); // fix weird culling behaviour near water surfaces
		game::cvar_uncheat_and_set_int("r_WaterDrawReflection", 0); // perf?

		game::cvar_uncheat_and_set_int("r_ClipAreaFrustums", 0); // needed for R_CullNode Mode 2

		game::cvar_uncheat_and_set_int("r_threaded_particles", 0);
		game::cvar_uncheat_and_set_int("r_entityclips", 0);

		game::cvar_uncheat_and_set_int("cl_fastdetailsprites", 0);
		game::cvar_uncheat_and_set_int("cl_brushfastpath", 0);
		game::cvar_uncheat_and_set_int("cl_tlucfastpath", 0); // 
		game::cvar_uncheat_and_set_int("cl_modelfastpath", 0); // gain 4-5 fps on some act 4 maps but FF rendering not implemented
		game::cvar_uncheat_and_set_int("mat_queue_mode", 0); // does improve performance but breaks rendering
		game::cvar_uncheat_and_set_int("mat_softwarelighting", 0);
		game::cvar_uncheat_and_set_int("mat_parallaxmap", 0);
		game::cvar_uncheat_and_set_int("mat_frame_sync_enable", 0);
		game::cvar_uncheat_and_set_int("mat_dof_enabled", 0);
		game::cvar_uncheat_and_set_int("mat_displacementmap", 0);
		game::cvar_uncheat_and_set_int("mat_drawflat", 0);
		game::cvar_uncheat_and_set_int("mat_normalmaps", 0);
		game::cvar_uncheat_and_set_int("r_3dsky", 0);
		game::cvar_uncheat_and_set_int("r_skybox_draw_last", 0);
		game::cvar_uncheat_and_set_int("r_flashlightrender", 0); // fix emissive "bug" when moon portal opens on finale4 
		game::cvar_uncheat_and_set_int("mat_fullbright", 1);
		game::cvar_uncheat_and_set_int("mat_softwareskin", 1);
		game::cvar_uncheat_and_set_int("mat_phong", 1);
		game::cvar_uncheat_and_set_int("mat_fastnobump", 1);
		game::cvar_uncheat_and_set_int("mat_disable_bloom", 1);

		// graphic settings

		// lvl 0
		game::cvar_uncheat_and_set_int("cl_particle_fallback_base", 3); // 0 = render portalgun viewmodel effects
		game::cvar_uncheat_and_set_int("cl_particle_fallback_multiplier", 2);
		//game::cvar_uncheat_and_set_int("cl_footstep_fx", 0); // does not exist
		game::cvar_uncheat_and_set_int("cl_impacteffects_limit_general", 10);
		game::cvar_uncheat_and_set_int("cl_impacteffects_limit_exit", 3);
		game::cvar_uncheat_and_set_int("cl_impacteffects_limit_water", 2);
		game::cvar_uncheat_and_set_int("mat_depthfeather_enable", 0);
		game::cvar_uncheat_and_set_int("mat_force_vertexfog", 1); // does not exist

		game::cvar_uncheat_and_set_int("cl_detaildist", 1024);
		game::cvar_uncheat_and_set_int("cl_detailfade", 400);
		game::cvar_uncheat_and_set_int("r_drawmodeldecals", 1);
		game::cvar_uncheat_and_set_int("r_decalstaticprops", 1);
		game::cvar_uncheat_and_set_int("cl_player_max_decal_count", 32);
		game::cvar_uncheat_and_set_int("r_paintblob_force_single_pass", 1);
		game::cvar_uncheat_and_set_int("r_paintblob_max_number_of_threads", 1);
		game::cvar_uncheat_and_set_float("r_paintblob_highres_cube", 1.0);

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

		// CViewRender::RenderView :: "start" of current frame (after CViewRender::DrawMonitors)
		utils::hook(CLIENT_BASE + 0x1D7113, cviewrenderer_renderview_stub).install()->quick();
		HOOK_RETN_PLACE(cviewrenderer_renderview_retn, CLIENT_BASE + 0x1D7118);
	}

	main_module::~main_module()
	{
	}
}
