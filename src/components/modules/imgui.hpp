#pragma once

namespace components
{
	class imgui : public component
	{
	public:
		imgui();
		~imgui();

		static inline imgui* p_this = nullptr;
		static imgui* get() { return p_this; }

		static void endscene_stub();

#if USE_IMGUI
		void devgui();
		bool input_message(UINT message_type, WPARAM wparam, LPARAM lparam);

		bool m_menu_active = false;
		bool m_initialized_device = false;
#endif

		float m_flashlight_fwd_offset = 0.0f;
		float m_flashlight_horz_offset = 0.0f;
		float m_flashlight_vert_offset = 0.0f;

	private:
	};
}
