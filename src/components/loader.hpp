#pragma once
//#include "sdk/vgui/surface/c_surface_mgr.hpp"

namespace components
{
	class component
	{
	public:
		component() {}
		virtual ~component() {}
	};

	class loader
	{
	public:
		static void initialize();
		static void uninitialize();
		static void _register(component* component);

		static utils::memory::allocator* get_alloctor();

	private:
		static std::vector<component*> components_;
		static utils::memory::allocator mem_allocator_;
	};

	/*class interfaces
	{
	public:
		interfaces();
		~interfaces() = delete;

		static inline interfaces* p_this = nullptr;
		static interfaces* get() { return p_this; }

		sdk::surface* m_surface = nullptr;

	private:
		template <typename m_interface>
		static m_interface* get_interface(const std::string& module_name, const std::string& interface_name);
	};*/
}

#include "modules/interfaces.hpp"
#include "modules/flags.hpp"
#include "modules/main_module.hpp"
#include "modules/model_render.hpp"
#include "modules/imgui.hpp"
