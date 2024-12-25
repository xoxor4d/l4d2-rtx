#pragma once

#define HOOK_JUMP true
#define HOOK_CALL false

#define HOOK_RETN_PLACE_DEF(NAME)		DWORD (NAME) = 0u
#define HOOK_RETN_PLACE(NAME, OFFSET)	(NAME) = (OFFSET)

namespace utils
{
	class hook
	{
	public:

		hook() : initialized(false), installed(false), place(nullptr), stub(nullptr), original(nullptr), useJump(false), protection(0) { ZeroMemory(this->buffer, sizeof(this->buffer)); }

		hook(void* place, void* stub, bool useJump = true) : hook() { this->initialize(place, stub, useJump); }
		hook(void* place, void(*stub)(), bool useJump = true) : hook(place, reinterpret_cast<void*>(stub), useJump) {}

		hook(DWORD place, void* stub, bool useJump = true) : hook(reinterpret_cast<void*>(place), stub, useJump) {}
		hook(DWORD place, DWORD stub, bool useJump = true) : hook(reinterpret_cast<void*>(place), reinterpret_cast<void*>(stub), useJump) {}
		hook(DWORD place, void(*stub)(), bool useJump = true) : hook(reinterpret_cast<void*>(place), reinterpret_cast<void*>(stub), useJump) {}
		~hook();

		hook* initialize(void* place, void* stub, bool useJump = true);
		hook* initialize(DWORD place, void* stub, bool useJump = true);
		hook* initialize(DWORD place, void(*stub)(), bool useJump = true); // For lambdas
		hook* install(bool unprotect = true, bool keepUnportected = false);
		hook* uninstall(bool unprotect = true);

		void* get_address();
		void quick();

		template <typename T> static std::function<T> call(DWORD function)
		{
			return std::function<T>(reinterpret_cast<T*>(function));
		}

		template <typename T> static std::function<T> call(FARPROC function)
		{
			return call<T>(reinterpret_cast<DWORD>(function));
		}

		template <typename T> static std::function<T> call(void* function)
		{
			return call<T>(reinterpret_cast<DWORD>(function));
		}

		static void set_string(void* place, const char* string, size_t length);
		static void set_string(DWORD place, const char* string, size_t length);

		static void set_string(void* place, const char* string);
		static void set_string(DWORD place, const char* string);

		static void write_string(void* place, const std::string& string);
		static void write_string(DWORD place, const std::string& string);

		static void nop(void* place, size_t length);
		static void nop(DWORD place, size_t length);

		static void redirect_jump(void* place, void* stub);
		static void redirect_jump(DWORD place, void* stub);

		template <typename T> static void set(void* place, T value)
		{
			DWORD oldProtect;
			VirtualProtect(place, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);

			*static_cast<T*>(place) = value;

			VirtualProtect(place, sizeof(T), oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), place, sizeof(T));
		}

		template <typename T> static void set(DWORD place, T value)
		{
			return set<T>(reinterpret_cast<void*>(place), value);
		}

		template <std::size_t Index, typename ReturnType, typename... Args>
		__forceinline static ReturnType call_virtual(void* instance, Args... args)
		{
			using Fn = ReturnType(__thiscall*)(void*, Args...);

			auto function = (*static_cast<Fn**>(instance))[Index];
			return function(instance, args...);
		}

	private:
		bool initialized;
		bool installed;

		void* place;
		void* stub;
		void* original;
		char buffer[5];
		bool useJump;

		DWORD protection;

		std::mutex stateMutex;
	};


	// #
	// #

	class vtable
	{
	public:
		bool init(const void* table)
		{
			m_base_pointer_ = (unsigned int**)(table);
			while ((*m_base_pointer_)[m_size_])
			{
				m_size_ += 1u;
			}

			m_originals_ = std::make_unique<void* []>(m_size_);
			return (m_base_pointer_ && m_size_);
		}

		bool hook(void* place, const unsigned int index)
		{
			if (m_base_pointer_ && m_size_)
			{
				return (MH_CreateHook((*reinterpret_cast<void***>(m_base_pointer_))[index], place, &m_originals_[index]) == MH_STATUS::MH_OK);
			}

			return false;
		}

		template<typename FN>
		FN original(const unsigned int index) const
		{
			return reinterpret_cast<FN>(m_originals_[index]);
		}

	private:
		unsigned int**				m_base_pointer_ = nullptr;
		unsigned int				m_size_ = 0u;
		std::unique_ptr<void* []>	m_originals_ = { };
	};


	// #
	// #

	class cinterface
	{
	public:
		template<typename T>
		T get(const char* const sz_module, const char* const sz_object)
		{
			if (const auto hmodule = GetModuleHandleA(sz_module); hmodule)
			{
				if (auto* interface_ret = get_interface(hmodule, sz_object); interface_ret)
				{
					return static_cast<T>(interface_ret);
				}
				
				MessageBoxA(HWND_DESKTOP, sz_object, "Failed to find interface:", MB_ICONERROR);
				return NULL;
			}

			return NULL;
		}

	private:
		void* get_interface(HMODULE hmodule, const char* sz_object);
	};

	inline cinterface module_interface;
}
