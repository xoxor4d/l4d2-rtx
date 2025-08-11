#pragma once

namespace components
{
	namespace cmd
	{
		extern bool sound_debug_printing;
	}

	class sound_events : public component
	{
	public:
		sound_events();
		~sound_events() = default;

		static inline sound_events* p_this = nullptr;
		static auto get() { return p_this; }
	};
}
