#include <std_include.hpp>

namespace sdk
{
	c_base_entity* entity_list::get_client_entity(int i)
	{
		using original_fn = c_base_entity * (__thiscall*)(entity_list*, int);
		return (*(original_fn * *)this)[3](this, i);
	}

	int entity_list::get_max_entity()
	{
		using original_fn = int(__thiscall*)(entity_list*);
		return (*(original_fn * *)this)[8](this);
	}
}
