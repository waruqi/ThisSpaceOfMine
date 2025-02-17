// Copyright (C) 2024 Jérôme Leclercq
// This file is part of the "This Space Of Mine" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CommonLib/Scripting/ScriptingUtils.hpp>
#include <entt/entt.hpp>
#include <stdexcept>

namespace tsom
{
	entt::handle AssertScriptEntity(sol::table entityTable)
	{
		entt::handle entity = RetrieveScriptEntity(entityTable);
		if (!entity)
			throw std::runtime_error("invalid entity");

		return entity;
	}

	entt::handle RetrieveScriptEntity(sol::table entityTable)
	{
		sol::object entityObject = entityTable["_Entity"];
		if (!entityObject)
			return {};

		return entityObject.as<entt::handle>();
	}

	[[noreturn]] void TriggerLuaError(lua_State* L, const std::string& errMessage)
	{
		luaL_error(L, errMessage.c_str());
		std::abort();
	}

	[[noreturn]] void TriggerLuaArgError(lua_State* L, int argIndex, const char* errMessage)
	{
		luaL_argerror(L, argIndex, errMessage);
		std::abort();
	}

	[[noreturn]] void TriggerLuaArgError(lua_State* L, int argIndex, const std::string& errMessage)
	{
		luaL_argerror(L, argIndex, errMessage.c_str());
		std::abort();
	}
}
