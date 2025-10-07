
#include "InventorySlotWidget.h"
#include "InventoryWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "Blueprint/DragDropOperation.h"

void UInventorySlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UpdateVisuals();
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        ClickCount++;

        if (ClickCount == 1)
        {
            // Single click
            GetWorld()->GetTimerManager().SetTimer(DoubleClickTimerHandle, [this]()
                {
                    if (ClickCount == 1)
                    {
                        OnSlotClicked();
                    }
                    ClickCount = 0;
                }, 0.3f, false);
        }
        else if (ClickCount == 2)
        {
            // Double click
            GetWorld()->GetTimerManager().ClearTimer(DoubleClickTimerHandle);
            OnSlotDoubleClicked();
            ClickCount = 0;
        }

        return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
    }
    else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        // Right click = quick use/equip
        OnSlotDoubleClicked();
        return FReply::Handled();
    }

    return FReply::Unhandled();
}

void UInventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    bIsHovered = true;

    if (!bIsSelected)
    {
        if (SlotBorder)
        {
            SlotBorder->SetBrushColor(HoverColor);
        }
    }

    ShowTooltip();
}

void UInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    bIsHovered = false;

    if (!bIsSelected)
    {
        UpdateVisuals();
    }

    HideTooltip();
}

FReply UInventorySlotWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    return FReply::Handled();
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    if (bIsEmpty)
    {
        return;
    }

    // Create drag drop operation
    UDragDropOperation* DragDrop = NewObject<UDragDropOperation>();
    if (DragDrop)
    {
        DragDrop->Payload = this;
        DragDrop->DefaultDragVisual = this;
        DragDrop->Pivot = EDragPivot::CenterCenter;

        OutOperation = DragDrop;
    }
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    if (!InOperation || !InOperation->Payload)
    {
        return false;
    }

    UInventorySlotWidget* SourceSlot = Cast<UInventorySlotWidget>(InOperation->Payload);
    if (!SourceSlot || SourceSlot == this)
    {
        return false;
    }

    // Get inventory component from parent
    if (!ParentInventoryWidget || !ParentInventoryWidget->InventoryComponent)
    {
        return false;
    }

    UInventoryComponent* Inventory = ParentInventoryWidget->InventoryComponent;

    // Swap items
    if (SourceSlot->SlotIndex >= 0 && SlotIndex >= 0)
    {
        // Check if both are in main inventory or both in quickbar
        if (SourceSlot->bIsQuickbarSlot == bIsQuickbarSlot)
        {
            TArray<FInventorySlot>& TargetArray = bIsQuickbarSlot ?
                Inventory->QuickbarSlots : Inventory->InventorySlots;

            if (SourceSlot->SlotIndex < TargetArray.Num() && SlotIndex < TargetArray.Num())
            {
                // Swap
                FInventorySlot Temp = TargetArray[SourceSlot->SlotIndex];
                TargetArray[SourceSlot->SlotIndex] = TargetArray[SlotIndex];
                TargetArray[SlotIndex] = Temp;

                Inventory->OnInventoryUpdated.Broadcast();
            }
        }
        // From inventory to quickbar
        else if (!SourceSlot->bIsQuickbarSlot && bIsQuickbarSlot)
        {
            Inventory->AssignToQuickbar(SourceSlot->SlotData.ItemID, SlotIndex);
        }
    }

    return true;
}

// ============================================
// UPDATE FUNCTIONS
// ============================================
void UInventorySlotWidget::UpdateSlot(const FInventorySlot& NewSlotData)
{
    SlotData = NewSlotData;
    bIsEmpty = !SlotData.IsValid();

    if (!bIsEmpty && ParentInventoryWidget && ParentInventoryWidget->InventoryComponent)
    {
        ParentInventoryWidget->InventoryComponent->GetItemData(SlotData.ItemID, ItemData);
    }

    UpdateVisuals();
}

void UInventorySlotWidget::UpdateVisuals()
{
    if (bIsEmpty)
    {
        // Empty slot
        if (ItemIcon)
        {
            ItemIcon->SetVisibility(ESlateVisibility::Hidden);
        }

        if (QuantityText)
        {
            QuantityText->SetVisibility(ESlateVisibility::Hidden);
        }

        if (QualityBorder)
        {
            QualityBorder->SetVisibility(ESlateVisibility::Hidden);
        }

        if (SlotBorder)
        {
            SlotBorder->SetBrushColor(EmptyColor);
        }
    }
    else
    {
        // Has item
        if (ItemIcon && ItemData.Icon)
        {
            ItemIcon->SetBrushFromTexture(ItemData.Icon);
            ItemIcon->SetVisibility(ESlateVisibility::Visible);
        }

        // Quantity
        if (QuantityText)
        {
            if (SlotData.Quantity > 1)
            {
                QuantityText->SetText(FText::AsNumber(SlotData.Quantity));
                QuantityText->SetVisibility(ESlateVisibility::Visible);
            }
            else
            {
                QuantityText->SetVisibility(ESlateVisibility::Hidden);
            }
        }

        // Rarity border
        if (QualityBorder)
        {
            QualityBorder->SetColorAndOpacity(GetRarityColor(ItemData.Rarity));
            QualityBorder->SetVisibility(ESlateVisibility::Visible);
        }

        // Background color
        if (SlotBorder)
        {
            if (bIsSelected)
            {
                SlotBorder->SetBrushColor(SelectedColor);
            }
            else if (bIsHovered)
            {
                SlotBorder->SetBrushColor(HoverColor);
            }
            else
            {
                SlotBorder->SetBrushColor(NormalColor);
            }
        }
    }

    // Hotkey text (only for quickbar)
    if (HotkeyText)
    {
        if (bIsQuickbarSlot)
        {
            HotkeyText->SetText(FText::AsNumber(SlotIndex + 1));
            HotkeyText->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            HotkeyText->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void UInventorySlotWidget::SetSelected(bool bSelected)
{
    bIsSelected = bSelected;
    UpdateVisuals();
}

// ============================================
// CLICK HANDLERS
// ============================================
void UInventorySlotWidget::OnSlotClicked()
{
    if (bIsEmpty)
    {
        return;
    }

    if (ParentInventoryWidget)
    {
        ParentInventoryWidget->ShowItemDetails(SlotIndex);
        SetSelected(true);
    }
}

void UInventorySlotWidget::OnSlotDoubleClicked()
{
    if (bIsEmpty || !ParentInventoryWidget || !ParentInventoryWidget->InventoryComponent)
    {
        return;
    }

    UInventoryComponent* Inventory = ParentInventoryWidget->InventoryComponent;

    if (bIsQuickbarSlot)
    {
        // Use quickbar item
        Inventory->UseQuickbarSlot(SlotIndex);
    }
    else
    {
        // Use item from main inventory
        Inventory->UseItem(SlotData.ItemID);
    }
}

// ============================================
// TOOLTIP
// ============================================
void UInventorySlotWidget::ShowTooltip()
{
    if (bIsEmpty)
    {
        return;
    }

    // TODO: Create and show tooltip widget
    // This would typically create a floating widget with item details
    /*
    if (!TooltipWidget && TooltipWidgetClass)
    {
        TooltipWidget = CreateWidget<UItemTooltipWidget>(GetOwningPlayer(), TooltipWidgetClass);
        if (TooltipWidget)
        {
            TooltipWidget->SetItemData(ItemData, SlotData);
            TooltipWidget->AddToViewport(100); // High Z-order
        }
    }
    */
}

void UInventorySlotWidget::HideTooltip()
{
    // TODO: Hide tooltip widget
    /*
    if (TooltipWidget)
    {
        TooltipWidget->RemoveFromParent();
        TooltipWidget = nullptr;
    }
    */
}

// ============================================
// HELPER FUNCTIONS
// ============================================
FLinearColor UInventorySlotWidget::GetRarityColor(EItemRarity Rarity) const
{
    switch (Rarity)
    {
    case EItemRarity::Common:
        return CommonColor;
    case EItemRarity::Uncommon:
        return UncommonColor;
    case EItemRarity::Rare:
        return RareColor;
    case EItemRarity::Unique:
        return UniqueColor;
    default:
        return FLinearColor::White;
    }
}