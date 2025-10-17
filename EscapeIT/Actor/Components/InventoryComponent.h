// InventoryComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "EscapeIT/Data/ItemData.h"
#include "InventoryComponent.generated.h"

class UStaticMesh;
class USoundBase;
class USkeletalMeshComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, FName, ItemID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, FName, ItemID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUsed, FName, ItemID, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemEquipped, FName, ItemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUnequipped, FName, ItemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemCooldownUpdated, FName, ItemID, float, CooldownRemaining, float, MaxCooldown);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPEIT_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ============================================
    // PUBLIC PROPERTIES
    // ============================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
    int32 MaxInventorySlots = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
    int32 QuickbarSize = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Data")
    UDataTable* ItemDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State")
    TArray<FInventorySlot> InventorySlots;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State")
    TArray<FInventorySlot> QuickbarSlots;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State")
    FName CurrentEquippedItemID;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State")
    int32 CurrentEquippedSlotIndex = -1;

    // ============================================
    // DELEGATES
    // ============================================
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

    // ============================================
    // PUBLIC FUNCTIONS
    // ============================================

    // Core inventory functions
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(FName ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(FName ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(FName ItemID);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool DropItem(FName ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HasItem(FName ItemID, int32 Quantity = 1) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetItemQuantity(FName ItemID) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool GetItemData(FName ItemID, FItemData& OutItemData) const;

    // Equipment functions
    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool EquipQuickbarSlot(int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool UseEquippedItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool DropEquippedItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    void UnequipCurrentItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    FName GetCurrentEquippedItemID() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool IsItemEquipped() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool GetEquippedItem(FItemData& OutItemData) const;

    // Quickbar functions
    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    bool AssignToQuickbar(FName ItemID, int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    bool RemoveFromQuickbar(int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    FInventorySlot GetQuickbarSlot(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    bool GetQuickbarSlotItem(int32 SlotIndex, FItemData& OutItemData) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    void SyncQuickbarSlot(int32 QuickbarIndex);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool IsInventoryFull() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void PrintInventory();

    // NEW: Swap function for drag & drop
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool SwapInventorySlots(int32 SlotA, int32 SlotB);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool SwapQuickbarSlots(int32 SlotA, int32 SlotB);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool MoveInventoryToQuickbar(int32 InventoryIndex, int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool MoveQuickbarToInventory(int32 QuickbarIndex, int32 InventoryIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 FindInventorySlotByItemID(const FName& ItemID) const;

    // Legacy support
    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar", meta = (DeprecatedFunction))
    bool UseQuickbarSlot(int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar", meta = (DeprecatedFunction))
    bool RemoveItemFromQuickbar(int32 QuickbarIndex, int32 Quantity);

private:
    // ============================================
    // PRIVATE PROPERTIES
    // ============================================
    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> EquippedItemMesh;

    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> CharacterMesh;

    // ============================================
    // PRIVATE FUNCTIONS
    // ============================================
    FInventorySlot* FindItemSlot(FName ItemID);
    int32 FindEmptySlot() const;
    bool AttachItemToHand(const FItemData& ItemData);
    void ApplyItemEffect(const FItemData& ItemData);
    void UpdateCooldowns(float DeltaTime);
    void PlayItemSound(USoundBase* Sound);
    bool TryAutoAssignToQuickbar(FName ItemID);
    void SyncAllQuickbarSlots();
};