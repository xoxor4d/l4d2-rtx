#pragma once

namespace sdk
{
	#define INETVAR( table, prop, func_name, type ) type& func_name() { \
	    static uintptr_t offset = 0; \
	    if( !offset ) { \
	        offset = g_netvars.get_netvar( fnv::hash( table ), fnv::hash( prop ) ); \
	    } \
	    return *reinterpret_cast< type* >( uintptr_t( this ) + offset ); \
	}
	
	#define INETVARPTR( table, prop, func_name, type ) type* func_name() { \
	    static uintptr_t offset = 0; \
	    if( !offset ) { \
	        offset = g_netvars.get_netvar( fnv::hash( table ), fnv::hash( prop ) ); \
	    } \
	    return reinterpret_cast< type* >( uintptr_t( this ) + offset ); \
	}
	
	#define IOFFSET( type, var, offset ) type& var() { \
	    return *( type* )( uintptr_t( this ) + offset ); \
	}

	struct netvar_manager {
	public:
		uintptr_t get_netvar(uint32_t table, uint32_t prop);
	};

	extern netvar_manager g_netvars;
}
