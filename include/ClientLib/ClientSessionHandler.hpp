// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "This Space Of Mine" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef TSOM_CLIENTLIB_CLIENTSESSIONHANDLER_HPP
#define TSOM_CLIENTLIB_CLIENTSESSIONHANDLER_HPP

#include <ClientLib/Export.hpp>
#include <CommonLib/EntityRegistry.hpp>
#include <CommonLib/EnvironmentTransform.hpp>
#include <CommonLib/SessionHandler.hpp>
#include <CommonLib/Protocol/Packets.hpp>
#include <CommonLib/Scripting/ScriptingContext.hpp>
#include <Nazara/Core/Node.hpp>
#include <NazaraUtils/Signal.hpp>
#include <entt/entt.hpp>
#include <tsl/hopscotch_map.h>

namespace Nz
{
	class Animation;
	class ApplicationBase;
	class EnttWorld;
	class Model;
	class TextSprite;
}

namespace tsom
{
	class ClientBlockLibrary;
	class GravityController;
	struct PlayerAnimationAssets;

	class TSOM_CLIENTLIB_API ClientSessionHandler : public SessionHandler
	{
		public:
			struct PlayerInfo;

			ClientSessionHandler(NetworkSession* session, Nz::ApplicationBase& app, Nz::EnttWorld& world, ClientBlockLibrary& blockLibrary);
			~ClientSessionHandler();

			void EnableShipControl(bool enable);

			inline entt::handle GetControlledEntity() const;
			inline const GravityController* GetGravityController(std::size_t environmentIndex) const;
			inline ScriptingContext& GetScriptingContext();

			void HandlePacket(Packets::AuthResponse&& authResponse);
			void HandlePacket(Packets::ChatMessage&& chatMessage);
			void HandlePacket(Packets::ChunkCreate&& chunkCreate);
			void HandlePacket(Packets::ChunkDestroy&& chunkDestroy);
			void HandlePacket(Packets::ChunkReset&& chunkReset);
			void HandlePacket(Packets::ChunkUpdate&& chunkUpdate);
			void HandlePacket(Packets::DebugDrawLineList&& debugDrawLineList);
			void HandlePacket(Packets::EntitiesCreation&& entitiesCreation);
			void HandlePacket(Packets::EntitiesDelete&& entitiesDelete);
			void HandlePacket(Packets::EntitiesStateUpdate&& stateUpdate);
			void HandlePacket(Packets::EntityEnvironmentUpdate&& environmentUpdate);
			void HandlePacket(Packets::EntityProcedureCall&& procedureCall);
			void HandlePacket(Packets::EntityPropertyUpdate&& propertyUpdate);
			void HandlePacket(Packets::EnvironmentCreate&& envCreate);
			void HandlePacket(Packets::EnvironmentDestroy&& envDestroy);
			void HandlePacket(Packets::EnvironmentUpdate&& envUpdate);
			void HandlePacket(Packets::GameData&& gameData);
			void HandlePacket(Packets::NetworkStrings&& networkStrings);
			void HandlePacket(Packets::PlayerJoin&& playerJoin);
			void HandlePacket(Packets::PlayerLeave&& playerLeave);
			void HandlePacket(Packets::PlayerNameUpdate&& playerNameUpdate);
			void HandlePacket(Packets::UpdateRootEnvironment&& playerEnv);

			void LoadScripts(bool isReloading = false);

			NazaraSignal(OnAuthResponse, const Packets::AuthResponse& /*authResponse*/);
			NazaraSignal(OnChatMessage, const std::string& /*message*/);
			NazaraSignal(OnControlledEntityChanged, entt::handle /*newEntity*/);
			NazaraSignal(OnControlledEntityStateUpdate, InputIndex /*lastInputIndex*/, const Packets::EntitiesStateUpdate::ControlledCharacter& /*characterData*/);
			NazaraSignal(OnPlayerChatMessage, const std::string& /*message*/, const PlayerInfo& /*playerInfo*/);
			NazaraSignal(OnPlayerJoined, const PlayerInfo& /*playerInfo*/);
			NazaraSignal(OnPlayerLeave, const PlayerInfo& /*playerInfo*/);
			NazaraSignal(OnPlayerNameUpdate, const PlayerInfo& /*playerInfo*/, const std::string& /*newNickname*/);
			NazaraSignal(OnShipControlUpdated, bool /*isControllingShip*/);

			static constexpr Packets::Helper::EntityId InvalidEntity = Nz::MaxValue();

			struct PlayerInfo
			{
				std::string nickname;
				bool isAuthenticated;
				std::shared_ptr<Nz::TextSprite> textSprite;
			};

		private:
			inline PlayerInfo* FetchPlayerInfo(PlayerIndex playerIndex);
			inline const PlayerInfo* FetchPlayerInfo(PlayerIndex playerIndex) const;
			void SetupEntity(entt::handle entity, Packets::Helper::PlayerControlledData&& entityData);

			struct EnvironmentData
			{
				Nz::Bitset<Nz::UInt64> entities;
				entt::handle rootEntity;
				entt::handle visualRootEntity;
				EnvironmentTransform transform;
				GravityController* gravityController;
			};

			struct EntityData
			{
				Packets::Helper::EnvironmentId environmentIndex;
				entt::handle entity;
			};

			struct PlayerModel
			{
				std::shared_ptr<Nz::Model> model;
			};

			entt::handle m_playerControlledEntity;
			std::optional<PlayerModel> m_playerModel;
			std::shared_ptr<PlayerAnimationAssets> m_playerAnimAssets;
			std::vector<std::optional<EntityData>> m_entities; //< FIXME: Nz::SparseVector
			std::vector<std::optional<EnvironmentData>> m_environments; //< FIXME: Nz::SparseVector
			std::vector<std::optional<PlayerInfo>> m_players; //< FIXME: Nz::SparseVector
			Nz::ApplicationBase& m_app;
			Nz::EnttWorld& m_world;
			ClientBlockLibrary& m_blockLibrary;
			Nz::UInt16 m_lastTickIndex;
			Nz::UInt16 m_ownPlayerIndex;
			Packets::Helper::EnvironmentId m_currentEnvironmentIndex;
			ScriptingContext m_scriptingContext;
			EntityRegistry m_entityRegistry;
			InputIndex m_lastInputIndex;
	};
}

#include <ClientLib/ClientSessionHandler.inl>

#endif // TSOM_CLIENTLIB_CLIENTSESSIONHANDLER_HPP
