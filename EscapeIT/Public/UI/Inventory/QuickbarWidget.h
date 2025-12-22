
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuickbarWidget.generated.h"

class UInventoryComponent;
class UFlashlightComponent;
class UInventorySlotWidget;
class UHorizontalBox;
class UProgressBar;
class UTextBlock;
class UImage;

UCLASS()
class ESCAPEIT_API UQuickbarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ========================================================================
    // LIFECYCLE
    // ========================================================================
    
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual void NativeDestruct() override;

    // ========================================================================
    // INITIALIZATION
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void InitQuickBar(UInventoryComponent* InInventoryComp, UFlashlightComponent* InFlashlightComp);

    // ========================================================================
    // UI WIDGET REFERENCES
    // ========================================================================
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> QuickbarContainer;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> BatteryBar;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> BatteryIcon;

    // ========================================================================
    // CONFIGURATION
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quickbar|Config")
    TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quickbar|Config")
    int32 QuickbarSlotCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quickbar|Visual")
    FLinearColor HighBatteryColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quickbar|Visual")
    FLinearColor MediumBatteryColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quickbar|Visual")
    FLinearColor LowBatteryColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quickbar|Visual")
    FLinearColor CriticalBatteryColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quickbar|Config")
    float LowBatteryThreshold = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quickbar|Config")
    float PulseSpeed = 5.0f;

    // ========================================================================
    // PUBLIC API
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void RefreshQuickbar();

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void UpdateSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void HighlightSlot(int32 SlotIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void ClearHighlight();

protected:
    // ========================================================================
    // INVENTORY EVENT CALLBACKS
    // ========================================================================
    
    UFUNCTION()
    void OnInventoryUpdated();

    UFUNCTION()
    void OnItemAdded(FName ItemID, int32 Quantity);

    UFUNCTION()
    void OnBatteryChanged(float CurrentBattery, float MaxBattery);

    UFUNCTION()
    void OnFlashlightToggled(bool bIsOn);
    
    UFUNCTION()
    void OnFlashlightEquippedChanged(bool bIsEquipped);

    // ========================================================================
    // INTERNAL FUNCTIONS
    // ========================================================================
    
    void CreateQuickbarSlots();
    void UnbindAllEvents();
    void UpdateBatteryBarVisibility();
    void UpdateBatteryBar();
    void UpdateLowBatteryWarning(float DeltaTime);
    void ResetSlotVisuals(int32 SlotIndex);
    void ActivateLowBatteryWarning();
    void DeactivateLowBatteryWarning();
    bool ValidateQuickbarState();
    
    int32 FindFlashlightSlot() const;
    bool IsFlashlightItem(FName ItemID) const;
    FLinearColor GetBatteryColor(float BatteryPercent) const;

private:
    // ========================================================================
    // CACHED REFERENCES
    // ========================================================================
    
    UPROPERTY()
    TObjectPtr<UInventoryComponent> InventoryComponent;

    UPROPERTY()
    TObjectPtr<UFlashlightComponent> FlashlightComponent;

    // ========================================================================
    // SLOT WIDGETS
    // ========================================================================
    
    UPROPERTY()
    TArray<TObjectPtr<UInventorySlotWidget>> QuickbarSlots;

    // ========================================================================
    // STATE
    // ========================================================================
    
    bool bIsInitialized = false;
    int32 CurrentSelectedSlot = -1;
    int32 CachedFlashlightSlot = -1;
    bool bLowBatteryWarningActive = false;
    float BatteryCheckTimer = 0.0f;
    
    static constexpr float BATTERY_CHECK_INTERVAL = 0.1f;
    
    UFUNCTION()
    void UpdateFlashlightIcon(UTexture2D* Icon);
    
    void ShowBatteryBar();
    void HideBatteryBar();
    
    UPROPERTY()
    TObjectPtr<UInventorySlotWidget> InventorySlotWidget;  
};