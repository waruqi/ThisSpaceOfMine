// Copyright (C) 2023 Jérôme "Lynix" Leclercq (lynix680@gmail.com)
// This file is part of the "This Space Of Mine" project
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Widgets/Debug.hpp>

namespace tsom
{
	inline float Planet::GetCornerRadius() const
	{
		return m_cornerRadius;
	}

	inline std::size_t Planet::GetGridDimensions() const
	{
		return m_gridDimensions;
	}

	inline float Planet::GetTileSize() const
	{
		return m_tileSize;
	}
}

#include <Nazara/Widgets/DebugOff.hpp>

