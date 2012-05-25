// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

// // all lua addon library will be included in addon-wrapper.h header.
// #include "../addon/addon-wrapper.h"
}

// original header doesn't define OK return value
#ifndef LUA_OK
#define LUA_OK 0
#endif
