/*
 * Copyright 2015-2020 Max Kellermann <max.kellermann@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LUA_UTIL_HXX
#define LUA_UTIL_HXX

#include "Assert.hxx"
#include "util/Compiler.h"

extern "C" {
#include <lua.h>
}

#include <cstddef>
#include <string_view>
#include <tuple>

namespace Lua {

/**
 * This type represents an index on the Lua stack.
 */
struct StackIndex {
	int idx;

	explicit constexpr StackIndex(int _idx) noexcept
		:idx(_idx) {}
};

struct LightUserData {
	void *value;

	explicit constexpr LightUserData(void *_value) noexcept
		:value(_value) {}
};

template<typename... T>
struct CClosure {
	lua_CFunction fn;

	std::tuple<T...> values;
};

template<typename... T>
CClosure<T...>
MakeCClosure(lua_CFunction fn, T&&... values)
{
	return {fn, std::make_tuple(std::forward<T>(values)...)};
}

static inline void
Push(lua_State *L, std::nullptr_t) noexcept
{
	lua_pushnil(L);
}

static inline void
Push(lua_State *L, StackIndex i) noexcept
{
	lua_pushvalue(L, i.idx);
}

gcc_nonnull_all
static inline void
Push(lua_State *L, bool value) noexcept
{
	lua_pushboolean(L, value);
}

gcc_nonnull_all
static inline void
Push(lua_State *L, const char *value) noexcept
{
	lua_pushstring(L, value);
}

gcc_nonnull_all
static inline void
Push(lua_State *L, std::string_view value) noexcept
{
	lua_pushlstring(L, value.data(), value.size());
}

gcc_nonnull_all
static inline void
Push(lua_State *L, int value) noexcept
{
	lua_pushinteger(L, value);
}

gcc_nonnull_all
static inline void
Push(lua_State *L, double value) noexcept
{
	lua_pushnumber(L, value);
}

template<std::size_t i>
struct _PushTuple {
	template<typename T>
	static void PushTuple(lua_State *L, const T &t) {
		_PushTuple<i - 1>::template PushTuple<T>(L, t);
		Push(L, std::get<i - 1>(t));
	}
};

template<>
struct _PushTuple<0> {
	template<typename T>
	static void PushTuple(lua_State *, const T &) {
	}
};

template<typename... T>
gcc_nonnull_all
void
Push(lua_State *L, const std::tuple<T...> &t)
{
	const ScopeCheckStack check_stack(L, sizeof...(T));

	_PushTuple<sizeof...(T)>::template PushTuple<std::tuple<T...>>(L, t);
}

gcc_nonnull_all
static inline void
Push(lua_State *L, lua_CFunction value) noexcept
{
	lua_pushcfunction(L, value);
}

template<typename... T>
gcc_nonnull_all
void
Push(lua_State *L, const CClosure<T...> &value) noexcept
{
	Push(L, value.values);
	lua_pushcclosure(L, value.fn, sizeof...(T));
}

gcc_nonnull_all
static inline void
Push(lua_State *L, LightUserData value) noexcept
{
	lua_pushlightuserdata(L, value.value);
}

/**
 * Internal helper type generated by Lambda().  It is designed to be
 * optimized away.
 */
template<typename T>
struct _Lambda : T {
	template<typename U>
	_Lambda(U &&u):T(std::forward<U>(u)) {}
};

/**
 * Instantiate a #_Lambda instance for the according Push() overload.
 */
template<typename T>
static inline _Lambda<T>
Lambda(T &&t)
{
	return _Lambda<T>(std::forward<T>(t));
}

/**
 * Push a value on the Lua stack by invoking a C++ lambda.  With
 * C++17, we could use std::is_callable, but with C++14, we need the
 * detour with struct _Lambda and Lambda().
 */
template<typename T>
gcc_nonnull_all
static inline void
Push(lua_State *L, _Lambda<T> l)
{
	const ScopeCheckStack check_stack(L, 1);

	l();
}

template<typename V>
void
SetGlobal(lua_State *L, const char *name, V &&value) noexcept
{
	Push(L, std::forward<V>(value));
	lua_setglobal(L, name);
}

template<typename K>
void
GetTable(lua_State *L, int idx, K &&key) noexcept
{
	const ScopeCheckStack check_stack(L, 1);

	Push(L, std::forward<K>(key));
	lua_gettable(L, idx);
}

template<typename K, typename V>
void
SetTable(lua_State *L, int idx, K &&key, V &&value) noexcept
{
	const ScopeCheckStack check_stack(L);

	Push(L, std::forward<K>(key));
	Push(L, std::forward<V>(value));
	lua_settable(L, idx);
}

template<typename V>
void
SetField(lua_State *L, int idx, const char *name, V &&value) noexcept
{
	const ScopeCheckStack check_stack(L);

	Push(L, std::forward<V>(value));
	lua_setfield(L, idx, name);
}

template<typename V>
static inline void
SetRegistry(lua_State *L, const char *name, V &&value) noexcept
{
	SetField(L, LUA_REGISTRYINDEX, name, std::forward<V>(value));
}

static inline void *
GetRegistryLightUserData(lua_State *L, const char *name) noexcept
{
	const ScopeCheckStack check_stack(L);

	lua_getfield(L, LUA_REGISTRYINDEX, name);
	void *value = lua_touserdata(L, -1);
	lua_pop(L, 1);
	return value;
}

template<typename V>
static inline void
SetField(lua_State *L, const char *package,
	 const char *name, V &&value) noexcept
{
	const ScopeCheckStack check_stack(L);

	lua_getglobal(L, package);
	SetField(L, -2, name, std::forward<V>(value));
	lua_pop(L, 1);
}

/**
 * Sets the variable "package.path", which controls the package
 * search path for the "require" command.
 */
gcc_nonnull_all
static inline void
SetPackagePath(lua_State *L, const char *path) noexcept
{
	SetField(L, "package", "path", path);
}

}

#endif