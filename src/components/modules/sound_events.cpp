#include "std_include.hpp"

namespace components
{
	namespace cmd
	{
		bool sound_debug_printing = false;
	}

	// each of these stands for something .. that we don't care about
	char* skip_sound_chars(const char* pch)
	{
		auto str = (char*)pch;
		while (true)
		{
			if (*str != '*' && *str != '?' && *str != '!' && *str != '#' && *str != '@' && *str != '(' && 
				*str != '>' && *str != '<' && *str != '^' && *str != ')' && *str != '}' && *str != '$') 
			{
				break;
			} str++;
		}
		return str;
	}
	
	void on_start_sound_hk(const StartSoundParams_t* parms)
	{
		if (parms->pSfx) 
		{
			char buff[264];
	
			if (const char* sound_name = skip_sound_chars(parms->pSfx->vftable->getname(parms->pSfx, buff, 260u)); 
				sound_name)
			{
				// check if we need to hash sounds
				bool lights_use_hash = map_settings::get_map_settings().using_any_light_sound_hash;
				bool transition_use_hash = map_settings::get_map_settings().using_any_transition_sound_hash;
				bool transition_use_name = map_settings::get_map_settings().using_any_transition_sound_name;

				std::string forward_slashes = sound_name;
				utils::replace_all(forward_slashes, "\\", "/");

				uint32_t hash = 0u;
				if (lights_use_hash || transition_use_hash || cmd::sound_debug_printing)
				{
					hash = utils::hash32_combine(hash, sound_name);
					hash = utils::hash32_combine(hash, parms->delay);
					hash = utils::hash32_combine(hash, parms->fvol);
					hash = utils::hash32_combine(hash, parms->origin.x);
					hash = utils::hash32_combine(hash, parms->origin.y);
					hash = utils::hash32_combine(hash, parms->origin.z);

					if (cmd::sound_debug_printing) 
					{
						game::print_ingame("[sound_hk] HASH: ( 0x%x ) -- %s -- delay: %.2f -- vol: %.2f -- origin: [%.5f %.5f %.5f] @ time: %.2f\n",
							hash, !forward_slashes.empty() ? forward_slashes.c_str() : "NULL", parms->delay, parms->fvol,
							parms->origin.x, parms->origin.y, parms->origin.z, interfaces::get()->m_globals->curtime);
					}
				}

				if (lights_use_hash) {
					remix_lights::on_sound_start(hash);
				}

				if (transition_use_hash || transition_use_name) {
					remix_vars::on_sound_start(hash, forward_slashes);
				}
			}
		}
	}

	HOOK_RETN_PLACE_DEF(on_start_sound_stub_retn);
	__declspec(naked) void on_start_sound_stub()
	{
		__asm
		{
			pushad;
			push	eax;
			call	on_start_sound_hk;
			add		esp, 4;
			popad;
	
			// og
			push    ebx;
			push    esi;
			xor		ecx, ecx;
			push    edi;
			jmp		on_start_sound_stub_retn;
		}
	}

	ConCommand xo_debug_sound_print_cmd {};
	void xo_debug_sound_print_fn()
	{
		cmd::sound_debug_printing = !cmd::sound_debug_printing;
	}

	sound_events::sound_events()
	{
		// S_StartSound
		utils::hook(ENGINE_BASE + 0x1C0B6, on_start_sound_stub).install()->quick(); // 2001
		HOOK_RETN_PLACE(on_start_sound_stub_retn, ENGINE_BASE + 0x1C0BB); // 2001

		// ----
		game::con_add_command(&xo_debug_sound_print_cmd, "xo_debug_sound_print", xo_debug_sound_print_fn, "Toggle sound debug prints (HASH for map_settings)");
	}
}