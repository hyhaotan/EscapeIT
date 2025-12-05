// InventorySlotWidget.h - IMPROVED DRAG & DROP SUPPORT

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/ItemData.h"
#include "InventorySlotWidget.generated.h"

class UInventoryWidget;
class UImage;
class UTextBlock;
class UButton;
class UBorder;
class UItemDragDrop;
class UInventoryComponent;

UCLASS()
class ESCAPEIT_API UInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ========================================================================
    // WIDGET REFERENCES
    // ========================================================================

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> SlotBorder;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ItemIcon;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> QuantityText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> HotkeyText;

    // ========================================================================
    // SLOT PROPERTIES
    // ========================================================================
    
    UPROPERTY(BlueprintReadOnly, Category = "Slot")
    int32 SlotIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Slot")
    bool bIsQuickbarSlot = false;

    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    TObjectPtr<UInventoryWidget> ParentInventoryWidget;
    
    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    TObjectPtr<UInventoryComponent> InventoryComponentRef;

    // ========================================================================
    // VISUAL COLORS
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor NormalColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor HighlightColor = FLinearColor(0.3f, 0.3f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor SelectedColor = FLinearColor(0.5f, 0.5f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor EquippedColor = FLinearColor(0.2f, 0.6f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor EmptyColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.5f);

    // ========================================================================
    // DRAG & DROP VISUAL FEEDBACK
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual|DragDrop")
    FLinearColor ValidDropColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual|DragDrop")
    FLinearColor InvalidDropColor = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual|DragDrop")
    FLinearColor DraggingColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.5f);

    // ========================================================================
    // CORE FUNCTIONS
    // ========================================================================
    
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UpdateSlot(const FInventorySlot& SlotData);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetSelected(bool bSelected);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetEquipped(bool bEquipped);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetHighlight(bool bHighlight);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UpdateVisuals();
    
    UFUNCTION(BlueprintPure, Category = "Inventory")
    FORCEINLINE FInventorySlot GetSlotData() const { return CachedSlotData; }

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FORCEINLINE bool IsEmpty() const { return bIsEmpty; }

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FORCEINLINE bool IsDragging() const { return bIsDragging; }

    // ========================================================================
    // DRAG & DROP INTERFACE
    // ========================================================================
    
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

protected:

    // ========================================================================
    // DRAG & DROP HELPERS
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "DragDrop")
    bool CanAcceptDrop(UItemDragDrop* DragOp);

    UFUNCTION(BlueprintCallable, Category = "DragDrop")
    void ShowDropFeedback(bool bIsValid);

    UFUNCTION(BlueprintCallable, Category = "DragDrop")
    void ClearDropFeedback();

private:
    // ========================================================================
    // CACHED DATA
    // ========================================================================
    
    FInventorySlot CachedSlotData;

    // ========================================================================
    // STATE FLAGS
    // ========================================================================
    
    bool bIsSelected = false;
    bool bIsEquipped = false;
    bool bIsHovered = false;
    bool bIsEmpty = true;

    // ========================================================================
    // DRAG & DROP STATE
    // ========================================================================
    
    bool bIsDragging = false;
    bool bIsValidDropTarget = false;
    bool bIsInvalidDropTarget = false;
    FLinearColor OriginalBorderColor;
};