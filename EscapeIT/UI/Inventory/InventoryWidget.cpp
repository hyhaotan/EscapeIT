// InventoryWidget.cpp - Horror Game Complete Implementation (FIXED)

#include "InventoryWidget.h"
#include "InventorySlotWidget.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "EscapeIT/Actor/Components/FlashlightComponent.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "Animation/WidgetAnimation.h"

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // ========================================================================
    // BIND BUTTON EVENTS
    // ========================================================================
    
    if (Btn_Use) Btn_Use->OnClicked.AddDynamic(this, &UInventoryWidget::OnUseButtonClicked);
    if (Btn_Drop) Btn_Drop->OnClicked.AddDynamic(this, &UInventoryWidget::OnDropButtonClicked);
    if (Btn_Examine) Btn_Examine->OnClicked.AddDynamic(this, &UInventoryWidget::OnExamineButtonClicked);
    if (Btn_Close) Btn_Close->OnClicked.AddDynamic(this, &UInventoryWidget::OnCloseButtonClicked);
    if (Btn_All) Btn_All->OnClicked.AddDynamic(this, &UInventoryWidget::OnFilterAllClicked);
    if (Btn_Consumables) Btn_Consumables->OnClicked.AddDynamic(this, &UInventoryWidget::OnFilterConsumablesClicked);
    if (Btn_Tools) Btn_Tools->OnClicked.AddDynamic(this, &UInventoryWidget::OnFilterToolsClicked);
    if (Btn_Documents) Btn_Documents->OnClicked.AddDynamic(this, &UInventoryWidget::OnFilterDocumentsClicked);

    if (BatteryBar && Text_BatteryPercent)
    {
        BatteryBar->SetVisibility(ESlateVisibility::Collapsed);
        Text_BatteryPercent->SetVisibility(ESlateVisibility::Collapsed);
    }

    // Initial state
    bShowAllItems = true;
    CurrentFilter = EItemType::Consumable;

    HideItemDetails();
}

void UInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Update battery indicator smoothly
    if (FlashlightComponent && BatteryBar)
    {
        RefreshBatteryIndicator();
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

    // ✅ FIX: Get FlashlightComponent from owner
    if (AActor* Owner = InventoryComponent->GetOwner())
    {
        FlashlightComponent = Owner->FindComponentByClass<UFlashlightComponent>();
        
        if (FlashlightComponent)
        {
            UE_LOG(LogTemp, Log, TEXT("InitInventory: FlashlightComponent found"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("InitInventory: FlashlightComponent not found!"));
        }
    }

    // Bind to inventory events
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

    // ✅ FIX: Bind to FlashlightComponent events (not InventoryComponent)
    if (FlashlightComponent)
    {
        if (!FlashlightComponent->OnBatteryChanged.IsAlreadyBound(this, &UInventoryWidget::OnBatteryChanged))
        {
            FlashlightComponent->OnBatteryChanged.AddDynamic(this, &UInventoryWidget::OnBatteryChanged);
            UE_LOG(LogTemp, Log, TEXT("InitInventory: Bound to FlashlightComponent::OnBatteryChanged"));
        }

        if (!FlashlightComponent->OnBatteryLow.IsAlreadyBound(this, &UInventoryWidget::OnLowBattery))
        {
            FlashlightComponent->OnBatteryLow.AddDynamic(this, &UInventoryWidget::OnLowBattery);
            UE_LOG(LogTemp, Log, TEXT("InitInventory: Bound to FlashlightComponent::OnBatteryLow"));
        }
    }

    // Create UI
    CreateSlotWidgets();
    RefreshInventory();
    RefreshQuickbar();
    RefreshBatteryIndicator();

    UE_LOG(LogTemp, Log, TEXT("InitInventory: Inventory UI initialized successfully"));
}

void UInventoryWidget::CreateSlotWidgets()
{
    if (!SlotWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateSlotWidgets: SlotWidgetClass not set!"));
        return;
    }

    // ========================================================================
    // CREATE INVENTORY GRID (2x5 = 10 slots)
    // ========================================================================
    
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

        UE_LOG(LogTemp, Log, TEXT("CreateSlotWidgets: Created %d inventory slots"), SlotWidgets.Num());
    }

    // ========================================================================
    // CREATE QUICKBAR (4 slots horizontal)
    // ========================================================================
    
    if (QuickbarGrid)
    {
        QuickbarGrid->ClearChildren();
        QuickbarSlotWidgets.Empty();

        for (int32 i = 0; i < 4; i++)
        {
            UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
            if (SlotWidget)
            {
                SlotWidget->SlotIndex = i;
                SlotWidget->bIsQuickbarSlot = true;
                SlotWidget->ParentInventoryWidget = this;

                UUniformGridSlot* GridSlot = QuickbarGrid->AddChildToUniformGrid(SlotWidget, 0, i);
                if (GridSlot)
                {
                    GridSlot->SetHorizontalAlignment(HAlign_Fill);
                    GridSlot->SetVerticalAlignment(VAlign_Fill);
                }

                QuickbarSlotWidgets.Add(SlotWidget);
            }
        }

        UE_LOG(LogTemp, Log, TEXT("CreateSlotWidgets: Created %d quickbar slots"), QuickbarSlotWidgets.Num());
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

    UE_LOG(LogTemp, Verbose, TEXT("RefreshInventory: %d items, ShowAll=%s"),
        InventoryComponent->InventorySlots.Num(),
        bShowAllItems ? TEXT("Yes") : TEXT("No"));

    // Update all slots
    for (int32 i = 0; i < SlotWidgets.Num(); i++)
    {
        UpdateSlotWidget(i);
    }
    
    UpdateFilterButtonStates();
    HighlightEquippedSlots();
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

    // Check if slot exists in inventory
    if (SlotIndex < InventoryComponent->InventorySlots.Num())
    {
        FInventorySlot SlotData = InventoryComponent->InventorySlots[SlotIndex];

        // Apply filter if not showing all
        if (!bShowAllItems && SlotData.IsValid())
        {
            FItemData ItemData;
            if (InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
            {
                // Hide items that don't match filter
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

// ✅ FIX: RefreshBatteryIndicator sử dụng FlashlightComponent
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

    // ✅ Get battery data from FlashlightComponent
    float Percentage = FlashlightComponent->GetBatteryPercentage(); 
    float CurrentDuration = FlashlightComponent->GetBatteryDuration(); 

    // Update progress bar (convert percentage to 0.0-1.0)
    BatteryBar->SetPercent(Percentage / 100.0f);

    // Update color based on level
    FLinearColor BatteryColor = GetBatteryColor(Percentage);
    BatteryBar->SetFillColorAndOpacity(BatteryColor);

    // Update text
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
        UE_LOG(LogTemp, Error, TEXT("ShowItemDetails: Missing component/panel"));
        return;
    }

    SelectedSlotIndex = SlotIndex;

    // Validate slot
    if (SlotIndex < 0 || SlotIndex >= InventoryComponent->InventorySlots.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("ShowItemDetails: Invalid slot %d"), SlotIndex);
        HideItemDetails();
        return;
    }

    FInventorySlot SlotData = InventoryComponent->InventorySlots[SlotIndex];
    if (!SlotData.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("ShowItemDetails: Empty slot %d"), SlotIndex);
        HideItemDetails();
        return;
    }

    // Get item data
    FItemData ItemData;
    if (!InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("ShowItemDetails: No data for %s"), *SlotData.ItemID.ToString());
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

    if (ItemData.ItemType == EItemType::Tool)
    {
        if (ItemData.ToolType == EToolType::Flashlight)
        {
            if (BatteryBar && Text_BatteryPercent)
            {
                BatteryBar->SetVisibility(ESlateVisibility::Visible);
                Text_BatteryPercent->SetVisibility(ESlateVisibility::Visible);
            }
        
            RefreshBatteryIndicator();
        }
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

    UE_LOG(LogTemp, Log, TEXT("ShowItemDetails: Showing '%s'"), *ItemData.ItemName.ToString());
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

    if (Btn_Use)
    {
        Btn_Use->SetIsEnabled(false);
    }

    if (Btn_Drop)
    {
        Btn_Drop->SetIsEnabled(false);
    }

    if (Btn_Examine)
    {
        Btn_Examine->SetIsEnabled(false);
    }

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

    if (SelectedSlotIndex >= InventoryComponent->InventorySlots.Num())
    {
        return;
    }

    FInventorySlot SlotData = InventoryComponent->InventorySlots[SelectedSlotIndex];
    if (!SlotData.IsValid())
    {
        return;
    }

    // Use/Equip item
    FItemData ItemData;
    if (InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
    {
        if (ItemData.ToolType == EToolType::Flashlight ||
            ItemData.ToolType == EToolType::Wrench ||
            ItemData.ToolType == EToolType::MasterKey||
            ItemData.ToolType == EToolType::Lighter)
        {
            // Find item in quickbar and equip it
            for (int32 i = 0; i < 4; i++)
            {
                FInventorySlot QBSlot = InventoryComponent->GetQuickbarSlot(i);
                if (QBSlot.ItemID == SlotData.ItemID)
                {
                    InventoryComponent->EquipQuickbarSlot(i);
                    break;
                }
            }
        }
        else
        {
            // Consumable - use directly
            InventoryComponent->UseItem(SlotData.ItemID);
        }
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
    // TODO: Show detailed examination screen
    UE_LOG(LogTemp, Log, TEXT("OnExamineButtonClicked: Examine feature coming soon!"));
}

void UInventoryWidget::OnCloseButtonClicked()
{
    // Hide widget
    RemoveFromParent();

    // Reset controller state
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->SetPause(false);
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }

    UE_LOG(LogTemp, Log, TEXT("OnCloseButtonClicked: Inventory closed"));
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

    // Refresh selected item details if still valid
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

    UE_LOG(LogTemp, Log, TEXT("OnItemEquipped: %s equipped"), *ItemID.ToString());
}

void UInventoryWidget::OnItemUnequipped(FName ItemID)
{
    HighlightEquippedSlots();
    RefreshQuickbar();

    UE_LOG(LogTemp, Log, TEXT("OnItemUnequipped: %s unequipped"), *ItemID.ToString());
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
    
    float Percentage = (MaxBattery > 0.0f) ? (CurrentBattery / MaxBattery) * 100.0f : 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("OnBatteryChanged: %.1f%% (%.1fs/%.1fs)"), 
        Percentage, CurrentBattery, MaxBattery);
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

    UE_LOG(LogTemp, Warning, TEXT("OnLowBattery: LOW BATTERY WARNING!"));
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

FString UInventoryWidget::BuildStatsText(const FItemData& ItemData, const FInventorySlot& SlotData)
{
    FString StatsText;

    // Quantity (if stacked)
    if (SlotData.Quantity > 1)
    {
        StatsText += FString::Printf(TEXT("Quantity: %d\n"), SlotData.Quantity);
    }

    // Sanity effects
    if (ItemData.SanityRestoreAmount > 0.0f)
    {
        StatsText += FString::Printf(TEXT("❤ Sanity Restore: +%.0f\n"), ItemData.SanityRestoreAmount);
    }

    if (ItemData.PassiveSanityDrainReduction > 0.0f)
    {
        StatsText += FString::Printf(TEXT("🛡 Reduces Sanity Drain: -%.0f%%\n"), ItemData.PassiveSanityDrainReduction);
    }


    if (ItemData.ItemType == EItemType::Tool && FlashlightComponent)
    {
        if (ItemData.ToolType == EToolType::Flashlight && FlashlightComponent)
        {
            float BatteryPercent = FlashlightComponent->GetBatteryPercentage();
            float BatteryDuration = FlashlightComponent->GetBatteryDuration();
        
            int32 Minutes = FMath::FloorToInt(BatteryDuration / 60.0f);
            int32 Seconds = FMath::FloorToInt(BatteryDuration) % 60;
        
            StatsText += FString::Printf(TEXT("\n🔋 Battery: %.1f%% (%d:%02d remaining)\n"), 
                                         BatteryPercent, Minutes, Seconds);
        }
    }

    // Item category info
    FString TextInfo;
    switch (ItemData.ToolType)
    {
        case EToolType::Flashlight:
            TextInfo = TEXT("💡 Light Source - Press F to toggle");
            break;
        case EToolType::MasterKey:
            TextInfo = TEXT("🔑 Opens Locked Doors");
            break;
        case EToolType::Wrench:
            TextInfo = TEXT("🔧 Repair Tool");
            break;
        default:
            break;
    }

    switch (ItemData.ConsumableType)
    {
    case EConsumableType::Battery:
        TextInfo = TEXT("Increase Battery");
        break;
    case EConsumableType::Medicine:
        TextInfo = TEXT("Increase Sanity");
        break;
    case EConsumableType::Food:
        TextInfo = TEXT("Increase Sanity");
        break;
    }

    if (!TextInfo.IsEmpty())
    {
        StatsText += TEXT("\n") + TextInfo;
    }

    // Consumable warning
    if (ItemData.bIsSingleUse)
    {
        StatsText += TEXT("\n⚠ Single Use Item");
    }

    return StatsText;
}

void UInventoryWidget::ConfigureActionButtons(const FItemData& ItemData)
{
    // USE/EQUIP BUTTON
    if (Btn_Use)
    {
        bool bCanUse = ItemData.bCanBeUsed || ItemData.ItemType == EItemType::Tool;
        Btn_Use->SetIsEnabled(bCanUse);

        // Change button text based on item type
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
            else if (ItemData.ItemType == EItemType::Document)
            {
                UseButtonText->SetText(FText::FromString(TEXT("READ")));
            }  
            else if (ItemData.ItemType == EItemType::Key)
            {
                UseButtonText->SetText(FText::FromString(TEXT("INTERACT")));
            }
            else
            {
                UseButtonText->SetText(FText::FromString(TEXT("USE")));
            }
        }
    }

    // DROP BUTTON
    if (Btn_Drop)
    {
        bool bCanDrop = ItemData.bCanBeDropped && ItemData.ItemType != EItemType::QuestItem;
        Btn_Drop->SetIsEnabled(bCanDrop);
    }

    // EXAMINE BUTTON
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
    // Visually mark which filter is active by disabling the active button (simple approach)
    if (Btn_All)        Btn_All->SetIsEnabled(!bShowAllItems ? true : false);
    if (Btn_Consumables) Btn_Consumables->SetIsEnabled(!( !bShowAllItems && CurrentFilter == EItemType::Consumable ));
    if (Btn_Tools)      Btn_Tools->SetIsEnabled(!( !bShowAllItems && CurrentFilter == EItemType::Tool ));
    if (Btn_Documents)  Btn_Documents->SetIsEnabled(!( !bShowAllItems && CurrentFilter == EItemType::Document ));

    // Alternative: you can change visual appearance instead of enabling/disabling.
    // This simple approach prevents clicking the "active" filter again (but you can adjust UX).
}

void UInventoryWidget::HighlightEquippedSlots()
{
    if (!InventoryComponent)
    {
        return;
    }

    // Clear all first
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

    // 1) Highlight quickbar equipped slot by index (InventoryComponent exposes CurrentEquippedSlotIndex used elsewhere)
    int32 EquippedQuickbarIndex = -1;
    // try to read CurrentEquippedSlotIndex if it exists (we used this earlier in RefreshQuickbar)
    // assume InventoryComponent has member CurrentEquippedSlotIndex
    EquippedQuickbarIndex = InventoryComponent->CurrentEquippedSlotIndex;

    if (QuickbarSlotWidgets.IsValidIndex(EquippedQuickbarIndex))
    {
        QuickbarSlotWidgets[EquippedQuickbarIndex]->SetEquipped(true);
    }

    // 2) If an item in quickbar is equipped, highlight any matching item in the main inventory grid
    if (EquippedQuickbarIndex >= 0 && EquippedQuickbarIndex < 4)
    {
        FInventorySlot QBData = InventoryComponent->GetQuickbarSlot(EquippedQuickbarIndex);
        if (QBData.IsValid())
        {
            FName EquippedItemID = QBData.ItemID;

            for (int32 i = 0; i < SlotWidgets.Num(); ++i)
            {
                if (!SlotWidgets.IsValidIndex(i) || !InventoryComponent)
                {
                    continue;
                }

                if (i < InventoryComponent->InventorySlots.Num())
                {
                    const FInventorySlot& SlotData = InventoryComponent->InventorySlots[i];
                    if (SlotData.IsValid() && SlotData.ItemID == EquippedItemID)
                    {
                        SlotWidgets[i]->SetEquipped(true);
                    }
                }
            }
        }
    }
}

FLinearColor UInventoryWidget::GetBatteryColor(float Percentage) const
{
    // Percentage expected 0..100
    if (Percentage >= 75.0f)
    {
        return BatteryHighColor;
    }
    else if (Percentage >= 40.0f)
    {
        return BatteryMediumColor;
    }
    else if (Percentage >= 15.0f)
    {
        return BatteryLowColor;
    }
    else
    {
        return BatteryCriticalColor;
    }
}

void UInventoryWidget::PlayBatteryWarningAnimation()
{
    // 1) If an animation was bound in UMG, prefer playing it
    if (BatteryWarningAnim)
    {
        PlayAnimation(BatteryWarningAnim);
        return;
    }

    // 2) Otherwise fallback: flash BatteryBar fill color to critical then restore after short delay
    if (!BatteryBar)
    {
        return;
    }

    // Save current color
    FLinearColor OriginalColor = BatteryBar->PercentDelegate.IsBound() ? BatteryBar->GetFillColorAndOpacity() : BatteryBar->GetFillColorAndOpacity();

    // Set critical color
    BatteryBar->SetFillColorAndOpacity(BatteryCriticalColor);

    // Ensure any existing timer cleared
    if (GetWorld())
    {
        if (GetWorld()->GetTimerManager().IsTimerActive(BatteryFlashRestoreHandle))
        {
            GetWorld()->GetTimerManager().ClearTimer(BatteryFlashRestoreHandle);
        }

        // After 0.5s restore to corresponding battery color (based on current percent)
        FTimerDelegate RestoreDelegate;
        RestoreDelegate.BindLambda([this, OriginalColor]()
        {
            if (!BatteryBar || !FlashlightComponent)
            {
                if (BatteryBar)
                {
                    BatteryBar->SetFillColorAndOpacity(OriginalColor);
                }
                return;
            }

            float Percentage = FlashlightComponent->GetBatteryPercentage();
            BatteryBar->SetFillColorAndOpacity(GetBatteryColor(Percentage));
        });

        GetWorld()->GetTimerManager().SetTimer(BatteryFlashRestoreHandle, RestoreDelegate, 0.5f, false);
    }
}

void UInventoryWidget::OnBatteryLevelChanged(float CurrentLevel, float MaxLevel)
{
    // Call the existing handler (OnBatteryChanged) or directly refresh
    OnBatteryChanged(CurrentLevel, MaxLevel);
}