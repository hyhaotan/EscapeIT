#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SanityStructs.generated.h"

UENUM(BlueprintType)
enum class ESanityLevel : uint8
{
    High     UMETA(DisplayName = "High (70-100%)"),
    Medium   UMETA(DisplayName = "Medium (50-70%)"),
    Low      UMETA(DisplayName = "Low (30-50%)"),
    Critical UMETA(DisplayName = "Critical (0-30%)")
};

USTRUCT(BlueprintType)
struct FSanitySaveData
{
    GENERATED_BODY()

    // Constructor with defaults
    FSanitySaveData()
        : CurrentSanity(100.f)
        , MaxSanity(100.f)
        , MinSanity(0.f)
        , CurrentLevel(ESanityLevel::High)
        , RecoveryMultiplier(1.f)
        , bIsInSafeZone(false)
        , bIsInDarkZone(false)
        , bAutoDecay(false)
        , SanityDecayRate(1.f)
        , SaveTime(FDateTime::UtcNow())
    {
    }

    // Mark properties with SaveGame so USaveGame serializes them
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
    float CurrentSanity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
    float MaxSanity;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
    float MinSanity;

    // Important: make CurrentLevel a UPROPERTY so it will be serialized/visible in Blueprints
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
    ESanityLevel CurrentLevel;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
    float RecoveryMultiplier;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
    bool bIsInSafeZone;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
    bool bIsInDarkZone;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
    bool bAutoDecay;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
    float SanityDecayRate;

    // Save timestamp
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
    FDateTime SaveTime;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame)
    TArray<FString> ActiveEffects;
};

USTRUCT(BlueprintType)
struct FSanityEventData
{
    GENERATED_BODY()

    FSanityEventData()
        : Amount(0.f)
        , EventName(TEXT(""))
        , bShowNotification(true)
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Amount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowNotification;
};
