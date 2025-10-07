#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "ItemData.generated.h"

// forward declare AItemPickupActor để dùng TSubclassOf mà không include header (tránh circular)
class AItemPickupActor;

// Enum cho loại item
UENUM(BlueprintType)
enum class EItemType : uint8
{
    Consumable      UMETA(DisplayName = "Consumable"),     // Medkit, Painkiller
    Tool            UMETA(DisplayName = "Tool"),           // Flashlight, MasterKey
    Battery         UMETA(DisplayName = "Battery"),        // Pin đèn pin
    QuestItem       UMETA(DisplayName = "Quest Item"),     // Fuse, PuzzlePiece
    Document        UMETA(DisplayName = "Document"),       // Notes, Diary
    Special         UMETA(DisplayName = "Special")         // Teddy Bear
};

// Enum cho độ hiếm
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    Common          UMETA(DisplayName = "Common"),
    Uncommon        UMETA(DisplayName = "Uncommon"),
    Rare            UMETA(DisplayName = "Rare"),
    Unique          UMETA(DisplayName = "Unique")
};

// Struct cho dữ liệu item (dùng với DataTable)
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    // Thông tin cơ bản
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    UTexture2D* Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    EItemType ItemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    EItemRarity Rarity;

    // Stack và sử dụng
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stack")
    int32 MaxStackSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Usage")
    bool bIsConsumable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Usage")
    bool bCanBeDropped;

    // Hiệu ứng sanity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float SanityRestoreAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float PassiveSanityDrainReduction; // % giảm drain (Teddy Bear = 5%)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float UsageCooldown; // Cooldown sau khi dùng (giây)

    // Durability (cho tools)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability")
    bool bHasDurability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability")
    int32 MaxUses; // Số lần dùng (Master Key = 3)

    // World visual & actor class to spawn when dropping
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    UStaticMesh* WorldMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    TSubclassOf<AItemPickupActor> PickupActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* PickupSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* UseSound;

    // Weight (optional)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
    float Weight;

    // Constructor - khởi tạo giá trị mặc định
    FItemData()
        : ItemID(NAME_None)
        , ItemName(FText::FromString("Unknown"))
        , Description(FText::FromString(""))
        , Icon(nullptr)
        , ItemType(EItemType::Consumable)
        , Rarity(EItemRarity::Common)
        , MaxStackSize(1)
        , bIsConsumable(false)
        , bCanBeDropped(true)
        , SanityRestoreAmount(0.0f)
        , PassiveSanityDrainReduction(0.0f)
        , UsageCooldown(0.0f)
        , bHasDurability(false)
        , MaxUses(1)
        , WorldMesh(nullptr)
        , PickupActorClass(nullptr)
        , PickupSound(nullptr)
        , UseSound(nullptr)
        , Weight(1.0f)
    {
    }
};

// Struct cho inventory slot
USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RemainingUses; // Cho items có durability

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownRemaining; // Thời gian cooldown còn lại

    // LƯU Ý: FItemData là struct (non-UObject). Không nên dùng UPROPERTY cho con trỏ tới struct non-UObject.
    // Ta cache con trỏ const để tham chiếu nhanh tới dữ liệu từ DataTable (nếu cần).
    const FItemData* ItemData;

    FInventorySlot()
        : ItemID(NAME_None)
        , Quantity(0)
        , RemainingUses(-1)
        , CooldownRemaining(0.0f)
        , ItemData(nullptr)
    {
    }

    FInventorySlot(FName InItemID, int32 InQuantity)
        : ItemID(InItemID)
        , Quantity(InQuantity)
        , RemainingUses(-1)
        , CooldownRemaining(0.0f)
        , ItemData(nullptr)
    {
    }

    bool IsValid() const
    {
        return !ItemID.IsNone() && Quantity > 0;
    }

    void SetItemData(const FItemData* InData)
    {
        ItemData = InData;
        if (ItemData && ItemData->bHasDurability)
        {
            RemainingUses = ItemData->MaxUses;
        }
        else
        {
            RemainingUses = -1;
        }
    }
};
