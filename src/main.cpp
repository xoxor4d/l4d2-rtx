#include "std_include.hpp"

#include <Psapi.h>
#include <wincrypt.h>

std::unordered_set<HWND> wnd_class_list; // so we don't print the same window strings over and over again

namespace l4d2
{
	BOOL CALLBACK enum_windows_proc(HWND hwnd, LPARAM lParam)
	{
		DWORD window_pid, target_pid = static_cast<DWORD>(lParam);
		GetWindowThreadProcessId(hwnd, &window_pid);

		if (window_pid == target_pid && IsWindowVisible(hwnd))
		{
			char class_name[256];
			GetClassNameA(hwnd, class_name, sizeof(class_name));

			if (!wnd_class_list.contains(hwnd))
			{
				char debug_msg[256];
				wsprintfA(debug_msg, "> HWND: %p, PID: %u, Class: %s, Visible: %d \n", hwnd, window_pid, class_name, IsWindowVisible(hwnd));
				utils::log("Main", debug_msg, utils::LOG_TYPE::LOG_TYPE_DEFAULT, false);
				wnd_class_list.insert(hwnd);
			}

			if (std::string_view(class_name).contains("Valve001"s))
			{
				glob::main_window = hwnd;
				return FALSE;
			}
		}

		return TRUE;
	}


/*#define GET_MODULE_HANDLE(HANDLE_OUT, NAME, T) \
	while (!(HANDLE_OUT)) { \
		if ((HANDLE_OUT) = (DWORD)GetModuleHandleA(NAME); !(HANDLE_OUT)) { \
			Sleep(100); (T) += 100u; \
			if ((T) >= 30000) { \
				utils::log("Main", "Failed to find module: "s + (NAME) + "\n"s); \
				return TRUE; \
			} \
		} \
	}*/

	BOOL get_module_handle_and_size(utils::mem::module_info& module_info, LPCSTR name, uint32_t& timeout)
	{
		while (!module_info.handle)
		{
			HMODULE module = GetModuleHandleA(name);
			if (module)
			{
				MODULEINFO info{};
				if (!GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info)))
				{
					utils::log("Main", std::format("Failed to get module information for {}. Error: (0x{:X})", name ? name : "<exe>", GetLastError()));
					return TRUE;
				}

				module_info.handle = reinterpret_cast<DWORD>(module);
				module_info.size = info.SizeOfImage;
				module_info.name = name;
				return FALSE;
			}

			Sleep(100);
			timeout += 100;

			if (timeout >= 30000)
			{
				utils::log("Main", "Failed to find module: "s + (name ? name : "<exe>"));
				return TRUE;
			}
		}

		return FALSE;
	}

	DWORD WINAPI find_game_window_by_class([[maybe_unused]] LPVOID lpParam)
	{
		std::uint32_t T = 0;

		char exe_path[MAX_PATH]; GetModuleFileNameA(nullptr, exe_path, MAX_PATH);

		utils::log("Main", "Waiting for window with classname containing 'Valve001'...", utils::LOG_TYPE::LOG_TYPE_DEFAULT, false);
		{
			while (!glob::main_window)
			{
				EnumWindows(enum_windows_proc, static_cast<LPARAM>(GetCurrentProcessId()));
				if (!glob::main_window) {
					Sleep(1u); T += 1u;
				}

				if (T >= 30000)
				{
					Beep(300, 100); Sleep(100); Beep(200, 100);
					utils::log("Main", "Could not find 'Valve001' Window. Not loading RTX Compatibility Mod.", utils::LOG_TYPE::LOG_TYPE_ERROR, true);
					return TRUE;
				}
			}
		}

		if (!utils::flags::has_flag("nobeep")) {
			Beep(523, 100);
		}

		get_module_handle_and_size(game::shaderapidx9_module, "shaderapidx9.dll", T);
		get_module_handle_and_size(game::studiorender_module, "studiorender.dll", T);
		//get_module_handle_and_size(game::materialsystem_module, "materialsystem.dll", T);
		get_module_handle_and_size(game::engine_module, "engine.dll", T);
		get_module_handle_and_size(game::client_module, "client.dll", T);
		get_module_handle_and_size(game::server_module, "server.dll", T);
		get_module_handle_and_size(game::vstdlib_module, "vstdlib.dll", T);

		// Wait a little ..
		Sleep(50u);

		l4d2::main();
		return 0;
	}
}

BOOL APIENTRY DllMain(HMODULE hmodule, const DWORD ul_reason_for_call, LPVOID)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) 
	{
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

		utils::console();
		glob::setup_dll_module(hmodule);
		glob::setup_exe_module();
		glob::setup_homepath();

		utils::set_console_color_blue(true);
		std::cout << "Launching L4D2 RTX Remix Compatiblity Mod Version [" << COMP_MOD_VERSION_MAJOR << "." << COMP_MOD_VERSION_MINOR << "." << COMP_MOD_VERSION_PATCH << "]";

		if constexpr (COMP_MOD_PRE_RELEASE_NUM != 0) {
			std::cout << " - Pre-Release " << std::to_string(COMP_MOD_PRE_RELEASE_NUM) << "\n";
		}
		else {
			std::cout << "\n";
		}

		std::cout << "> Compiled On : " + std::string(__DATE__) + " " + std::string(__TIME__) + "\n";
		std::cout << "> https://github.com/xoxor4d/l4d2-rtx\n\n";
		utils::set_console_color_default();

		if (const auto MH_INIT_STATUS = MH_Initialize(); MH_INIT_STATUS != MH_STATUS::MH_OK)
		{
			log("Main", std::format("MinHook failed to initialize with code: {:d}", static_cast<int>(MH_INIT_STATUS)), utils::LOG_TYPE::LOG_TYPE_ERROR, true);
			return TRUE;
		}

#if DEBUG
		// hook OutputDebugString
		game::SetupDebugOutputHook();
#endif

		// init early modules here <> (can not place hooks directly)

		if (const auto t = CreateThread(nullptr, 0, l4d2::find_game_window_by_class, nullptr, 0, nullptr); t) {
			CloseHandle(t);
		}
	}

	return TRUE;
}
