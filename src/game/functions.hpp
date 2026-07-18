#pragma once

#define RENDERER_MOD			game::shaderapidx9_module
#define STUDIORENDER_MOD		game::studiorender_module
//#define MATERIALSTYSTEM_MOD	game::materialsystem_module
#define ENGINE_MOD				game::engine_module
#define CLIENT_MOD				game::client_module
#define SERVER_MOD				game::server_module
#define VSTDLIB_MOD			game::vstdlib_module

#define RENDERER_BASE			game::shaderapidx9_module.handle
#define STUDIORENDER_BASE		game::studiorender_module.handle
//#define MATERIALSTYSTEM_BASE	game::materialsystem_module.handle
#define ENGINE_BASE				game::engine_module.handle
#define CLIENT_BASE				game::client_module.handle
#define SERVER_BASE				game::server_module.handle
#define VSTDLIB_BASE			game::vstdlib_module.handle

#include "utils/hooking.hpp"

using namespace components;

namespace game
{
	extern std::vector<std::string> loaded_modules;
	extern std::string root_path;

	extern utils::mem::module_info shaderapidx9_module;
	extern utils::mem::module_info studiorender_module;
	extern utils::mem::module_info materialsystem_module;
	extern utils::mem::module_info engine_module;
	extern utils::mem::module_info client_module;
	extern utils::mem::module_info server_module;
	extern utils::mem::module_info vstdlib_module;

	extern const D3DXMATRIX IDENTITY;
	extern const D3DXMATRIX TC_TRANSLATE_TO_CENTER;
	extern const D3DXMATRIX TC_TRANSLATE_FROM_CENTER_TO_TOP_LEFT;

	//inline CCvar* get_icvar() { return reinterpret_cast<CCvar*>((VSTDLIB_BASE + 0x2C0D0)); }

	extern ConVar* find_cvar(const char* name);
	extern const ConVar* find_cvar_const(const char* name);

	// returns C_BaseAnimating class pointer for a given IClientRenderable
	C_BaseAnimating* get_base_animating_for_client_renderable(IClientRenderable* pRenderable);

	extern void con_add_command(ConCommand* cmd, const char* name, void(__cdecl* callback)(), const char* desc);
	extern void debug_add_text_overlay(const float* pos, float duration, const char* text);
	extern void debug_add_text_overlay(const float* pos, const char* text, int line_offset = 0, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
	extern void cbaseentity_remove(void* cbaseentity_ptr);

	extern void cvar_uncheat(const char* name);
	extern void cvar_uncheat_and_set_int(const char* name, int val);
	extern void cvar_uncheat_and_set_float(const char* name, float val);

	extern void print_ingame(const char* msg, ...);

	// ::
	// debug print console redirects

	static void(WINAPI* OriginalOutputDebugStringA)(LPCSTR lpOutputString) = nullptr;
	static void(WINAPI* OriginalOutputDebugStringW)(LPCWSTR lpOutputString) = nullptr;

	inline void WINAPI HookedOutputDebugStringA(LPCSTR lpOutputString)
	{
		if (lpOutputString) 
		{
			utils::log(">", lpOutputString, utils::LOG_TYPE::LOG_TYPE_DEFAULT, false, false, true);
			fflush(stdout);
		}

		// og func
		if (OriginalOutputDebugStringA) {
			OriginalOutputDebugStringA(lpOutputString);
		}
	}

	inline void WINAPI HookedOutputDebugStringW(LPCWSTR lpOutputString)
	{
		if (lpOutputString) 
		{
			// Convert wide string to multibyte string for printf
			char buffer[1024];
			WideCharToMultiByte(CP_UTF8, 0, lpOutputString, -1, buffer, sizeof(buffer), NULL, NULL);

			utils::log(">", buffer, utils::LOG_TYPE::LOG_TYPE_DEFAULT, false, false, true);
			fflush(stdout); 
		}

		// og func
		if (OriginalOutputDebugStringW) {
			OriginalOutputDebugStringW(lpOutputString);
		}
	}

	inline void SetupDebugOutputHook()
	{
		if (MH_CreateHook(&OutputDebugStringA, &HookedOutputDebugStringA, reinterpret_cast<LPVOID*>(&OriginalOutputDebugStringA)) != MH_OK) 
		{
			utils::log("Functions", "Failed to create hook for OutputDebugStringA", utils::LOG_TYPE::LOG_TYPE_ERROR);
			return;
		}

		if (MH_CreateHook(&OutputDebugStringW, &HookedOutputDebugStringW, reinterpret_cast<LPVOID*>(&OriginalOutputDebugStringW)) != MH_OK) 
		{
			utils::log("Functions", "Failed to create hook for OutputDebugStringW", utils::LOG_TYPE::LOG_TYPE_ERROR);
			return;
		}

		if (MH_EnableHook(&OutputDebugStringA) != MH_OK || MH_EnableHook(&OutputDebugStringW) != MH_OK) 
		{
			utils::log("Functions", "Failed to enable hooks for OutputDebugStringA & OutputDebugStringW", utils::LOG_TYPE::LOG_TYPE_ERROR);
			return;
		}
	}
}
