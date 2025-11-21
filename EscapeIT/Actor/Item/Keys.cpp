// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeIT/Actor/Item/Keys.h"

AKeys::AKeys()
{
	// Set default values
	KeyType = EKeyType::RustyKey;
	KeyID = FName("Key_Default");
	bIsSingleUse = false;
	MaxUses = -1; // -1 means infinite uses
	CurrentUses = MaxUses;
    
	// Set item type to tool
	if (ItemDataTable)
	{
		ItemID = KeyID;
	}
}

void AKeys::BeginPlay()
{
	Super::BeginPlay();
    
	CurrentUses = MaxUses;
    
	// Automatically set ItemID based on KeyID if not set
	if (ItemID.IsNone() && !KeyID.IsNone())
	{
		ItemID = KeyID;
	}
}

bool AKeys::CanBeUsed() const
{
	if (MaxUses < 0)
	{
		return true; // Infinite uses
	}
    
	return CurrentUses > 0;
}