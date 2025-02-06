#pragma once

namespace common
{
	void get_and_add_integers_to_set(char* str, std::unordered_set<std::uint32_t>& set, const std::uint32_t& buf_len = 0u, bool clear_buf = false);
	void get_and_remove_integers_from_set(char* str, std::unordered_set<std::uint32_t>& set, const std::uint32_t& buf_len = 0u, bool clear_buf = false);
}

namespace ImGui
{
	void TextWrapped_IntegersFromUnorderedSet(const std::unordered_set<std::uint32_t>& set);
}