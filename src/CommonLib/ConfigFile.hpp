// Copyright (C) 2024 Jérôme Leclercq
// This file is part of the "This Space Of Mine" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef TSOM_COMMONLIB_CONFIGFILE_HPP
#define TSOM_COMMONLIB_CONFIGFILE_HPP

#include <CommonLib/Export.hpp>
#include <NazaraUtils/Bitset.hpp>
#include <NazaraUtils/Result.hpp>
#include <NazaraUtils/Signal.hpp>
#include <sol/forward.hpp>
#include <filesystem>
#include <fstream>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

namespace tsom
{
	class TSOM_COMMONLIB_API ConfigFile
	{
		public:
			ConfigFile() = default;
			ConfigFile(const ConfigFile&) = delete;
			ConfigFile(ConfigFile&&) noexcept = default;
			~ConfigFile() = default;

			inline bool GetBoolValue(const std::string& optionName) const;
			template<typename T> T GetFloatValue(const std::string& optionName) const;
			template<typename T> T GetIntegerValue(const std::string& optionName) const;
			inline std::size_t GetOptionIndex(const std::string& optionName) const;
			inline const std::string& GetStringValue(const std::string& optionName) const;

			inline Nz::Signal<bool>& GetBoolUpdateSignal(const std::string& optionName);
			inline Nz::Signal<double>& GetFloatUpdateSignal(const std::string& optionName);
			inline Nz::Signal<long long>& GetIntegerUpdateSignal(const std::string& optionName);
			inline Nz::Signal<const std::string&>& GetStringUpdateSignal(const std::string& optionName);

			bool LoadFromFile(const std::filesystem::path& filePath);
			bool SaveToFile(const std::filesystem::path& filePath);

			inline bool SetBoolValue(const std::string& optionName, bool value);
			bool SetFloatValue(const std::string& optionName, double value);
			bool SetIntegerValue(const std::string& optionName, long long value);
			bool SetStringValue(const std::string& optionName, std::string value);

			ConfigFile& operator=(const ConfigFile&) = delete;
			ConfigFile& operator=(ConfigFile&&) = delete;

		protected:
			using FloatValidation = std::function<Nz::Result<double, std::string>(double value)>;
			using IntegerValidation = std::function<Nz::Result<long long, std::string>(long long value)>;
			using StringValidation = std::function<Nz::Result<std::string, std::string>(std::string value)>;

			inline void RegisterBoolOption(std::string optionName, std::optional<bool> defaultValue = std::nullopt);
			inline void RegisterFloatOption(std::string optionName, std::optional<double> defaultValue = std::nullopt, FloatValidation validation = nullptr);
			inline void RegisterFloatOption(std::string optionName, double minBounds, double maxBounds, std::optional<double> defaultValue = std::nullopt, FloatValidation validation = nullptr);
			inline void RegisterIntegerOption(std::string optionName, std::optional<long long> defaultValue = std::nullopt, IntegerValidation validation = nullptr);
			inline void RegisterIntegerOption(std::string optionName, long long minBounds, long long maxBounds, std::optional<long long> defaultValue = std::nullopt, IntegerValidation validation = nullptr);
			inline void RegisterStringOption(std::string optionName, std::optional<std::string> defaultValue = std::nullopt, StringValidation validation = nullptr);

		private:
			struct BoolOption
			{
				bool value;
				std::optional<bool> defaultValue;

				NazaraSignal(OnValueUpdate, bool /*newValue*/);
			};

			struct FloatOption
			{
				double maxBounds;
				double minBounds;
				double value;
				FloatValidation validation;
				std::optional<double> defaultValue;

				NazaraSignal(OnValueUpdate, double /*newValue*/);
			};

			struct IntegerOption
			{
				long long maxBounds;
				long long minBounds;
				long long value;
				IntegerValidation validation;
				std::optional<long long> defaultValue;

				NazaraSignal(OnValueUpdate, long long /*newValue*/);
			};

			struct StringOption
			{
				std::string value;
				StringValidation validation;
				std::optional<std::string> defaultValue;

				NazaraSignal(OnValueUpdate, const std::string& /*newValue*/);
			};

			using ConfigData = std::variant<BoolOption, FloatOption, IntegerOption, StringOption>;

			struct ConfigOption
			{
				std::size_t index;
				std::string name;
				ConfigData data;
			};

			struct ConfigSection
			{
				std::string sectionName;
				std::unordered_map<std::string, std::size_t> options;
				std::unordered_map<std::string, std::unique_ptr<ConfigSection>> subsections;
			};

			bool LoadSection(sol::table table, ConfigSection& section, Nz::Bitset<>& missingOptions, const std::string& prefix = {});

			template<typename T> void RegisterOption(std::string optionName, T&& optionData);
			void RegisterConfig(std::string optionName, ConfigData value);

			void SaveSectionToFile(std::fstream& file, const ConfigSection& section, std::size_t indentCount);

			std::unordered_map<std::string, std::size_t> m_optionByName;
			std::vector<ConfigOption> m_options;
			ConfigSection m_globalSection;
	};
}

#include <CommonLib/ConfigFile.inl>

#endif // TSOM_COMMONLIB_CONFIGFILE_HPP
