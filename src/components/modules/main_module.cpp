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



	struct CViewRender
	{
		char pad[0x3E8];
		FadeData_t m_FadeData;
	};

	// cpu_level + 1
	FadeData_t g_aFadeData[] =
	{
		// pxmin  pxmax    width   scale
		{  0.0f,   0.0f,  1280.0f,  1.0f },
		{ 10.0f,  15.0f,   800.0f,  1.0f },
		{  5.0f,  10.0f,  1024.0f,  1.0f },
		{  0.0f,   0.0f,  1280.0f,  0.1f },
		{ 36.0f, 144.0f,   720.0f,  1.0f },
		{  0.0f,   0.0f,  1280.0f,  1.0f },
	};

	void init_fade_data_hk(CViewRender* view_render)
	{
		view_render->m_FadeData = g_aFadeData[3];
	}

	HOOK_RETN_PLACE_DEF(init_fade_data_retn);
	__declspec(naked) void init_fade_data_stub()
	{
		__asm
		{
			push	esi; // CViewRender (this)
			call	init_fade_data_hk;
			add		esp, 4;
			jmp		init_fade_data_retn;
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
		game::cvar_uncheat_and_set_int("r_3dsky", 1);
		game::cvar_uncheat_and_set_int("r_skybox_draw_last", 0);
		game::cvar_uncheat_and_set_int("r_flashlightrender", 0); // fix emissive "bug" when moon portal opens on finale4 
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


	main_module::main_module()
	{
		{ // init filepath var
			char path[MAX_PATH]; GetModuleFileNameA(nullptr, path, MAX_PATH);
			game::root_path = path; utils::erase_substring(game::root_path, "left4dead2.exe");
		}

		// CViewRender::RenderView :: "start" of current frame (after CViewRender::DrawMonitors)
		utils::hook(CLIENT_BASE + 0x1D7113, cviewrenderer_renderview_stub).install()->quick();
		HOOK_RETN_PLACE(cviewrenderer_renderview_retn, CLIENT_BASE + 0x1D7118);

		// CViewRender::InitFadeData :: manually set fade data and not rely on cpu_level
		/*utils::hook(CLIENT_BASE + 0x1DFC33, init_fade_data_stub, HOOK_JUMP).install()->quick();
		HOOK_RETN_PLACE(init_fade_data_retn, CLIENT_BASE + 0x1DFC59);*/
	}

	main_module::~main_module()
	{
	}
}
