#include "std_include.hpp"

namespace glob
{
	bool spawned_external_console = false;
	HWND main_window = nullptr;
}

namespace game
{
	std::vector<std::string> loaded_modules;
	std::string root_path;
	DWORD shaderapidx9_module = 0u;
	DWORD studiorender_module = 0u;
	DWORD materialsystem_module = 0u;
	DWORD engine_module = 0u;
	DWORD client_module = 0u;
	DWORD server_module = 0u;
	DWORD vstdlib_module = 0u;

	const D3DXMATRIX IDENTITY =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	const D3DXMATRIX TC_TRANSLATE_TO_CENTER =
	{
		 1.0f,  0.0f, 0.0f, 0.0f,	// identity
		 0.0f,  1.0f, 0.0f, 0.0f,	// identity
		 0.0f,  0.0f, 1.0f, 0.0f,	// identity
		-0.5f, -0.5f, 0.0f, 1.0f,	// translate to center
	};

	const D3DXMATRIX TC_TRANSLATE_FROM_CENTER_TO_TOP_LEFT =
	{
		1.0f, 0.0f, 0.0f, 0.0f,	// identity
		0.0f, 1.0f, 0.0f, 0.0f,	// identity
		0.0f, 0.0f, 1.0f, 0.0f,	// identity
		0.5f, 0.5f, 0.0f, 1.0f,	// translate back to the top left corner
	};

	// adds a simple console command
	void con_add_command(ConCommand* cmd, const char* name, void(__cdecl* callback)(), const char* desc)
	{
		// ConCommand *this, const char *pName, void (__cdecl *callback)(), const char *pHelpString, int flags, int (__cdecl *completionFunc)(const char *, char (*)[64]
		utils::hook::call<void(__fastcall)(ConCommand* this_ptr, void* null, const char*, void(__cdecl*)(), const char*, int, int(__cdecl*)(const char*, char(*)[64]))>(CLIENT_BASE + 0x3D36F0)
			(cmd, nullptr, name, callback, desc, 0x20000, nullptr);
	}

	/**
	 * Calls CDebugOverlay::AddTextOverlay
	 * @param pos		Position of text in 3D Space
	 * @param duration	Duration in which text is visible - use 0.0f for per frame stuff
	 * @param text		The text
	 */
	void debug_add_text_overlay(const float* pos, float duration, const char* text)
	{
		utils::hook::call<void(__cdecl)(const float*, float, const char*)>(ENGINE_BASE + 0xA8620)
			(pos, duration, text);
	}

	/**
	 * Calls CDebugOverlay::AddTextOverlay
	 * @param pos			Position of text in 3D Space
	 * @param text			The text
	 * @param line_offset	Offset text position
	 * @param r				red (0-1)
	 * @param g				green (0-1)
	 * @param b				blue (0-1)
	 * @param a				alpha (0-1)
	 */
	void debug_add_text_overlay(const float* pos, const char* text, const int line_offset, const float r, const float g, const float b, const float a)
	{
		utils::hook::call<void(__cdecl)(const float*, int, float, float, float, float, float, const char*)>(ENGINE_BASE + 0xA8AA0)
			(pos, line_offset, 0.0f, r, g, b, a, text);
	}

	// remove/destroy a given CBaseEntity
	void cbaseentity_remove(void* cbaseentity_ptr)
	{
		if (cbaseentity_ptr)
		{
			// UTIL_Remove
			utils::hook::call<void(__cdecl)(void* cbaseentity)>(SERVER_BASE + 0x2071E0)(cbaseentity_ptr); // #OFFS 2501
		}
	}

	void cvar_uncheat(const char* name)
	{
		if (const auto ivar = game::get_icvar(); ivar)
		{
			if (auto var = ivar->vftable->FindVar(ivar, name); var) 
			{
				var->m_nFlags &= ~(1 << 1); // FCVAR_DEVELOPMENTONLY
				var->m_nFlags &= ~(1 << 4); // FCVAR_HIDDEN
				var->m_nFlags &= ~(1 << 14); // FCVAR_CHEAT
			}
		}
	}

	void cvar_uncheat_and_set_int(const char* name, const int val)
	{
		if (const auto ivar = game::get_icvar(); ivar)
		{
			if (auto var = ivar->vftable->FindVar(ivar, name); var)
			{
				var->vtbl->SetValue_Int(var, val);
				var->m_nFlags &= ~(1 << 1); // FCVAR_DEVELOPMENTONLY
				var->m_nFlags &= ~(1 << 4); // FCVAR_HIDDEN
				var->m_nFlags &= ~(1 << 14); // FCVAR_CHEAT
			}
		}
	}

	void cvar_uncheat_and_set_float(const char* name, const float val)
	{
		if (const auto ivar = game::get_icvar(); ivar)
		{
			if (auto var = ivar->vftable->FindVar(ivar, name); var)
			{
				var->vtbl->SetValue_Float(var, val);
				var->m_nFlags &= ~(1 << 1); // FCVAR_DEVELOPMENTONLY
				var->m_nFlags &= ~(1 << 4); // FCVAR_HIDDEN
				var->m_nFlags &= ~(1 << 14); // FCVAR_CHEAT
			}
		}
	}
}
