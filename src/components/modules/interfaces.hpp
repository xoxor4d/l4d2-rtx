#pragma once

namespace components
{
	class interfaces : public component
	{
	public:
		interfaces();

		static inline interfaces* p_this = nullptr;
		static interfaces* get() { return p_this; }

		sdk::surface* m_surface = nullptr;

private:
		template <typename m_interface>
		static m_interface* get_interface(const std::string& module_name, const std::string& interface_name);
	};
}