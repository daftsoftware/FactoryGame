// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

/**
 * Fast path TBitArray helpers to go turbo speed because the built in versions
 * suck but somehow this one sucks less.
 */
class FFGBitArrayMemory
{
public:
	
	static void SetBit(TBitArray<>& BitArray, int32 BitIndex, bool Value);
	
	static FORCEINLINE void SetRangeFromRange(TBitArray<>& BitArray, int32 VoxelIndex, int32 BitsPerVoxel, const uint32 VoxelType)
	{
		//check(BitsPerVoxel <= 32);  // Ensure BitsPerVoxel is within the size of uint32_t

		uint32_t* RESTRICT BitArrayDataPtr = BitArray.GetData();

		size_t startBit = VoxelIndex * BitsPerVoxel;
		size_t endBit = startBit + BitsPerVoxel;

		//check(endBit <= BitArray.Num());

		size_t wordStart = startBit / 32;
		size_t bitOffsetStart = startBit % 32;
		size_t wordEnd = (endBit - 1) / 32;  // Corrected to include the last bit
		size_t bitOffsetEnd = endBit % 32;

		if (wordStart == wordEnd) {
			uint32_t mask = ((1u << BitsPerVoxel) - 1) << bitOffsetStart;
			BitArrayDataPtr[wordStart] = (BitArrayDataPtr[wordStart] & ~mask) | ((VoxelType << bitOffsetStart) & mask);
		} else {
			uint32_t startMask = ~0u << bitOffsetStart;
			BitArrayDataPtr[wordStart] = (BitArrayDataPtr[wordStart] & ~startMask) | ((VoxelType << bitOffsetStart) & startMask);

			for (size_t word = wordStart + 1; word < wordEnd; ++word) {
				BitArrayDataPtr[word] = (VoxelType >> (32 * (word - wordStart - 1))) & ~0u;
			}

			uint32_t endMask = (1u << bitOffsetEnd) - 1;
			BitArrayDataPtr[wordEnd] = (BitArrayDataPtr[wordEnd] & ~endMask) | ((VoxelType >> (32 - bitOffsetStart)) & endMask);
		}
	}
	
	FORCEINLINE static void GetRange(uint32_t* RESTRICT BitArrayDataPtr, int32 VoxelIndex, int32 BitsPerVoxel, uint32& OutVoxelType)
	{
		size_t startBit = VoxelIndex * BitsPerVoxel;
		size_t endBit = startBit + BitsPerVoxel;

		size_t wordStart = startBit / 32;
		size_t bitOffsetStart = startBit % 32;
		size_t wordEnd = (endBit - 1) / 32;
		size_t bitOffsetEnd = endBit % 32;

		if (wordStart == wordEnd) {
			uint32_t mask = ((1u << BitsPerVoxel) - 1) << bitOffsetStart;
			OutVoxelType = (BitArrayDataPtr[wordStart] & mask) >> bitOffsetStart;
		} else {
			uint32_t startMask = ~0u << bitOffsetStart;
			uint32_t result = (BitArrayDataPtr[wordStart] & startMask) >> bitOffsetStart;

			size_t bitsFetched = 32 - bitOffsetStart;

			// Use SIMD to fetch multiple words at once if possible
			for (size_t word = wordStart + 1; word < wordEnd; ++word) {
				result |= BitArrayDataPtr[word] << bitsFetched;
				bitsFetched += 32;
			}

			uint32_t endMask = (1u << bitOffsetEnd) - 1;
			result |= (BitArrayDataPtr[wordEnd] & endMask) << bitsFetched;

			OutVoxelType = result;
		}
	}
};