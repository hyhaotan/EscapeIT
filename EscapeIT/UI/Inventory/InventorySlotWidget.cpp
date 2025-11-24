// InventorySlotWidget.cpp - IMPROVED DRAG & DROP

#include "InventorySlotWidget.h"
#include "InventoryWidget.h"
#include "ItemDragDrop.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UInventorySlotWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Initial state
    UpdateSlot(FInventorySlot());
}

void UInventorySlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
}

void UInventorySlotWidget::UpdateSlot(const FInventorySlot& SlotData)
{
    CachedSlotData = SlotData;
    
    // Check if slot is empty
    bIsEmpty = !SlotData.IsValid();
    
    if (bIsEmpty)
    {
        // EMPTY SLOT
        if (ItemIcon)
        {
            ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
        }
        
        if (QuantityText)
        {
            QuantityText->SetVisibility(ESlateVisibility::Collapsed);
        }
        
        // Set empty color
        if (SlotBorder)
        {
            SlotBorder->SetBrushColor(EmptyColor);
        }
    }
    else
    {
        // HAS ITEM - Get item data
        if (ParentInventoryWidget && ParentInventoryWidget->InventoryComponent)
        {
            FItemData ItemData;
            if (ParentInventoryWidget->InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
            {
                // Update icon
                if (ItemIcon && ItemData.Icon)
                {
                    ItemIcon->SetBrushFromTexture(ItemData.Icon);
                    ItemIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
                }
                
                // Update quantity (only if > 1)
                if (QuantityText)
                {
                    if (SlotData.Quantity > 1)
                    {
                        QuantityText->SetText(FText::AsNumber(SlotData.Quantity));
                        QuantityText->SetVisibility(ESlateVisibility::HitTestInvisible);
                    }
                    else
                    {
                        QuantityText->SetVisibility(ESlateVisibility::Collapsed);
                    }
                }
            }
        }
    }
    
    // Update hotkey display for quickbar slots
    if (bIsQuickbarSlot && HotkeyText)
    {
        HotkeyText->SetText(FText::AsNumber(SlotIndex + 1));
        HotkeyText->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    
    // Update visual state
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
    
    FLinearColor BorderColor = NormalColor;
    
    // Priority: Dragging > Drop Feedback > Equipped > Selected > Hovered > Normal
    if (bIsDragging)
    {
        BorderColor = DraggingColor;
    }
    else if (bIsValidDropTarget)
    {
        BorderColor = ValidDropColor;
    }
    else if (bIsInvalidDropTarget)
    {
        BorderColor = InvalidDropColor;
    }
    else if (bIsEquipped)
    {
        BorderColor = EquippedColor;
    }
    else if (bIsSelected)
    {
        BorderColor = SelectedColor;
    }
    else if (bIsHovered)
    {
        BorderColor = HighlightColor;
    }
    else if (bIsEmpty)
    {
        BorderColor = EmptyColor;
    }
    
    SlotBorder->SetBrushColor(BorderColor);
}

void UInventorySlotWidget::OnSlotClicked()
{
    if (ParentInventoryWidget)
    {
        if (bIsQuickbarSlot)
        {
            // Quickbar slot clicked - equip item
            if (ParentInventoryWidget->InventoryComponent && !bIsEmpty)
            {
                ParentInventoryWidget->InventoryComponent->EquipQuickbarSlot(SlotIndex);
            }
        }
        else
        {
            // Inventory slot clicked - show details
            ParentInventoryWidget->ShowItemDetails(SlotIndex);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("OnSlotClicked: Slot %d, Quickbar=%s, Empty=%s"),
        SlotIndex, bIsQuickbarSlot ? TEXT("Yes") : TEXT("No"), bIsEmpty ? TEXT("Yes") : TEXT("No"));
}

// ============================================================================
// ✅ DRAG & DROP IMPLEMENTATION
// ============================================================================

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        // Click event
        OnSlotClicked();
        
        // Only allow drag if slot has item
        if (!bIsEmpty)
        {
            return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
        }
        
        return FReply::Handled();
    }
    else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        // Right-click to quick use/equip
        if (!bIsEmpty && ParentInventoryWidget && ParentInventoryWidget->InventoryComponent)
        {
            if (bIsQuickbarSlot)
            {
                ParentInventoryWidget->InventoryComponent->EquipQuickbarSlot(SlotIndex);
            }
            else
            {
                ParentInventoryWidget->InventoryComponent->UseItem(CachedSlotData.ItemID);
            }
        }
        return FReply::Handled();
    }
    
    return FReply::Unhandled();
}

void UInventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    
    bIsHovered = true;
    UpdateVisuals();
    
    // Show tooltip if item exists
    if (!bIsEmpty && ParentInventoryWidget && ParentInventoryWidget->InventoryComponent)
    {
        FItemData ItemData;
        if (ParentInventoryWidget->InventoryComponent->GetItemData(CachedSlotData.ItemID, ItemData))
        {
            // You can show tooltip here
            // ShowTooltip(ItemData);
        }
    }
}

void UInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
    
    bIsHovered = false;
    UpdateVisuals();
    
    // Hide tooltip
    // HideTooltip();
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
    
    if (bIsEmpty || !ParentInventoryWidget)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("🎯 DragDetected: Slot %d (%s) - Item: %s, Qty: %d"), 
        SlotIndex, 
        bIsQuickbarSlot ? TEXT("Quickbar") : TEXT("Inventory"),
        *CachedSlotData.ItemID.ToString(),
        CachedSlotData.Quantity);
    
    // Create drag operation
    UItemDragDrop* DragOp = NewObject<UItemDragDrop>();
    DragOp->SourceSlot = this;
    DragOp->SlotIndex = SlotIndex;
    DragOp->bIsQuickbarSlot = bIsQuickbarSlot;
    DragOp->ItemID = CachedSlotData.ItemID;
    DragOp->Quantity = CachedSlotData.Quantity;
    
    // Create visual widget for dragging
    UInventorySlotWidget* DragVisual = CreateWidget<UInventorySlotWidget>(this, GetClass());
    if (DragVisual)
    {
        DragVisual->UpdateSlot(CachedSlotData);
        DragVisual->SetRenderOpacity(0.7f);
        DragOp->DefaultDragVisual = DragVisual;
    }
    
    DragOp->Pivot = EDragPivot::CenterCenter;
    
    // Update visual state
    bIsDragging = true;
    UpdateVisuals();
    
    OutOperation = DragOp;
}

void UInventorySlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
    
    UItemDragDrop* DragOp = Cast<UItemDragDrop>(InOperation);
    if (DragOp && DragOp->SourceSlot)
    {
        DragOp->SourceSlot->bIsDragging = false;
        DragOp->SourceSlot->UpdateVisuals();
    }
    
    UE_LOG(LogTemp, Log, TEXT("❌ Drag Cancelled"));
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
    
    UItemDragDrop* DragOp = Cast<UItemDragDrop>(InOperation);
    if (!DragOp)
    {
        ClearDropFeedback();
        return false;
    }
    
    bool bSuccess = false;
    
    if (CanAcceptDrop(DragOp))
    {
        ExecuteDrop(DragOp);
        bSuccess = true;
    }
    
    // Clear drag state
    if (DragOp->SourceSlot)
    {
        DragOp->SourceSlot->bIsDragging = false;
        DragOp->SourceSlot->UpdateVisuals();
    }
    
    ClearDropFeedback();
    
    UE_LOG(LogTemp, Log, TEXT("📦 OnDrop: Success=%s"), bSuccess ? TEXT("Yes") : TEXT("No"));
    
    return bSuccess;
}

void UInventorySlotWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
    
    UItemDragDrop* DragOp = Cast<UItemDragDrop>(InOperation);
    if (DragOp && DragOp->SourceSlot != this)
    {
        bool bIsValid = CanAcceptDrop(DragOp);
        ShowDropFeedback(bIsValid);
        
        UE_LOG(LogTemp, Verbose, TEXT("➡️ DragEnter: Slot %d - Valid=%s"), SlotIndex, bIsValid ? TEXT("Yes") : TEXT("No"));
    }
}

void UInventorySlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragLeave(InDragDropEvent, InOperation);
    ClearDropFeedback();
    
    UE_LOG(LogTemp, Verbose, TEXT("⬅️ DragLeave: Slot %d"), SlotIndex);
}

// ============================================================================
// ✅ DRAG & DROP LOGIC
// ============================================================================

bool UInventorySlotWidget::CanAcceptDrop(UItemDragDrop* DragOp)
{
    if (!DragOp || !DragOp->SourceSlot || !ParentInventoryWidget || !ParentInventoryWidget->InventoryComponent)
    {
        return false;
    }
    
    // Can't drop on self
    if (DragOp->SourceSlot == this)
    {
        return false;
    }
    
    UInventoryComponent* InvComp = ParentInventoryWidget->InventoryComponent;
    
    // Get item data
    FItemData ItemData;
    if (!InvComp->GetItemData(DragOp->ItemID, ItemData))
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠️ CanAcceptDrop: Item data not found for %s"), *DragOp->ItemID.ToString());
        return false;
    }
    
    // ========================================================================
    // CASE 1: Quickbar → Quickbar (ALWAYS ALLOW SWAP)
    // ========================================================================
    if (DragOp->bIsQuickbarSlot && bIsQuickbarSlot)
    {
        UE_LOG(LogTemp, Verbose, TEXT("✅ Quickbar → Quickbar: Allowed"));
        return true;
    }
    
    // ========================================================================
    // CASE 2: Quickbar → Inventory (ALLOW IF TARGET IS EMPTY)
    // ========================================================================
    if (DragOp->bIsQuickbarSlot && !bIsQuickbarSlot)
    {
        if (!bIsEmpty)
        {
            UE_LOG(LogTemp, Warning, TEXT("⚠️ Quickbar → Inventory: Target slot occupied"));
            return false;
        }
        
        UE_LOG(LogTemp, Verbose, TEXT("✅ Quickbar → Inventory: Allowed (removing from quickbar)"));
        return true;
    }
    
    // ========================================================================
    // CASE 3: Inventory → Quickbar (CHECK COMPATIBILITY)
    // ========================================================================
    if (!DragOp->bIsQuickbarSlot && bIsQuickbarSlot)
    {
        // Only allow Tools and Consumables in quickbar
        if (ItemData.ItemType != EItemType::Tool && ItemData.ItemType != EItemType::Consumable)
        {
            UE_LOG(LogTemp, Warning, TEXT("⚠️ Inventory → Quickbar: Only Tools/Consumables allowed (Type: %d)"), 
                static_cast<int32>(ItemData.ItemType));
            return false;
        }
        
        // Check if item already exists in another quickbar slot
        for (int32 i = 0; i < 4; i++)
        {
            if (i == SlotIndex) continue; // Skip current slot
            
            FInventorySlot QBSlot = InvComp->GetQuickbarSlot(i);
            if (QBSlot.IsValid() && QBSlot.ItemID == DragOp->ItemID)
            {
                UE_LOG(LogTemp, Warning, TEXT("⚠️ Inventory → Quickbar: Item already in Quickbar slot %d"), i);
                return false;
            }
        }
        
        UE_LOG(LogTemp, Verbose, TEXT("✅ Inventory → Quickbar: Allowed"));
        return true;
    }
    
    // ========================================================================
    // CASE 4: Inventory → Inventory (ALWAYS ALLOW)
    // ========================================================================
    if (!DragOp->bIsQuickbarSlot && !bIsQuickbarSlot)
    {
        UE_LOG(LogTemp, Verbose, TEXT("✅ Inventory → Inventory: Allowed"));
        return true;
    }
    
    return false;
}

void UInventorySlotWidget::ExecuteDrop(UItemDragDrop* DragOp)
{
    if (!DragOp || !DragOp->SourceSlot || !ParentInventoryWidget || !ParentInventoryWidget->InventoryComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ ExecuteDrop: Invalid parameters"));
        return;
    }
    
    UInventoryComponent* InvComp = ParentInventoryWidget->InventoryComponent;
    
    // ========================================================================
    // CASE 1: Quickbar → Quickbar (SWAP)
    // ========================================================================
    if (DragOp->bIsQuickbarSlot && bIsQuickbarSlot)
    {
        InvComp->SwapQuickbarSlots(DragOp->SlotIndex, SlotIndex);
        
        UE_LOG(LogTemp, Log, TEXT("✅ Swapped Quickbar slots %d ↔ %d"), 
            DragOp->SlotIndex, SlotIndex);
        return;
    }
    
    // ========================================================================
    // CASE 2: Quickbar → Inventory (REMOVE FROM QUICKBAR)
    // ========================================================================
    if (DragOp->bIsQuickbarSlot && !bIsQuickbarSlot)
    {
        if (!bIsEmpty)
        {
            UE_LOG(LogTemp, Error, TEXT("❌ ExecuteDrop: Target inventory slot is not empty!"));
            return;
        }
        
        InvComp->RemoveFromQuickbar(DragOp->SlotIndex);
        
        UE_LOG(LogTemp, Log, TEXT("✅ Removed '%s' from Quickbar slot %d"), 
            *DragOp->ItemID.ToString(), DragOp->SlotIndex);
        return;
    }
    
    // ========================================================================
    // CASE 3: Inventory → Quickbar (ADD/REPLACE)
    // ========================================================================
    if (!DragOp->bIsQuickbarSlot && bIsQuickbarSlot)
    {
        if (bIsEmpty)
        {
            // Empty quickbar slot - just add
            InvComp->AssignToQuickbar(DragOp->ItemID, SlotIndex);
            
            UE_LOG(LogTemp, Log, TEXT("✅ Added '%s' to Quickbar slot %d"), 
                *DragOp->ItemID.ToString(), SlotIndex);
        }
        else
        {
            // Occupied quickbar slot - replace
            FName OldItemID = CachedSlotData.ItemID;
            InvComp->AssignToQuickbar(DragOp->ItemID, SlotIndex);
            
            UE_LOG(LogTemp, Log, TEXT("✅ Replaced Quickbar slot %d: '%s' → '%s'"), 
                SlotIndex, *OldItemID.ToString(), *DragOp->ItemID.ToString());
        }
        return;
    }
    
    // ========================================================================
    // CASE 4: Inventory → Inventory (MOVE/SWAP)
    // ========================================================================
    if (!DragOp->bIsQuickbarSlot && !bIsQuickbarSlot)
    {
        if (bIsEmpty)
        {
            // Empty target - just move
            InvComp->MoveItemToSlot(DragOp->SlotIndex, SlotIndex);
            
            UE_LOG(LogTemp, Log, TEXT("✅ Moved item from slot %d → %d"), 
                DragOp->SlotIndex, SlotIndex);
        }
        else
        {
            // Both occupied - swap
            InvComp->SwapInventorySlots(DragOp->SlotIndex, SlotIndex);
            
            UE_LOG(LogTemp, Log, TEXT("✅ Swapped Inventory slots %d ↔ %d"), 
                DragOp->SlotIndex, SlotIndex);
        }
        return;
    }
    
    UE_LOG(LogTemp, Error, TEXT("❌ ExecuteDrop: Unhandled case!"));
}

void UInventorySlotWidget::ShowDropFeedback(bool bIsValid)
{
    if (bIsValid)
    {
        bIsValidDropTarget = true;
        bIsInvalidDropTarget = false;
    }
    else
    {
        bIsValidDropTarget = false;
        bIsInvalidDropTarget = true;
    }
    
    UpdateVisuals();
}

void UInventorySlotWidget::ClearDropFeedback()
{
    bIsValidDropTarget = false;
    bIsInvalidDropTarget = false;
    UpdateVisuals();
}