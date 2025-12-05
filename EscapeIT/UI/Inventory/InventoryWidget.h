// InventoryWidget.h - Complete Horror Game Inventory UI

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/ItemData.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UFlashlightComponent;
class UInventorySlotWidget;
class UUniformGridPanel;
class UTextBlock;
class UButton;
class UImage;
class UProgressBar;
class UBorder;
class UWidgetAnimation;

UCLASS()
class ESCAPEIT_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ========================================================================
    // LIFECYCLE
    // ========================================================================
    
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ========================================================================
    // INITIALIZATION
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void InitInventory(UInventoryComponent* InInventoryComp);

    // ========================================================================
    // UI WIDGET REFERENCES
    // ========================================================================
    
    // Main Panels
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> ItemDetailPanel;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUniformGridPanel> InventoryGrid;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUniformGridPanel> QuickbarGrid;

    // Item Details
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ItemIcon;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_ItemName;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_ItemDescription;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_ItemStats;

    // Battery Indicator (for Flashlight)
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> BatteryBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_BatteryPercent;

    // Action Buttons
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Btn_Use;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Btn_Drop;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Btn_Examine;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Btn_Close;

    // Filter Buttons
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Btn_All;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Btn_Consumables;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Btn_Tools;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Btn_Documents;

    // ========================================================================
    // CONFIGURATION
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
    TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
    int32 GridRows = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
    int32 GridColumns = 5;

    // ========================================================================
    // BATTERY COLORS
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual|Battery")
    FLinearColor BatteryHighColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual|Battery")
    FLinearColor BatteryMediumColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual|Battery")
    FLinearColor BatteryLowColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual|Battery")
    FLinearColor BatteryCriticalColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

    // ========================================================================
    // ANIMATIONS
    // ========================================================================
    
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> BatteryWarningAnim;

    // ========================================================================
    // PUBLIC API
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RefreshInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RefreshQuickbar();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ShowItemDetails(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void HideItemDetails();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void FilterByType(EItemType ItemType);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ShowAllItems();

protected:
    // ========================================================================
    // BUTTON CALLBACKS
    // ========================================================================
    
    UFUNCTION()
    void OnUseButtonClicked();

    UFUNCTION()
    void OnDropButtonClicked();

    UFUNCTION()
    void OnExamineButtonClicked();

    UFUNCTION()
    void OnCloseButtonClicked();

    UFUNCTION()
    void OnFilterAllClicked();

    UFUNCTION()
    void OnFilterConsumablesClicked();

    UFUNCTION()
    void OnFilterToolsClicked();

    UFUNCTION()
    void OnFilterDocumentsClicked();

    // ========================================================================
    // INVENTORY EVENT CALLBACKS
    // ========================================================================
    
    UFUNCTION()
    void OnInventoryUpdated();

    UFUNCTION()
    void OnItemEquipped(FName ItemID);

    UFUNCTION()
    void OnItemUnequipped(FName ItemID);

    UFUNCTION()
    void OnBatteryChanged(float CurrentBattery, float MaxBattery);

    UFUNCTION()
    void OnLowBattery();

    // ========================================================================
    // INTERNAL FUNCTIONS
    // ========================================================================
    
    void CreateSlotWidgets();
    void UpdateSlotWidget(int32 SlotIndex);
    void RefreshBatteryIndicator();
    void ClearSelection();
    void UpdateFilterButtonStates();
    void HighlightEquippedSlots();
    
    FString BuildStatsText(const FItemData& ItemData, const FInventorySlot& SlotData);
    void ConfigureActionButtons(const FItemData& ItemData);
    
    FLinearColor GetBatteryColor(float Percentage) const;
    void PlayBatteryWarningAnimation();

    // Quickbar helpers
    int32 FindItemInQuickbar(FName ItemID) const;
    int32 FindEmptyQuickbarSlot() const;

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
    TArray<TObjectPtr<UInventorySlotWidget>> SlotWidgets;

    UPROPERTY()
    TArray<TObjectPtr<UInventorySlotWidget>> QuickbarSlotWidgets;

    // ========================================================================
    // STATE
    // ========================================================================
    
    int32 SelectedSlotIndex = -1;
    bool bShowAllItems = true;
    EItemType CurrentFilter = EItemType::Consumable;
};