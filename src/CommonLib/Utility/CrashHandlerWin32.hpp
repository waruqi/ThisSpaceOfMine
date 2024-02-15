// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "This Space Of Mine" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef TSOM_COMMONLIB_UTILITY_CRASHHANDLERWIN32_HPP
#define TSOM_COMMONLIB_UTILITY_CRASHHANDLERWIN32_HPP

#include <CommonLib/Utility/CrashHandler.hpp>
#include <Nazara/Core/DynLib.hpp>
#include <windows.h>
#include <Dbghelp.h> //< Must be included after windows.h

namespace tsom
{
	class CrashHandlerWin32 : public CrashHandler
	{
		public:
			CrashHandlerWin32() = default;
			~CrashHandlerWin32();

			bool Install() override;
			void Uninstall() override;

		private:
			Nz::DynLib m_windbg;
	};
}

#include <CommonLib/Utility/CrashHandlerWin32.inl>

#endif // TSOM_COMMONLIB_UTILITY_CRASHHANDLERWIN32_HPP
