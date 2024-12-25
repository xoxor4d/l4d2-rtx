#pragma once

namespace components
{
	class flags : public component
	{
	public:
		static bool has_flag(const std::string& flag);
		static void parse_flags();

	private:
		static std::vector<std::string> enabled_flags;
	};
}