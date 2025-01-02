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
	extern HWND main_window;
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

	inline CRender* get_engine_renderer() { return reinterpret_cast<CRender*>(ENGINE_BASE + 0x601F00); }
	inline IDirect3DDevice9* get_d3d_device() { return reinterpret_cast<IDirect3DDevice9*>(*(DWORD*)(RENDERER_BASE + 0xD3EE8)); }
	inline IShaderAPIDX8* get_shaderapi() { return reinterpret_cast<IShaderAPIDX8*>(*(DWORD*)(RENDERER_BASE + 0xC9C50)); }
	inline IMaterialSystem* get_material_system() { return reinterpret_cast<IMaterialSystem*>(*(DWORD*)(CLIENT_BASE + 0x88B7F0)); }
	inline worldbrushdata_t* get_hoststate_worldbrush_data() { return reinterpret_cast<worldbrushdata_t*>(*(DWORD*)(ENGINE_BASE + 0x42FFB8)); }
	inline CCvar* get_icvar() { return reinterpret_cast<CCvar*>((VSTDLIB_BASE + 0x2C0D0)); }

	inline Vector* get_current_view_origin() { return reinterpret_cast<Vector*>(ENGINE_BASE + 0x501344); }
	inline Vector* get_current_view_forward() { return reinterpret_cast<Vector*>(ENGINE_BASE + 0x427A30); }
	inline Vector* get_current_view_right() { return reinterpret_cast<Vector*>(ENGINE_BASE + 0x427A3C); }
	inline Vector* get_current_view_up() { return reinterpret_cast<Vector*>(ENGINE_BASE + 0x427A48); }

	inline int get_visframecount() { return *reinterpret_cast<int*>(ENGINE_BASE + 0x6AFDD8); }
	inline view_id get_viewid() { return *reinterpret_cast<view_id*>(CLIENT_BASE + 0x6DF6CC); }

	extern void con_add_command(ConCommand* cmd, const char* name, void(__cdecl* callback)(), const char* desc);
	extern void debug_add_text_overlay(const float* pos, float duration, const char* text);
	extern void debug_add_text_overlay(const float* pos, const char* text, int line_offset = 0, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
	extern void cbaseentity_remove(void* cbaseentity_ptr);

	extern void cvar_uncheat(const char* name);
	extern void cvar_uncheat_and_set_int(const char* name, int val);
	extern void cvar_uncheat_and_set_float(const char* name, float val);


	// CM_PointLeafnum
	inline int get_leaf_from_position(const Vector& pos) { return utils::hook::call<int(__cdecl)(const float*)>(ENGINE_BASE + 0x14B130)(&pos.x); }

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
