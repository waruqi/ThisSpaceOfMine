// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "This Space Of Mine" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <ServerLib/Session/InitialSessionHandler.hpp>
#include <CommonLib/GameConstants.hpp>
#include <CommonLib/InternalConstants.hpp>
#include <CommonLib/Version.hpp>
#include <ServerLib/PlayerTokenAppComponent.hpp>
#include <ServerLib/ServerEnvironment.hpp>
#include <ServerLib/ServerInstance.hpp>
#include <ServerLib/ServerPlayer.hpp>
#include <ServerLib/Session/PlayerSessionHandler.hpp>
#include <Nazara/Core/ApplicationBase.hpp>
#include <fmt/color.h>
#include <fmt/format.h>

namespace tsom
{
	constexpr SessionHandler::SendAttributeTable s_packetAttributes = SessionHandler::BuildAttributeTable({
		{ PacketIndex<Packets::AuthResponse>,   { .channel = 0, .flags = Nz::ENetPacketFlag::Reliable } },
		{ PacketIndex<Packets::NetworkStrings>, { .channel = 1, .flags = Nz::ENetPacketFlag::Reliable } }
	});

	InitialSessionHandler::InitialSessionHandler(ServerInstance& instance, NetworkSession* session) :
	SessionHandler(session),
	m_instance(instance)
	{
		SetupHandlerTable(this);
		SetupAttributeTable(s_packetAttributes);

		auto& stringStore = session->GetStringStore();
		instance.GetEntityRegistry().ForEachClass([&](const std::string& className, const EntityClass& /*entityClass*/)
		{
			stringStore.RegisterString(className);
		});
	}

	void InitialSessionHandler::HandlePacket(Packets::AuthRequest&& authRequest)
	{
		std::uint32_t majorVersion, minorVersion, patchVersion;
		DecodeVersion(authRequest.gameVersion, majorVersion, minorVersion, patchVersion);

		auto FailAuth = [&](AuthError err)
		{
			Packets::AuthResponse response;
			response.authResult = Nz::Err(err);

			GetSession()->SendPacket(response);

			GetSession()->Disconnect(DisconnectionType::Later);
		};

		std::optional<Nz::Uuid> uuid;
		std::string login;
		PlayerPermissionFlags permissions;
		bool success = std::visit(Nz::Overloaded
		{
			[&](Packets::AuthRequest::AnonymousPlayerData& anonymousData) -> bool
			{
				login = std::move(anonymousData.nickname).Str();

				fmt::print("{0} authenticated (anonymous)\n", login);
				return true;
			},
			[&](Packets::AuthRequest::AuthenticatedPlayerData& authenticatedData) -> bool
			{
				const std::array<Nz::UInt8, 32>& key = m_instance.GetConnectionTokenEncryptionKey();

				ConnectionTokenPrivate tokenPrivate;
				ConnectionTokenAuth errorCode = ConnectionTokenPrivate::AuthAndDecrypt(authenticatedData.connectionToken, key, &tokenPrivate);
				if (errorCode != ConnectionTokenAuth::Success)
				{
					fmt::print(fg(fmt::color::red), "connection token is invalid\n", login);
					FailAuth(AuthError::InvalidToken);
					return false;
				}

				uuid = tokenPrivate.player.uuid;
				login = std::move(tokenPrivate.player.nickname);

				for (const std::string& permissionStr : tokenPrivate.player.permissions)
				{
					if (std::optional<PlayerPermission> permissionOpt = PlayerPermissionFromString(permissionStr))
						permissions |= *permissionOpt;
					else
						fmt::print(fg(fmt::color::yellow), "unknown permission \"{}\"\n", permissionStr);
				}

				// Register player token (FIXME: not really the right place)
				auto& playerTokens = m_instance.GetApplication().GetComponent<PlayerTokenAppComponent>();
				playerTokens.Register(tokenPrivate.player.uuid, tokenPrivate.api.url, tokenPrivate.api.refreshToken);

				fmt::print("{0} authenticated\n", login);
				return true;
			}
		}, authRequest.token);

		if (!success)
			return;

		if (login.empty() || login != Nz::Trim(login, Nz::UnicodeAware{}))
		{
			fmt::print(fg(fmt::color::red), "{0} nickname hasn't been trimmed\n", login);
			return FailAuth(AuthError::ProtocolError);
		}

		fmt::print("Auth request from {0} (version {1}.{2}.{3})\n", login, majorVersion, minorVersion, patchVersion);

		if (authRequest.gameVersion < Constants::ProtocolRequiredClientVersion)
		{
			fmt::print(fg(fmt::color::red), "{0} authentication failed (version is too old)\n", login);
			return FailAuth(AuthError::UpgradeRequired);
		}

		if (authRequest.gameVersion > GameVersion)
		{
			fmt::print(fg(fmt::color::red), "{0} authentication failed (version is more recent than server's)\n", login);
			return FailAuth(AuthError::ServerIsOutdated);
		}

		// Make a special dev version
		if (IsDevVersion())
			authRequest.gameVersion = Nz::MaxValue();

		NetworkSession* session = GetSession();
		session->SetProtocolVersion(authRequest.gameVersion);

		ServerPlayer* player;
		if (uuid.has_value())
			player = m_instance.CreateAuthenticatedPlayer(session, *uuid, std::move(login), permissions);
		else
			player = m_instance.CreateAnonymousPlayer(session, std::move(login));

		if (!player)
			return FailAuth(AuthError::InternalError);

		Packets::AuthResponse response;
		response.authResult = Nz::Ok();
		response.ownPlayerIndex = player->GetPlayerIndex();

		session->SendPacket(response);

		session->SendPacket(session->GetStringStore().BuildPacket());

		session->SetupHandler<PlayerSessionHandler>(player);
	}

	void InitialSessionHandler::OnDeserializationError(std::size_t packetIndex)
	{
		if (packetIndex == PacketIndex<Packets::AuthRequest>)
		{
			fmt::print("failed to deserialize Auth packet from peer {0}\n", GetSession()->GetPeerId());

			Packets::AuthResponse response;
			response.authResult = Nz::Err(AuthError::ProtocolError);

			GetSession()->SendPacket(response);

			GetSession()->Disconnect(DisconnectionType::Later);
			return;
		}
		else
		{
			fmt::print("failed to deserialize unexpected packet {1} from peer {0}\n", GetSession()->GetPeerId(), PacketNames[packetIndex]);
			GetSession()->Disconnect(DisconnectionType::Kick);
		}
	}

	void InitialSessionHandler::OnUnexpectedPacket(std::size_t packetIndex)
	{
		fmt::print("received unexpected packet {1} from peer {0}\n", GetSession()->GetPeerId(), PacketNames[packetIndex]);
		GetSession()->Disconnect(DisconnectionType::Kick);
	}

	void InitialSessionHandler::OnUnknownOpcode(Nz::UInt8 opcode)
	{
		fmt::print("received unknown packet (opcode: {1}) from peer {0}\n", GetSession()->GetPeerId(), +opcode);
		GetSession()->Disconnect(DisconnectionType::Kick);
	}
}
