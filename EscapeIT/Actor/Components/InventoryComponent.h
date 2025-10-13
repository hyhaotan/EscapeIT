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

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPEIT_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========== CORE INVENTORY ==========
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(FName ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(FName ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(FName ItemID);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HasItem(FName ItemID, int32 Quantity = 1) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetItemQuantity(FName ItemID) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool GetItemData(FName ItemID, FItemData& OutItemData) const;

    // ========== EQUIP SYSTEM (NEW) ==========
    UFUNCTION(BlueprintCallable, Category = "Inventory|Equip")
    bool EquipQuickbarSlot(int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equip")
    bool UseEquippedItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equip")
    void UnequipCurrentItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equip")
    bool DropEquippedItem();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool GetEquippedItem(FItemData& OutItemData) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool GetQuickbarSlotItem(int32 SlotIndex, FItemData& OutItemData) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equip")
    FName GetCurrentEquippedItemID() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equip")
    bool IsItemEquipped() const;

    // ========== QUICKBAR ==========
    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    bool AssignToQuickbar(FName ItemID, int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    bool UseQuickbarSlot(int32 QuickbarIndex); // Legacy - now calls EquipQuickbarSlot

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    FInventorySlot GetQuickbarSlot(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    bool RemoveItemFromQuickbar(int32 QuickbarIndex, int32 Quantity);

    // ========== DROP ==========
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool DropItem(FName ItemID, int32 Quantity = 1);

    // ========== UTILITY ==========
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool IsInventoryFull() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool IsOverWeight() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    float GetWeightPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void PrintInventory();

    // ========== PROPERTIES ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
    UDataTable* ItemDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
    int32 MaxInventorySlots = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
    int32 QuickbarSize = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
    bool bUseWeightSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
    float MaxWeight = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State")
    float CurrentWeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State")
    TArray<FInventorySlot> InventorySlots;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|State")
    TArray<FInventorySlot> QuickbarSlots;

    // ========== EQUIPPED ITEM STATE ==========
    UPROPERTY(BlueprintReadOnly, Category = "Inventory|Equip")
    FName CurrentEquippedItemID = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory|Equip")
    int32 CurrentEquippedSlotIndex = -1;

    UPROPERTY()
    UStaticMeshComponent* EquippedItemMesh = nullptr;

    UPROPERTY()
    USkeletalMeshComponent* CharacterMesh = nullptr;

    // ========== DELEGATES ==========
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

protected:
    // ========== INTERNAL FUNCTIONS ==========
    FInventorySlot* FindItemSlot(FName ItemID);
    int32 FindEmptySlot() const;
    void RecalculateWeight();
    void ApplyItemEffect(const FItemData& ItemData);
    void UpdateCooldowns(float DeltaTime);
    void PlayItemSound(USoundBase* Sound);
    bool TryAutoAssignToQuickbar(FName ItemID);
    EItemCategory ItemCategory;

    // Equip helpers
    bool AttachItemToHand(const FItemData& ItemData);
};