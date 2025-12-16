// Fill out your copyright notice in the Description page of Project Settings.

#include "GameInstance/EscapeITSubsystem.h"
#include "SaveGames/EscapeITSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"

void UEscapeITSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("UEscapeITSubsystem initialized"));
	// Optionally auto-load a save here or keep it lazy.
	CurrentSaveGame = nullptr;
}

void UEscapeITSubsystem::Deinitialize()
{
	Super::Deinitialize();
	CurrentSaveGame = nullptr;
	UE_LOG(LogTemp, Log, TEXT("UEscapeITSubsystem deinitialized"));
}

void UEscapeITSubsystem::CreateNewSave()
{
	// Create a new SaveGame object instance and set defaults
	UEscapeITSaveGame* NewSave = Cast<UEscapeITSaveGame>(UGameplayStatics::CreateSaveGameObject(UEscapeITSaveGame::StaticClass()));
	if (NewSave)
	{
		NewSave->PlayerName = TEXT("Player");
		NewSave->LastLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
		NewSave->PlayTime = 0.0f;
		NewSave->SaveDateTime = FDateTime::Now();
		NewSave->CurrentCheckpoint = 0;

		// Default player location/rotation can be left as zero or set to player start if available
		NewSave->PlayerLocation = FVector::ZeroVector;
		NewSave->PlayerRotation = FRotator::ZeroRotator;

		CurrentSaveGame = NewSave;

		UE_LOG(LogTemp, Log, TEXT("New save game created"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create new save game object"));
	}
}

bool UEscapeITSubsystem::SaveToSlot(const FString& SlotName)
{
	if (!CurrentSaveGame)
	{
		CreateNewSave();
		if (!CurrentSaveGame)
		{
			UE_LOG(LogTemp, Error, TEXT("No CurrentSaveGame available to save"));
			return false;
		}
	}

	// Update save data with current runtime values
	UpdateSaveData();

	const bool bSuccess = UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SlotName, UserIndex);
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Game saved successfully to slot: %s"), *SlotName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save game to slot: %s"), *SlotName);
	}
	return bSuccess;
}

bool UEscapeITSubsystem::SaveGame()
{
	return SaveToSlot(SaveSlotName);
}

bool UEscapeITSubsystem::QuickSave()
{
	return SaveToSlot(QuickSaveSlotName);
}

bool UEscapeITSubsystem::LoadFromSlot(const FString& SlotName)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Save file does not exist in slot: %s"), *SlotName);
		return false;
	}

	UEscapeITSaveGame* Loaded = Cast<UEscapeITSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	if (Loaded)
	{
		CurrentSaveGame = Loaded;
		UE_LOG(LogTemp, Log, TEXT("Game loaded successfully from slot: %s"), *SlotName);

		// Apply saved data to the current world/runtime
		ApplySaveData();

		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to load game from slot: %s"), *SlotName);
	return false;
}

bool UEscapeITSubsystem::LoadGame()
{
	return LoadFromSlot(SaveSlotName);
}

bool UEscapeITSubsystem::QuickLoad()
{
	return LoadFromSlot(QuickSaveSlotName);
}

void UEscapeITSubsystem::ApplySaveData()
{
	if (!CurrentSaveGame) return;

	// Get player controller and pawn
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	if (Pawn)
	{
		// Teleport pawn to saved location/rotation
		Pawn->SetActorLocation(CurrentSaveGame->PlayerLocation);
		Pawn->SetActorRotation(CurrentSaveGame->PlayerRotation);
	}
	else
	{
		// Pawn not yet possessed; you may need to handle this after character spawn/possession.
		UE_LOG(LogTemp, Verbose, TEXT("No pawn possessed at ApplySaveData time - consider applying player transform after spawn/possession"));
	}

	// If saved level is different from current level, you might want to open it:
	 const FString& SavedLevel = CurrentSaveGame->LastLevelName;
	 if (!SavedLevel.IsEmpty() && SavedLevel != UGameplayStatics::GetCurrentLevelName(GetWorld()))
	 {
	     UGameplayStatics::OpenLevel(GetWorld(), FName(*SavedLevel));
	 }

	// TODO: apply other data (inventory, quests, stats...)
}

void UEscapeITSubsystem::UpdateSaveData()
{
	if (!CurrentSaveGame) return;

	// Update player transform
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		APawn* Pawn = PC->GetPawn();
		if (Pawn)
		{
			CurrentSaveGame->PlayerLocation = Pawn->GetActorLocation();
			CurrentSaveGame->PlayerRotation = Pawn->GetActorRotation();
		}
	}

	CurrentSaveGame->PlayTime += GetWorld() ? GetWorld()->GetTimeSeconds() - LastPlayTimeCheckpoint : 0.0f;
	LastPlayTimeCheckpoint = GetWorld() ? GetWorld()->GetTimeSeconds() : LastPlayTimeCheckpoint;

	// Update level name and timestamp
	CurrentSaveGame->LastLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	CurrentSaveGame->SaveDateTime = FDateTime::Now();

	// Update other fields as needed (play time, inventory, etc.)
}

bool UEscapeITSubsystem::DoesSaveExist() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex);
}

void UEscapeITSubsystem::DeleteSave()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
	{
		const bool bSuccess = UGameplayStatics::DeleteGameInSlot(SaveSlotName, UserIndex);
		if (bSuccess)
		{
			CurrentSaveGame = nullptr;
			UE_LOG(LogTemp, Log, TEXT("Save file deleted successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to delete save file"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No save file to delete in slot: %s"), *SaveSlotName);
	}
}

bool UEscapeITSubsystem::GetSaveInfo(FString& OutLevelName, FDateTime& OutSaveTime, float& OutPlayTime)
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
	{
		return false;
	}

	UEscapeITSaveGame* TempSave = Cast<UEscapeITSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));
	if (TempSave)
	{
		OutLevelName = TempSave->LastLevelName;
		OutSaveTime = TempSave->SaveDateTime;
		OutPlayTime = TempSave->PlayTime;
		return true;
	}

	return false;
}
