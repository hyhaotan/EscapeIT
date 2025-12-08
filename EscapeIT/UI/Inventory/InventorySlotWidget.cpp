// InventorySlotWidget.cpp - IMPROVED DRAG & DROP

#include "InventorySlotWidget.h"
#include "InventoryWidget.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "EscapeIT/UI/Inventory/ItemDragDrop.h"

void UInventorySlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Store original border color
    if (SlotBorder)
    {
        OriginalBorderColor = SlotBorder->GetBrushColor();
    }

    UpdateVisuals();
}

void UInventorySlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

// ============================================================================
// UPDATE FUNCTIONS
// ============================================================================

void UInventorySlotWidget::UpdateSlot(const FInventorySlot& SlotData)
{
    CachedSlotData = SlotData;
    bIsEmpty = !SlotData.IsValid();

    // Update icon
    if (ItemIcon)
    {
        if (bIsEmpty)
        {
            ItemIcon->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            // Get item data to display icon
            if (InventoryComponentRef)
            {
                FItemData ItemData;
                if (InventoryComponentRef->GetItemData(SlotData.ItemID, ItemData))
                {
                    if (ItemData.Icon)
                    {
                        ItemIcon->SetBrushFromTexture(ItemData.Icon);
                        ItemIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
                    }
                }
            }
        }
    }

    // Update quantity text
    if (QuantityText)
    {
        if (bIsEmpty || SlotData.Quantity <= 1)
        {
            QuantityText->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            QuantityText->SetText(FText::AsNumber(SlotData.Quantity));
            QuantityText->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
    }

    UpdateVisuals();
}

void UInventorySlotWidget::SetSelected(bool bSelected)
{
    bIsSelected = bSelected;
    UpdateVisuals();
}

void UInventorySlotWidget::SetEquipped(bool bEquipped)
{
    bIsEquipped = bEquipped;
    UpdateVisuals();
}

void UInventorySlotWidget::SetHighlight(bool bHighlight)
{
    bIsHovered = bHighlight;
    UpdateVisuals();
}

void UInventorySlotWidget::UpdateVisuals()
{
    if (!SlotBorder)
    {
        return;
    }

    FLinearColor BorderColor;

    // ✅ Priority order for visual states
    if (bIsValidDropTarget)
    {
        BorderColor = ValidDropColor;
    }
    else if (bIsInvalidDropTarget)
    {
        BorderColor = InvalidDropColor;
    }
    else if (bIsDragging)
    {
        BorderColor = DraggingColor;
    }
    else if (bIsEquipped)
    {
        BorderColor = EquippedColor;
    }
    else if (bIsSelected)
    {
        BorderColor = SelectedColor;
    }
    else if (bIsHovered && !bIsEmpty)
    {
        BorderColor = HighlightColor;
    }
    else if (bIsEmpty)
    {
        BorderColor = EmptyColor;
    }
    else
    {
        BorderColor = NormalColor;
    }

    SlotBorder->SetBrushColor(BorderColor);
}

// ============================================================================
// MOUSE EVENTS
// ============================================================================

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (!bIsEmpty)
        {
            // Detect drag
            return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
        }
        else
        {
            // Click on empty slot
            if (ParentInventoryWidget)
            {
                ParentInventoryWidget->HideItemDetails();
            }
        }
    }
    else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        // Right-click: Show details or quick-use
        if (!bIsEmpty && ParentInventoryWidget)
        {
            ParentInventoryWidget->ShowItemDetails(SlotIndex);
        }
    }

    return FReply::Handled();
}

void UInventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    
    bIsHovered = true;
    UpdateVisuals();
}

void UInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
    
    bIsHovered = false;
    UpdateVisuals();
}

// ============================================================================
// DRAG & DROP
// ============================================================================

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    if (bIsEmpty || !InventoryComponentRef)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("✅ Drag detected: Slot %d (%s)"), 
        SlotIndex, bIsQuickbarSlot ? TEXT("Quickbar") : TEXT("Inventory"));

    // Create drag operation
    UItemDragDrop* DragOp = NewObject<UItemDragDrop>();
    DragOp->SourceSlotIndex = SlotIndex;
    DragOp->bIsFromQuickbar = bIsQuickbarSlot;
    DragOp->DraggedItemData = CachedSlotData;
    DragOp->DefaultDragVisual = this;
    DragOp->Pivot = EDragPivot::CenterCenter;

    bIsDragging = true;
    UpdateVisuals();

    OutOperation = DragOp;
}

void UInventorySlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UE_LOG(LogTemp, Log, TEXT("  → Drag cancelled"));
    
    bIsDragging = false;
    UpdateVisuals();
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UItemDragDrop* DragOp = Cast<UItemDragDrop>(InOperation);
    if (!DragOp || !InventoryComponentRef)
    {
        ClearDropFeedback();
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("✅ Drop: Source[%d] → Target[%d]"), 
        DragOp->SourceSlotIndex, SlotIndex);

    bool bSuccess = false;

    // ========================================================================
    // CASE 1: INVENTORY → INVENTORY
    // ========================================================================
    if (!DragOp->bIsFromQuickbar && !bIsQuickbarSlot)
    {
        if (bIsEmpty)
        {
            // Move to empty slot
            bSuccess = InventoryComponentRef->MoveItemToSlot(DragOp->SourceSlotIndex, SlotIndex);
        }
        else
        {
            // Swap with occupied slot
            bSuccess = InventoryComponentRef->SwapInventorySlots(DragOp->SourceSlotIndex, SlotIndex);
        }
    }
    // ========================================================================
    // CASE 2: QUICKBAR → QUICKBAR
    // ========================================================================
    else if (DragOp->bIsFromQuickbar && bIsQuickbarSlot)
    {
        bSuccess = InventoryComponentRef->SwapQuickbarSlots(DragOp->SourceSlotIndex, SlotIndex);
    }
    // ========================================================================
    // CASE 3: INVENTORY → QUICKBAR
    // ========================================================================
    else if (!DragOp->bIsFromQuickbar && bIsQuickbarSlot)
    {
        // Assign inventory item to quickbar
        bSuccess = InventoryComponentRef->MoveInventoryToQuickbar(DragOp->SourceSlotIndex, SlotIndex);
    }
    // ========================================================================
    // CASE 4: QUICKBAR → INVENTORY
    // ========================================================================
    else if (DragOp->bIsFromQuickbar && !bIsQuickbarSlot)
    {
        // Remove from quickbar (item stays in inventory)
        bSuccess = InventoryComponentRef->RemoveQuickbarToInventory(DragOp->SourceSlotIndex);
    }

    ClearDropFeedback();
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("  → Drop SUCCESS"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("  → Drop FAILED"));
    }

    return bSuccess;
}

void UInventorySlotWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UItemDragDrop* DragOp = Cast<UItemDragDrop>(InOperation);
    if (!DragOp)
    {
        return;
    }

    bool bCanDrop = CanAcceptDrop(DragOp);
    ShowDropFeedback(bCanDrop);
}

void UInventorySlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    ClearDropFeedback();
}

// ============================================================================
// DRAG & DROP HELPERS
// ============================================================================

bool UInventorySlotWidget::CanAcceptDrop(UItemDragDrop* DragOp)
{
    if (!DragOp || !InventoryComponentRef)
    {
        return false;
    }

    // ✅ Don't allow dropping on self
    if (DragOp->SourceSlotIndex == SlotIndex && DragOp->bIsFromQuickbar == bIsQuickbarSlot)
    {
        return false;
    }

    // ✅ INVENTORY → INVENTORY: Always allowed (swap or move)
    if (!DragOp->bIsFromQuickbar && !bIsQuickbarSlot)
    {
        return true;
    }

    // ✅ QUICKBAR → QUICKBAR: Always allowed (swap)
    if (DragOp->bIsFromQuickbar && bIsQuickbarSlot)
    {
        return true;
    }

    // ✅ INVENTORY → QUICKBAR: Check if item is tool/consumable
    if (!DragOp->bIsFromQuickbar && bIsQuickbarSlot)
    {
        FItemData ItemData;
        if (InventoryComponentRef->GetItemData(DragOp->DraggedItemData.ItemID, ItemData))
        {
            return (ItemData.ItemType == EItemType::Tool || ItemData.ItemType == EItemType::Consumable);
        }
        return false;
    }

    // ✅ QUICKBAR → INVENTORY: Always allowed (just removes from quickbar)
    if (DragOp->bIsFromQuickbar && !bIsQuickbarSlot)
    {
        return true;
    }

    return false;
}

void UInventorySlotWidget::ShowDropFeedback(bool bIsValid)
{
    bIsValidDropTarget = bIsValid;
    bIsInvalidDropTarget = !bIsValid;
    UpdateVisuals();
}

void UInventorySlotWidget::ClearDropFeedback()
{
    bIsValidDropTarget = false;
    bIsInvalidDropTarget = false;
    bIsDragging = false;
    UpdateVisuals();
}

