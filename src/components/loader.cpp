#include "std_include.hpp"

namespace components
{
	utils::memory::allocator loader::mem_allocator_;
	std::vector<std::unique_ptr<component>> loader::components_;

	void loader::initialize()
	{
		mem_allocator_.clear();
		register_component<interfaces>();
		register_component<flags>();
		register_component<game_settings>();
		register_component<remix_api>();
		register_component<choreo_events>();
		register_component<sound_events>();
		register_component<main_module>();
		register_component<model_render>();
		register_component<imgui>();
		register_component<remix_vars>();
		register_component<remix_markers>();
		register_component<map_settings>();
		register_component<remix_lights>();


		XASSERT(MH_EnableHook(MH_ALL_HOOKS) != MH_STATUS::MH_OK);
	}

	void loader::uninitialize()
	{

		components_.clear();
		mem_allocator_.clear();
		fflush(stdout);
		fflush(stderr);
	}


	utils::memory::allocator* loader::get_alloctor() {
		return &loader::mem_allocator_;
	}
}
