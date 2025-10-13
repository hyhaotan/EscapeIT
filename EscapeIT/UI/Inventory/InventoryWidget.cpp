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
    bShowAllItems = true;

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

        // Apply filter if needed
        if (!bShowAllItems && CurrentFilter != EItemType::Consumable) // Assuming Consumable as "no filter"
        {
            FItemData ItemData;
            if (InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
            {
                if (ItemData.ItemType != CurrentFilter)
                {
                    // Hide this slot by showing empty
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
        UE_LOG(LogTemp, Error, TEXT("ShowItemDetails failed: InventoryComponent=%s, ItemDetailPanel=%s"),
            InventoryComponent ? TEXT("Valid") : TEXT("NULL"),
            ItemDetailPanel ? TEXT("Valid") : TEXT("NULL"));
        return;
    }

    SelectedSlotIndex = SlotIndex;

    // Validate slot index
    if (SlotIndex < 0 || SlotIndex >= InventoryComponent->InventorySlots.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid slot index: %d (Max: %d)"),
            SlotIndex,
            InventoryComponent->InventorySlots.Num() - 1);
        HideItemDetails();
        return;
    }

    FInventorySlot SlotData = InventoryComponent->InventorySlots[SlotIndex];
    if (!SlotData.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Slot %d has invalid data"), SlotIndex);
        HideItemDetails();
        return;
    }

    // Get item data
    FItemData ItemData;
    if (!InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get item data for ItemID: %s"),
            *SlotData.ItemID.ToString());
        HideItemDetails();
        return;
    }

    // SUCCESS - Show the panel
    UE_LOG(LogTemp, Warning, TEXT("Showing item details for: %s"), *ItemData.ItemName.ToString());

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

        // Rarity
        FString RarityStr;
        switch (ItemData.Rarity)
        {
        case EItemRarity::Common: RarityStr = TEXT("Common"); break;
        case EItemRarity::Uncommon: RarityStr = TEXT("Uncommon"); break;
        case EItemRarity::Rare: RarityStr = TEXT("Rare"); break;
        case EItemRarity::Unique: RarityStr = TEXT("Unique"); break;
        default: RarityStr = TEXT("Unknown"); break;
        }
        StatsText += FString::Printf(TEXT("Rarity: %s\n"), *RarityStr);

        // Type
        FString TypeStr;
        switch (ItemData.ItemType)
        {
        case EItemType::Consumable: TypeStr = TEXT("Consumable"); break;
        case EItemType::Tool: TypeStr = TEXT("Tool"); break;
        case EItemType::Document: TypeStr = TEXT("Document"); break;
        default: TypeStr = TEXT("Unknown"); break;
        }
        StatsText += FString::Printf(TEXT("Type: %s\n\n"), *TypeStr);

        // Stats
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
            float DurabilityPercent = (float)SlotData.RemainingUses / (float)ItemData.MaxUses * 100.0f;
            StatsText += FString::Printf(TEXT("Durability: %d / %d (%.0f%%)\n"),
                SlotData.RemainingUses,
                ItemData.MaxUses,
                DurabilityPercent);
        }

        if (ItemData.UsageCooldown > 0.0f)
        {
            StatsText += FString::Printf(TEXT("Cooldown: %.1fs\n"), ItemData.UsageCooldown);
        }

        StatsText += FString::Printf(TEXT("\nWeight: %.1f kg\n"), ItemData.Weight);

        if (SlotData.Quantity > 1)
        {
            StatsText += FString::Printf(TEXT("Quantity: %d"), SlotData.Quantity);
        }

        Text_ItemStats->SetText(FText::FromString(StatsText));
    }

    // Enable/disable action buttons based on item properties
    if (Btn_Use)
    {
        bool bCanUse = (ItemData.bIsConsumable || ItemData.ItemType == EItemType::Tool);
        Btn_Use->SetIsEnabled(bCanUse);

        // Update button text based on item type
        if (UTextBlock* UseButtonText = Cast<UTextBlock>(Btn_Use->GetChildAt(0)))
        {
            if (ItemData.ItemType == EItemType::Tool)
            {
                UseButtonText->SetText(FText::FromString(TEXT("Equip")));
            }
            else if (ItemData.bIsConsumable)
            {
                UseButtonText->SetText(FText::FromString(TEXT("Use")));
            }
            else
            {
                UseButtonText->SetText(FText::FromString(TEXT("Examine")));
            }
        }
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
    bShowAllItems = true;
    RefreshInventory();
}

void UInventoryWidget::OnFilterConsumablesClicked()
{
    bShowAllItems = false;
    FilterByType(EItemType::Consumable);
}

void UInventoryWidget::OnFilterToolsClicked()
{
    bShowAllItems = false;
    FilterByType(EItemType::Tool);
}

void UInventoryWidget::OnFilterDocumentsClicked()
{
    bShowAllItems = false;
    FilterByType(EItemType::Document);
}

void UInventoryWidget::OnInventoryUpdated()
{
    RefreshInventory();
    RefreshQuickbar();

    // If we had an item selected, refresh its details
    if (SelectedSlotIndex >= 0)
    {
        ShowItemDetails(SelectedSlotIndex);
    }
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