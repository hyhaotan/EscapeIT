// InventoryWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/ItemData.h" // chứa FItemData, EItemType, EItemCategory, v.v.
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UInventorySlotWidget;
class UUniformGridPanel;
class UTextBlock;
class UButton;
class UImage;
class UProgressBar;
class UBorder;
class UWidgetAnimation;
class UFlashlightComponent;

/**
 * Main Inventory Screen Widget for Horror Game
 * - Inventory grid + quickbar
 * - Item details panel
 * - Category filtering
 * - Battery indicator (connected to UFlashlightComponent)
 */
UCLASS()
class ESCAPEIT_API UInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Overrides
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ========================================================================
    // INITIALIZATION
    // ========================================================================
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void InitInventory(UInventoryComponent* InInventoryComp);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void CreateSlotWidgets();

    // ========================================================================
    // REFRESH FUNCTIONS
    // ========================================================================
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RefreshInventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RefreshQuickbar();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UpdateSlotWidget(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RefreshBatteryIndicator();

    // ========================================================================
    // ITEM DETAILS
    // ========================================================================
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ShowItemDetails(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void HideItemDetails();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearSelection();

    // ========================================================================
    // FILTERING
    // ========================================================================
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void FilterByType(EItemType ItemType);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ShowAllItems();

    // ========================================================================
    // WIDGET REFERENCES (Bind in UMG)
    // ========================================================================
    // === INVENTORY GRID ===
    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* InventoryGrid;

    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* QuickbarGrid;

    // === ITEM DETAIL PANEL ===
    UPROPERTY(meta = (BindWidget))
    UBorder* ItemDetailPanel;

    UPROPERTY(meta = (BindWidget))
    UImage* ItemIcon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_ItemName;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_ItemDescription;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_ItemStats;

    // === ACTION BUTTONS ===
    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Use;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Drop;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Examine;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Close;

    // === FILTER BUTTONS ===
    UPROPERTY(meta = (BindWidget))
    UButton* Btn_All;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Consumables;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Tools;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Documents;

    // === HORROR: BATTERY INDICATOR ===
    UPROPERTY(meta = (BindWidget))
    UProgressBar* BatteryBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_BatteryPercent;

    // ========================================================================
    // SETTINGS
    // ========================================================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
    TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
    int32 GridRows = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
    int32 GridColumns = 3;

    // HORROR: UI Colors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror|UI")
    FLinearColor BatteryHighColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror|UI")
    FLinearColor BatteryMediumColor = FLinearColor::Yellow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror|UI")
    FLinearColor BatteryLowColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Horror|UI")
    FLinearColor BatteryCriticalColor = FLinearColor(0.5f, 0.0f, 0.0f); // Dark red

    // ========================================================================
    // INTERNAL DATA
    // ========================================================================
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    UInventoryComponent* InventoryComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TArray<UInventorySlotWidget*> SlotWidgets;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TArray<UInventorySlotWidget*> QuickbarSlotWidgets;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    int32 SelectedSlotIndex = -1;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    bool bShowAllItems = true;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    EItemType CurrentFilter = EItemType::Consumable;

    // Flashlight component (found at InitInventory time)
    UPROPERTY(BlueprintReadOnly, Category = "Inventory", Transient)
    UFlashlightComponent* FlashlightComponent;

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

    // Filter callbacks
    UFUNCTION()
    void OnFilterAllClicked();

    UFUNCTION()
    void OnFilterConsumablesClicked();

    UFUNCTION()
    void OnFilterToolsClicked();

    UFUNCTION()
    void OnFilterDocumentsClicked();

    // ========================================================================
    // EVENT CALLBACKS
    // ========================================================================
    UFUNCTION()
    void OnInventoryUpdated();

    UFUNCTION()
    void OnItemEquipped(FName ItemID);

    UFUNCTION()
    void OnItemUnequipped(FName ItemID);

    // Battery callbacks (both declared because .cpp defines both)
    UFUNCTION()
    void OnBatteryChanged(float CurrentBattery, float MaxBattery);

    UFUNCTION()
    void OnBatteryLevelChanged(float CurrentLevel, float MaxLevel);

    UFUNCTION()
    void OnLowBattery();

    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================
    FString BuildStatsText(const FItemData& ItemData, const FInventorySlot& SlotData);

    void ConfigureActionButtons(const FItemData& ItemData);

    void UpdateFilterButtonStates();

    void HighlightEquippedSlots();

    FLinearColor GetBatteryColor(float Percentage) const;

    void PlayBatteryWarningAnimation();

    // Optional: bind a UMG animation named "BatteryWarning" if you create it in the widget blueprint
    UPROPERTY(Transient, meta = (BindWidgetAnim), BlueprintReadOnly)
    UWidgetAnimation* BatteryWarningAnim;

    // Timer handle used for color restore / flashing fallback
    FTimerHandle BatteryFlashRestoreHandle;
};
