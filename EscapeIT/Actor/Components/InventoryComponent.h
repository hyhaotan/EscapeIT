
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EscapeIT/Data//ItemData.h"
#include "InventoryComponent.generated.h"

class USoundBase;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class AFlashlight;

// ============================================================================
// DELEGATES
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, FName, ItemID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, FName, ItemID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUsed, FName, ItemID, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemEquipped, FName, ItemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUnequipped, FName, ItemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemCooldownUpdated, FName, ItemID, float, CurrentCooldown, float, MaxCooldown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemDurabilityChanged, FName, ItemID, int32, CurrentUses, int32, MaxUses);

// ============================================================================
// MAIN COMPONENT
// ============================================================================

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPEIT_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> InventorySlots;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> QuickbarSlots;
    
    UPROPERTY(EditDefaultsOnly, Category = "Items|Flashlight")
    TSubclassOf<AFlashlight> FlashlightClass;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    FName CurrentEquippedItemID = NAME_None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 CurrentEquippedSlotIndex = -1;
    
    UPROPERTY()
    AFlashlight* SpawnedFlashlightActor;
    
    // ========================================================================
    // DATA TABLE
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
    UDataTable* ItemDataTable;

    // ========================================================================
    // INVENTORY SETTINGS
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings", meta = (ClampMin = "1", ClampMax = "50"))
    int32 MaxInventorySlots = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings", meta = (ClampMin = "1", ClampMax = "10"))
    int32 QuickbarSize = 4;

    // ========================================================================
    // HORROR: AUDIO
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror|Audio")
    USoundBase* InventoryFullSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror|Audio")
    USoundBase* ItemBreakSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror|Audio")
    USoundBase* CannotDropSound;

    // ========================================================================
    // DELEGATES / EVENTS
    // ========================================================================
    
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnInventoryUpdated OnInventoryUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemAdded OnItemAdded;

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemRemoved OnItemRemoved;

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemUsed OnItemUsed;

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemEquipped OnItemEquipped;

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemUnequipped OnItemUnequipped;

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemCooldownUpdated OnItemCooldownUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnItemDurabilityChanged OnItemDurabilityChanged;

    // ========================================================================
    // ADD / REMOVE ITEMS
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(FName ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(FName ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HasItem(FName ItemID, int32 Quantity = 1) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetItemQuantity(FName ItemID) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool GetItemData(FName ItemID, FItemData& OutItemData) const;

    // ========================================================================
    // USE ITEMS
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(FName ItemID);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseEquippedItem();

    // ========================================================================
    // EQUIP / UNEQUIP
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool EquipQuickbarSlot(int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UnequipCurrentItem();

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FName GetCurrentEquippedItemID() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsItemEquipped() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool GetEquippedItem(FItemData& OutItemData) const;

    // ========================================================================
    // DROP ITEMS
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool DropItem(FName ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool DropEquippedItem();

    // ========================================================================
    // QUICKBAR
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    bool AssignToQuickbar(FName ItemID, int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    bool RemoveFromQuickbar(int32 QuickbarIndex);

    UFUNCTION(BlueprintPure, Category = "Inventory|Quickbar")
    FInventorySlot GetQuickbarSlot(int32 Index) const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Quickbar")
    bool GetQuickbarSlotItem(int32 SlotIndex, FItemData& OutItemData) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    void SyncQuickbarSlot(int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    void SyncAllQuickbarSlots();

    // Legacy support
    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar", meta = (DeprecatedFunction, DeprecationMessage = "Use EquipQuickbarSlot instead"))
    bool UseQuickbarSlot(int32 QuickbarIndex) { return EquipQuickbarSlot(QuickbarIndex); }

    // ========================================================================
    // DRAG & DROP
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory|DragDrop")
    bool SwapInventorySlots(int32 SlotA, int32 SlotB);

    UFUNCTION(BlueprintCallable, Category = "Inventory|DragDrop")
    bool SwapQuickbarSlots(int32 SlotA, int32 SlotB);

    UFUNCTION(BlueprintCallable, Category = "Inventory|DragDrop")
    bool MoveInventoryToQuickbar(int32 InventoryIndex, int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|DragDrop")
    bool MoveQuickbarToInventory(int32 QuickbarIndex, int32 InventoryIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory|DragDrop")
    void MoveItemToSlot(int32 SourceIndex, int32 TargetIndex);

    // ========================================================================
    // UTILITY
    // ========================================================================
    
    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsInventoryFull() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void PrintInventory();

    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<FInventorySlot> GetAllInventorySlots() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void LoadInventorySlots(const TArray<FInventorySlot>& SavedSlots);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    void SaveQuickbarSetup(TArray<FInventorySlot>& OutQuickbarSlots) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    void LoadQuickbarSetup(const TArray<FInventorySlot>& SavedQuickbar);

    // ========================================================================
    // HORROR: PASSIVE EFFECTS
    // ========================================================================
    
    UFUNCTION(BlueprintPure, Category = "Horror|Effects")
    float GetPassiveSanityDrainReduction() const;

    // ========================================================================
    // DEBUG / CHEATS
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory|Debug", meta = (DevelopmentOnly))
    void GiveAllItems();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Debug", meta = (DevelopmentOnly))
    void RemoveAllItems();

protected:
    // ========================================================================
    // INTERNAL DATA
    // ========================================================================
    
    // Mesh reference
    UPROPERTY()
    UStaticMeshComponent* EquippedItemMesh;

    UPROPERTY()
    USkeletalMeshComponent* CharacterMesh;

    // ========================================================================
    // INTERNAL FUNCTIONS
    // ========================================================================
    
    bool AttachItemToHand(const FItemData& ItemData);
    
    FInventorySlot* FindItemSlot(FName ItemID);
    
    int32 FindInventorySlotByItemID(const FName& ItemID) const;
    
    bool ApplyItemEffect(const FItemData& ItemData);
    
    void PlayItemSound(USoundBase* Sound);
    
    void PlayInventoryFullSound();
    
    void PlayItemBreakSound();
    
    void PlayCannotDropSound();
    
    UPROPERTY()
    AActor* CurrentAttachedItemActor;
    
    // Helper function to spawn flashlight
    bool SpawnFlashlightActor(const FItemData& ItemData);
};