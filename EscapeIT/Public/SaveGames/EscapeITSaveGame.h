// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EscapeITSaveGame.generated.h"

/**
 * Save Game object containing all player progress data
 */
UCLASS()
class ESCAPEIT_API UEscapeITSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UEscapeITSaveGame();

	// ============= Player Info =============
	UPROPERTY(VisibleAnywhere, Category = "Save|Player")
	FString PlayerName;

	UPROPERTY(VisibleAnywhere, Category = "Save|Player")
	FString LastLevelName;

	UPROPERTY(VisibleAnywhere, Category = "Save|Player")
	FVector PlayerLocation;

	UPROPERTY(VisibleAnywhere, Category = "Save|Player")
	FRotator PlayerRotation;

	UPROPERTY(VisibleAnywhere, Category = "Save|Player")
	float PlayerHealth;

	UPROPERTY(VisibleAnywhere, Category = "Save|Player")
	float PlayerStamina;

	// ============= Game Progress =============
	UPROPERTY(VisibleAnywhere, Category = "Save|Progress")
	int32 CurrentCheckpoint;

	UPROPERTY(VisibleAnywhere, Category = "Save|Progress")
	TArray<FName> CompletedObjectives;

	UPROPERTY(VisibleAnywhere, Category = "Save|Progress")
	TMap<FName, bool> UnlockedDoors;

	UPROPERTY(VisibleAnywhere, Category = "Save|Progress")
	TMap<FName, bool> CollectedItems;

	UPROPERTY(VisibleAnywhere, Category = "Save|Progress")
	TArray<FName> DiscoveredLocations;

	// ============= Inventory =============
	UPROPERTY(VisibleAnywhere, Category = "Save|Inventory")
	TArray<FName> InventoryItems;

	UPROPERTY(VisibleAnywhere, Category = "Save|Inventory")
	TMap<FName, int32> ItemQuantities;

	UPROPERTY(VisibleAnywhere, Category = "Save|Inventory")
	FName EquippedItem;

	// ============= Puzzles =============
	UPROPERTY(VisibleAnywhere, Category = "Save|Puzzles")
	TMap<FName, bool> SolvedPuzzles;

	UPROPERTY(VisibleAnywhere, Category = "Save|Puzzles")
	TMap<FName, FString> PuzzleStates; // JSON strings for complex puzzle states

	// ============= Notes & Documents =============
	UPROPERTY(VisibleAnywhere, Category = "Save|Documents")
	TArray<FName> CollectedNotes;

	UPROPERTY(VisibleAnywhere, Category = "Save|Documents")
	TArray<FName> ReadDocuments;

	// ============= Statistics =============
	UPROPERTY(VisibleAnywhere, Category = "Save|Stats")
	float PlayTime;

	UPROPERTY(VisibleAnywhere, Category = "Save|Stats")
	int32 DeathCount;

	UPROPERTY(VisibleAnywhere, Category = "Save|Stats")
	int32 ItemsCollected;

	UPROPERTY(VisibleAnywhere, Category = "Save|Stats")
	FDateTime SaveDateTime;

	UPROPERTY(VisibleAnywhere, Category = "Save|Stats")
	FDateTime FirstPlayDateTime;

	// ============= Horror Events =============
	UPROPERTY(VisibleAnywhere, Category = "Save|Horror")
	TArray<FName> TriggeredHorrorEvents;

	UPROPERTY(VisibleAnywhere, Category = "Save|Horror")
	float SanityLevel;

	UPROPERTY(VisibleAnywhere, Category = "Save|Horror")
	int32 JumpScareCount;

	// ============= Audio Logs =============
	UPROPERTY(VisibleAnywhere, Category = "Save|Audio")
	TArray<FName> UnlockedAudioLogs;

	UPROPERTY(VisibleAnywhere, Category = "Save|Audio")
	TArray<FName> PlayedAudioLogs;

	// ============= Difficulty Settings =============
	UPROPERTY(VisibleAnywhere, Category = "Save|Settings")
	int32 DifficultyLevel; // 0=Easy, 1=Normal, 2=Hard, 3=Nightmare

	UPROPERTY(VisibleAnywhere, Category = "Save|Settings")
	bool bHintsEnabled;

	// Helper functions
	UFUNCTION(BlueprintCallable, Category = "Save")
	void AddCollectedItem(FName ItemName);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool HasCollectedItem(FName ItemName) const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	void UnlockDoor(FName DoorName);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool IsDoorUnlocked(FName DoorName) const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	void CompletePuzzle(FName PuzzleName);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool IsPuzzleSolved(FName PuzzleName) const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	void AddToInventory(FName ItemName, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Save")
	void RemoveFromInventory(FName ItemName, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Save")
	int32 GetItemQuantity(FName ItemName) const;
};