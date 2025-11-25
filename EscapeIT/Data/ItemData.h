#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "NiagaraFunctionLibrary.h"
#include "ItemData.generated.h"

class AItemPickupActor;

// ============================================================================
// ITEM CATEGORIES
// ============================================================================

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Consumable      UMETA(DisplayName = "Consumable"),      // Items that can be consumed
    Tool            UMETA(DisplayName = "Tool"),            // Permanent utility items
    Key             UMETA(DisplayName = "Key"),             // Keys for unlocking
    Document        UMETA(DisplayName = "Document"),        // Readable lore items
    QuestItem       UMETA(DisplayName = "Quest Item"),      // Story-critical items
    Passive         UMETA(DisplayName = "Passive Effect")   // Items with passive effects
};

// ============================================================================
// CONSUMABLE TYPES
// ============================================================================

UENUM(BlueprintType)
enum class EConsumableType : uint8
{
    Medicine        UMETA(DisplayName = "Medicine"),        // Heals sanity
    Battery         UMETA(DisplayName = "Battery"),         // Powers flashlight
    Food            UMETA(DisplayName = "Food"),            // Future use
    Other           UMETA(DisplayName = "Other")
};

// Medicine categories for different restoration amounts
UENUM(BlueprintType)
enum class EMedicineType : uint8
{
    Painkiller      UMETA(DisplayName = "Painkiller"),      // +10 Sanity (common)
    WaterBottle     UMETA(DisplayName = "Water Bottle"),    // +5 Sanity (common)
    Medkit          UMETA(DisplayName = "Medkit"),          // +15 Sanity (uncommon)
    LargeMedkit     UMETA(DisplayName = "Large Medkit"),    // +20 Sanity (rare)
    Sedative        UMETA(DisplayName = "Sedative")         // +25 Sanity (very rare)
};

// ============================================================================
// TOOL TYPES
// ============================================================================

UENUM(BlueprintType)
enum class EToolType : uint8
{
    Flashlight      UMETA(DisplayName = "Flashlight"),      // Main light source
    Lighter         UMETA(DisplayName = "Lighter"),         // Temporary light
    Wrench          UMETA(DisplayName = "Wrench"),          // Opens panels
    MasterKey       UMETA(DisplayName = "Master Key"),      // Opens multiple doors
    Other           UMETA(DisplayName = "Other")
};

// ============================================================================
// KEY TYPES (Based on GDD rooms)
// ============================================================================

UENUM(BlueprintType)
enum class EKeyType : uint8
{
    LibraryKey      UMETA(DisplayName = "Library Key"),     // Room 2
    LabKey          UMETA(DisplayName = "Lab Key"),         // Room 3
    BedroomKey      UMETA(DisplayName = "Bedroom Key"),     // Room 4
    BasementKey     UMETA(DisplayName = "Basement Key"),    // Room 5
    RitualKey       UMETA(DisplayName = "Ritual Key"),      // Room 6
    MasterKey       UMETA(DisplayName = "Master Key"),      // Opens multiple
    SpecialKey      UMETA(DisplayName = "Special Key")      // Bonus rooms
};

// ============================================================================
// DOCUMENT TYPES
// ============================================================================

UENUM(BlueprintType)
enum class EDocumentType : uint8
{
    Note            UMETA(DisplayName = "Note"),            // Small hints
    DiaryPage       UMETA(DisplayName = "Diary Page"),      // Story pieces
    LabReport       UMETA(DisplayName = "Lab Report"),      // Room 3 lore
    Photo           UMETA(DisplayName = "Photo"),           // Visual clues
    FinalNote       UMETA(DisplayName = "Final Note")       // Ending reveal
};

// ============================================================================
// PASSIVE EFFECT TYPES (Based on GDD)
// ============================================================================

UENUM(BlueprintType)
enum class EPassiveEffectType : uint8
{
    TeddyBear       UMETA(DisplayName = "Teddy Bear"),      // -5% drain rate
    Charm           UMETA(DisplayName = "Protective Charm"), // -50% drain rate (Room 6)
    Crucifix        UMETA(DisplayName = "Crucifix"),        // Fear reduction
    Other           UMETA(DisplayName = "Other")
};

// ============================================================================
// MAIN ITEM DATA STRUCTURE
// ============================================================================

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    // ========================================================================
    // BASIC INFORMATION
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic", meta = (MultiLine = true))
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    TObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    EItemType ItemType;

    // ========================================================================
    // TYPE-SPECIFIC CATEGORIES
    // ========================================================================

    // Consumable subcategories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type Settings",
        meta = (EditCondition = "ItemType == EItemType::Consumable", EditConditionHides))
    EConsumableType ConsumableType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type Settings",
        meta = (EditCondition = "ConsumableType == EConsumableType::Medicine && ItemType == EItemType::Consumable", EditConditionHides))
    EMedicineType MedicineType;

    // Tool subcategories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type Settings",
        meta = (EditCondition = "ItemType == EItemType::Tool", EditConditionHides))
    EToolType ToolType;

    // Key subcategories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type Settings",
        meta = (EditCondition = "ItemType == EItemType::Key", EditConditionHides))
    EKeyType KeyType;

    // Document subcategories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type Settings",
        meta = (EditCondition = "ItemType == EItemType::Document", EditConditionHides))
    EDocumentType DocumentType;

    // Passive effect subcategories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type Settings",
        meta = (EditCondition = "ItemType == EItemType::Passive", EditConditionHides))
    EPassiveEffectType PassiveEffectType;

    // ========================================================================
    // INVENTORY PROPERTIES
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory",
        meta = (ClampMin = "1", ClampMax = "99"))
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    bool bCanBeDropped = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    bool bShowInQuickbar = true;  // Can be assigned to quickbar

    // ========================================================================
    // SANITY EFFECTS (Core mechanic from GDD)
    // ========================================================================

    // Direct sanity restoration (for consumables)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity Effects",
        meta = (EditCondition = "ItemType == EItemType::Consumable", EditConditionHides, 
                ClampMin = "0.0", ClampMax = "100.0"))
    float SanityRestoreAmount = 0.0f;

    // Passive sanity drain reduction (for passive items)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity Effects",
        meta = (EditCondition = "ItemType == EItemType::Passive", EditConditionHides,
                ClampMin = "0.0", ClampMax = "100.0", Tooltip = "Percentage reduction in sanity drain rate (5.0 = 5% reduction)"))
    float PassiveSanityDrainReduction = 0.0f;

    // Fear reduction (for specific items like crucifix)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity Effects",
        meta = (EditCondition = "ItemType == EItemType::Passive", EditConditionHides,
                ClampMin = "0.0", ClampMax = "100.0"))
    float FearReduction = 0.0f;

    // ========================================================================
    // BATTERY PROPERTIES (Based on GDD: 120s per battery, stack 4)
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battery",
        meta = (EditCondition = "ConsumableType == EConsumableType::Battery && ItemType == EItemType::Consumable", EditConditionHides,
                Tooltip = "Battery duration in seconds (GDD: 120s standard)"))
    float BatteryDuration = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battery",
        meta = (EditCondition = "ConsumableType == EConsumableType::Battery && ItemType == EItemType::Consumable", EditConditionHides,
                ClampMin = "0.0", ClampMax = "100.0", Tooltip = "Percentage of flashlight charge restored"))
    float BatteryChargePercent = 100.0f;

    // ========================================================================
    // TOOL DURABILITY (For flashlight, lighter)
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability",
        meta = (EditCondition = "ItemType == EItemType::Tool", EditConditionHides))
    bool bHasDurability = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability",
        meta = (EditCondition = "bHasDurability && ItemType == EItemType::Tool", EditConditionHides))
    float MaxDurability = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability",
        meta = (EditCondition = "bHasDurability && ItemType == EItemType::Tool", EditConditionHides,
                Tooltip = "Durability consumed per second of use"))
    float DurabilityDrainRate = 0.833f;  // 120 seconds to deplete (100 / 120)

    // ========================================================================
    // USAGE PROPERTIES
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Usage")
    bool bCanBeUsed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Usage",
        meta = (EditCondition = "bCanBeUsed", EditConditionHides))
    bool bIsSingleUse = true;  // Most consumables are single use

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Usage",
        meta = (EditCondition = "bCanBeUsed && !bIsSingleUse", EditConditionHides))
    int32 MaxUses = 1;

    // ========================================================================
    // KEY PROPERTIES (Compatible locks)
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Properties",
        meta = (EditCondition = "ItemType == EItemType::Key", EditConditionHides,
                Tooltip = "List of door IDs this key can unlock"))
    TArray<FName> CompatibleLockIDs;

    // ========================================================================
    // DOCUMENT PROPERTIES
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document",
        meta = (EditCondition = "ItemType == EItemType::Document", EditConditionHides, MultiLine = true))
    FText DocumentContent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Document",
        meta = (EditCondition = "ItemType == EItemType::Document", EditConditionHides))
    TObjectPtr<UTexture2D> DocumentImage;  // For photos or illustrated notes

    // ========================================================================
    // QUEST ITEM PROPERTIES
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest",
        meta = (EditCondition = "ItemType == EItemType::QuestItem", EditConditionHides))
    FName AssociatedQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest",
        meta = (EditCondition = "ItemType == EItemType::QuestItem", EditConditionHides))
    bool bIsQuestComplete = false;

    // ========================================================================
    // WORLD REPRESENTATION
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    TObjectPtr<UStaticMesh> ItemMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    TSubclassOf<AItemPickupActor> PickupActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    FVector MeshScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    FRotator MeshRotation = FRotator::ZeroRotator;

    // ========================================================================
    // AUDIO (Horror game needs good audio feedback)
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundBase> PickupSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundBase> UseSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundBase> EquipSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TObjectPtr<USoundBase> DropSound;

    // ========================================================================
    // VISUAL EFFECTS
    // ========================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TObjectPtr<UParticleSystem> UseParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TObjectPtr<UNiagaraSystem> UseNiagaraEffect;

    // ========================================================================
    // CONSTRUCTOR WITH GDD VALUES
    // ========================================================================

    FItemData()
        : ItemID(NAME_None)
        , ItemName(FText::FromString("Unknown Item"))
        , Description(FText::FromString(""))
        , Icon(nullptr)
        , ItemType(EItemType::Consumable)
        , ConsumableType(EConsumableType::Other)
        , MedicineType(EMedicineType::Painkiller)
        , ToolType(EToolType::Other)
        , KeyType(EKeyType::MasterKey)
        , DocumentType(EDocumentType::Note)
        , PassiveEffectType(EPassiveEffectType::Other)
        , MaxStackSize(1)
        , bCanBeDropped(true)
        , bShowInQuickbar(true)
        , SanityRestoreAmount(0.0f)
        , PassiveSanityDrainReduction(0.0f)
        , FearReduction(0.0f)
        , BatteryDuration(120.0f)
        , BatteryChargePercent(100.0f)
        , bHasDurability(false)
        , MaxDurability(100.0f)
        , DurabilityDrainRate(0.833f)
        , bCanBeUsed(true)
        , bIsSingleUse(true)
        , MaxUses(1)
        , bIsQuestComplete(false)
        , ItemMesh(nullptr)
        , PickupActorClass(nullptr)
        , PickupSound(nullptr)
        , UseSound(nullptr)
        , EquipSound(nullptr)
        , DropSound(nullptr)
        , UseParticleEffect(nullptr)
        , UseNiagaraEffect(nullptr)
    {
    }

    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================

    // Check if item can be used (based on type)
    FORCEINLINE bool CanBeUsed() const
    {
        if (!bCanBeUsed) return false;
        
        switch (ItemType)
        {
            case EItemType::Consumable:
                return true;
            case EItemType::Tool:
                return (ToolType == EToolType::Flashlight || ToolType == EToolType::Lighter);
            case EItemType::Document:
                return true;  // Can be "used" to read
            case EItemType::Key:
                return false; // Keys are used automatically
            case EItemType::Passive:
                return false; // Passive items work automatically
            default:
                return false;
        }
    }

    // Check if item is stackable
    FORCEINLINE bool IsStackable() const
    {
        return MaxStackSize > 1;
    }

    // Get item type display name
    FText GetItemTypeText() const
    {
        switch (ItemType)
        {
            case EItemType::Consumable:
                if (ConsumableType == EConsumableType::Medicine)
                    return FText::FromString("Medicine");
                else if (ConsumableType == EConsumableType::Battery)
                    return FText::FromString("Battery");
                return FText::FromString("Consumable");
            case EItemType::Tool:
                return FText::FromString("Tool");
            case EItemType::Key:
                return FText::FromString("Key");
            case EItemType::Document:
                return FText::FromString("Document");
            case EItemType::QuestItem:
                return FText::FromString("Quest Item");
            case EItemType::Passive:
                return FText::FromString("Passive");
            default:
                return FText::FromString("Unknown");
        }
    }

    // Calculate actual sanity restore (with potential multipliers)
    float GetEffectiveSanityRestore(float Multiplier = 1.0f) const
    {
        return SanityRestoreAmount * Multiplier;
    }

    // Get battery time in minutes for UI display
    FText GetBatteryTimeText() const
    {
        if (ConsumableType == EConsumableType::Battery)
        {
            int32 Minutes = FMath::FloorToInt(BatteryDuration / 60.0f);
            int32 Seconds = FMath::FloorToInt(BatteryDuration) % 60;
            return FText::FromString(FString::Printf(TEXT("%d:%02d"), Minutes, Seconds));
        }
        return FText::GetEmpty();
    }
};

// ============================================================================
// INVENTORY SLOT STRUCTURE
// ============================================================================

USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity = 0;

    // For items with durability (flashlight, lighter)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentDurability = -1.0f;

    // For items with limited uses (multi-use items)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RemainingUses = -1;

    FInventorySlot()
        : ItemID(NAME_None)
        , Quantity(0)
        , CurrentDurability(-1.0f)
        , RemainingUses(-1)
    {
    }

    FInventorySlot(FName InItemID, int32 InQuantity)
        : ItemID(InItemID)
        , Quantity(InQuantity)
        , CurrentDurability(-1.0f)
        , RemainingUses(-1)
    {
    }

    bool IsValid() const
    {
        return !ItemID.IsNone() && Quantity > 0;
    }

    bool IsEmpty() const
    {
        return ItemID.IsNone() || Quantity <= 0;
    }

    void Clear()
    {
        ItemID = NAME_None;
        Quantity = 0;
        CurrentDurability = -1.0f;
        RemainingUses = -1;
    }
};