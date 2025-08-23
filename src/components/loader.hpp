#pragma once
#include<memory>

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

		static utils::memory::allocator* get_alloctor();

	private:
		static std::vector<std::unique_ptr<component>> components_;
		static utils::memory::allocator mem_allocator_;

		template<class ComponentType>
		static void register_component()
		{
			components_.emplace_back(std::make_unique<ComponentType>());
		}

	};
}

#include "modules/interfaces.hpp"
#include "modules/flags.hpp"
#include "modules/game_settings.hpp"
#include "modules/remix_api.hpp"
#include "modules/choreo_events.hpp"
#include "modules/sound_events.hpp"
#include "modules/remix_lights.hpp"
#include "modules/remix_vars.hpp"
#include "modules/remix_markers.hpp"
#include "modules/main_module.hpp"
#include "modules/model_render.hpp"
#include "modules/map_settings.hpp"
#include "modules/imgui.hpp"
