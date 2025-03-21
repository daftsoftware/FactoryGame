// Copyright (C) Daft Software 2024, All Rights Reserved.
// Author: Sunny Blake-Webber

#pragma once

/*
	Namespaced Id that works a little bit like a Gameplay Tag / FName.
	
	The purpose is to add namespacing to primary asset IDs, which essentially
	allows clear separation of base game and external content such as modded
	items while avoiding name conflicts.
	
	It works as an alias for a resource and can be used to specify blocks
	or game objects. You can construct it with either the ID or the namespace
	itself.
*/
struct FFGNamespacedId
{
	int16	NumericalId		= INDEX_NONE;
	FName	Namespace		= NAME_None;
	FName	Name			= NAME_None;

	bool operator==(const FFGNamespacedId& Other) const
	{
		return Other.NumericalId == NumericalId;
	}

	uint32 GetTypeHash(const FFGNamespacedId& Key) const
	{
		checkf(Key.NumericalId != INDEX_NONE, TEXT("Numerical Id wasn't valid."));
		return Key.NumericalId;
	}
	
	bool IsValid() const
	{
		return !Namespace.IsNone() && !Name.IsNone() && NumericalId != INDEX_NONE;
	}
	
	FString ToString() const
	{
		return FString::Printf(TEXT("%s:%s"), *Namespace.ToString(), *Name.ToString());
	}

	FFGNamespacedId(FName InNamespace = TEXT("Base"), FName InName = NAME_None, int16 InNumericalId = INDEX_NONE)
		: NumericalId(InNumericalId)
		, Namespace(InNamespace)
		, Name(InName)
	{}

	FFGNamespacedId(FName InNamespace = TEXT("Base"), FName InName = NAME_None)
		: Namespace(InNamespace)
		, Name(InName)
	{
		// @TODO: Fetch numerical ID.
		checkf(NumericalId != INDEX_NONE, TEXT("Numerical Id wasn't valid."));
	}

	FFGNamespacedId(FString InString = TEXT("Base:None"))
	{
		FString NamespaceStr;
		FString NameStr;
		InString.Split(":", &NamespaceStr, &NameStr);
		Namespace = FName(*NamespaceStr);
		Name = FName(*NameStr);

		// @TODO: Fetch numerical ID.
		checkf(NumericalId != INDEX_NONE, TEXT("Numerical Id wasn't valid."));
	}

	FFGNamespacedId(int16 InNumericalId = INDEX_NONE)
		: NumericalId(InNumericalId)
	{
		// @TODO: Fetch namespace and name.
		checkf(!Name.IsNone() && !Namespace.IsNone(), TEXT("Name or Namespace wasn't valid."));
	}
};
