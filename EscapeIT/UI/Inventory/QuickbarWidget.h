#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "EscapeIT/Data/ItemData.h"
#include "QuickbarWidget.generated.h"

UCLASS()
class ESCAPEIT_API UQuickbarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ============================================
    // WIDGET BINDINGS (Must match UMG names exactly!)
    // ============================================

    // Slot 1 (Flashlight)
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

    // Slot 2
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

    // Slot 3
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

    // Slot 4
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
    // PROPERTIES
    // ============================================

    UPROPERTY(BlueprintReadWrite, Category = "Quickbar")
    class UInventoryComponent* InventoryComponent;

    UPROPERTY(BlueprintReadWrite, Category = "Quickbar")
    class UFlashlightComponent* FlashlightComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Quickbar|Style")
    FLinearColor NormalBorderColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.8f);

    UPROPERTY(EditDefaultsOnly, Category = "Quickbar|Style")
    FLinearColor SelectedBorderColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);

    UPROPERTY(EditDefaultsOnly, Category = "Quickbar|Style")
    FLinearColor EmptySlotColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.3f);

    UPROPERTY(EditDefaultsOnly, Category = "Quickbar|Style")
    FLinearColor LowBatteryColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

    // ============================================
    // FUNCTIONS
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void Initialize(UInventoryComponent* InInventoryComp, UFlashlightComponent* InFlashlightComp);

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void RefreshQuickbar();

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void UpdateSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void HighlightSlot(int32 SlotIndex);

protected:
    void UpdateSlotVisuals(int32 SlotIndex, const FInventorySlot& SlotData);
    void UpdateBatteryDisplay(float Percentage);
    void UpdateCooldownDisplay(int32 SlotIndex, float CooldownPercent);

    UFUNCTION()
    void OnInventoryUpdated();

    UFUNCTION()
    void OnBatteryChanged(float Percentage);

    UFUNCTION()
    void OnFlashlightToggled(bool bIsOn);

private:
    int32 CurrentSelectedSlot = -1;
    UDataTable* ItemDataTable;
    bool bLowBatteryWarningActive = false;
};