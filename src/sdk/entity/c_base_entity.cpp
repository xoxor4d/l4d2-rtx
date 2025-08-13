#include <std_include.hpp>

namespace sdk
{
	void* c_base_entity::networkable()
	{
		return reinterpret_cast<void*>(uintptr_t(this) + 8);
	}
	
	collideable_t* c_base_entity::get_collideable()
	{
		using original_fn = collideable_t * (__thiscall*)(void*);
		return (*(original_fn * *)this)[3](this);
	}
	
	Vector c_base_entity::get_absolute_origin()
	{
		if (!this) {
			return Vector(0, 0, 0);
		}

		using original_fn = Vector & (__thiscall*)(c_base_entity*);
		return (*(original_fn * *)this)[10](this);
	}
	
	c_client_class* c_base_entity::client_class()
	{
		using original_fn = c_client_class* (__thiscall*)(void*);
		return (*(original_fn * *)networkable())[1](networkable());
	}
	
	bool c_base_entity::is_dormant()
	{
		using original_fn = bool(__thiscall*)(void*);
		return (*static_cast<original_fn**>(networkable()))[7](networkable());
	}

	const char* c_base_entity::get_player_model_name()
	{
		if (!this) {
			return "";
		}

		using original_fn = const char* (__thiscall*)(c_base_entity*);
		return (*(original_fn**)this)[306](this);
	}

	bool c_base_entity::is_player()
	{
		if (!this) {
			return false;
		}

		using original_fn = bool (__thiscall*)(c_base_entity*);
		return (*(original_fn**)this)[143](this);
	}

	bool c_base_entity::is_local_player()
	{
		if (is_player())
		{
			auto e = reinterpret_cast<byte*>(this);
			return e[0x1688];
		}

		return false;
	}

	std::int16_t c_base_entity::get_model_index()
	{
		auto e = reinterpret_cast<byte*>(this);
		return e[0xA0];
	}

	model_t* c_base_entity::get_model()
	{
		return *((model_t**)this + 0x18);
	}

	Vector c_base_entity::get_eye_pos()
	{
		return origin() + view_offset();
	}
}
