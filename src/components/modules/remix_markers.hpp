#pragma once

namespace components
{
	class remix_markers final : public loader::component_module
	{
	public:
		remix_markers();

		static inline remix_markers* p_this = nullptr;
		static remix_markers* get() { return p_this; }

		static bool is_initialized()
		{
			if (const auto mod = get(); mod && mod->m_initialized) {
				return true;
			}
			return false;
		}

		static void draw_nocull_markers();
		static void on_sound_start(const std::uint32_t& hash, const std::string& sound_name);
		static void on_event_start(const std::string_view& name, const std::string_view& actor, const std::string_view& event, const std::string_view& param1);
		static void on_client_frame();

	private:
		bool m_initialized = false;
	};
}
