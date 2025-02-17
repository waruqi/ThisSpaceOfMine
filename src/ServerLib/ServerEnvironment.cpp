// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "This Space Of Mine" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ServerLib/ServerEnvironment.hpp>
#include <CommonLib/Physics/PhysicsSettings.hpp>
#include <ServerLib/Systems/EnvironmentProxySystem.hpp>
#include <ServerLib/Systems/NetworkedEntitiesSystem.hpp>
#include <Nazara/Physics3D/Systems/Physics3DSystem.hpp>

namespace tsom
{
	ServerEnvironment::ServerEnvironment(ServerInstance& serverInstance, ServerEnvironmentType type) :
	m_type(type),
	m_serverInstance(serverInstance)
	{
		m_world = m_serverInstance.RegisterEnvironment(this);

		auto& registry = m_world->GetRegistry();
		registry.ctx().insert_or_assign<ServerEnvironment*>(this);

		m_world->AddSystem<EnvironmentProxySystem>();
		m_world->AddSystem<NetworkedEntitiesSystem>(*this);

		// Setup physics
		Nz::Physics3DSystem::Settings physSettings = Physics::BuildSettings();
		physSettings.stepSize = m_serverInstance.GetTickDuration();

		m_world->AddSystem<Nz::Physics3DSystem>(std::move(physSettings));
	}

	ServerEnvironment::~ServerEnvironment()
	{
		// Destroy all entities first
		auto& registry = m_world->GetRegistry();
		registry.clear();

		ForEachPlayer([this](ServerPlayer& player)
		{
			player.RemoveFromEnvironment(this);
		});

		for (auto&& [environment, transform] : m_connectedEnvironments)
			environment->Disconnect(*this);

		m_world->ClearSystems();
		m_serverInstance.UnregisterEnvironment(this, std::move(m_world));
	}

	void ServerEnvironment::Connect(ServerEnvironment& environment, const EnvironmentTransform& transform)
	{
		NazaraAssertMsg(!m_connectedEnvironments.contains(&environment), "environment is already connected");
		m_connectedEnvironments.emplace(&environment, transform);

		environment.ForEachPlayer([&](ServerPlayer& player)
		{
			if (player.GetRootEnvironment() == &environment)
				player.AddToEnvironment(this);
		});
	}

	void ServerEnvironment::Disconnect(ServerEnvironment& environment)
	{
		auto it = m_connectedEnvironments.find(&environment);
		NazaraAssertMsg(it != m_connectedEnvironments.end(), "environment is not connected");
		m_connectedEnvironments.erase(it);

		environment.ForEachPlayer([&](ServerPlayer& player)
		{
			if (player.GetRootEnvironment() == &environment)
				player.RemoveFromEnvironment(this);
		});
	}

	entt::handle ServerEnvironment::CreateEntity()
	{
		return m_world->CreateEntity();
	}

	void ServerEnvironment::OnTick(Nz::Time elapsedTime)
	{
		m_world->Update(elapsedTime);
	}

	void ServerEnvironment::RegisterPlayer(ServerPlayer* player)
	{
		NazaraAssertMsg(!m_registeredPlayers.UnboundedTest(player->GetPlayerIndex()), "player was already registered");
		m_registeredPlayers.UnboundedSet(player->GetPlayerIndex());
	}

	void ServerEnvironment::UnregisterPlayer(ServerPlayer* player)
	{
		NazaraAssertMsg(m_registeredPlayers.UnboundedTest(player->GetPlayerIndex()), "player is not registered");
		m_registeredPlayers.Reset(player->GetPlayerIndex());
	}
}
