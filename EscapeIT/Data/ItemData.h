#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "ItemData.generated.h"

class AItemPickupActor;

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Consumable      UMETA(DisplayName = "Consumable"),
    Tool            UMETA(DisplayName = "Tool"),
    Battery         UMETA(DisplayName = "Battery"),
    QuestItem       UMETA(DisplayName = "Quest Item"),
    Document        UMETA(DisplayName = "Document"),
    Special         UMETA(DisplayName = "Special")
};

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    Flashlight      UMETA(DisplayName = "Flashlight"),
    MasterKey       UMETA(DisplayName = "Master Key"),
    Wrench          UMETA(DisplayName = "Wrench"),
    Other           UMETA(DisplayName = "Other")
};

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    TObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    EItemType ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic",
        meta = (EditCondition = "ItemType == EItemType::Tool", EditConditionHides))
    EItemCategory ItemCategory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stack")
    int32 MaxStackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Usage")
    bool bIsConsumable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Usage")
    bool bCanBeDropped;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float SanityRestoreAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float PassiveSanityDrainReduction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float UsageCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability")
    bool bHasDurability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability")
    int32 MaxUses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    TObjectPtr<UStaticMesh> ItemMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    TSubclassOf<AItemPickupActor> PickupActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundBase> PickupSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundBase> UseSound;

    FItemData()
        : ItemID(NAME_None)
        , ItemName(FText::FromString("Unknown"))
        , Description(FText::FromString(""))
        , Icon(nullptr)
        , ItemType(EItemType::Consumable)
        , ItemCategory(EItemCategory::Other)
        , MaxStackSize(1)
        , bIsConsumable(false)
        , bCanBeDropped(true)
        , SanityRestoreAmount(0.0f)
        , PassiveSanityDrainReduction(0.0f)
        , UsageCooldown(0.0f)
        , bHasDurability(false)
        , MaxUses(1)
        , ItemMesh(nullptr)
        , PickupActorClass(nullptr)
        , PickupSound(nullptr)
        , UseSound(nullptr)
    {
    }
};

// FIXED: Remove ItemData field, only store ItemID
USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RemainingUses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownRemaining;

    FInventorySlot()
        : ItemID(NAME_None)
        , Quantity(0)
        , RemainingUses(-1)
        , CooldownRemaining(0.0f)
    {
    }

    FInventorySlot(FName InItemID, int32 InQuantity)
        : ItemID(InItemID)
        , Quantity(InQuantity)
        , RemainingUses(-1)
        , CooldownRemaining(0.0f)
    {
    }

    bool IsValid() const
    {
        return !ItemID.IsNone() && Quantity > 0;
    }
};