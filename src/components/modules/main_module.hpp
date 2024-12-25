#pragma once

namespace components
{
	class main_module : public component
	{
	public:
		main_module();
		~main_module();

		static void force_cvars();
	private:
	};
}
