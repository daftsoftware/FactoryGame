// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGVoxelChunk.h"
#include "FGVoxelUtils.h"

void FFGVoxelChunk::SetVoxel(FIntVector VoxelCoordinate, uint32 VoxelType)
{
	SetVoxel(UFGVoxelUtils::FlattenVoxelCoord(VoxelCoordinate), VoxelType);
}

void FFGVoxelChunk::SetVoxel(int32 VoxelIndex, uint32 VoxelType)
{
	uint32 PaletteIndex = GetVoxelBits(VoxelIndex * BitsPerVoxel);

	FPaletteEntry* RESTRICT PaletteDataPtr = Palette.GetData();
	PaletteDataPtr[PaletteIndex].RefCount -= 1;

	// Does the block type already exist in the palette?
	const int32& IndexToReplace = Palette.IndexOfByPredicate([&VoxelType](const FPaletteEntry& Entry)
	{
		return Entry.VoxelType == VoxelType;
	});

	if(IndexToReplace != INDEX_NONE) // Use existing palette entry.
	{
		SetVoxelBits(VoxelIndex * BitsPerVoxel, IndexToReplace);
		Palette[IndexToReplace].RefCount += 1;
		return;
	}

	// Can we overwrite the current palette entry?
	if(PaletteDataPtr[PaletteIndex].RefCount == 0) // Overwrite entry.
	{
		PaletteDataPtr[PaletteIndex].VoxelType = VoxelType;
		PaletteDataPtr[PaletteIndex].RefCount = 1;
		return;
	}

	// New palette entry is needed, get first free palette entry, growing if needed.
	const uint32 NewEntry = AddPaletteEntry();
	Palette[NewEntry] = FPaletteEntry(1, VoxelType);
	SetVoxelBits(VoxelIndex * BitsPerVoxel, NewEntry);

	PaletteCount += 1;
}

uint32& FFGVoxelChunk::GetVoxel(FIntVector VoxelCoordinate)
{
	return GetVoxel(UFGVoxelUtils::FlattenVoxelCoord(VoxelCoordinate));
}

uint32& FFGVoxelChunk::GetVoxel(int32 VoxelIndex)
{
	uint32 PaletteIndex = GetVoxelBits(VoxelIndex* BitsPerVoxel);

	FPaletteEntry* RESTRICT EntryPtr = Palette.GetData();
	return (EntryPtr + PaletteIndex)->VoxelType;
}

void FFGVoxelChunk::ShrinkPalette()
{
	using namespace FG::Const;
	
	// Remove old entries.
	for(int32 Index = 0; Index < Palette.Num(); Index++)
	{
		if(Palette[Index].RefCount == 0)
		{
			Palette[Index] = FPaletteEntry();
			PaletteCount -= 1;
		}
	}

	// Is the palette less than half of it's closest power of 2?
	if(PaletteCount > FMath::RoundUpToPowerOfTwo(PaletteCount / 2))
	{
		return; // Palette cannot be shrunk.
	}

	// Decode all indices
	TArray<uint32, TFixedAllocator<ChunkSizeXYZ>> Indices;
	for(int32 Index = 0; Index < ChunkSizeXYZ; Index++)
	{
		VoxelData.GetRange(Index * BitsPerVoxel, BitsPerVoxel, Indices.GetData(), Index * BitsPerVoxel);
	}

	// Create new palette, halfing it in size
	BitsPerVoxel = BitsPerVoxel >> 1;
	TArray<FPaletteEntry> NewPalette;
	NewPalette.AddDefaulted(pow(2, BitsPerVoxel));

	// Compress the palette entries
	uint32 PaletteCounter = 0;
	for(int32 PaletteIndex = 0; PaletteIndex < Palette.Num(); PaletteIndex++, PaletteCounter++)
	{
		const FPaletteEntry& Entry = NewPalette[PaletteIndex] = Palette[PaletteIndex];

		// Re-encode the indices (find and replace; with limit)
		int32 FoundCounter = 0;
		for(int32 DecodeIndex = 0; DecodeIndex < Indices.Num() && FoundCounter < Entry.RefCount; DecodeIndex++)
		{
			if(PaletteIndex == Indices[DecodeIndex])
			{
				Indices[DecodeIndex] = PaletteCounter;
				FoundCounter += 1;
			}
		}
	}

	// Allocate new bit array
	VoxelData = TBitArray(NULL, ChunkSizeXYZ * BitsPerVoxel);

	// Encode indices
	for(int32 Index = 0; Index < Indices.Num(); Index++)
	{
		//VoxelData.SetRangeFromRange(BitsPerVoxel * Index, BitsPerVoxel, &Indices[Index]);
		SetVoxelBits(BitsPerVoxel * Index, Indices[Index]);
	}
}

uint32 FFGVoxelChunk::AddPaletteEntry()
{
	const int32 FirstFree = Palette.IndexOfByPredicate([](const FPaletteEntry& Entry)
	{
		return (Entry.RefCount == 0);
	});

	if(FirstFree != INDEX_NONE) // Use existing free entry.
	{
		return FirstFree;
	}

	// No free entries, grow the palette.
	GrowPalette();

	// Recursively retry the operation.
	return AddPaletteEntry();
}

void FFGVoxelChunk::GrowPalette()
{
	using namespace FG::Const;
	
	// Decode indices
	TStaticArray<uint32, ChunkSizeXYZ> Indices;
	for(int32 Index = 0; Index < Indices.Num(); Index++)
	{
		Indices[Index] = GetVoxelBits(Index * BitsPerVoxel);
	}

	// Create new palette, doubling it in size
	BitsPerVoxel = BitsPerVoxel << 1;
	const uint32 NewNumElems = 1 << BitsPerVoxel; // 2^BitsPerVoxel

	// Resize and copy the old palette to the new one
	TArray<FPaletteEntry> NewPalette;
	NewPalette.AddUninitialized(NewNumElems);
	DefaultConstructItems<FPaletteEntry>(NewPalette.GetData(), NewNumElems);
	FMemory::Memcpy(NewPalette.GetData(), Palette.GetData(), PaletteCount * sizeof(FPaletteEntry)); // Copy the old palette to the new one
	Palette = MoveTemp(NewPalette); // Move the new palette to the old one
	
	// Allocate new voxel data
	VoxelData = TBitArray(false, ChunkSizeXYZ * BitsPerVoxel);

	// Encode indices
	for(int32 Index = 0; Index < Indices.Num(); Index++)
	{
		SetVoxelBits(Index * BitsPerVoxel, Indices[Index]);
	}
}
