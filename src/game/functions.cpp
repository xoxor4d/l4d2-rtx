#include "std_include.hpp"

namespace glob
{
	bool spawned_external_console = false;
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

	void cvar_uncheat(const char* name)
	{
		if (const auto ivar = game::get_icvar(); ivar)
		{
			if (auto var = ivar->vftable->FindVar(ivar, name); var) {
				var->m_nFlags &= ~0x4000;
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
				var->m_nFlags &= ~0x4000;
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
				var->m_nFlags &= ~0x4000;
			}
		}
	}
}
