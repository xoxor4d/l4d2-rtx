#pragma once

namespace components
{
	namespace cmd
	{
		extern bool sound_debug_printing;
	}

	class sound_events final : public loader::component_module
	{
	public:
		sound_events();
		~sound_events() = default;

		static inline sound_events* p_this = nullptr;
		static auto get() { return p_this; }

		static bool is_initialized()
		{
			if (const auto mod = get(); mod && mod->m_initialized) {
				return true;
			}
			return false;
		}

	private:
		bool m_initialized = false;
	};
}
