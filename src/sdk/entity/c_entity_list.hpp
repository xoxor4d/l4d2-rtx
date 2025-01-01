#pragma once
#include "c_base_entity.hpp"

#define CLIENT_ENTITY_INTERFACE_VERSION "VClientEntityList003"

namespace sdk
{
	class entity_list {
	public:
		c_base_entity* get_client_entity(int i);
		int get_max_entity();
	};
}

