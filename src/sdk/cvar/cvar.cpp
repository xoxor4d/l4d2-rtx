#include <std_include.hpp>
#include "cvar.hpp"

namespace sdk
{
	void CCvar::register_con_command(ConCommandBase* c)
	{
		using original_fn = void(__thiscall*)(CCvar*, ConCommandBase*);
		return (*(original_fn**)this)[6](this, c);
	}

	void CCvar::unregister_con_command(ConCommandBase* c)
	{
		using original_fn = void(__thiscall*)(CCvar*, ConCommandBase*);
		return (*(original_fn**)this)[7](this, c);
	}

	void CCvar::unregister_con_commands(int num)
	{
		using original_fn = void(__thiscall*)(CCvar*, int);
		return (*(original_fn**)this)[8](this, num);
	}

	const char* CCvar::get_command_line_value(const char* name)
	{
		using original_fn = const char* (__thiscall*)(CCvar*, const char*);
		return (*(original_fn**)this)[9](this, name);
	}

	const ConCommandBase* CCvar::find_command_base_const(const char* name)
	{
		using original_fn = const ConCommandBase* (__thiscall*)(CCvar*, const char*);
		return (*(original_fn**)this)[10](this, name);
	}

	ConCommandBase* CCvar::find_command_base(const char* name)
	{
		using original_fn = ConCommandBase * (__thiscall*)(CCvar*, const char*);
		return (*(original_fn**)this)[11](this, name);
	}

	const ConVar* CCvar::find_var_const(const char* name)
	{
		using original_fn = const ConVar* (__thiscall*)(CCvar*, const char*);
		return (*(original_fn**)this)[12](this, name);
	}

	ConVar* CCvar::find_var(const char* name)
	{
		using original_fn = ConVar * (__thiscall*)(CCvar*, const char*);
		return (*(original_fn**)this)[13](this, name);
	}

	const ConCommand* CCvar::find_command_const(const char* name)
	{
		using original_fn = const ConCommand* (__thiscall*)(CCvar*, const char*);
		return (*(original_fn**)this)[14](this, name);
	}

	ConCommand* CCvar::find_command(const char* name)
	{
		using original_fn = ConCommand * (__thiscall*)(CCvar*, const char*);
		return (*(original_fn**)this)[15](this, name);
	}
}
