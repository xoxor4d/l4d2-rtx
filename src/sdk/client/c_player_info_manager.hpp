#pragma once
#define PLAYER_INFO_MANAGER_INTERFACE_VERSION "PlayerInfoManager002"

namespace sdk
{
	class player_info_manager
	{
	public:
		components::CGlobalVarsBase* get_global_vars();
	};
}
