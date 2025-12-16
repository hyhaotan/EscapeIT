// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EscapeITSubsystem.generated.h"

class UEscapeITSaveGame;

/**
 * GameInstance subsystem để quản lý save/load game
 */
UCLASS()
class ESCAPEIT_API UEscapeITSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Save/Load operations
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool SaveGame();

	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool LoadGame();

	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool DoesSaveExist() const;

	UFUNCTION(BlueprintCallable, Category = "Save System")
	void DeleteSave();

	UFUNCTION(BlueprintCallable, Category = "Save System")
	UEscapeITSaveGame* GetCurrentSaveGame() const { return CurrentSaveGame; }

	// Quick save/load
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool QuickSave();

	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool QuickLoad();

	// Create new save
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void CreateNewSave();

	// Get save info without loading
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool GetSaveInfo(FString& OutLevelName, FDateTime& OutSaveTime, float& OutPlayTime);

private:
	// Current loaded/edited save object
	UPROPERTY()
	TObjectPtr<UEscapeITSaveGame> CurrentSaveGame;

	// Defaults
	const FString SaveSlotName = TEXT("PlayerSave");
	const FString QuickSaveSlotName = TEXT("QuickSave");
	const int32 UserIndex = 0;
	double LastPlayTimeCheckpoint;

	// Helper functions
	bool SaveToSlot(const FString& SlotName);
	bool LoadFromSlot(const FString& SlotName);
	void UpdateSaveData();
	void ApplySaveData();
};
