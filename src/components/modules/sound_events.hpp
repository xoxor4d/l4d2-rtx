#pragma once

namespace components
{
	class sound_events : public component
	{
	public:
		sound_events();
		~sound_events() = default;

		static inline sound_events* p_this = nullptr;
		static auto get() { return p_this; }
	};
}
