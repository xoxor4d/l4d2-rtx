#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#define COMPMOD_NAME "L4D2-RTX"
#define COMPMOD_ASSET_DIR "l4d2-rtx\\"
#define WINDOW_TITLE_STR "Left 4 Dead 2 - Direct3D 9"

constexpr auto COMP_MOD_VERSION_MAJOR = 1;
constexpr auto COMP_MOD_VERSION_MINOR = 2;
constexpr auto COMP_MOD_VERSION_PATCH = 0;

// adjust for pre-release builds
constexpr auto COMP_MOD_PRE_RELEASE_NUM = 0;

// enable/disable benchmark logic
//#define BENCHMARK

// Version number
#include <version.hpp>

#define NOMINMAX
#include <windows.h>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shellapi.h>
#include <chrono>
#include <mutex>
#include <filesystem>
#include <cassert>
#include <map>
#include <set>
#include <unordered_set>
#include <fstream>
#include <iostream>
#include <xmmintrin.h>
#include <intrin.h>
#include <numbers>

#pragma warning(push)
#pragma warning(disable: 26495)
#include <d3d9.h>
#include <d3dx9.h>
#pragma warning(pop)

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)
#define AssertSize(x, size)								static_assert(sizeof(x) == size, STRINGIZE(x) " structure has an invalid size.")
#define STATIC_ASSERT_SIZE(struct, size)				static_assert(sizeof(struct) == size, "Size check")
#define STATIC_ASSERT_OFFSET(struct, member, offset)	static_assert(offsetof(struct, member) == offset, "Offset check")
#define XASSERT(x) if (x) MessageBoxA(HWND_DESKTOP, #x, "FATAL ERROR", MB_ICONERROR)

#include "MinHook.h"
#include "toml.hpp"
#include "bridge_remix_api.h"

#pragma warning(push)
#pragma warning(disable: 6011)
#pragma warning(disable: 28182)
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>
#include <misc/cpp/imgui_stdlib.h>
#pragma warning(pop)

#include "game/globals.hpp"
#include "game/structs.hpp"
#include "utils/fnv.hpp"
#include "utils/utils.hpp"
#include "utils/vector.hpp"
#include "utils/console.hpp"
#include "utils/flags.hpp"
#include "utils/hooking.hpp"
#include "utils/function.hpp"
#include "utils/memory.hpp"

#include "components/loader.hpp"
#include "components/common/toml.hpp"

#include "game/functions.hpp"
#include "game/l4d2.hpp"

#include "sdk/netvar/netvar.hpp"
#include "sdk/client/c_base_client.hpp"
#include "sdk/client/c_player_info_manager.hpp"
#include "sdk/engine/c_engine_client.hpp"
#include "sdk/client/c_collideable.hpp"
#include "sdk/entity/c_base_entity.hpp"
#include "sdk/entity/c_entity_list.hpp"
#include "sdk/vgui/surface/c_surface_mgr.hpp"

using namespace std::literals;
