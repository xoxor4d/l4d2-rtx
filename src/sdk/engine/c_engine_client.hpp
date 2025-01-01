#pragma once
#define ENGINE_INTERFACE_VERSION "VEngineClient013"

namespace sdk
{
	struct player_info_t 
	{
	private:
		char __pad00[0x8];

	public:
		char name[32];
		int  userid;

	private:
		char __pad01[0x150];
	};

	class engine_client {
	public:
		void get_screen_size(int& width, int& height);
		bool is_in_game();
		bool is_connected();
		bool is_playing();
		int get_local_player();
		int get_player_for_user_id(int user_id);
		int get_max_clients();
		const char* get_level_name();
		bool is_console_visible();
		components::VMatrix& world_to_screen_matrix();
		bool get_player_info(int id, player_info_t* info);
		const char* get_game_directory();
		bool is_game_paused();
		unsigned int get_engine_build_number();
		const char* get_product_version_string();
		void execute_client_cmd(const char* m_cmd);
		int get_app_id();
		bool is_hltv();
		bool is_taking_screenshot();
		bool is_paused();
		int is_box_visible(const Vector& mins, const Vector& maxs);
		void* get_achievement_manager(); // achievement_mgr
	};
}

