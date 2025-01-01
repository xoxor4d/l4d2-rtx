#include <std_include.hpp>

namespace sdk
{
	Vector& collideable_t::mins() {
		using original_fn = Vector& (__thiscall*)(void*);
		return (*(original_fn**)this)[1](this);
	}

	Vector& collideable_t::maxs() {
		using original_fn = Vector& (__thiscall*)(void*);
		return (*(original_fn**)this)[2](this);
	}
}
