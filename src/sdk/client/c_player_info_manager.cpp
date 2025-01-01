#include <std_include.hpp>

namespace sdk
{
	CGlobalVarsBase* player_info_manager::get_global_vars()
	{
		using original_fn = CGlobalVarsBase* (__thiscall*)(player_info_manager*);
		return (*(original_fn**)this)[1](this);
	}
}
