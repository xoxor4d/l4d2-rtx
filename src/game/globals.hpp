#pragma once

namespace glob
{
	extern D3DXMATRIX IDENTITY;
	extern D3DXMATRIX TC_TRANSLATE_TO_CENTER;
	extern D3DXMATRIX TC_TRANSLATE_FROM_CENTER_TO_TOP_LEFT;

	extern std::string root_path;
	extern HWND main_window;

#define EXE_BASE shared::globals::exe_module_addr

	extern HMODULE exe_hmodule;
	extern DWORD exe_module_addr;
	extern DWORD exe_size;
	extern void setup_exe_module();

	extern HMODULE dll_hmodule;
	extern DWORD dll_module_addr;
	extern void setup_dll_module(const HMODULE mod);

	extern void setup_homepath();

	extern IDirect3DDevice9* d3d_device;
	extern IDirect3D9* d3d9_interface;

	extern bool imgui_is_rendering;
	extern bool imgui_menu_open;
	extern bool imgui_allow_input_bypass;
	extern bool imgui_wants_text_input;
	extern uint32_t imgui_allow_input_bypass_timeout;

	extern std::chrono::high_resolution_clock::time_point last_frame_time;
	extern float frame_time_ms;

	extern bool detoured_warning_fn_origin;
	extern bool detoured_msg_fn_origin;
}