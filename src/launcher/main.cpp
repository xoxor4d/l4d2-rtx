#include <iostream>
#include <windows.h>
#include <string>
#include <filesystem>
#include <version.hpp> // git version

#include "detours.h" 
#pragma comment(lib, "detours.lib")

# define ENDL "\n"

bool find_window_by_process_id(const DWORD proc_id)
{
	HWND hwnd = FindWindowA(nullptr, nullptr); // start with the first window
	while (hwnd != nullptr)
	{
		DWORD pid;
		GetWindowThreadProcessId(hwnd, &pid);

		if (pid == proc_id) {
			return true;
		}

		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT); // move to the next window
	}

	return false;
} 

int wmain(int argc, wchar_t* argv[])
{
	std::filesystem::path game_path = std::filesystem::current_path();

	std::filesystem::path exe_path = game_path;
	exe_path.append("left4dead2.exe");

	std::filesystem::path dll_path = game_path;
	dll_path.append("l4d2-rtx.dll");

	if (!exists(exe_path))
	{
		std::cout << "[!] Could not find 'left4dead2.exe'. Path was: " << exe_path.generic_string().c_str() << ENDL;
		system("pause");
		return -1;
	}

	if (!exists(dll_path))
	{
		std::cout << "[!] Could not find 'l4d2-rtx.dll'. Path was: " << dll_path.generic_string().c_str() << ENDL;
		system("pause");
		return -1;
	}

	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	const DWORD flags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;

	// first arg has to be the executable name
	//std::wstring command_line = L"left4dead2.exe -novid -disable_d3d9_hacks -limitvsconst -softparticlesdefaultoff -disallowhwmorph -no_compressed_verts +mat_phong 1";

	std::wstring command_line = L"left4dead2.exe -novid -insecure -disable_d3d9_hacks -limitvsconst -softparticlesdefaultoff -disallowhwmorph -no_compressed_verts +mat_phong 1"
								"+r_WaterDrawRefraction 0 +r_WaterDrawReflection 0 +cl_brushfastpath 0 +cl_tlucfastpath 0 +cl_modelfastpath 0 +mat_queue_mode 0 +mat_softwarelighting 0 +mat_parallaxmap 0 +mat_frame_sync_enable 0"
								"+mat_displacementmap 0 +mat_drawflat 0 +mat_normalmaps 0 +r_3dsky 0 +mat_fullbright 1 +mat_softwareskin 1 +mat_fastnobump 1 +mat_disable_bloom 1 +mat_depthfeather_enable 0 +mat_force_vertexfog 1 +fog_override 1 +fog_enable 0";

	// get launcher arguments
	for (int i = 1; i < argc; ++i) // skip the launcher name
	{
		command_line += L" ";
		command_line += argv[i];
	}

	const std::string narrow_dll_path = dll_path.string();
	LPCSTR dll_str = narrow_dll_path.c_str();

	if (!DetourCreateProcessWithDllsW(exe_path.c_str(), command_line.data(), nullptr, nullptr, NULL, flags, nullptr, game_path.c_str(), &si, &pi, 1, &dll_str, nullptr))
	{
		DWORD err = GetLastError();
		std::cout << "[!] !DetourCreateProcessWithDllsW - Failed to launch portal2.exe" << ENDL;
		std::cout << "[!] Error:" << err << ENDL;
		std::cout << "[!] |> Game: " << game_path.generic_string().c_str() << ENDL;
		std::cout << "[!] |> DLL: " << dll_path.generic_string().c_str() << ENDL;
		system("pause");
		return -1;
	}

	ResumeThread(pi.hThread);
	bool error = false;

	// check if we want to debug the child process
	// can not use WaitForObject as that would cause a deadlock with child process debugging
	if (IsDebuggerPresent())
	{
		Sleep(50);
		std::uint32_t time = 0;

		bool process_spawned = false;
		while (!process_spawned)
		{
			process_spawned = find_window_by_process_id(pi.dwProcessId);
			if (!process_spawned)
			{
				Sleep(50);
				time += 50;

				if (time >= 30000)
				{
					error = true;
					std::cout << "[Debug] Failed to find spawned process!" << ENDL;
					break;
				}
			}
		}
	}

	//else { // normal startup
	//	WaitForSingleObject(pi.hThread, INFINITE);
	//}

	if (error) {
		system("pause");
	}

	return 0;
}