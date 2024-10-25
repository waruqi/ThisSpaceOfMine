// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "This Space Of Mine" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef TSOM_CLIENTLIB_SYSTEMS_NETWORKMOVEMENTINTERPOLATIONSYSTEM_HPP
#define TSOM_CLIENTLIB_SYSTEMS_NETWORKMOVEMENTINTERPOLATIONSYSTEM_HPP

#include <ClientLib/Export.hpp>
#include <Nazara/Core/Time.hpp>
#include <NazaraUtils/TypeList.hpp>
#include <entt/entt.hpp>

namespace Nz
{
	class NodeComponent;
}

namespace tsom
{
	class TSOM_CLIENTLIB_API NetworkMovementInterpolationSystem
	{
		public:
			static constexpr bool AllowConcurrent = true;
			static constexpr Nz::Int64 ExecutionOrder = 0;
			using Components = Nz::TypeList<class NetworkInterpolationComponent, Nz::NodeComponent>;

			NetworkMovementInterpolationSystem(entt::registry& registry, Nz::Time movementTickDuration, std::size_t targetMovementPoints = 4);
			NetworkMovementInterpolationSystem(const NetworkMovementInterpolationSystem&) = delete;
			NetworkMovementInterpolationSystem(NetworkMovementInterpolationSystem&&) = delete;
			~NetworkMovementInterpolationSystem() = default;

			void Update(Nz::Time elapsedTime);

			NetworkMovementInterpolationSystem& operator=(const NetworkMovementInterpolationSystem&) = delete;
			NetworkMovementInterpolationSystem& operator=(NetworkMovementInterpolationSystem&&) = delete;

		private:
			entt::observer m_interpolatedObserver;
			std::size_t m_targetMovementPoints;
			Nz::Time m_movementTickDuration;
			entt::registry& m_registry;
	};
}

#include <ClientLib/Systems/NetworkMovementInterpolationSystem.inl>

#endif // TSOM_CLIENTLIB_SYSTEMS_NETWORKMOVEMENTINTERPOLATIONSYSTEM_HPP
