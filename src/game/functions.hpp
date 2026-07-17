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

	inline CRender* get_engine_renderer() { return reinterpret_cast<CRender*>(ENGINE_BASE + 0x601F00); }
	inline IDirect3DDevice9* get_d3d_device() { return reinterpret_cast<IDirect3DDevice9*>(*(DWORD*)(RENDERER_BASE + 0xD3EE8)); }
	inline IShaderAPIDX8* get_shaderapi() { return reinterpret_cast<IShaderAPIDX8*>(*(DWORD*)(RENDERER_BASE + 0xC9C50)); }
	inline IMaterialSystem* get_material_system() { return reinterpret_cast<IMaterialSystem*>(*(DWORD*)(CLIENT_BASE + 0x88B7F0)); }
	inline worldbrushdata_t* get_hoststate_worldbrush_data() { return reinterpret_cast<worldbrushdata_t*>(*(DWORD*)(ENGINE_BASE + 0x42FFB8)); }
	inline CCvar* get_icvar() { return reinterpret_cast<CCvar*>((VSTDLIB_BASE + 0x2C0D0)); }
	inline IVModelInfo* get_modelinfo() { return reinterpret_cast<IVModelInfo*>(*(DWORD*)(CLIENT_BASE + 0x735E58)); }

	extern ConVar* find_cvar(const char* name);
	extern const ConVar* find_cvar_const(const char* name);

	inline Vector* get_current_view_origin() { return reinterpret_cast<Vector*>(ENGINE_BASE + 0x501344); }
	inline Vector* get_current_view_forward() { return reinterpret_cast<Vector*>(ENGINE_BASE + 0x427A30); }
	inline Vector* get_current_view_right() { return reinterpret_cast<Vector*>(ENGINE_BASE + 0x427A3C); }
	inline Vector* get_current_view_up() { return reinterpret_cast<Vector*>(ENGINE_BASE + 0x427A48); }

	inline Vector* get_camera_forward_vector() { return reinterpret_cast<Vector*>(CLIENT_BASE + 0x7A2638); } // same as engine one above?

	// returns C_BaseAnimating class pointer for a given IClientRenderable
	C_BaseAnimating* get_base_animating_for_client_renderable(IClientRenderable* pRenderable);

	namespace namespaces
	{
		namespace C_BaseAnimating
		{
			// returns bone matrix for given bone index
			/// @param this_ptr			C_BaseAnimating ptr
			/// @param bone				bone index
			/// @param boneToWorld		out bone matrix
			inline void GetBoneTransform(void* this_ptr, const int bone, matrix3x4_t* boneToWorld)
			{
				// 55 8B EC 56 8B F1 83 BE ? ? ? ? ? 57 75 ? 8B 46 ? 8B 50 ? 8D 4E ? FF D2 85 C0 74 ? 8B CE E8 ? ? ? ? 8B 86
				utils::hook::call<void(__fastcall)(void* this_ptr, void* null, int bone, matrix3x4_t* boneToWorld)>(CLIENT_BASE + 0x331C0)
					(this_ptr, nullptr, bone, boneToWorld);
			}

			// returns bone index for given bone name
			/// @param this_ptr			C_BaseAnimating ptr
			/// @param bone_name		bone name
			/// @return					bone index
			inline int LookupBone(void* this_ptr, const char* bone_name)
			{
				//xref "doorhandlebone"
				return utils::hook::call<int(__fastcall)(void* this_ptr, void* null, const char* bone_name)>(CLIENT_BASE + 0x2F380)
					(this_ptr, nullptr, bone_name);
			}

			// returns CStudioHdr pointer for given C_BaseAnimating pointer
			/// @param this_ptr			C_BaseAnimating ptr
			/// @return					CStudioHdr ptr
			inline CStudioHdr* GetModelPtr(void* this_ptr)
			{
				// 56 8B F1 83 BE ? ? ? ? ? 75 ? 8B 46 ? 8B 50 ? 8D 4E ? FF D2 85 C0 74 ? 8B CE E8 ? ? ? ? 8B 86 ? ? ? ? 5E 85 C0 74 ? ? ? ? 75 ? 33 C0 C3
				return utils::hook::call<CStudioHdr * (__fastcall)(void* this_ptr, void* null)>(CLIENT_BASE + 0x2140)
					(this_ptr, nullptr);
			}
		}
	}

	inline int get_visframecount() { return *reinterpret_cast<int*>(ENGINE_BASE + 0x6AFDD8); }
	inline view_id get_viewid() { return *reinterpret_cast<view_id*>(CLIENT_BASE + 0x6DF6CC); }

	extern void con_add_command(ConCommand* cmd, const char* name, void(__cdecl* callback)(), const char* desc);
	extern void debug_add_text_overlay(const float* pos, float duration, const char* text);
	extern void debug_add_text_overlay(const float* pos, const char* text, int line_offset = 0, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
	extern void cbaseentity_remove(void* cbaseentity_ptr);

	extern void cvar_uncheat(const char* name);
	extern void cvar_uncheat_and_set_int(const char* name, int val);
	extern void cvar_uncheat_and_set_float(const char* name, float val);

	extern void print_ingame(const char* msg, ...);

	// CM_PointLeafnum
	inline int get_leaf_from_position(const Vector& pos) { return utils::hook::call<int(__cdecl)(const float*)>(ENGINE_BASE + 0x14B130)(&pos.x); }

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
			utils::log("Functions >", "Failed to create hook for OutputDebugStringA", utils::LOG_TYPE::LOG_TYPE_ERROR);
			return;
		}

		if (MH_CreateHook(&OutputDebugStringW, &HookedOutputDebugStringW, reinterpret_cast<LPVOID*>(&OriginalOutputDebugStringW)) != MH_OK) 
		{
			utils::log("Functions >", "Failed to create hook for OutputDebugStringW", utils::LOG_TYPE::LOG_TYPE_ERROR);
			return;
		}

		if (MH_EnableHook(&OutputDebugStringA) != MH_OK || MH_EnableHook(&OutputDebugStringW) != MH_OK) 
		{
			utils::log("Functions >", "Failed to enable hooks for OutputDebugStringA & OutputDebugStringW", utils::LOG_TYPE::LOG_TYPE_ERROR);
			return;
		}
	}
}
