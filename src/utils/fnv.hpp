#pragma once
// https://github.com/gh-0x/l4d2-internal-base

template<typename S>
struct fnv_internal;

template<typename S>
struct fnv1a;

template<>
struct fnv_internal<uint32_t>
{
	constexpr static uint32_t default_offset_basis = 0x811C9DC5;
	constexpr static uint32_t prime = 0x01000193;
};

template<>
struct fnv1a<uint32_t> : fnv_internal<uint32_t>
{
	constexpr static uint32_t hash(char const* string, const uint32_t val = default_offset_basis) {
		return (string[0] == '\0') ? val : hash(&string[1], (val ^ static_cast<uint32_t>(string[0])) * prime);
	}

	constexpr static uint32_t hash(wchar_t const* string, const uint32_t val = default_offset_basis) {
		return (string[0] == L'\0') ? val : hash(&string[1], (val ^ static_cast<uint32_t>(string[0])) * prime);
	}
};

using fnv = fnv1a<uint32_t>;