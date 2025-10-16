// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/Border.h"
#include "EscapeIT/Data/ItemData.h"
#include "InventoryWidget.generated.h"

class UInventoryComponent;
class UInventorySlotWidget;

UCLASS()
class ESCAPEIT_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ============================================
    // WIDGET BINDINGS
    // ============================================

    // Main container
    UPROPERTY(meta = (BindWidget))
    UBorder* MainContainer;

    // Inventory grid (main items)
    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* InventoryGrid;

    // Quickbar section
    UPROPERTY(meta = (BindWidget))
    UUniformGridPanel* QuickbarGrid;

    // Item details panel
    UPROPERTY(meta = (BindWidget))
    UBorder* ItemDetailPanel;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_ItemName;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_ItemDescription;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_ItemStats;

    // Category tabs (optional)
    UPROPERTY(meta = (BindWidget))
    UButton* Btn_All;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Consumables;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Tools;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Documents;

    // Action buttons
    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Use;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Drop;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_Close;

    // Documents scroll box
    UPROPERTY(meta = (BindWidget))
    UScrollBox* DocumentScrollBox;

    // ============================================
    // PROPERTIES
    // ============================================

    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    UInventoryComponent* InventoryComponent;

    // Slot widget class
    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

    // Grid dimensions
    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int32 GridColumns = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int32 GridRows = 3;

    // Current filter
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    EItemType CurrentFilter;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    int32 SelectedSlotIndex = -1;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    bool bShowAllItems = true;

    // ============================================
    // FUNCTIONS
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void Initialize(UInventoryComponent* InInventoryComp);

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
    void ClearSelection();

    FString BuildStatsText(const FItemData& ItemData, const FInventorySlot& SlotData);

    void UpdateItemCountDisplay();

    void ConfigureActionButtons(const FItemData& ItemData);

protected:
    // Button callbacks
    UFUNCTION()
    void OnUseButtonClicked();

    UFUNCTION()
    void OnDropButtonClicked();

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

    // Inventory events
    UFUNCTION()
    void OnInventoryUpdated();

    // Helper functions
    void CreateSlotWidgets();
    void UpdateSlotWidget(int32 SlotIndex);

    TArray<class UInventorySlotWidget*> SlotWidgets;
    TArray<class UInventorySlotWidget*> QuickbarSlotWidgets;
};
