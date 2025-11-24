#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/ItemData.h"
#include "QuickbarWidget.generated.h"

// Forward declarations
class UInventoryComponent;
class UFlashlightComponent;
class UImage;
class UTextBlock;
class UProgressBar;
class UBorder;
class UDataTable;

UCLASS()
class ESCAPEIT_API UQuickbarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ============================================
    // INITIALIZATION
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void InitQuickBar(UInventoryComponent* InInventoryComp, UFlashlightComponent* InFlashlightComp);

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void RefreshQuickbar();

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void UpdateSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void HighlightSlot(int32 SlotIndex);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ============================================
    // SLOT 1 WIDGETS
    // ============================================
    
    UPROPERTY(meta = (BindWidget))
    UImage* Slot1_Icon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot1_Quantity;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot1_Hotkey;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* Slot1_Battery;

    UPROPERTY(meta = (BindWidget))
    UBorder* Slot1_Border;

    // ============================================
    // SLOT 2 WIDGETS
    // ============================================
    
    UPROPERTY(meta = (BindWidget))
    UImage* Slot2_Icon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot2_Quantity;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot2_Hotkey;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* Slot2_Battery;

    UPROPERTY(meta = (BindWidget))
    UBorder* Slot2_Border;

    // ============================================
    // SLOT 3 WIDGETS
    // ============================================
    
    UPROPERTY(meta = (BindWidget))
    UImage* Slot3_Icon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot3_Quantity;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot3_Hotkey;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* Slot3_Battery;

    UPROPERTY(meta = (BindWidget))
    UBorder* Slot3_Border;

    // ============================================
    // SLOT 4 WIDGETS
    // ============================================
    
    UPROPERTY(meta = (BindWidget))
    UImage* Slot4_Icon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot4_Quantity;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot4_Hotkey;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* Slot4_Battery;

    UPROPERTY(meta = (BindWidget))
    UBorder* Slot4_Border;

    // ============================================
    // STYLE PROPERTIES
    // ============================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quickbar|Style")
    FLinearColor NormalBorderColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quickbar|Style")
    FLinearColor SelectedBorderColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quickbar|Style")
    FLinearColor EmptySlotColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quickbar|Style")
    FLinearColor LowBatteryColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

private:
    // ============================================
    // COMPONENT REFERENCES
    // ============================================
    
    UPROPERTY()
    UInventoryComponent* InventoryComponent = nullptr;

    UPROPERTY()
    UFlashlightComponent* FlashlightComponent = nullptr;

    UPROPERTY()
    UDataTable* ItemDataTable = nullptr;

    // ============================================
    // STATE TRACKING
    // ============================================

    bool bIsInitialized = false;
    int32 CurrentSelectedSlot = -1;
    bool bLowBatteryWarningActive = false;

    // ============================================
    // INTERNAL FUNCTIONS
    // ============================================

    void UpdateSlotVisuals(int32 SlotIndex, const FInventorySlot& SlotData);
    void UpdateBatteryDisplay(int32 SlotIndex, float Percentage);
    int32 FindFlashlightSlot();
    bool IsFlashlightItem(const FItemData& ItemData);
    UProgressBar* GetBatteryBarForSlot(int32 SlotIndex);
    UBorder* GetBorderForSlot(int32 SlotIndex);

    // ============================================
    // EVENT CALLBACKS
    // ============================================

    UFUNCTION()
    void OnInventoryUpdated();

    UFUNCTION()
    void OnBatteryChanged(float CurrentBattery, float MaxBattery);

    UFUNCTION()
    void OnFlashlightToggled(bool bIsOn);
};