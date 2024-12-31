#include "std_include.hpp"
#include <wincrypt.h>

std::string hash_file_sha1(const char* file_path)
{
	const auto file = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE) {
		return {};
	}

	HCRYPTPROV prov_handle = 0;
	HCRYPTHASH hash_handle = 0;

	BYTE buffer[4096];
	DWORD bytes_read = 0;

	BYTE hash[20]; // SHA-1 produces a 20-byte hash
	DWORD hash_len = sizeof(hash);

	if (!CryptAcquireContext(&prov_handle, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT) ||
		!CryptCreateHash(prov_handle, CALG_SHA1, 0, 0, &hash_handle))
	{
		CloseHandle(file);
		return {};
	}

	while (ReadFile(file, buffer, sizeof(buffer), &bytes_read, nullptr) && bytes_read > 0)
	{
		if (!CryptHashData(hash_handle, buffer, bytes_read, 0))
		{
			CryptDestroyHash(hash_handle);
			CryptReleaseContext(prov_handle, 0);
			CloseHandle(file);
			return {};
		}
	}

	std::string hash_string;
	if (CryptGetHashParam(hash_handle, HP_HASHVAL, hash, &hash_len, 0))
	{
		std::ostringstream oss;
		for (DWORD i = 0; i < hash_len; ++i) {
			oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
		}

		hash_string = oss.str();
	}

	CryptDestroyHash(hash_handle);
	CryptReleaseContext(prov_handle, 0);
	CloseHandle(file);
	return hash_string;
}

void init_fail_msg_setup()
{
	Beep(300, 100); Sleep(100); Beep(200, 100);
	game::console(); std::cout << "[!][INIT FAILED] Not loading P2-RTX Compatibility Mod" << std::endl;

	if (char file_path[MAX_PATH] = {};
		GetModuleFileNameA(nullptr, file_path, MAX_PATH))
	{
		std::string hash = hash_file_sha1(file_path);
		if (hash != "007F496DBAC6C45A450E8966958CB741ACEE6702") 
		{
			std::cout << "---------------> Unexpected left4dead2.exe hash. Hash was: " << hash.c_str() << std::endl;
			std::cout << "---------------> Path was: " << file_path << std::endl;
		}
	}
}

void init_fail_msg_post()
{
	std::cout << "\n\tMake sure that:" << std::endl;
	std::cout << "\t- Steam is running." << std::endl;
	std::cout << "\t- That it is a legit copy of the game." << std::endl;
	std::cout << "\t- That you followed the install instructions and installed everything correctly." << std::endl;
	std::cout << "\n\tPlease copy/paste the contents of this window when you open a GitHub issue." << std::endl;
}

#define GET_MODULE_HANDLE(HANDLE_OUT, NAME, T) \
	while (!(HANDLE_OUT)) { \
		if ((HANDLE_OUT) = (DWORD)GetModuleHandleA(NAME); !(HANDLE_OUT)) { \
			Sleep(100); (T) += 100u; \
			if ((T) >= 30000) { \
				init_fail_msg_setup(); std::cout << "---------------> Failed to find module: " << (NAME) << std::endl; init_fail_msg_post(); \
				return TRUE; \
			} \
		} \
	}

DWORD WINAPI find_window_loop(LPVOID)
{
	std::uint32_t T = 0;

	// wait for window creation
	while (!glob::main_window)
	{
		// get main window hwnd
		if (!glob::main_window) {
			glob::main_window = FindWindowA(nullptr, "Left 4 Dead 2 - Direct3D 9");
		}

		Sleep(100); T += 100;
		if (T >= 30000) 
		{
			init_fail_msg_setup();
			std::cout << "---------------> Failed to find main window with name: Left 4 Dead 2 - Direct3D 9" << std::endl;
			init_fail_msg_post();
			return TRUE;
		}
	}

	GET_MODULE_HANDLE(game::shaderapidx9_module, "shaderapidx9.dll", T);
	GET_MODULE_HANDLE(game::studiorender_module, "studiorender.dll", T);
	GET_MODULE_HANDLE(game::materialsystem_module, "materialsystem.dll", T);
	GET_MODULE_HANDLE(game::engine_module, "engine.dll", T);
	GET_MODULE_HANDLE(game::client_module, "client.dll", T);
	GET_MODULE_HANDLE(game::server_module, "server.dll", T);
	GET_MODULE_HANDLE(game::vstdlib_module, "vstdlib.dll", T);

	if (const auto MH_INIT_STATUS = MH_Initialize(); MH_INIT_STATUS != MH_STATUS::MH_OK) 
	{
		init_fail_msg_setup();
		std::cout << "---------------> MinHook failed to initialize with code: " << MH_INIT_STATUS << std::endl;
		init_fail_msg_post();
		return TRUE;
	}

#undef GET_MODULE_HANDLE
	
#ifdef DEBUG
	Beep(523, 100);
#endif
		
#ifdef GIT_DESCRIBE
	SetWindowTextA(glob::main_window, utils::va("Left 4 Dead 2 - RTX - %s", GIT_DESCRIBE));
#else
	SetWindowTextA(glob::main_window, "Left 4 Dead 2 - RTX");
#endif

	loader::initialize();
	return TRUE;
}

BOOL APIENTRY DllMain(HMODULE, const DWORD ul_reason_for_call, LPVOID)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		CreateThread(nullptr, 0, find_window_loop, nullptr, 0, nullptr);
	}

	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		loader::uninitialize();
	}

	return TRUE;
}
