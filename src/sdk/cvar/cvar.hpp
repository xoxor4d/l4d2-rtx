#pragma once

namespace sdk
{
#define CVAR_INTERFACE_VERSION "VEngineCvar007"

	class CCvar
	{
	public:
		void register_con_command(ConCommandBase*);
		void unregister_con_command(ConCommandBase*);
		void unregister_con_commands(int);
		const char* get_command_line_value(const char*);
		const ConCommandBase* find_command_base_const(const char*);
		ConCommandBase* find_command_base(const char*);
		const ConVar* find_var_const(const char*);
		ConVar* find_var(const char*);
		const ConCommand* find_command_const(const char*);
		ConCommand* find_command(const char*);
	};
}
