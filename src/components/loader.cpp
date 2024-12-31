#include "std_include.hpp"

namespace components
{
	std::vector<component*> loader::components_;
	utils::memory::allocator loader::mem_allocator_;

	void loader::initialize()
	{
		mem_allocator_.clear();
		_register(new interfaces());
		_register(new flags());
		_register(new main_module());
		_register(new model_render());
		_register(new imgui());
		XASSERT(MH_EnableHook(MH_ALL_HOOKS) != MH_STATUS::MH_OK);
	}

	void loader::uninitialize()
	{
		std::ranges::reverse(components_.begin(), components_.end());
		for (const auto component : components_) {
			delete component;
		}

		components_.clear();
		mem_allocator_.clear();
		fflush(stdout);
		fflush(stderr);
	}

	void loader::_register(component* component)
	{
		if (component) {
			components_.push_back(component);
		}
	}

	utils::memory::allocator* loader::get_alloctor() {
		return &loader::mem_allocator_;
	}



	/*template <typename m_interface>
	m_interface* interfaces::get_interface(const std::string& module_name, const std::string& interface_name)
	{
		using create_interface_fn = void* (*)(const char*, int*);
		const auto fn = reinterpret_cast<create_interface_fn>(GetProcAddress(GetModuleHandleA(module_name.c_str()), "CreateInterface"));

		if (!fn) {
			return nullptr;
		}

		return static_cast<m_interface*>(fn(interface_name.c_str(), {}));
	}

	interfaces::interfaces()
	{
		m_surface = get_interface<sdk::surface>("vguimatsurface.dll", VGUI_MAT_SURFACE_INTERFACE_VERSION);
	}*/
}
