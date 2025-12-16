#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Data/SanityStructs.h"
#include "SanitySaveGame.generated.h"

UCLASS()
class ESCAPEIT_API USanitySaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    USanitySaveGame();

    UPROPERTY(VisibleAnywhere, Category = "Sanity")
    FSanitySaveData SanityData;

    // Thông tin save slot
    UPROPERTY(VisibleAnywhere, Category = "SaveInfo")
    FString SaveSlotName;

    UPROPERTY(VisibleAnywhere, Category = "SaveInfo")
    int32 UserIndex;

    // Game progress
    UPROPERTY(VisibleAnywhere, Category = "Progress")
    int32 CurrentRoomIndex;

    UPROPERTY(VisibleAnywhere, Category = "Progress")
    FVector PlayerLocation;

    UPROPERTY(VisibleAnywhere, Category = "Progress")
    FRotator PlayerRotation;

    // Statistics
    UPROPERTY(VisibleAnywhere, Category = "Stats")
    float TotalPlayTime;

    UPROPERTY(VisibleAnywhere, Category = "Stats")
    int32 DeathCount;

    UPROPERTY(VisibleAnywhere, Category = "Stats")
    int32 PuzzlesSolved;
};