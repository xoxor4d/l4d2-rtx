#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

// enable/disable benchmark logic
//#define BENCHMARK

//#ifdef DEBUG
//	#define USE_IMGUI 1
//#else
//	#define USE_IMGUI 0
//#endif

#define USE_IMGUI 1

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
#include <unordered_set>
#include <fstream>
#include <iostream>
#include <xmmintrin.h>
#include <intrin.h>

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

#if USE_IMGUI
	#pragma warning(push)
	#pragma warning(disable: 6011)
	#pragma warning(disable: 28182)
#define IMGUI_DEFINE_MATH_OPERATORS
	#include "imgui.h"
	#include <backends/imgui_impl_dx9.h>
	#include <backends/imgui_impl_win32.h>
	#include <misc/cpp/imgui_stdlib.h>
	#pragma warning(pop)
#endif

#include "bridge_remix_api.h"

#include "utils/fnv.hpp"
#include "utils/utils.hpp"
#include "utils/vector.hpp"
#include "game/structs.hpp"

#include "sdk/netvar/netvar.hpp"
#include "sdk/client/c_base_client.hpp"
#include "sdk/client/c_player_info_manager.hpp"
#include "sdk/engine/c_engine_client.hpp"
#include "sdk/client/c_collideable.hpp"
//#include "sdk/engine/c_engine_vgui.hpp"
//#include "sdk/engine/c_achievement_mgr.hpp"
//#include "sdk/engine/c_model_info.hpp"
//#include "sdk/engine/c_input_system.hpp"
#include "sdk/entity/c_base_entity.hpp"
#include "sdk/entity/c_entity_list.hpp"
//#include "sdk/movement/c_user_cmd.hpp"
//#include "sdk/vec/vec.hpp"
//#include "sdk/vgui/c_panel.hpp"
#include "sdk/vgui/surface/c_surface_mgr.hpp"
//#include "sdk/world/c_debug_overlay.hpp"
//#include "sdk/world/c_matrix.hpp"
//#include "sdk/world/c_view_matrix.hpp"
//#include "sdk/world/c_trace_ray.hpp"

#include "utils/hooking.hpp"
#include "utils/memory.hpp"
#include "utils/function.hpp"
#include "game/functions.hpp"

#include "components/loader.hpp"

using namespace std::literals;
