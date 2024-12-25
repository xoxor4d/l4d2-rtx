#pragma once

namespace components
{
	class component
	{
	public:
		component() {};
		virtual ~component() {};
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
}

#include "modules/flags.hpp"
#include "modules/main_module.hpp"
#include "modules/model_render.hpp"
