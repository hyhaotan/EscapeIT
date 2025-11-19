// QuickbarWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/ItemData.h"
#include "QuickbarWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
class UBorder;

UCLASS()
class ESCAPEIT_API UQuickbarWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
    // ============================================
    // INITIALIZATION
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void InitQuickBar(class UInventoryComponent* InInventoryComp, class UFlashlightComponent* InFlashlightComp);

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void RefreshQuickbar();

    // ============================================
    // SLOT WIDGETS - SLOT 1 (Flashlight)
    // ============================================

    UPROPERTY(meta = (BindWidget))
    UImage* Slot1_Icon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot1_Quantity;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* Slot1_Battery;

    UPROPERTY(meta = (BindWidget))
    UBorder* Slot1_Border;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot1_Hotkey;

    // ============================================
    // SLOT WIDGETS - SLOT 2
    // ============================================

    UPROPERTY(meta = (BindWidget))
    UImage* Slot2_Icon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot2_Quantity;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* Slot2_Cooldown;

    UPROPERTY(meta = (BindWidget))
    UBorder* Slot2_Border;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot2_Hotkey;

    // ============================================
    // SLOT WIDGETS - SLOT 3
    // ============================================

    UPROPERTY(meta = (BindWidget))
    UImage* Slot3_Icon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot3_Quantity;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* Slot3_Cooldown;

    UPROPERTY(meta = (BindWidget))
    UBorder* Slot3_Border;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot3_Hotkey;

    // ============================================
    // SLOT WIDGETS - SLOT 4
    // ============================================

    UPROPERTY(meta = (BindWidget))
    UImage* Slot4_Icon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot4_Quantity;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* Slot4_Cooldown;

    UPROPERTY(meta = (BindWidget))
    UBorder* Slot4_Border;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Slot4_Hotkey;

    // ============================================
    // COLORS
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor NormalBorderColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor SelectedBorderColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor EmptySlotColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
    FLinearColor LowBatteryColor = FLinearColor::Red;

    // ============================================
    // PUBLIC FUNCTIONS
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void UpdateSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void HighlightSlot(int32 SlotIndex);

protected:
    // ============================================
    // COMPONENT REFERENCES
    // ============================================

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<UInventoryComponent> InventoryComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<UFlashlightComponent> FlashlightComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Data")
    TObjectPtr<UDataTable> ItemDataTable;

    // ============================================
    // STATE
    // ============================================

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentSelectedSlot = -1;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bLowBatteryWarningActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsInitialized = false;

    // ============================================
    // EVENT HANDLERS
    // ============================================

    UFUNCTION()
    void OnInventoryUpdated();

    UFUNCTION()
    void OnBatteryChanged(float Percentage);

    UFUNCTION()
    void OnFlashlightToggled(bool bIsOn);

private:
    // ============================================
    // INTERNAL FUNCTIONS
    // ============================================

    void UpdateSlotVisuals(int32 SlotIndex, const struct FInventorySlot& SlotData);
    void UpdateBatteryDisplay(float Percentage);
    void UpdateCooldownDisplay(int32 SlotIndex, float CooldownPercent);
};