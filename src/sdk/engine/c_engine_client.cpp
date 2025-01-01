#include <std_include.hpp>

namespace sdk
{
	void engine_client::get_screen_size(int& width, int& height) {
		using original_fn = void(__thiscall*)(engine_client*, int&, int&);
		return (*(original_fn * *)this)[5](this, width, height);
	}
	
	bool engine_client::is_in_game()
	{
		using original_fn = bool(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[26](this);
	}
	
	bool engine_client::is_connected()
	{
		using original_fn = bool(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[27](this);
	}
	
	bool engine_client::is_playing()
	{
		return is_connected() && is_in_game();
	}
	
	int engine_client::get_local_player()
	{
		using original_fn = int(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[12](this);
	}
	
	int engine_client::get_player_for_user_id(int user_id)
	{
		using original_fn = int(__thiscall*)(engine_client*, int);
		return (*(original_fn * *)this)[9](this, user_id);
	}
	
	int engine_client::get_max_clients()
	{
		using original_fn = int(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[21](this);
	}
	
	const char* engine_client::get_level_name()
	{
		using original_fn = const char*(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[53](this);
	}
	
	bool engine_client::is_console_visible()
	{
		using original_fn = bool(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[11](this);
	}

	VMatrix& engine_client::world_to_screen_matrix()
	{
		using original_fn = VMatrix&(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[37](this);
	}
	
	bool engine_client::get_player_info(int id, player_info_t* info)
	{
		using original_fn = bool(__thiscall*)(engine_client*, int, player_info_t*);
		return (*(original_fn * *)this)[8](this, id, info);
	}
	
	const char* engine_client::get_game_directory()
	{
		using original_fn = const char*(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[36](this);
	}
	
	bool engine_client::is_game_paused()
	{
		using original_fn = const char*(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[86](this);
	}
	
	unsigned int engine_client::get_engine_build_number()
	{
		using original_fn = unsigned int(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[99](this);
	}
	
	const char* engine_client::get_product_version_string()
	{
		using original_fn = const char*(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[100](this);
	}
	
	void engine_client::execute_client_cmd(const char* m_cmd)
	{
		using original_fn = void(__thiscall*)(engine_client*, const char*);
		return (*(original_fn * *)this)[103](this, m_cmd);
	}
	
	int engine_client::get_app_id()
	{
		using original_fn = int(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[105](this);
	}
	
	bool engine_client::is_hltv()
	{
		using original_fn = bool(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[88](this);
	}
	
	bool engine_client::is_taking_screenshot()
	{
		using original_fn = bool(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[87](this);
	}
	
	bool engine_client::is_paused()
	{
		using original_fn = bool(__thiscall*)(engine_client*);
		return (*(original_fn * *)this)[86](this);
	}
	
	int engine_client::is_box_visible(const Vector& mins, const Vector& maxs)
	{
		using original_fn = int(__thiscall*)(engine_client*, Vector, Vector);
		return (*(original_fn * *)this)[31](this, mins, maxs);
	}
	
	void* engine_client::get_achievement_manager() // achievement_mgr
	{
		using original_fn = void* (__thiscall*)(engine_client*); // achievement_mgr
		return (*(original_fn * *)this)[115](this);
	}
}
