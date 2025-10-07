// InventoryScreenWidget.cpp - Implementation

#include "InventoryWidget.h"
#include "InventorySlotWidget.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button events
    if (Btn_Use)
    {
        Btn_Use->OnClicked.AddDynamic(this, &UInventoryWidget::OnUseButtonClicked);
    }

    if (Btn_Drop)
    {
        Btn_Drop->OnClicked.AddDynamic(this, &UInventoryWidget::OnDropButtonClicked);
    }

    if (Btn_Sort)
    {
        Btn_Sort->OnClicked.AddDynamic(this, &UInventoryWidget::OnSortButtonClicked);
    }

    if (Btn_Close)
    {
        Btn_Close->OnClicked.AddDynamic(this, &UInventoryWidget::OnCloseButtonClicked);
    }

    // Filter buttons
    if (Btn_All)
    {
        Btn_All->OnClicked.AddDynamic(this, &UInventoryWidget::OnFilterAllClicked);
    }

    if (Btn_Consumables)
    {
        Btn_Consumables->OnClicked.AddDynamic(this, &UInventoryWidget::OnFilterConsumablesClicked);
    }

    if (Btn_Tools)
    {
        Btn_Tools->OnClicked.AddDynamic(this, &UInventoryWidget::OnFilterToolsClicked);
    }

    if (Btn_Documents)
    {
        Btn_Documents->OnClicked.AddDynamic(this, &UInventoryWidget::OnFilterDocumentsClicked);
    }

    // Initialize with no filter
    CurrentFilter = EItemType::Consumable; // Will show all

    HideItemDetails();
}

void UInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    UpdateInfoPanel();
}

void UInventoryWidget::Initialize(UInventoryComponent* InInventoryComp)
{
    InventoryComponent = InInventoryComp;

    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryUpdated.AddDynamic(this, &UInventoryWidget::OnInventoryUpdated);
    }

    CreateSlotWidgets();
    RefreshInventory();
    RefreshQuickbar();
}

void UInventoryWidget::CreateSlotWidgets()
{
    if (!SlotWidgetClass || !InventoryGrid)
    {
        return;
    }

    // Clear existing
    InventoryGrid->ClearChildren();
    SlotWidgets.Empty();

    // Create grid slots (5x4 = 20 slots)
    for (int32 Row = 0; Row < GridRows; Row++)
    {
        for (int32 Col = 0; Col < GridColumns; Col++)
        {
            int32 SlotIndex = Row * GridColumns + Col;

            UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
            if (SlotWidget)
            {
                SlotWidget->SlotIndex = SlotIndex;
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

    // Create quickbar slots (4 slots)
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
    }
}

void UInventoryWidget::RefreshInventory()
{
    if (!InventoryComponent)
    {
        return;
    }

    for (int32 i = 0; i < SlotWidgets.Num(); i++)
    {
        UpdateSlotWidget(i);
    }

    UpdateInfoPanel();
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
        }
    }
}

void UInventoryWidget::UpdateSlotWidget(int32 SlotIndex)
{
    if (!SlotWidgets.IsValidIndex(SlotIndex) || !InventoryComponent)
    {
        return;
    }

    if (SlotIndex < InventoryComponent->InventorySlots.Num())
    {
        FInventorySlot SlotData = InventoryComponent->InventorySlots[SlotIndex];
        SlotWidgets[SlotIndex]->UpdateSlot(SlotData);
    }
    else
    {
        // Empty slot
        SlotWidgets[SlotIndex]->UpdateSlot(FInventorySlot());
    }
}

void UInventoryWidget::UpdateInfoPanel()
{
    if (!InventoryComponent)
    {
        return;
    }

    // Update capacity
    if (Text_Capacity)
    {
        FText CapacityText = FText::FromString(FString::Printf(TEXT("%d / %d"),
            InventoryComponent->InventorySlots.Num(),
            InventoryComponent->MaxInventorySlots));
        Text_Capacity->SetText(CapacityText);
    }

    // Update weight
    if (Text_Weight && InventoryComponent->bUseWeightSystem)
    {
        FText WeightText = FText::FromString(FString::Printf(TEXT("%.1f / %.1f kg"),
            InventoryComponent->CurrentWeight,
            InventoryComponent->MaxWeight));
        Text_Weight->SetText(WeightText);

        // Color code based on weight
        if (InventoryComponent->IsOverWeight())
        {
            Text_Weight->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
        }
        else if (InventoryComponent->GetWeightPercentage() > 80.0f)
        {
            Text_Weight->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
        }
        else
        {
            Text_Weight->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        }
    }
}

void UInventoryWidget::ShowItemDetails(int32 SlotIndex)
{
    if (!InventoryComponent || !ItemDetailPanel)
    {
        return;
    }

    SelectedSlotIndex = SlotIndex;

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

    // Update texts
    if (Text_ItemName)
    {
        Text_ItemName->SetText(ItemData.ItemName);
    }

    if (Text_ItemDescription)
    {
        Text_ItemDescription->SetText(ItemData.Description);
    }

    if (Text_ItemStats)
    {
        FString StatsText;

        if (ItemData.SanityRestoreAmount > 0.0f)
        {
            StatsText += FString::Printf(TEXT("Sanity Restore: +%.0f\n"), ItemData.SanityRestoreAmount);
        }

        if (ItemData.PassiveSanityDrainReduction > 0.0f)
        {
            StatsText += FString::Printf(TEXT("Sanity Drain: -%.0f%%\n"), ItemData.PassiveSanityDrainReduction);
        }

        if (ItemData.bHasDurability)
        {
            StatsText += FString::Printf(TEXT("Uses: %d / %d\n"), SlotData.RemainingUses, ItemData.MaxUses);
        }

        if (ItemData.UsageCooldown > 0.0f)
        {
            StatsText += FString::Printf(TEXT("Cooldown: %.0fs\n"), ItemData.UsageCooldown);
        }

        StatsText += FString::Printf(TEXT("Weight: %.1f kg"), ItemData.Weight);

        Text_ItemStats->SetText(FText::FromString(StatsText));
    }

    // Enable/disable action buttons
    if (Btn_Use)
    {
        Btn_Use->SetIsEnabled(ItemData.bIsConsumable || ItemData.ItemType == EItemType::Tool);
    }

    if (Btn_Drop)
    {
        Btn_Drop->SetIsEnabled(ItemData.bCanBeDropped);
    }
}

void UInventoryWidget::HideItemDetails()
{
    if (ItemDetailPanel)
    {
        ItemDetailPanel->SetVisibility(ESlateVisibility::Collapsed);
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
}

void UInventoryWidget::FilterByType(EItemType ItemType)
{
    CurrentFilter = ItemType;
    RefreshInventory();
}

void UInventoryWidget::SortInventory()
{
    if (!InventoryComponent)
    {
        return;
    }

    // Sort by item type
    InventoryComponent->InventorySlots.Sort([this](const FInventorySlot& A, const FInventorySlot& B)
        {
            FItemData DataA, DataB;
            InventoryComponent->GetItemData(A.ItemID, DataA);
            InventoryComponent->GetItemData(B.ItemID, DataB);

            if (DataA.ItemType != DataB.ItemType)
            {
                return DataA.ItemType < DataB.ItemType;
            }

            return DataA.ItemName.CompareTo(DataB.ItemName) < 0;
        });

    RefreshInventory();
}

// ============================================
// BUTTON CALLBACKS
// ============================================
void UInventoryWidget::OnUseButtonClicked()
{
    if (InventoryComponent && SelectedSlotIndex >= 0)
    {
        FInventorySlot SlotData = InventoryComponent->InventorySlots[SelectedSlotIndex];
        InventoryComponent->UseItem(SlotData.ItemID);
    }
}

void UInventoryWidget::OnDropButtonClicked()
{
    if (InventoryComponent && SelectedSlotIndex >= 0)
    {
        FInventorySlot SlotData = InventoryComponent->InventorySlots[SelectedSlotIndex];
        InventoryComponent->DropItem(SlotData.ItemID, 1);
        HideItemDetails();
    }
}

void UInventoryWidget::OnSortButtonClicked()
{
    SortInventory();
}

void UInventoryWidget::OnCloseButtonClicked()
{
    RemoveFromParent();

    // Notify controller
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->SetPause(false);
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }
}

void UInventoryWidget::OnFilterAllClicked()
{
    RefreshInventory(); // Show all
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

void UInventoryWidget::OnInventoryUpdated()
{
    RefreshInventory();
    RefreshQuickbar();
}

void UInventoryWidget::ClearSelection()
{
    HideItemDetails();

    // Visual clear selection on all slots
    for (UInventorySlotWidget* Slots : SlotWidgets)
    {
        if (Slots)
        {
            Slots->SetSelected(false);
        }
    }
}