
#include "UI/Inventory/InventoryWidget.h"
#include "UI/Inventory/InventorySlotWidget.h"
#include "Actor/Components/InventoryComponent.h"
#include "Actor/Components/FlashlightComponent.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind buttons
    if (Btn_Use) Btn_Use->OnClicked.AddDynamic(this, &UInventoryWidget::OnUseButtonClicked);
    if (Btn_Drop) Btn_Drop->OnClicked.AddDynamic(this, &UInventoryWidget::OnDropButtonClicked);
    if (Btn_Examine) Btn_Examine->OnClicked.AddDynamic(this, &UInventoryWidget::OnExamineButtonClicked);
    if (Btn_Close) Btn_Close->OnClicked.AddDynamic(this, &UInventoryWidget::OnCloseButtonClicked);
    if (Btn_All) Btn_All->OnClicked.AddDynamic(this, &UInventoryWidget::OnFilterAllClicked);
    if (Btn_Consumables) Btn_Consumables->OnClicked.AddDynamic(this, &UInventoryWidget::OnFilterConsumablesClicked);
    if (Btn_Tools) Btn_Tools->OnClicked.AddDynamic(this, &UInventoryWidget::OnFilterToolsClicked);
    if (Btn_Documents) Btn_Documents->OnClicked.AddDynamic(this, &UInventoryWidget::OnFilterDocumentsClicked);

    if (DetailsTutorialWidgetClass)
    {
        DetailsTutorialWidget = CreateWidget<UUserWidget>(this, DetailsTutorialWidgetClass);
        if (DetailsTutorialWidget)
        {
            DetailsTutorialWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
    
    if (BatteryBar && Text_BatteryPercent)
    {
        BatteryBar->SetVisibility(ESlateVisibility::Collapsed);
        Text_BatteryPercent->SetVisibility(ESlateVisibility::Collapsed);
    }

    bShowAllItems = true;
    bIsTutorialVisible = false;
    HideItemDetails();
}

void UInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (FlashlightComponent && BatteryBar)
    {
        RefreshBatteryIndicator();
    }

    if (bIsTutorialVisible && DetailsTutorialWidget)
    {
        UpdateTutorialPosition();
    }
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void UInventoryWidget::InitInventory(UInventoryComponent* InInventoryComp)
{
    if (!InInventoryComp)
    {
        UE_LOG(LogTemp, Error, TEXT("InitInventory: NULL InventoryComponent!"));
        return;
    }

    InventoryComponent = InInventoryComp;
    
    if (AActor* Owner = InventoryComponent->GetOwner())
    {
        FlashlightComponent = Owner->FindComponentByClass<UFlashlightComponent>();
    }

    // Bind events
    if (!InventoryComponent->OnInventoryUpdated.IsAlreadyBound(this, &UInventoryWidget::OnInventoryUpdated))
    {
        InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UInventoryWidget::OnInventoryUpdated);
    }

    if (!InventoryComponent->OnItemEquipped.IsAlreadyBound(this, &UInventoryWidget::OnItemEquipped))
    {
        InventoryComponent->OnItemEquipped.AddDynamic(this, &UInventoryWidget::OnItemEquipped);
    }

    if (!InventoryComponent->OnItemUnequipped.IsAlreadyBound(this, &UInventoryWidget::OnItemUnequipped))
    {
        InventoryComponent->OnItemUnequipped.AddDynamic(this, &UInventoryWidget::OnItemUnequipped);
    }
    
    if (FlashlightComponent)
    {
        if (!FlashlightComponent->OnBatteryChanged.IsAlreadyBound(this, &UInventoryWidget::OnBatteryChanged))
        {
            FlashlightComponent->OnBatteryChanged.AddDynamic(this, &UInventoryWidget::OnBatteryChanged);
        }

        if (!FlashlightComponent->OnBatteryLow.IsAlreadyBound(this, &UInventoryWidget::OnLowBattery))
        {
            FlashlightComponent->OnBatteryLow.AddDynamic(this, &UInventoryWidget::OnLowBattery);
        }
    }

    CreateSlotWidgets();
    RefreshInventory();
    RefreshQuickbar();
    RefreshBatteryIndicator();

    UE_LOG(LogTemp, Log, TEXT("InventoryWidget initialized"));
}

void UInventoryWidget::CreateSlotWidgets()
{
    if (!SlotWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateSlotWidgets: SlotWidgetClass not set!"));
        return;
    }

    // Create inventory grid
    if (InventoryGrid)
    {
        InventoryGrid->ClearChildren();
        SlotWidgets.Empty();

        for (int32 Row = 0; Row < GridRows; Row++)
        {
            for (int32 Col = 0; Col < GridColumns; Col++)
            {
                int32 SlotIndex = Row * GridColumns + Col;

                UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
                if (SlotWidget)
                {
                    SlotWidget->SlotIndex = SlotIndex;
                    SlotWidget->bIsQuickbarSlot = false;
                    SlotWidget->ParentInventoryWidget = this;
                    SlotWidget->InventoryComponentRef = InventoryComponent;

                    UUniformGridSlot* GridSlot = InventoryGrid->AddChildToUniformGrid(SlotWidget, Row, Col);
                    if (GridSlot)
                    {
                        GridSlot->SetHorizontalAlignment(HAlign_Fill);
                        GridSlot->SetVerticalAlignment(VAlign_Fill);
                    }

                    SlotWidgets.Add(SlotWidget);
                }
            }
        }
    }

    // Create quickbar
    if (QuickbarGrid)
    {
        QuickbarGrid->ClearChildren();
        QuickbarSlotWidgets.Empty();

        for (int32 i = 0; i < 3; i++)
        {
            UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
            if (SlotWidget)
            {
                SlotWidget->SlotIndex = i;
                SlotWidget->bIsQuickbarSlot = true;
                SlotWidget->ParentInventoryWidget = this;
                SlotWidget->InventoryComponentRef = InventoryComponent;

                UUniformGridSlot* GridSlot = QuickbarGrid->AddChildToUniformGrid(SlotWidget, 0, i);
                if (GridSlot)
                {
                    GridSlot->SetHorizontalAlignment(HAlign_Fill);
                    GridSlot->SetVerticalAlignment(VAlign_Fill);
                }

                QuickbarSlotWidgets.Add(SlotWidget);
            }
        }
    }
}

// ============================================================================
// REFRESH FUNCTIONS
// ============================================================================

void UInventoryWidget::RefreshInventory()
{
    if (!InventoryComponent)
    {
        return;
    }
    
    // Validate selected slot before refreshing
    ValidateSelectedSlot();

    // Update all slots
    for (int32 i = 0; i < SlotWidgets.Num(); i++)
    {
        UpdateSlotWidget(i);
    }
    
    UpdateFilterButtonStates();
    HighlightEquippedSlots();
    
    // Refresh details if a slot is selected
    if (SelectedSlotIndex >= 0 && SelectedSlotIndex < InventoryComponent->InventorySlots.Num())
    {
        ShowItemDetails(SelectedSlotIndex);
    }
}

void UInventoryWidget::RefreshQuickbar()
{
    if (!InventoryComponent)
    {
        return;
    }

    for (int32 i = 0; i < QuickbarSlotWidgets.Num(); i++)
    {
        if (QuickbarSlotWidgets.IsValidIndex(i))
        {
            // ✅ Get slot data from inventory component (uses new logic)
            FInventorySlot SlotData = InventoryComponent->GetQuickbarSlot(i);
            QuickbarSlotWidgets[i]->UpdateSlot(SlotData);

            // Highlight if equipped
            bool bIsEquipped = (InventoryComponent->CurrentEquippedSlotIndex == i);
            QuickbarSlotWidgets[i]->SetEquipped(bIsEquipped);
        }
    }
}

void UInventoryWidget::UpdateSlotWidget(int32 SlotIndex)
{
    if (!SlotWidgets.IsValidIndex(SlotIndex) || !InventoryComponent)
    {
        return;
    }

    // ✅ Check if slot exists in inventory
    if (SlotIndex < InventoryComponent->InventorySlots.Num())
    {
        FInventorySlot SlotData = InventoryComponent->InventorySlots[SlotIndex];

        // Apply filter
        if (!bShowAllItems && SlotData.IsValid())
        {
            FItemData ItemData;
            if (InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
            {
                if (ItemData.ItemType != CurrentFilter)
                {
                    SlotWidgets[SlotIndex]->UpdateSlot(FInventorySlot());
                    return;
                }
            }
        }

        SlotWidgets[SlotIndex]->UpdateSlot(SlotData);
    }
    else
    {
        // Empty slot
        SlotWidgets[SlotIndex]->UpdateSlot(FInventorySlot());
    }
}

void UInventoryWidget::RefreshBatteryIndicator()
{
    if (!FlashlightComponent || !BatteryBar)
    {
        return;
    }

    if (BatteryBar->GetVisibility() != ESlateVisibility::Visible)
    {
        return;
    }

    float Percentage = FlashlightComponent->GetBatteryPercentage();
    BatteryBar->SetPercent(Percentage / 100.0f);

    FLinearColor BatteryColor = GetBatteryColor(Percentage);
    BatteryBar->SetFillColorAndOpacity(BatteryColor);

    if (Text_BatteryPercent)
    {
        Text_BatteryPercent->SetText(FText::Format(
            FText::FromString("{0}%"),
            FText::AsNumber(FMath::RoundToInt(Percentage))
        ));
        Text_BatteryPercent->SetColorAndOpacity(FSlateColor(BatteryColor));
    }
}

// ============================================================================
// ITEM DETAILS
// ============================================================================

void UInventoryWidget::ShowItemDetails(int32 SlotIndex)
{
    if (!InventoryComponent || !ItemDetailPanel)
    {
        return;
    }

    SelectedSlotIndex = SlotIndex;

    // Validate slot
    if (SlotIndex < 0 || SlotIndex >= InventoryComponent->InventorySlots.Num())
    {
        HideItemDetails();
        return;
    }

    FInventorySlot SlotData = InventoryComponent->InventorySlots[SlotIndex];
    if (!SlotData.IsValid())
    {
        HideItemDetails();
        return;
    }

    // Get item data
    FItemData ItemData;
    if (!InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
    {
        HideItemDetails();
        return;
    }

    // Show panel
    ItemDetailPanel->SetVisibility(ESlateVisibility::Visible);

    // Update icon
    if (ItemIcon && ItemData.Icon)
    {
        ItemIcon->SetBrushFromTexture(ItemData.Icon);
        ItemIcon->SetVisibility(ESlateVisibility::Visible);
    }

    // Update name
    if (Text_ItemName)
    {
        Text_ItemName->SetText(ItemData.ItemName);
    }

    // Update description
    if (Text_ItemDescription)
    {
        Text_ItemDescription->SetText(ItemData.Description);
    }

    // Update stats
    if (Text_ItemStats)
    {
        FString StatsText = BuildStatsText(ItemData, SlotData);
        Text_ItemStats->SetText(FText::FromString(StatsText));
    }

    // Show battery for flashlight
    if (ItemData.ItemType == EItemType::Tool && ItemData.ToolType == EToolType::Flashlight)
    {
        if (BatteryBar && Text_BatteryPercent)
        {
            BatteryBar->SetVisibility(ESlateVisibility::Visible);
            Text_BatteryPercent->SetVisibility(ESlateVisibility::Visible);
        }
        RefreshBatteryIndicator();
    }
    else
    {
        if (BatteryBar && Text_BatteryPercent)
        {
            BatteryBar->SetVisibility(ESlateVisibility::Collapsed);
            Text_BatteryPercent->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // Configure action buttons
    ConfigureActionButtons(ItemData);

    // Update selection visual
    ClearSelection();
    if (SlotWidgets.IsValidIndex(SlotIndex))
    {
        SlotWidgets[SlotIndex]->SetSelected(true);
    }
}

void UInventoryWidget::HideItemDetails()
{
    if (ItemDetailPanel)
    {
        ItemDetailPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    if (BatteryBar && Text_BatteryPercent)
    {
        BatteryBar->SetVisibility(ESlateVisibility::Collapsed);
        Text_BatteryPercent->SetVisibility(ESlateVisibility::Collapsed);
    }

    SelectedSlotIndex = -1;

    if (Btn_Use) Btn_Use->SetIsEnabled(false);
    if (Btn_Drop) Btn_Drop->SetIsEnabled(false);
    if (Btn_Examine) Btn_Examine->SetIsEnabled(false);

    ClearSelection();
}

void UInventoryWidget::ClearSelection()
{
    for (UInventorySlotWidget* Slots : SlotWidgets)
    {
        if (Slots)
        {
            Slots->SetSelected(false);
        }
    }
}

// ============================================================================
// FILTERING
// ============================================================================

void UInventoryWidget::FilterByType(EItemType ItemType)
{
    bShowAllItems = false;
    CurrentFilter = ItemType;
    RefreshInventory();
    UpdateFilterButtonStates();
}

void UInventoryWidget::ShowAllItems()
{
    bShowAllItems = true;
    RefreshInventory();
    UpdateFilterButtonStates();
}

// ============================================================================
// BUTTON CALLBACKS
// ============================================================================

void UInventoryWidget::OnUseButtonClicked()
{
      if (!InventoryComponent || SelectedSlotIndex < 0)
    {
        return;
    }

    // Validate slot index
    if (SelectedSlotIndex >= InventoryComponent->InventorySlots.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid selected slot index"));
        HideItemDetails();
        return;
    }

    FInventorySlot SlotData = InventoryComponent->InventorySlots[SelectedSlotIndex];
    if (!SlotData.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid slot data"));
        HideItemDetails();
        return;
    }

    FItemData ItemData;
    if (!InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot get item data for %s"), *SlotData.ItemID.ToString());
        return;
    }

    // ========================================================================
    // TOOL ITEMS (Need to equip to quickbar)
    // ========================================================================
    if (ItemData.ItemType == EItemType::Tool)
    {
        // Check if item is already in quickbar
        int32 QuickbarIndex = InventoryComponent->FindQuickbarSlotByInventoryIndex(SelectedSlotIndex);
        
        if (QuickbarIndex < 0)
        {
            // Not in quickbar - try to assign
            QuickbarIndex = InventoryComponent->GetFirstEmptyQuickbarSlot();
            
            if (QuickbarIndex < 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("Quickbar is full! Cannot equip tool."));
                ShowQuickbarFullMessage();
                return;
            }
            
            // Assign to quickbar
            if (!InventoryComponent->AssignToQuickbar(SelectedSlotIndex, QuickbarIndex))
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to assign to quickbar"));
                return;
            }
            
            UE_LOG(LogTemp, Log, TEXT("Assigned to quickbar slot %d"), QuickbarIndex);
        }
        
        // Equip the tool
        bool bSuccess = InventoryComponent->EquipQuickbarSlot(QuickbarIndex);
        
        if (bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("Equipped %s"), *ItemData.ItemName.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to equip item"));
        }
        
        return;
    }
    
    // ========================================================================
    // CONSUMABLE ITEMS (Use directly)
    // ========================================================================
    if (ItemData.ItemType == EItemType::Consumable)
    {
        bool bSuccess = InventoryComponent->UseItem(SlotData.ItemID);
        
        if (bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("Used consumable: %s"), *ItemData.ItemName.ToString());
            
            // Hide details if item was consumed
            if (InventoryComponent->GetItemQuantity(SlotData.ItemID) <= 0)
            {
                HideItemDetails();
            }
            else
            {
                // Refresh if there are more items
                ShowItemDetails(SelectedSlotIndex);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to use item"));
        }
        
        return;
    }
    
    // ========================================================================
    // OTHER ITEMS
    // ========================================================================
    if (ItemData.bCanBeUsed)
    {
        InventoryComponent->UseItem(SlotData.ItemID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Item cannot be used"));
    }
}

void UInventoryWidget::OnDropButtonClicked()
{
    if (!InventoryComponent || SelectedSlotIndex < 0)
    {
        return;
    }

    if (SelectedSlotIndex >= InventoryComponent->InventorySlots.Num())
    {
        return;
    }

    FInventorySlot SlotData = InventoryComponent->InventorySlots[SelectedSlotIndex];
    if (SlotData.IsValid())
    {
        bool bSuccess = InventoryComponent->DropItem(SlotData.ItemID, 1);
        
        if (bSuccess)
        {
            HideItemDetails();
        }
    }
}

void UInventoryWidget::OnExamineButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Examine feature coming soon!"));
}

void UInventoryWidget::OnCloseButtonClicked()
{
    RemoveFromParent();

    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->SetPause(false);
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }
}

void UInventoryWidget::OnFilterAllClicked()
{
    ShowAllItems();
}

void UInventoryWidget::OnFilterConsumablesClicked()
{
    FilterByType(EItemType::Consumable);
}

void UInventoryWidget::OnFilterToolsClicked()
{
    FilterByType(EItemType::Tool);
}

void UInventoryWidget::OnFilterDocumentsClicked()
{
    FilterByType(EItemType::Document);
}

// ============================================================================
// EVENT CALLBACKS
// ============================================================================

void UInventoryWidget::OnInventoryUpdated()
{
    RefreshInventory();
    RefreshQuickbar();

    if (SelectedSlotIndex >= 0 && SelectedSlotIndex < InventoryComponent->InventorySlots.Num())
    {
        ShowItemDetails(SelectedSlotIndex);
    }
    else
    {
        HideItemDetails();
    }
}

void UInventoryWidget::OnItemEquipped(FName ItemID)
{
    HighlightEquippedSlots();
    RefreshQuickbar();
}

void UInventoryWidget::OnItemUnequipped(FName ItemID)
{
    HighlightEquippedSlots();
    RefreshQuickbar();
}

void UInventoryWidget::OnBatteryChanged(float CurrentBattery, float MaxBattery)
{
    if (SelectedSlotIndex >= 0 && InventoryComponent)
    {
        if (SelectedSlotIndex < InventoryComponent->InventorySlots.Num())
        {
            FInventorySlot SlotData = InventoryComponent->InventorySlots[SelectedSlotIndex];
            if (SlotData.IsValid())
            {
                FItemData ItemData;
                if (InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
                {
                    if (ItemData.ToolType == EToolType::Flashlight)
                    {
                        RefreshBatteryIndicator();
                    }
                }
            }
        }
    }
}

void UInventoryWidget::OnLowBattery()
{
    if (SelectedSlotIndex >= 0 && InventoryComponent)
    {
        if (SelectedSlotIndex < InventoryComponent->InventorySlots.Num())
        {
            FInventorySlot SlotData = InventoryComponent->InventorySlots[SelectedSlotIndex];
            if (SlotData.IsValid())
            {
                FItemData ItemData;
                if (InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
                {
                    if (ItemData.ToolType == EToolType::Flashlight)
                    {
                        PlayBatteryWarningAnimation();
                    }
                }
            }
        }
    }
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

FString UInventoryWidget::BuildStatsText(const FItemData& ItemData, const FInventorySlot& SlotData)
{
    FString StatsText;

    if (SlotData.Quantity > 1)
    {
        StatsText += FString::Printf(TEXT("Quantity: %d\n"), SlotData.Quantity);
    }

    if (ItemData.SanityRestoreAmount > 0.0f)
    {
        StatsText += FString::Printf(TEXT("❤ Sanity Restore: +%.0f\n"), ItemData.SanityRestoreAmount);
    }

    if (ItemData.PassiveSanityDrainReduction > 0.0f)
    {
        StatsText += FString::Printf(TEXT("🛡 Reduces Sanity Drain: -%.0f%%\n"), ItemData.PassiveSanityDrainReduction);
    }

    if (ItemData.ItemType == EItemType::Tool && ItemData.ToolType == EToolType::Flashlight && FlashlightComponent)
    {
        float BatteryPercent = FlashlightComponent->GetBatteryPercentage();
        float BatteryDuration = FlashlightComponent->GetMaxBatteryDuration();
    
        int32 Minutes = FMath::FloorToInt(BatteryDuration / 60.0f);
        int32 Seconds = FMath::FloorToInt(BatteryDuration) % 60;
    
        StatsText += FString::Printf(TEXT("\n🔋 Battery: %.1f%% (%d:%02d remaining)\n"), 
                                     BatteryPercent, Minutes, Seconds);
    }

    return StatsText;
}

void UInventoryWidget::ConfigureActionButtons(const FItemData& ItemData)
{
    if (Btn_Use)
    {
        bool bCanUse = ItemData.bCanBeUsed || ItemData.ItemType == EItemType::Tool;
        Btn_Use->SetIsEnabled(bCanUse);

        if (UTextBlock* UseButtonText = Cast<UTextBlock>(Btn_Use->GetChildAt(0)))
        {
            if (ItemData.ItemType == EItemType::Tool)
            {
                UseButtonText->SetText(FText::FromString(TEXT("EQUIP")));
            }
            else if (ItemData.ItemType == EItemType::Consumable)
            {
                UseButtonText->SetText(FText::FromString(TEXT("USE")));
            }
            else
            {
                UseButtonText->SetText(FText::FromString(TEXT("USE")));
            }
        }
    }

    if (Btn_Drop)
    {
        bool bCanDrop = ItemData.bCanBeDropped && ItemData.ItemType != EItemType::QuestItem;
        Btn_Drop->SetIsEnabled(bCanDrop);
    }

    if (Btn_Examine)
    {
        bool bCanExamine = (ItemData.ItemType == EItemType::Document || 
                            ItemData.ItemType == EItemType::Passive || 
                            ItemData.ItemType == EItemType::QuestItem);
        Btn_Examine->SetIsEnabled(bCanExamine);
    }
}

void UInventoryWidget::UpdateFilterButtonStates()
{
    if (Btn_All) Btn_All->SetIsEnabled(!bShowAllItems);
    if (Btn_Consumables) Btn_Consumables->SetIsEnabled(bShowAllItems || CurrentFilter != EItemType::Consumable);
    if (Btn_Tools) Btn_Tools->SetIsEnabled(bShowAllItems || CurrentFilter != EItemType::Tool);
    if (Btn_Documents) Btn_Documents->SetIsEnabled(bShowAllItems || CurrentFilter != EItemType::Document);
}

void UInventoryWidget::HighlightEquippedSlots()
{
    if (!InventoryComponent)
    {
        return;
    }

    // Clear all highlights first
    for (UInventorySlotWidget* SlotWidget : SlotWidgets)
    {
        if (SlotWidget)
        {
            SlotWidget->SetEquipped(false);
        }
    }

    for (UInventorySlotWidget* QBWidget : QuickbarSlotWidgets)
    {
        if (QBWidget)
        {
            QBWidget->SetEquipped(false);
        }
    }

    // Get equipped slot
    int32 EquippedQuickbarIndex = InventoryComponent->CurrentEquippedSlotIndex;

    // Validate equipped slot
    if (EquippedQuickbarIndex < 0 || EquippedQuickbarIndex >= QuickbarSlotWidgets.Num())
    {
        return; // No valid equipped slot
    }

    // Highlight quickbar slot
    if (QuickbarSlotWidgets.IsValidIndex(EquippedQuickbarIndex))
    {
        QuickbarSlotWidgets[EquippedQuickbarIndex]->SetEquipped(true);
    }

    // Highlight corresponding inventory slot
    int32 EquippedInventoryIndex = InventoryComponent->GetQuickbarInventoryIndex(EquippedQuickbarIndex);
    
    if (EquippedInventoryIndex >= 0 && SlotWidgets.IsValidIndex(EquippedInventoryIndex))
    {
        SlotWidgets[EquippedInventoryIndex]->SetEquipped(true);
    }
}

FLinearColor UInventoryWidget::GetBatteryColor(float Percentage) const
{
    if (Percentage >= 75.0f) return BatteryHighColor;
    else if (Percentage >= 40.0f) return BatteryMediumColor;
    else if (Percentage >= 15.0f) return BatteryLowColor;
    else return BatteryCriticalColor;
}

void UInventoryWidget::PlayBatteryWarningAnimation()
{
    if (BatteryWarningAnim)
    {
        PlayAnimation(BatteryWarningAnim);
        return;
    }

    // Fallback: flash effect
    if (!BatteryBar)
    {
        return;
    }

    BatteryBar->SetFillColorAndOpacity(BatteryCriticalColor);

    if (GetWorld())
    {
        FTimerDelegate RestoreDelegate;
        RestoreDelegate.BindLambda([this]()
        {
            if (BatteryBar && FlashlightComponent)
            {
                float Percentage = FlashlightComponent->GetBatteryPercentage();
                BatteryBar->SetFillColorAndOpacity(GetBatteryColor(Percentage));
            }
        });

        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, RestoreDelegate, 0.5f, false);
    }
}

// ============================================================================
// QUICKBAR HELPERS
// ============================================================================

int32 UInventoryWidget::FindItemInQuickbar(FName ItemID) const
{
    if (!InventoryComponent)
    {
        return -1;
    }

    for (int32 i = 0; i < 3; i++)
    {
        FInventorySlot QBSlot = InventoryComponent->GetQuickbarSlot(i);
        if (QBSlot.IsValid() && QBSlot.ItemID == ItemID)
        {
            return i;
        }
    }

    return -1;
}

int32 UInventoryWidget::FindQuickbarSlotByItemID(FName ItemID) const
{
    if (!InventoryComponent)
    {
        return -1;
    }

    for (int32 i = 0; i < 3; i++)
    {
        FInventorySlot QBSlot = InventoryComponent->GetQuickbarSlot(i);
        if (QBSlot.IsValid() && QBSlot.ItemID == ItemID)
        {
            return i;
        }
    }

    return -1;
}

int32 UInventoryWidget::FindEmptyQuickbarSlot() const
{
    if (!InventoryComponent)
    {
        return -1;
    }

    return InventoryComponent->GetFirstEmptyQuickbarSlot();
}

void UInventoryWidget::ShowQuickbarFullMessage()
{
    // Implement visual feedback (e.g., flash quickbar, show text)
    if (QuickbarGrid)
    {
        // Flash effect hoặc thông báo UI
        UE_LOG(LogTemp, Warning, TEXT("QUICKBAR FULL - Clear a slot first!"));
        
        // PlayAnimation(QuickbarFullAnim);
    }
}

bool UInventoryWidget::TryAssignItemToQuickbar(int32 InventorySlotIndex, FName ItemID, int32 TargetQuickbarSlot)
{
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("TryAssignItemToQuickbar: No InventoryComponent!"));
        return false;
    }
    
    if (TargetQuickbarSlot < 0 || TargetQuickbarSlot >= 3)
    {
        UE_LOG(LogTemp, Error, TEXT("TryAssignItemToQuickbar: Invalid target slot %d"), TargetQuickbarSlot);
        return false;
    }

    int32 CurrentQuickbarSlot = InventoryComponent->FindQuickbarSlotByInventoryIndex(InventorySlotIndex);
    
    if (CurrentQuickbarSlot == TargetQuickbarSlot)
    {
        UE_LOG(LogTemp, Log, TEXT("Item already in target quickbar slot %d"), TargetQuickbarSlot);
        return true;
    }

    FInventorySlot TargetSlotData = InventoryComponent->GetQuickbarSlot(TargetQuickbarSlot);
    
    if (TargetSlotData.IsValid())
    {
        if (CurrentQuickbarSlot >= 0)
        {
            UE_LOG(LogTemp, Log, TEXT("Swapping quickbar slots %d ↔ %d"), 
                CurrentQuickbarSlot, TargetQuickbarSlot);
            return InventoryComponent->SwapQuickbarSlots(CurrentQuickbarSlot, TargetQuickbarSlot);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Replacing item in quickbar slot %d"), TargetQuickbarSlot);
            return InventoryComponent->AssignToQuickbar(InventorySlotIndex, TargetQuickbarSlot);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Assigning inventory[%d] to quickbar[%d]"), 
        InventorySlotIndex, TargetQuickbarSlot);
    return InventoryComponent->AssignToQuickbar(InventorySlotIndex, TargetQuickbarSlot);
}

void UInventoryWidget::ValidateSelectedSlot()
{
    if (SelectedSlotIndex < 0)
    {
        return;
    }

    // Check if slot still exists
    if (SelectedSlotIndex >= InventoryComponent->InventorySlots.Num())
    {
        HideItemDetails();
        return;
    }

    // Check if slot is still valid
    if (!InventoryComponent->InventorySlots[SelectedSlotIndex].IsValid())
    {
        HideItemDetails();
        return;
    }
}

void UInventoryWidget::ShowDetailsTutorial()
{
    if (!DetailsTutorialWidget) return;

    if (!DetailsTutorialWidget->IsInViewport())
    {
        DetailsTutorialWidget->AddToViewport(999);
    }
    
    bIsTutorialVisible = true;
    DetailsTutorialWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
    
    UpdateTutorialPosition();
}

void UInventoryWidget::HideDetailsTutorial()
{
    if (!DetailsTutorialWidget) return;
    
    bIsTutorialVisible = false;
    DetailsTutorialWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UInventoryWidget::UpdateTutorialPosition()
{
    if (!DetailsTutorialWidget || !DetailsTutorialWidget->IsInViewport()) return;
    
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;
    
    float MouseX,MouseY;
    if (PC->GetMousePosition(MouseX,MouseY))
    {
        LastMousePosition = FVector2D(MouseX,MouseY);
    }
    
    FVector2D NewPosition = LastMousePosition + TutorialOffset;

    if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
    {
        FVector2D ViewportSize;
        Viewport->GetViewportSize(ViewportSize);
        
        FVector2D WidgetSize = DetailsTutorialWidget->GetDesiredSize();

        if (NewPosition.X + WidgetSize.X > ViewportSize.X)
        {
            NewPosition.X = LastMousePosition.X - WidgetSize.X - 20.0f;
        }

        if (NewPosition.Y + WidgetSize.Y > ViewportSize.Y)
        {
            NewPosition.Y = LastMousePosition.Y - WidgetSize.Y - 10.0f;
        }
        
        NewPosition.X = FMath::Max(0.0f,NewPosition.X);
        NewPosition.Y = FMath::Max(0.0f,NewPosition.Y);
        
        TargetTutorialPosition = NewPosition;
        CurrentTutorialPosition = FMath::Vector2DInterpTo(
            CurrentTutorialPosition,
            TargetTutorialPosition,
            GetWorld()->GetDeltaSeconds(),
            10.0f
        );

        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(DetailsTutorialWidget->Slot))
        {
            CanvasSlot->SetPosition(CurrentTutorialPosition);
        }
    }
}
