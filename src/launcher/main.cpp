#include <iostream>
#include <windows.h>
#include <string>
#include <filesystem>

#include "detours.h" 
#pragma comment(lib, "detours.lib")

#include "toml.hpp"

#define ENDL "\n"
#define PAUSE system("pause")

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

#ifdef DEBUG
int main(int argc, char* argv[])
#else
int main(int, char*[])
#endif
{
	std::filesystem::path current_path = std::filesystem::current_path();

	std::filesystem::path toml_path = current_path;
	toml_path.append("comp-rtx-launcher.toml");

	if (!exists(toml_path)) 
	{
		std::cout << "[!] Could not find 'comp-rtx-launcher.toml' in " << current_path << ENDL;
		PAUSE; return -1;
	}

	std::filesystem::path exe_path = current_path;
	std::filesystem::path dll_path = current_path;
	std::string exe_name, dll_name, commandline;

	try
	{
		auto config = toml::parse(toml_path, toml::spec::v(1, 1, 0));
		if (config.contains("exe_name"))
		{
			if (const auto& exe = config.at("exe_name"); exe.is_string() && !exe.is_empty()) 
			{
				exe_name = exe.as_string();
				exe_path.append(exe_name);
			}
			else {
				std::cout << "exe_name: Empty or malformed!" << ENDL; PAUSE; return -1;
			}
		} else {
			std::cout << "exe_name: Unspecified!" << ENDL; PAUSE; return -1;
		}

		if (config.contains("dll_name"))
		{
			if (const auto& dll = config.at("dll_name"); dll.is_string() && !dll.is_empty()) 
			{
				dll_name = dll.as_string();
				dll_path.append(dll_name);
			}
			else {
				std::cout << "dll_name: Empty or malformed!" << ENDL; PAUSE; return -1;
			}
		} else {
			std::cout << "dll_name: Unspecified!" << ENDL; PAUSE; return -1;
		}

		if (config.contains("commandline"))
		{
			if (const auto& cmd = config.at("commandline"); cmd.is_string()) {
				commandline = cmd.as_string();
			}
			else {
				std::cout << "commandline: Expected a string! Launching without commandline arguments." << ENDL;
			}
		}
	}
	catch (const toml::syntax_error& err)
	{
		std::cout << err.what() << ENDL;
		PAUSE; return -1;
	}

	if (!exists(exe_path))
	{
		std::cout << "[!] Could not find '" << exe_name << "'. Path was: " << exe_path.generic_string() << ENDL;
		PAUSE; return -1;
	}

	if (!exists(dll_path))
	{
		std::cout << "[!] Could not find '" << dll_name << "'. Path was: " << dll_path.generic_string() << ENDL;
		PAUSE; return -1;
	}

#ifdef DEBUG // get launcher arguments
	std::string command_line_str = exe_name + " -insecure " + commandline;
	for (int i = 1; i < argc; ++i) // skip the launcher name
	{
		command_line_str += " ";
		command_line_str += argv[i];
	}
#else
	const std::string command_line_str = exe_name + " -insecure " + commandline;
#endif

	int size = MultiByteToWideChar(CP_UTF8, 0, command_line_str.c_str(), -1, nullptr, 0);
	std::wstring wide_cmd(size, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, command_line_str.c_str(), -1, wide_cmd.data(), size);

	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	const DWORD flags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;

	const std::string narrow_dll_path = dll_path.string();
	LPCSTR dll_str = narrow_dll_path.c_str();

	if (!DetourCreateProcessWithDllsW(exe_path.c_str(), wide_cmd.data(), nullptr, nullptr, NULL, flags, nullptr, current_path.c_str(), &si, &pi, 1, &dll_str, nullptr))
	{
		DWORD err = GetLastError();
		std::cout << "[!] !DetourCreateProcessWithDllsW - Failed to launch '" << exe_name << "'" << ENDL;
		std::cout << "[!] Error:" << err << ENDL;
		std::cout << "[!] |> EXE: " << current_path.generic_string() << ENDL;
		std::cout << "[!] |> DLL: " << dll_path.generic_string() << ENDL;
		std::cout << "[!] |> COMMANDLINE: " << commandline << ENDL;
		PAUSE; return -1;
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
					std::cout << "[!] Failed to find spawned process!" << ENDL;
					break;
				}
			}
		}
	}

	if (error) {
		PAUSE;
	}

	return 0;
}
