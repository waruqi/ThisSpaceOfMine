// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "This Space Of Mine" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef TSOM_SERVERLIB_SERVERENVIRONMENT_HPP
#define TSOM_SERVERLIB_SERVERENVIRONMENT_HPP

#include <ServerLib/Export.hpp>
#include <CommonLib/EnvironmentTransform.hpp>
#include <ServerLib/ServerInstance.hpp>
#include <Nazara/Core/EnttWorld.hpp>
#include <Nazara/Core/Node.hpp>
#include <tsl/hopscotch_map.h>
#include <memory>

namespace Nz
{
	class EnttWorld;
}

namespace tsom
{
	class GravityController;
	class ServerPlayer;

	enum class ServerEnvironmentType
	{
		Planet,
		Ship
	};

	class TSOM_SERVERLIB_API ServerEnvironment
	{
		public:
			ServerEnvironment(const ServerEnvironment&) = delete;
			ServerEnvironment(ServerEnvironment&&) = delete;
			virtual ~ServerEnvironment();

			inline bool CompareAndUpdateConnectedTransform(ServerEnvironment& environment, const EnvironmentTransform& transform);
			void Connect(ServerEnvironment& environment, const EnvironmentTransform& transform);
			virtual entt::handle CreateEntity() = 0;
			void Disconnect(ServerEnvironment& environment);

			template<typename F> void ForEachConnectedEnvironment(F&& callback) const;
			template<typename F> void ForEachPlayer(F&& callback);
			template<typename F> void ForEachPlayer(F&& callback) const;

			inline bool GetEnvironmentTransformation(ServerEnvironment& targetEnv, EnvironmentTransform* transform) const;
			virtual const GravityController* GetGravityController() const = 0;
			inline ServerEnvironmentType GetType() const;
			inline Nz::EnttWorld& GetWorld();
			inline const Nz::EnttWorld& GetWorld() const;

			virtual void OnSave() = 0;
			virtual void OnTick(Nz::Time elapsedTime);

			void RegisterPlayer(ServerPlayer* player);
			void UnregisterPlayer(ServerPlayer* player);

			inline void UpdateConnectedTransform(ServerEnvironment& environment, const EnvironmentTransform& transform);

			ServerEnvironment& operator=(const ServerEnvironment&) = delete;
			ServerEnvironment& operator=(ServerEnvironment&&) = delete;

		protected:
			ServerEnvironment(ServerInstance& serverInstance, ServerEnvironmentType type);

			std::unique_ptr<Nz::EnttWorld> m_world;
			tsl::hopscotch_map<ServerEnvironment*, EnvironmentTransform> m_connectedEnvironments;
			Nz::Bitset<Nz::UInt64> m_registeredPlayers;
			ServerEnvironmentType m_type;
			ServerInstance& m_serverInstance;
	};
}

#include <ServerLib/ServerEnvironment.inl>

#endif // TSOM_SERVERLIB_SERVERENVIRONMENT_HPP
