
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EscapeIT/Data/ItemData.h"
#include "InventoryComponent.generated.h"

// Delegate để notify UI cập nhật
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, FName, ItemID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, FName, ItemID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUsed, FName, ItemID, bool, bSuccess);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPEIT_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // ============================================
    // PROPERTIES
    // ============================================

    // DataTable chứa tất cả item definitions
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
    UDataTable* ItemDataTable;

    // Main inventory storage
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> InventorySlots;

    // Quickbar (4 slots)
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlot> QuickbarSlots;

    // Capacity
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
    int32 MaxInventorySlots = 20;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
    int32 QuickbarSize = 4;

    // Weight system (optional)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
    bool bUseWeightSystem = false;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    float CurrentWeight = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Inventory")
    float MaxWeight = 50.0f;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryUpdated OnInventoryUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnItemAdded OnItemAdded;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnItemRemoved OnItemRemoved;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnItemUsed OnItemUsed;

    // ============================================
    // PUBLIC FUNCTIONS
    // ============================================

    // Thêm item vào inventory
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(FName ItemID, int32 Quantity = 1);

    // Xóa item khỏi inventory
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(FName ItemID, int32 Quantity = 1);

    // Sử dụng item
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UseItem(FName ItemID);

    // Check có item không
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    bool HasItem(FName ItemID, int32 Quantity = 1) const;

    // Lấy số lượng item
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    int32 GetItemQuantity(FName ItemID) const;

    // Lấy item data từ DataTable
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    bool GetItemData(FName ItemID, FItemData& OutItemData) const;

    // Quickbar functions
    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    bool AssignToQuickbar(FName ItemID, int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Quickbar")
    bool UseQuickbarSlot(int32 QuickbarIndex);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Quickbar")
    FInventorySlot GetQuickbarSlot(int32 Index) const;

    // Drop item (optional)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool DropItem(FName ItemID, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItemFromQuickbar(int32 QuickbarIndex, int32 Quantity);

    // Utility
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    bool IsInventoryFull() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    bool IsOverWeight() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    float GetWeightPercentage() const;

    // Clear inventory (for testing/reset)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearInventory();

    // Debug
    UFUNCTION(BlueprintCallable, Category = "Inventory|Debug")
    void PrintInventory();

protected:
    // ============================================
    // INTERNAL FUNCTIONS
    // ============================================

    // Tìm slot có item ID
    FInventorySlot* FindItemSlot(FName ItemID);

    // Tìm slot trống
    int32 FindEmptySlot() const;

    // Check xem có thể stack không
    bool CanStackItem(const FItemData& ItemData) const;

    // Tính toán weight
    void RecalculateWeight();

    // Apply item effect
    void ApplyItemEffect(const FItemData& ItemData);

    // Handle cooldown
    void UpdateCooldowns(float DeltaTime);

    // Play sounds
    void PlayItemSound(USoundBase* Sound);
};