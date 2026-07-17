#include "std_include.hpp"

#include "components/modules/choreo_events.hpp"
#include "components/modules/game_settings.hpp"
#include "components/modules/imgui.hpp"
#include "components/modules/interfaces.hpp"
#include "components/modules/main_module.hpp"
#include "components/modules/model_render.hpp"
#include "components/modules/remix_api.hpp"
#include "components/modules/remix_lights.hpp"
#include "components/modules/remix_markers.hpp"
#include "components/modules/sound_events.hpp"

namespace l4d2
{
	void init_game_addresses()
	{
		
	}

	void main()
	{
		loader::module_loader::register_module(std::make_unique<interfaces>());
		loader::module_loader::register_module(std::make_unique<game_settings>());
		loader::module_loader::register_module(std::make_unique<remix_api>());
		loader::module_loader::register_module(std::make_unique<choreo_events>());
		loader::module_loader::register_module(std::make_unique<sound_events>());
		loader::module_loader::register_module(std::make_unique<main_module>());
		loader::module_loader::register_module(std::make_unique<model_render>());
		loader::module_loader::register_module(std::make_unique<imgui>());
		loader::module_loader::register_module(std::make_unique<remix_vars>());
		loader::module_loader::register_module(std::make_unique<remix_markers>());
		loader::module_loader::register_module(std::make_unique<map_settings>());
		loader::module_loader::register_module(std::make_unique<remix_lights>());

		MH_EnableHook(MH_ALL_HOOKS);
	}
}
