// InventorySlotWidget.cpp - Enhanced Implementation
#include "InventorySlotWidget.h"
#include "InventoryWidget.h"
#include "ItemDragDrop.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"

void UInventorySlotWidget::NativeConstruct()
{
    Super::NativeConstruct();
    UpdateVisuals();
}

void UInventorySlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Update cooldown visuals in real-time
    if (!bIsEmpty && SlotData.CooldownRemaining > 0.0f)
    {
        UpdateCooldownVisuals();
    }
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
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
                ParentInventoryWidget->InventoryComponent->UseItem(SlotData.ItemID);
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

    if (!bIsSelected && SlotBorder)
    {
        SlotBorder->SetBrushColor(HoverColor);
    }
}

void UInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    bIsHovered = false;

    if (!bIsSelected)
    {
        UpdateVisuals();
    }
}

FReply UInventorySlotWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    return FReply::Unhandled();
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    if (bIsEmpty)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Drag detected on slot %d (%s)"),
        SlotIndex, bIsQuickbarSlot ? TEXT("Quickbar") : TEXT("Inventory"));

    UItemDragDrop* DragDrop = NewObject<UItemDragDrop>();
    if (DragDrop)
    {
        DragDrop->SourceSlot = this;
        DragDrop->SlotIndex = SlotIndex;
        DragDrop->bIsQuickbarSlot = bIsQuickbarSlot;
        DragDrop->ItemID = SlotData.ItemID;

        // Set visual
        DragDrop->DefaultDragVisual = this;
        DragDrop->Pivot = EDragPivot::CenterCenter;
        DragDrop->Offset = FVector2D(0, 0);

        OutOperation = DragDrop;

        UE_LOG(LogTemp, Log, TEXT("Created drag operation for item: %s"), *SlotData.ItemID.ToString());
    }
}

void UInventorySlotWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

    if (!InOperation)
    {
        return;
    }

    UItemDragDrop* ItemDragDrop = Cast<UItemDragDrop>(InOperation);
    if (ItemDragDrop && ItemDragDrop->SourceSlot && ItemDragDrop->SourceSlot != this)
    {
        bIsDragHovered = true;

        if (SlotBorder)
        {
            SlotBorder->SetBrushColor(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f));
        }

        UE_LOG(LogTemp, Verbose, TEXT("Drag entered slot %d"), SlotIndex);
    }
}

void UInventorySlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragLeave(InDragDropEvent, InOperation);

    bIsDragHovered = false;
    UpdateVisuals();
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    bIsDragHovered = false;
    UpdateVisuals();

    if (!InOperation)
    {
        UE_LOG(LogTemp, Warning, TEXT("Drop failed: No operation"));
        return false;
    }

    // Cast to ItemDragDrop
    UItemDragDrop* ItemDragDrop = Cast<UItemDragDrop>(InOperation);
    if (!ItemDragDrop || !ItemDragDrop->SourceSlot)
    {
        UE_LOG(LogTemp, Warning, TEXT("Drop failed: Invalid drag operation"));
        return false;
    }

    UInventorySlotWidget* SourceSlot = ItemDragDrop->SourceSlot;

    // Không cho drop vào chính nó
    if (SourceSlot == this)
    {
        UE_LOG(LogTemp, Verbose, TEXT("Drop cancelled: Self-drop"));
        return false;
    }

    if (!ParentInventoryWidget || !ParentInventoryWidget->InventoryComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Drop failed: No parent widget or inventory component"));
        return false;
    }

    UInventoryComponent* Inventory = ParentInventoryWidget->InventoryComponent;

    UE_LOG(LogTemp, Log, TEXT("Drop: Source=[%d,%s] Target=[%d,%s] Item='%s'"),
        SourceSlot->SlotIndex, SourceSlot->bIsQuickbarSlot ? TEXT("QB") : TEXT("INV"),
        SlotIndex, bIsQuickbarSlot ? TEXT("QB") : TEXT("INV"),
        *ItemDragDrop->ItemID.ToString());

    bool bSuccess = false;

    // ===== CASE 1: Inventory -> Inventory =====
    if (!SourceSlot->bIsQuickbarSlot && !bIsQuickbarSlot)
    {
        // Nếu drop vào slot rỗng -> Move
        // Nếu drop vào slot có item -> Swap
        bSuccess = Inventory->SwapInventorySlots(SourceSlot->SlotIndex, SlotIndex);
        UE_LOG(LogTemp, Log, TEXT("  -> Inventory SWAP"));
    }
    // ===== CASE 2: Quickbar -> Quickbar =====
    else if (SourceSlot->bIsQuickbarSlot && bIsQuickbarSlot)
    {
        bSuccess = Inventory->SwapQuickbarSlots(SourceSlot->SlotIndex, SlotIndex);
        UE_LOG(LogTemp, Log, TEXT("  -> Quickbar SWAP"));
    }
    // ===== CASE 3: Inventory -> Quickbar =====
    else if (!SourceSlot->bIsQuickbarSlot && bIsQuickbarSlot)
    {
        // Move/Swap item từ inventory sang quickbar
        bSuccess = Inventory->MoveInventoryToQuickbar(SourceSlot->SlotIndex, SlotIndex);
        UE_LOG(LogTemp, Log, TEXT("  -> Inventory TO Quickbar"));
    }
    // ===== CASE 4: Quickbar -> Inventory =====
    else if (SourceSlot->bIsQuickbarSlot && !bIsQuickbarSlot)
    {
        // Move/Swap item từ quickbar sang inventory
        bSuccess = Inventory->MoveQuickbarToInventory(SourceSlot->SlotIndex, SlotIndex);
        UE_LOG(LogTemp, Log, TEXT("  -> Quickbar TO Inventory"));
    }

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Drop successful!"));

        // TODO: Play sound effect
        // if (DropSound)
        // {
        //     UGameplayStatics::PlaySound2D(this, DropSound);
        // }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✗ Drop operation failed"));
    }

    return bSuccess;
}

void UInventorySlotWidget::UpdateSlot(const FInventorySlot& NewSlotData)
{
    SlotData = NewSlotData;
    bIsEmpty = !SlotData.IsValid();

    // Get item data from inventory component
    if (!bIsEmpty && ParentInventoryWidget && ParentInventoryWidget->InventoryComponent)
    {
        if (!ParentInventoryWidget->InventoryComponent->GetItemData(SlotData.ItemID, ItemData))
        {
            bIsEmpty = true;
        }

        // Check if this item is equipped
        if (bIsQuickbarSlot && ParentInventoryWidget->InventoryComponent->GetCurrentEquippedItemID() == SlotData.ItemID)
        {
            bIsEquipped = true;
        }
        else
        {
            bIsEquipped = false;
        }
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

        if (DurabilityBar)
        {
            DurabilityBar->SetVisibility(ESlateVisibility::Hidden);
        }

        if (CooldownOverlay)
        {
            CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
        }

        if (CooldownBar)
        {
            CooldownBar->SetVisibility(ESlateVisibility::Hidden);
        }

        if (SlotBorder)
        {
            SlotBorder->SetBrushColor(EmptyColor);
        }
    }
    else
    {
        // Has item - update icon
        if (ItemIcon && ItemData.Icon)
        {
            ItemIcon->SetBrushFromTexture(ItemData.Icon);
            ItemIcon->SetVisibility(ESlateVisibility::Visible);
        }

        // Update quantity
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

        // Update border color based on state
        if (SlotBorder)
        {
            FLinearColor BorderColor;

            if (bIsEquipped)
            {
                BorderColor = EquippedColor;
            }
            else if (SlotData.CooldownRemaining > 0.0f)
            {
                BorderColor = OnCooldownColor;
            }
            else if (bIsSelected)
            {
                BorderColor = SelectedColor;
            }
            else if (bIsHovered || bIsDragHovered)
            {
                BorderColor = HoverColor;
            }
            else
            {
                BorderColor = NormalColor;
            }

            SlotBorder->SetBrushColor(BorderColor);
        }

        // Update durability bar
        UpdateDurabilityBar();

        // Update cooldown visuals
        UpdateCooldownVisuals();
    }

    // Update hotkey display (quickbar only)
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

void UInventorySlotWidget::UpdateDurabilityBar()
{
    if (!DurabilityBar || bIsEmpty)
    {
        if (DurabilityBar)
        {
            DurabilityBar->SetVisibility(ESlateVisibility::Hidden);
        }
        return;
    }

    // Only show durability bar for items with durability
    if (ItemData.bHasDurability && ItemData.MaxUses > 0)
    {
        float DurabilityPercent = (float)SlotData.RemainingUses / (float)ItemData.MaxUses;
        DurabilityBar->SetPercent(DurabilityPercent);

        // Color code: Green > Yellow > Red
        FLinearColor BarColor;
        if (DurabilityPercent > 0.5f)
        {
            BarColor = FLinearColor::Green;
        }
        else if (DurabilityPercent > 0.25f)
        {
            BarColor = FLinearColor::Yellow;
        }
        else
        {
            BarColor = FLinearColor::Red;
        }

        DurabilityBar->SetFillColorAndOpacity(BarColor);
        DurabilityBar->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    else
    {
        DurabilityBar->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UInventorySlotWidget::UpdateCooldownVisuals()
{
    if (bIsEmpty || SlotData.CooldownRemaining <= 0.0f)
    {
        if (CooldownOverlay)
        {
            CooldownOverlay->SetVisibility(ESlateVisibility::Hidden);
        }
        if (CooldownBar)
        {
            CooldownBar->SetVisibility(ESlateVisibility::Hidden);
        }
        return;
    }

    // Get max cooldown from item data
    float MaxCooldown = ItemData.UsageCooldown;
    if (MaxCooldown <= 0.0f)
    {
        return;
    }

    float CooldownPercent = SlotData.CooldownRemaining / MaxCooldown;

    // Show cooldown overlay
    if (CooldownOverlay)
    {
        CooldownOverlay->SetOpacity(CooldownPercent * 0.7f);
        CooldownOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
    }

    // Show cooldown bar
    if (CooldownBar)
    {
        CooldownBar->SetPercent(CooldownPercent);
        CooldownBar->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
}

void UInventorySlotWidget::SetSelected(bool bSelected)
{
    bIsSelected = bSelected;
    UpdateVisuals();
}

void UInventorySlotWidget::OnSlotClicked()
{
    if (bIsEmpty)
    {
        UE_LOG(LogTemp, Log, TEXT("Clicked empty slot %d"), SlotIndex);

        if (ParentInventoryWidget)
        {
            ParentInventoryWidget->HideItemDetails();
        }
        return;
    }

    if (ParentInventoryWidget)
    {
        ParentInventoryWidget->ClearSelection();
        ParentInventoryWidget->ShowItemDetails(SlotIndex);
        SetSelected(true);
    }
}