// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "This Space Of Mine" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef TSOM_COMMONLIB_SCRIPTING_SHAREDENTITYSCRIPTINGLIBRARY_HPP
#define TSOM_COMMONLIB_SCRIPTING_SHAREDENTITYSCRIPTINGLIBRARY_HPP

#include <CommonLib/Export.hpp>
#include <CommonLib/Scripting/ScriptingLibrary.hpp>
#include <entt/entt.hpp>
#include <sol/state.hpp>

namespace tsom
{
	class EntityRegistry;

	class TSOM_COMMONLIB_API SharedEntityScriptingLibrary : public ScriptingLibrary
	{
		public:
			inline SharedEntityScriptingLibrary(EntityRegistry& entityRegistry);
			SharedEntityScriptingLibrary(const SharedEntityScriptingLibrary&) = delete;
			SharedEntityScriptingLibrary(SharedEntityScriptingLibrary&&) = delete;
			virtual ~SharedEntityScriptingLibrary();

			void Register(sol::state& state) override;

			sol::table ToEntityTable(sol::state_view& state, entt::handle entity);

			SharedEntityScriptingLibrary& operator=(const SharedEntityScriptingLibrary&) = delete;
			SharedEntityScriptingLibrary& operator=(SharedEntityScriptingLibrary&&) = delete;

			using AddComponentFunc = sol::object(*)(sol::this_state L, entt::handle entity, sol::optional<sol::table> parameters);
			using GetComponentFunc = sol::object(*)(sol::this_state L, entt::handle entity);

			struct ComponentEntry
			{
				SharedEntityScriptingLibrary::AddComponentFunc addComponent;
				SharedEntityScriptingLibrary::GetComponentFunc getComponent;

				template<typename T> static constexpr AddComponentFunc DefaultAdd();
				template<typename T> static constexpr GetComponentFunc DefaultGet();
				template<typename T> static constexpr ComponentEntry Default();
			};

		protected:
			virtual void FillConstants(sol::state& state, sol::table constants);
			virtual void FillEntityMetatable(sol::state& state, sol::table entityMetatable);

			virtual void HandleInit(sol::table classMetatable, entt::handle entity);
			virtual bool RegisterEvent(sol::table classMetatable, std::string_view eventName, sol::protected_function callback);

			virtual AddComponentFunc RetrieveAddComponentHandler(std::string_view componentType);
			virtual GetComponentFunc RetrieveGetComponentHandler(std::string_view componentType);

		private:
			void RegisterComponents(sol::state& state);
			void RegisterConstants(sol::state& state);
			void RegisterEntityBuilder(sol::state& state);
			void RegisterEntityMetatable(sol::state& state);
			void RegisterEntityRegistry(sol::state& state);
			void RegisterPhysics(sol::state& state);

			EntityRegistry& m_entityRegistry;
			sol::table m_entityMetatable;
	};
}

#include <CommonLib/Scripting/SharedEntityScriptingLibrary.inl>

#endif // TSOM_COMMONLIB_SCRIPTING_SHAREDENTITYSCRIPTINGLIBRARY_HPP
