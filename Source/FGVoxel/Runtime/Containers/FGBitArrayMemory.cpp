// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "FGBitArrayMemory.h"

void FFGBitArrayMemory::SetBit(TBitArray<>& BitArray, int32 BitIndex, bool Value)
{
	uint32* RESTRICT BitArrayDataPtr = BitArray.GetData();
    uint32& Word = *(BitArrayDataPtr + BitIndex / NumBitsPerDWORD);
    uint32 BitOffset = (BitIndex % NumBitsPerDWORD);
    Word = (Word & ~(1 << BitOffset)) | (((uint32)Value) << BitOffset);
}
