// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "This Space Of Mine" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Game/GameConfigAppComponent.hpp>
#include <Nazara/Core/SystemDirectory.hpp>
#include <CommonLib/Version.hpp>
#include <NazaraUtils/PathUtils.hpp>
#include <fmt/color.h>
#include <fmt/format.h>

namespace tsom
{
	GameConfigFile::GameConfigFile()
	{
		RegisterStringOption("Api.Url");
		RegisterStringOption("Menu.Login", "Mingebag", [](std::string value) -> Nz::Result<std::string, std::string>
		{
			if (value.empty())
				return Nz::Err("name cannot be empty");

			if (value.size() > 32)
				return Nz::Err("name is too long");

			return Nz::Ok(std::move(value));
		});

		RegisterStringOption("Menu.ServerAddress", "tsom.digitalpulse.software");

		RegisterFloatOption("Input.MouseSensitivity", 0.0, 1.0, 0.3);

		RegisterStringOption("Player.Token", "", [](std::string value) -> Nz::Result<std::string, std::string>
		{
			if (value.size() > 64)
				return Nz::Err("Invalid token");

			return Nz::Ok(std::move(value));
		});

		RegisterIntegerOption("Server.Port", 1, 0xFFFF, 29536);
	}

	std::filesystem::path GameConfigFile::GetPath()
	{
		Nz::Result appDir = Nz::GetApplicationDirectory(Nz::ApplicationDirectory::Config, "ThisSpaceOfMine");
		if (!appDir)
		{
			fmt::print(fg(fmt::color::red), "failed to get application directory: {}\n", appDir.GetError());
			return {};
		}

		// Create if it doesn't exist
		std::filesystem::create_directories(appDir.GetValue());

		return appDir.GetValue() / Nz::Utf8Path(GameConfigFile::FileName);
	}

	GameConfigAppComponent::GameConfigAppComponent(Nz::ApplicationBase& app) :
	ApplicationComponent(app)
	{
		std::filesystem::path configPath;
		if (!IsDevVersion())
		{
			configPath = GameConfigFile::GetPath();
			if (!configPath.empty())
			{
				// In previous versions the gameconfig.lua file was stored next to the application
				if (!std::filesystem::exists(configPath))
				{
					std::filesystem::path prevConfigPath = Nz::Utf8Path(GameConfigFile::FileName);
					if (std::filesystem::is_regular_file(prevConfigPath))
						std::filesystem::rename(prevConfigPath, configPath);
				}
			}
		}

		if (configPath.empty())
			configPath = Nz::Utf8Path(GameConfigFile::FileName);

		if (!std::filesystem::is_regular_file(configPath))
		{
			// Load gameconfig.lua.default if config file is missing
			std::filesystem::path defaultConfigPath = Nz::Utf8Path(GameConfigFile::FileName);
			defaultConfigPath.replace_extension(Nz::Utf8Path(".lua.default"));
			if (std::filesystem::is_regular_file(defaultConfigPath))
				configPath = std::move(defaultConfigPath);
		}

		if (!m_configFile.LoadFromFile(configPath))
			fmt::print(fg(fmt::color::red), "failed to load game config\n");
	}

	void GameConfigAppComponent::Save()
	{
		std::filesystem::path configPath;
		if (!IsDevVersion())
			configPath = GameConfigFile::GetPath();

		if (configPath.empty())
			configPath = Nz::Utf8Path(GameConfigFile::FileName);

		if (!m_configFile.SaveToFile(configPath))
			fmt::print(fg(fmt::color::red), "failed to save game config\n");
	}
}
