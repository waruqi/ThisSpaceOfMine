// Copyright (C) 2024 Jérôme "SirLynix" Leclercq (lynix680@gmail.com)
// This file is part of the "This Space Of Mine" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <CommonLib/Ship.hpp>
#include <CommonLib/BlockLibrary.hpp>
#include <CommonLib/FlatChunk.hpp>
#include <CommonLib/GameConstants.hpp>
#include <Nazara/Physics3D/Collider3D.hpp>

namespace tsom
{
	Ship::Ship(float tileSize) :
	ChunkContainer(tileSize),
	m_upDirection(Nz::Vector3f::Up())
	{
	}

	FlatChunk& Ship::AddChunk(const BlockLibrary& blockLibrary, const ChunkIndices& indices, const Nz::FunctionRef<void(BlockIndex* blocks)>& initCallback)
	{
		assert(!m_chunks.contains(indices));
		ChunkData chunkData;
		chunkData.chunk = std::make_shared<FlatChunk>(blockLibrary, *this, indices, Nz::Vector3ui{ ChunkSize }, m_tileSize);

		if (initCallback)
			chunkData.chunk->Reset(initCallback);

		chunkData.onReset.Connect(chunkData.chunk->OnReset, [this](Chunk* chunk)
		{
			OnChunkUpdated(this, chunk, DirectionMask_All);
		});

		chunkData.onUpdated.Connect(chunkData.chunk->OnBlockUpdated, [this](Chunk* chunk, const Nz::Vector3ui& indices, BlockIndex /*newBlock*/)
		{
			DirectionMask neighborMask;
			if (indices.x == 0)
				neighborMask |= Direction::Left;
			else if (indices.x == chunk->GetSize().x - 1)
				neighborMask |= Direction::Right;

			if (indices.y == 0)
				neighborMask |= Direction::Front;
			else if (indices.y == chunk->GetSize().y - 1)
				neighborMask |= Direction::Back;

			if (indices.z == 0)
				neighborMask |= Direction::Up;
			else if (indices.z == chunk->GetSize().z - 1)
				neighborMask |= Direction::Down;

			OnChunkUpdated(this, chunk, neighborMask);
		});

		auto it = m_chunks.insert_or_assign(indices, std::move(chunkData)).first;

		OnChunkAdded(this, it->second.chunk.get());

		return *it->second.chunk;
	}

	std::shared_ptr<Nz::Collider3D> Ship::BuildHullCollider() const
	{
		if (m_chunks.size() > 1)
		{
			std::vector<Nz::CompoundCollider3D::ChildCollider> childColliders;
			for (auto&& [ChunkIndices, chunkData] : m_chunks)
			{
				auto chunkCollider = chunkData.chunk->BuildCollider();
				if (!chunkCollider)
					continue;

				auto& childCollider = childColliders.emplace_back();
				childCollider.collider = chunkData.chunk->BuildCollider();
				childCollider.offset = GetChunkOffset(chunkData.chunk->GetIndices());
			}

			if (childColliders.empty())
				return nullptr;

			return std::make_shared<Nz::CompoundCollider3D>(std::move(childColliders));
		}
		else
		{
			if (m_chunks.empty())
				return nullptr;

			return m_chunks.begin().value().chunk->BuildCollider();
		}
	}

	auto Ship::ComputeGravity(const Nz::Vector3f& /*position*/) const -> GravityForce
	{
		return GravityForce {
			.direction = -m_upDirection,
			.acceleration = Constants::ShipGravityAcceleration,
			.factor = 1.f
		};
	}

	void Ship::ForEachChunk(Nz::FunctionRef<void(const ChunkIndices& chunkIndices, Chunk& chunk)> callback)
	{
		for (auto&& [chunkIndices, chunkData] : m_chunks)
			callback(chunkIndices, *chunkData.chunk);
	}

	void Ship::ForEachChunk(Nz::FunctionRef<void(const ChunkIndices& chunkIndices, const Chunk& chunk)> callback) const
	{
		for (auto&& [chunkIndices, chunkData] : m_chunks)
			callback(chunkIndices, *chunkData.chunk);
	}

	void Ship::Generate(const BlockLibrary& blockLibrary, bool small)
	{
		FlatChunk& chunk = AddChunk(blockLibrary, { 0, 0, 0 });

		BlockIndex hullIndex = blockLibrary.GetBlockIndex("hull");
		if (hullIndex == InvalidBlockIndex)
			return;

		BlockIndex forcefieldIndex = blockLibrary.GetBlockIndex("forcefield");

		unsigned int boxSize = (small) ? 6 : 12;
		unsigned int height = (small) ? 4 : 6;
		Nz::Vector3ui startPos = chunk.GetSize() / 2 - Nz::Vector3ui(boxSize / 2, boxSize / 2, height / 2);

		chunk.LockWrite();
		chunk.Reset();

		for (unsigned int z = 0; z < height; ++z)
		{
			for (unsigned int y = 0; y < boxSize; ++y)
			{
				for (unsigned int x = 0; x < boxSize; ++x)
				{
					if (x != 0 && x != boxSize - 1 &&
						y != 0 && y != boxSize - 1 &&
						z != 0 && z != height - 1)
						continue;

					if (x == 0 && y == boxSize / 2 && z > 0 && z < height - 1)
						chunk.UpdateBlock(startPos + Nz::Vector3ui{ x, y, z }, forcefieldIndex);
					else
						chunk.UpdateBlock(startPos + Nz::Vector3ui{ x, y, z }, hullIndex);
				}
			}
		}

		chunk.UnlockWrite();
	}

	void Ship::RemoveChunk(const ChunkIndices& indices)
	{
		auto it = m_chunks.find(indices);
		assert(it != m_chunks.end());

		OnChunkRemove(this, it->second.chunk.get());
		m_chunks.erase(it);
	}
}
