// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

#include "FGBitArrayMemory.h"
#include "FGVoxelDefines.h"

/**
 * Technical Details about the Voxel Layout, Compression, and Allocation.
 *
 * Chunk Data uses palette compression because this gives us near enough
 * free lossless compression of voxel data, highly decreasing memory usage
 * but preserving cache locality, sequential, and random access times.
 *
 * Chunk Data itself is laid out in column major in order of Z, Y, X.
 * This is because during world generation we tend to iterate over the
 * XY plane rather than the Z planes, and it allows us to do some neat
 * memory tricks when it comes to dealing with heightmaps.
 */

/**
 * Palette based chunk data structure for storing voxel data.
 * Based on the following implementation inspired by Minecraft.
 * https://www.reddit.com/r/VoxelGameDev/comments/9yu8qy/palettebased_compression_for_chunked_discrete/
 *
 * Palette compression essentially works by scaling the integer
 * size that voxels are stored in by the number of unique block
 * types in the chunk. Aka if you only have 4 unique blocks in a
 * chunk, every voxel can be represented by 2 bits. If we have 256
 * unique blocks in a chunk, every voxel would be represented as a byte.
 *
 * This is incredibly efficient compared to just naively storing
 * the voxel data in a 3D array where every voxel is represented
 * by block type (uint16), which there are then 32x32x32 of in
 * a single chunk. It adds up in memory usage really quickly.
 *
 * @TODO: The only big ??? I have right now is how this changes things
 * on the GPU. I am unsure if you are able to do this kind of bitwise
 * math there, and even if you are, how does this affect uploading via
 * the scatter buffer. In theory it should be fine.
 */
struct FGVOXEL_API FFGVoxelChunk
{
	FFGVoxelChunk()
		: BitsPerVoxel(1),
		PaletteCount(1),
		VoxelData(false, FG::Const::ChunkSizeXYZ * BitsPerVoxel)
	{
		Palette.AddDefaulted(pow(2, BitsPerVoxel));
		// All the voxels should default to air
		Palette[0].VoxelType = 0;
		Palette[0].RefCount = FG::Const::ChunkSizeXYZ * BitsPerVoxel;
	}
		
	void SetVoxel(int32 VoxelIndex, uint32 VoxelType);
	void SetVoxel(FIntVector VoxelCoordinate, uint32 VoxelType);

	// @TODO: This returns a reference? this is wrong, if this is edited
	// the palette voxel type would change for the entire chunk.
	uint32& GetVoxel(int32 VoxelIndex);
	uint32& GetVoxel(FIntVector VoxelCoordinate);
	
	void	    ShrinkPalette();

	/**
	 * Set a voxel at a given index dynamically based on the bit size of the voxel.
	 * @param Index The bit that the int starts at.
	 * @param NewVoxelType The type of the voxel to set.
	 */
	FORCEINLINE void SetVoxelBits(int32 Index, const uint32 NewVoxelType)
	{
		VoxelData.SetRangeFromRange(Index, BitsPerVoxel, &NewVoxelType);
		//FFGBitArrayMemory::SetRangeFromRange(VoxelData, Index, BitsPerVoxel, NewVoxelType);
	}

	/**
	 * Get a voxel type at a given index based on the bit size of the voxel.
	 * @param Index The bit that the int starts at.
	 * @return The Decoded Voxel Type
	 */
	FORCEINLINE uint32 GetVoxelBits(int32 Index)
	{
		uint32 DecodedBits = VOXELTYPE_NONE;
		VoxelData.GetRange(Index, BitsPerVoxel, &DecodedBits);
		//FFGBitArrayMemory::GetRange(VoxelData.GetData(), Index, BitsPerVoxel, DecodedBits);
		return DecodedBits;
	}

	uint32& operator[] (uint32 Index)
	{
		uint32 PaletteIndex = VOXELTYPE_NONE;
		FFGBitArrayMemory::GetRange(VoxelData.GetData(), Index, BitsPerVoxel, PaletteIndex);

		FPaletteEntry* RESTRICT EntryPtr = Palette.GetData();
		return (EntryPtr + PaletteIndex)->VoxelType;
	}

	TArray<int32> GetVoxelTypesInChunk()
	{
		TArray<int32> OutVoxelTypes;
		OutVoxelTypes.Reserve(PaletteCount);
		
		FPaletteEntry* RESTRICT Entry = Palette.GetData();
		
		for(uint32 PaletteIndex = 0; PaletteIndex < PaletteCount; PaletteIndex++)
		{
			OutVoxelTypes.Emplace((Entry + PaletteIndex)->VoxelType);
		}
		return OutVoxelTypes;
	}

	FORCEINLINE uint32 GetTypeHash(const FFGVoxelChunk& Key) const
	{
		return ::GetTypeHash(Key.VoxelData);
	}

	bool operator==(const FFGVoxelChunk& Other) const
	{
		return GetTypeHash(*this) == GetTypeHash(Other);
	}

	bool HasAnyFlags(EFGChunkFlags InFlags) const
	{
		return (Flags & InFlags) != EFGChunkFlags::NoFlags;
	}

	void SetFlags(EFGChunkFlags InFlags)
	{
		Flags |= InFlags;
	}

	void ClearFlags(EFGChunkFlags InFlags)
	{
		Flags &= ~InFlags;
	}

private:
		
	uint32	AddPaletteEntry();
	void	GrowPalette();

	struct FPaletteEntry
	{
		int32 RefCount;		// How many voxels use this palette type.
		uint32 VoxelType;	// The type of the voxel (aka stone, dirt, etc.)

		FPaletteEntry()
			: RefCount(0),
			VoxelType(0)
		{}

		FPaletteEntry(uint32 InRefCount, uint32 InVoxelType)
			: RefCount(InRefCount),
			VoxelType(InVoxelType)
		{}

		FORCEINLINE uint32 GetTypeHash(const FPaletteEntry& Key) const
		{
			return ::GetTypeHash(Key.VoxelType);
		}
			
		bool operator==(const FPaletteEntry& Other) const
		{
			return GetTypeHash(*this) == GetTypeHash(Other);
		}
	};

	friend class UFGVoxelGrid;

	EFGChunkFlags			Flags;
	int32					BitsPerVoxel;
	uint32					PaletteCount;	// How many palette entries are in use.
	TArray<FPaletteEntry>	Palette;		// The palette of voxel types
	TBitArray<>				VoxelData;		// BitsPerVoxel * ChunkSizeXYZ
};
