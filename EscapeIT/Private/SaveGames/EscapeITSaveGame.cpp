// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveGames/EscapeITSaveGame.h"

UEscapeITSaveGame::UEscapeITSaveGame()
{
	// Default values
	PlayerName = TEXT("Player");
	LastLevelName = TEXT("MainMenu");
	PlayerLocation = FVector::ZeroVector;
	PlayerRotation = FRotator::ZeroRotator;
	PlayerHealth = 100.0f;
	PlayerStamina = 100.0f;

	CurrentCheckpoint = 0;
	PlayTime = 0.0f;
	DeathCount = 0;
	ItemsCollected = 0;
	SanityLevel = 100.0f;
	JumpScareCount = 0;

	DifficultyLevel = 1; // Normal
	bHintsEnabled = true;

	SaveDateTime = FDateTime::Now();
	FirstPlayDateTime = FDateTime::Now();
}

void UEscapeITSaveGame::AddCollectedItem(FName ItemName)
{
	if (!CollectedItems.Contains(ItemName))
	{
		CollectedItems.Add(ItemName, true);
		ItemsCollected++;
	}
}

bool UEscapeITSaveGame::HasCollectedItem(FName ItemName) const
{
	return CollectedItems.Contains(ItemName) && CollectedItems[ItemName];
}

void UEscapeITSaveGame::UnlockDoor(FName DoorName)
{
	UnlockedDoors.Add(DoorName, true);
}

bool UEscapeITSaveGame::IsDoorUnlocked(FName DoorName) const
{
	return UnlockedDoors.Contains(DoorName) && UnlockedDoors[DoorName];
}

void UEscapeITSaveGame::CompletePuzzle(FName PuzzleName)
{
	SolvedPuzzles.Add(PuzzleName, true);
}

bool UEscapeITSaveGame::IsPuzzleSolved(FName PuzzleName) const
{
	return SolvedPuzzles.Contains(PuzzleName) && SolvedPuzzles[PuzzleName];
}

void UEscapeITSaveGame::AddToInventory(FName ItemName, int32 Quantity)
{
	if (!InventoryItems.Contains(ItemName))
	{
		InventoryItems.Add(ItemName);
		ItemQuantities.Add(ItemName, Quantity);
	}
	else
	{
		ItemQuantities[ItemName] += Quantity;
	}
}

void UEscapeITSaveGame::RemoveFromInventory(FName ItemName, int32 Quantity)
{
	if (ItemQuantities.Contains(ItemName))
	{
		ItemQuantities[ItemName] -= Quantity;

		if (ItemQuantities[ItemName] <= 0)
		{
			ItemQuantities.Remove(ItemName);
			InventoryItems.Remove(ItemName);
		}
	}
}

int32 UEscapeITSaveGame::GetItemQuantity(FName ItemName) const
{
	if (ItemQuantities.Contains(ItemName))
	{
		return ItemQuantities[ItemName];
	}
	return 0;
}