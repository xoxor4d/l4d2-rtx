#pragma once

#define RENDERER_BASE			game::shaderapidx9_module
#define STUDIORENDER_BASE		game::studiorender_module
#define MATERIALSTYSTEM_BASE	game::materialsystem_module
#define ENGINE_BASE				game::engine_module
#define CLIENT_BASE				game::client_module
#define SERVER_BASE				game::server_module
#define VSTDLIB_BASE			game::vstdlib_module

using namespace components;

namespace glob
{
	extern bool spawned_external_console;
}

namespace game
{
	extern std::vector<std::string> loaded_modules;
	extern std::string root_path;
	extern DWORD shaderapidx9_module;
	extern DWORD studiorender_module;
	extern DWORD materialsystem_module;
	extern DWORD engine_module;
	extern DWORD client_module;
	extern DWORD server_module;
	extern DWORD vstdlib_module;

	extern const D3DXMATRIX IDENTITY;
	extern const D3DXMATRIX TC_TRANSLATE_TO_CENTER;
	extern const D3DXMATRIX TC_TRANSLATE_FROM_CENTER_TO_TOP_LEFT;

	inline components::CRender* get_engine_renderer() { return reinterpret_cast<components::CRender*>(ENGINE_BASE + 0x601F00); }
	inline IDirect3DDevice9* get_d3d_device() { return reinterpret_cast<IDirect3DDevice9*>(*(DWORD*)(RENDERER_BASE + 0xD3EE8)); }
	inline components::IShaderAPIDX8* get_shaderapi() { return reinterpret_cast<components::IShaderAPIDX8*>(*(DWORD*)(RENDERER_BASE + 0xC9C50)); }
	inline components::IMaterialSystem* get_material_system() { return reinterpret_cast<components::IMaterialSystem*>(*(DWORD*)(CLIENT_BASE + 0x88B7F0)); }
	inline components::CCvar* get_icvar() { return reinterpret_cast<components::CCvar*>((VSTDLIB_BASE + 0x2C0D0)); }

	extern void cvar_uncheat(const char* name);
	extern void cvar_uncheat_and_set_int(const char* name, int val);
	extern void cvar_uncheat_and_set_float(const char* name, float val);

	/**
	 * Creates an external console
	 */
	inline void console()
	{
		if (!glob::spawned_external_console)
		{
			glob::spawned_external_console = true;
			setvbuf(stdout, nullptr, _IONBF, 0);
			if (AllocConsole())
			{
				FILE* file = nullptr;
				freopen_s(&file, "CONIN$", "r", stdin);
				freopen_s(&file, "CONOUT$", "w", stdout);
				freopen_s(&file, "CONOUT$", "w", stderr);
				SetConsoleTitleA("L4D2-RTX Debug Console");
			}
		}
	}
}
