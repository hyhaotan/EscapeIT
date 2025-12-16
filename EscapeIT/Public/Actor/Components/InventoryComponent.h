// InventoryComponent.h - IMPROVED & FIXED

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Data/ItemData.h"
#include "InventoryComponent.generated.h"

class AItemPickupActor;
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

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPEIT_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // CONFIGURATION
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
    int32 MaxInventorySlots = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
    int32 QuickbarSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Data")
    TObjectPtr<UDataTable> ItemDataTable;

    // ========================================================================
    // FLASHLIGHT SYSTEM
    // ========================================================================
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory|Flashlight")
    TSubclassOf<AFlashlight> FlashlightClass;

    // ========================================================================
    // AUDIO
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Audio")
    TObjectPtr<USoundBase> InventoryFullSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Audio")
    TObjectPtr<USoundBase> ItemBreakSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Audio")
    TObjectPtr<USoundBase> CannotDropSound;

    // ========================================================================
    // INVENTORY DATA
    // ========================================================================
    
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> InventorySlots;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TArray<int32> QuickbarSlotIndices; 

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    FName CurrentEquippedItemID = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    int32 CurrentEquippedSlotIndex = -1;

    // ========================================================================
    // EVENTS
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

    // ========================================================================
    // CORE API
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(FName ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(FName ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(FName ItemID);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RemoveSlotAndUpdateReferences(int32 SlotIndexToRemove);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseEquippedItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool DropItem(FName ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool DropEquippedItem();
    
    // ========================================================================
    // EQUIP/UNEQUIP
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool EquipQuickbarSlot(int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UnequipCurrentItem();

    // ========================================================================
    // QUICKBAR MANAGEMENT (IMPROVED)
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    bool AssignToQuickbar(int32 InventoryIndex, int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    bool RemoveFromQuickbar(int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    FInventorySlot GetQuickbarSlot(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    int32 GetQuickbarInventoryIndex(int32 QuickbarIndex) const;

    // ========================================================================
    // DRAG & DROP (IMPROVED)
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory|DragDrop")
    bool SwapInventorySlots(int32 SlotA, int32 SlotB);

    UFUNCTION(BlueprintCallable, Category = "Inventory|DragDrop")
    bool MoveItemToSlot(int32 SourceIndex, int32 TargetIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|DragDrop")
    bool SwapQuickbarSlots(int32 SlotA, int32 SlotB);

    UFUNCTION(BlueprintCallable, Category = "Inventory|DragDrop")
    bool MoveInventoryToQuickbar(int32 InventoryIndex, int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|DragDrop")
    bool RemoveQuickbarToInventory(int32 QuickbarIndex);

    // ========================================================================
    // QUERY FUNCTIONS
    // ========================================================================
    
    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasItem(FName ItemID, int32 Quantity = 1) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetItemQuantity(FName ItemID) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool GetItemData(FName ItemID, FItemData& OutItemData) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsInventoryFull() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FName GetCurrentEquippedItemID() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsItemEquipped() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool GetEquippedItem(FItemData& OutItemData) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    float GetPassiveSanityDrainReduction() const;

    // ========================================================================
    // UTILITY
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void PrintInventory();

    // ========================================================================
    // SAVE/LOAD
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
    TArray<FInventorySlot> GetAllInventorySlots() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
    void LoadInventorySlots(const TArray<FInventorySlot>& SavedSlots);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
    void SaveQuickbarSetup(TArray<int32>& OutQuickbarIndices) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
    void LoadQuickbarSetup(const TArray<int32>& SavedQuickbar);

    // ========================================================================
    // DEBUG
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory|Debug")
    void GiveAllItems();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Debug")
    void RemoveAllItems();
    
    // ========================================================================
    // GETTER
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Inventory|Get")
    AFlashlight* GetFlashlight(){return SpawnedFlashlightActor;}
    
    // ========================================================================
    // Quickbar
    // ========================================================================
    
    bool IsQuickbarFull() const;
    int32 GetFirstEmptyQuickbarSlot() const;
    int32 FindQuickbarSlotByInventoryIndex(int32 InventoryIndex) const;
    bool IsItemInQuickbar(FName ItemID)const;
    void CompactInventory();
    void SortInventoryByType();
protected:
    // ========================================================================
    // INTERNAL HELPERS
    // ========================================================================
    
    FInventorySlot* FindItemSlot(FName ItemID);
    int32 FindInventorySlotByItemID(const FName& ItemID) const;
    bool ApplyItemEffect(const FItemData& ItemData);
    
    bool AttachItemToHand(const FItemData& ItemData);
    bool SpawnFlashlightActor(const FItemData& ItemData);
    void CleanupSpawnedActors();
    
    void PlayItemSound(USoundBase* Sound);
    void PlayInventoryFullSound();
    void PlayItemBreakSound();
    void PlayCannotDropSound();

    void TryAutoAssignToQuickbar(FName ItemID,int32 PreferredInventoryIndex);

    void ValidateQuickbarReferences();
    void ValidateInventoryIntegrity();
    void DebugPrintQuickbarState() const;

private:
    // ========================================================================
    // CACHED REFERENCES
    // ========================================================================
    
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> CharacterMesh;

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> EquippedItemMesh;

    UPROPERTY()
    TObjectPtr<AActor> CurrentAttachedItemActor;
    
    UPROPERTY()
    TObjectPtr<AFlashlight> SpawnedFlashlightActor;
};