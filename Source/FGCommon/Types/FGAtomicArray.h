// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#include "Containers/Array.h"

#pragma once

template<typename InElementType, typename InAllocator = FDefaultAllocator>
class TFGAtomicArray : public TArray<InElementType, InAllocator>
{
public:
	typedef InElementType ElementType;
	typedef InAllocator   Allocator;

	template <typename... ArgsType>
	int32 EmplaceThreadsafe(ArgsType&&... Args)
	{
		const int32 Index = AddUninitializedThreadsafe(1);
		new(this->GetData() + Index) ElementType(Forward<ArgsType>(Args)...);
		return Index;
	}


	/**
	 * Adds a given number of uninitialized elements into the array using an atomic increment on the array num
	 *
	 * Caution, the array must have sufficient slack or this will assert/crash. You must presize the array.
	 *
	 * Caution, AddUninitialized() will create elements without calling
	 * the constructor and this is not appropriate for element types that
	 * require a constructor to function properly.
	 *
	 * @param Count Number of elements to add.
	 *
	 * @returns Number of elements in array before addition.
	 */
	int32 AddUninitializedThreadsafe(int32 Count = 1)
	{
		checkSlow(Count >= 0);
		const int32 OldNum = FPlatformAtomics::InterlockedAdd(&this->ArrayNum, Count);
		check(OldNum + Count <= this->ArrayMax);
		return OldNum;
	}

	/**
	 * Adds a new item to the end of the array, using atomics to update the current size of the array
	 *
	 * Caution, the array must have sufficient slack or this will assert/crash. You must presize the array.
	 *
	 * @param Item	The item to add
	 * @return		Index to the new item
	 */
	FORCEINLINE int32 AddThreadsafe(const ElementType& Item)
	{
		return EmplaceThreadsafe(Item);
	}
};