#include "std_include.hpp"

namespace components
{
	// > scene_ent_on_start_event_hk && scene_ent_on_finish_event_hk can be used to detect when choreography's (vcd's) start & end
	// > use cmd 'xo_scene_print' or cvar 'scene_print' to get event names

	namespace cmd
	{
		bool scene_print = false;
	}

	// called from main_module::on_renderview()
	void choreo_events::on_client_frame()
	{
		//if (map_settings::is_level.c1m1_hotel)
		//{
		//	if (ev_sample.has_elapsed(5.0f))
		//	{
		//		// do something
		//		ev_sample.reset();
		//	}
		//}
	}

	// #
	// #

	void handle_confvar_transition(const std::string_view& sname, const std::string_view& actor, const std::string_view& event, const std::string_view& param1, const bool is_start)
	{
		auto& map_settings = map_settings::get_map_settings();
		for (auto t = map_settings.remix_transitions.begin(); t != map_settings.remix_transitions.end();)
		{
			// only handle choreo transitions
			if (t->trigger_type != map_settings::TRANSITION_TRIGGER_TYPE::CHOREO) {
				++t; continue;
			}

			bool iterpp = false;

			const bool mode = is_start
				? (t->mode == map_settings::TRANSITION_MODE::ONCE_ON_ENTER || t->mode == map_settings::TRANSITION_MODE::ALWAYS_ON_ENTER)
				: (t->mode == map_settings::TRANSITION_MODE::ONCE_ON_LEAVE || t->mode == map_settings::TRANSITION_MODE::ALWAYS_ON_LEAVE);

			if (mode)
			{
				if (sname.contains(t->choreo_name))
				{
					// check if opt. actor is defined and matches event actor
					if (!t->choreo_actor.empty() && !actor.contains(t->choreo_actor)) {
						++t; continue;
					}

					// check if opt. event is defined and matches event string
					if (!t->choreo_event.empty() && !event.contains(t->choreo_event)) {
						++t; continue;
					}

					// check if opt. param1 is defined and matches event param1
					if (!t->choreo_param1.empty() && !param1.contains(t->choreo_param1)) {
						++t; continue;
					}

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

						if (t->mode <= map_settings::TRANSITION_MODE::ONCE_ON_LEAVE)
						{
							t = map_settings.remix_transitions.erase(t);
							iterpp = true; // erase returns the next iterator
						}
					}
				}
			}

			if (!iterpp) {
				++t;
			}
		}
	}

	// 'CSceneEntity::StartEvent' :: triggered on event start
	void scene_ent_on_start_event_hk([[maybe_unused]] const CChoreoEvent* ev)
	{
		if (ev && ev->m_Name.string && ev->m_pScene)
		{
			if (std::string_view(ev->m_Name.string) != "NULL")
			{
				const char* actor_str = ev->m_pActor && ev->m_pActor->m_szName[0] ? ev->m_pActor->m_szName : "";
				const char* event_str = ev->m_Name.string ? ev->m_Name.string : "";
				const char* param1_str = ev->m_Parameters.string ? ev->m_Parameters.string : "";

				// ...
				std::string forward_slashes = ev->m_pScene->m_szFileName;
				utils::replace_all(forward_slashes, "\\", "/");

				if (cmd::scene_print)
				{
					game::print_ingame(
						"[SCENE] [Start] VCD: %s ~~~~ ACTOR: %s ~~~~ EV: %s ~~~~ PARM1: %s\n", 
						forward_slashes.c_str(),
						actor_str ? actor_str : "UNUSED", 
						event_str, param1_str ? param1_str : "UNUSED");
				}

				remix_lights::on_event_start(forward_slashes, actor_str, event_str, param1_str);

				// handle remix config transitions added via mapsettings
				handle_confvar_transition(forward_slashes, actor_str, event_str, param1_str, true);
			}
		}
	}

	HOOK_RETN_PLACE_DEF(scene_ent_on_start_event_retn);
	__declspec(naked) void scene_ent_on_start_event_stub()
	{
		__asm
		{
			// og
			mov     edi, [ebp + 0x10];
			mov     esi, ecx;

			pushad;
			push    edi;
			call	scene_ent_on_start_event_hk;
			add		esp, 4;
			popad;

			// og
			jmp		scene_ent_on_start_event_retn;
		}
	}

	// #
	// #

	// 'CSceneEntity::OnSceneFinished' :: triggered right after the audio stops - ignores postdelay
	void scene_ent_on_finish_event_hk(/*CChoreoSceneNew* scene,*/ const char* scene_name)
	{
		if (scene_name)
		{
			if (cmd::scene_print)
			{
				game::print_ingame(
					"[SCENE]   [End] VCD:     %s\n", scene_name);
			}

			const auto sname = std::string_view(scene_name);
			remix_lights::on_event_finish(sname);

			/*if (map_settings::is_level.c1m1_hotel)
			{
				if (sname.ends_with("choreoname.vcd")) {
					choreo_events::ev_sample.trigger();
				}
			}*/

			// ------

			// handle remix config transitions added via mapsettings
			handle_confvar_transition(sname, "", "", "", false);
			
		}
	}

	HOOK_RETN_PLACE_DEF(scene_ent_on_finish_event_retn);
	__declspec(naked) void scene_ent_on_finish_event_stub()
	{
		__asm
		{
			// og
			mov     eax, [esi + 0x430]; // CSceneEntity->m_iszSceneFile
			//mov		ecx, [esi + 0x4DC]; // scene

			pushad;
			push    eax;
			//push	ecx;
			call	scene_ent_on_finish_event_hk;
			add		esp, 4;
			popad;

			// og
			jmp		scene_ent_on_finish_event_retn;
		}
	}

	// #
	// #

	ConCommand xo_debug_scene_print_cmd {};
	void xo_debug_scene_print_fn()
	{
		cmd::scene_print = !cmd::scene_print;
	}

	// #
	// #

	choreo_events::choreo_events()
	{
		p_this = this;

		// CSceneEntity::StartEvent :: : can be used to detect the start of scene (vcd) entities
		utils::hook(SERVER_BASE + 0x1C0765, scene_ent_on_start_event_stub).install()->quick(); // 2001
		HOOK_RETN_PLACE(scene_ent_on_start_event_retn, SERVER_BASE + 0x1C076A); // 2001

		// CSceneEntity::OnSceneFinished
		utils::hook::nop(SERVER_BASE + 0x1C7813, 6); // 2001
		utils::hook(SERVER_BASE + 0x1C7813, scene_ent_on_finish_event_stub).install()->quick(); // 2001
		HOOK_RETN_PLACE(scene_ent_on_finish_event_retn, SERVER_BASE + 0x1C7819); // 2001

		// ----
		game::con_add_command(&xo_debug_scene_print_cmd, "xo_debug_scene_print", xo_debug_scene_print_fn, "Print choreography (vcd) infos (similar to scene_info cvar but only showing relevant data)");
	}
}
