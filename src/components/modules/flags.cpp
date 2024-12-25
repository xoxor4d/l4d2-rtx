#include "std_include.hpp"

namespace components
{
	std::vector<std::string> flags::enabled_flags;

	bool flags::has_flag(const std::string& flag)
	{
		parse_flags();

		for (const auto& entry : enabled_flags)
		{
			if (utils::str_to_lower(entry) == utils::str_to_lower(flag)) {
				return true;
			}
		}

		return false;
	}

	void flags::parse_flags()
	{
		// only parse flags once
		if (static auto flags_parsed = false; !flags_parsed)
		{
			flags_parsed = true;

			int num_args;
			auto* const argv = CommandLineToArgvW(GetCommandLineW(), &num_args);

			if (argv)
			{
				for (auto i = 0; i < num_args; ++i)
				{
					std::wstring wide_flag(argv[i]);
					if (wide_flag[0] == L'-')
					{
						wide_flag.erase(wide_flag.begin());
						enabled_flags.emplace_back(utils::convert_wstring(wide_flag));
					}
				}

				LocalFree(argv);
			}
		}
	}
}