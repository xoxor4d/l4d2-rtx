#include "std_include.hpp"

namespace components
{
	template <typename m_interface>
	m_interface* interfaces::get_interface(const std::string& module_name, const std::string& interface_name)
	{
		using create_interface_fn = void* (*)(const char*, int*);
		const auto fn = reinterpret_cast<create_interface_fn>(GetProcAddress(GetModuleHandleA(module_name.c_str()), "CreateInterface"));

		if (!fn) {
			return nullptr;
		}

		return static_cast<m_interface*>(fn(interface_name.c_str(), {}));
	}

#define GET_INTERFACE(DEST, MODULE_NAME, VERSION_STR)																										\
		if((DEST) = get_interface<sdk::surface>((MODULE_NAME), (VERSION_STR)); !(DEST)) {																	\
			Beep(300, 100); Sleep(100); Beep(200, 100);																										\
			game::console(); std::cout << "[!][Interfaces] Failed to get interface: '" << (VERSION_STR) << "' in: '" << (MODULE_NAME) << "'" << std::endl;	\
		}

	interfaces::interfaces()
	{
		p_this = this;
		GET_INTERFACE(m_surface, "vguimatsurface.dll", VGUI_MAT_SURFACE_INTERFACE_VERSION);
	}
}