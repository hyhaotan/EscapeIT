
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/ItemData.h"
#include "InventorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UBorder;
class UProgressBar;
class UInventoryWidget;

UCLASS()
class ESCAPEIT_API UInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ============================================
    // WIDGET BINDINGS
    // ============================================
    UPROPERTY(meta = (BindWidget))
    UBorder* SlotBorder;

    UPROPERTY(meta = (BindWidget))
    UImage* ItemIcon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* QuantityText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HotkeyText;

    UPROPERTY(meta = (BindWidgetOptional))
    UProgressBar* DurabilityBar;

    UPROPERTY(meta = (BindWidgetOptional))
    UProgressBar* CooldownBar;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* CooldownOverlay;

    // ============================================
    // PROPERTIES
    // ============================================
    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    int32 SlotIndex = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    bool bIsQuickbarSlot = false;

    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    bool bIsEmpty = true;

    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    FInventorySlot SlotData;

    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    FItemData ItemData;

    UPROPERTY()
    TObjectPtr<UInventoryWidget> ParentInventoryWidget;

    // ============================================
    // COLORS
    // ============================================
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot|Colors")
    FLinearColor NormalColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot|Colors")
    FLinearColor HoverColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot|Colors")
    FLinearColor SelectedColor = FLinearColor(0.5f, 0.5f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot|Colors")
    FLinearColor EmptyColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot|Colors")
    FLinearColor EquippedColor = FLinearColor(0.1f, 0.5f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot|Colors")
    FLinearColor OnCooldownColor = FLinearColor(0.5f, 0.1f, 0.1f, 1.0f);

    // ============================================
    // FUNCTIONS
    // ============================================
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Mouse events
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    // Drag & Drop
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
    virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    // Slot management
    UFUNCTION(BlueprintCallable, Category = "Slot")
    void UpdateSlot(const FInventorySlot& NewSlotData);

    UFUNCTION(BlueprintCallable, Category = "Slot")
    void UpdateVisuals();

    UFUNCTION(BlueprintCallable, Category = "Slot")
    void SetSelected(bool bSelected);

    UFUNCTION(BlueprintCallable, Category = "Slot")
    void OnSlotClicked();

private:
    bool bIsSelected = false;
    bool bIsHovered = false;
    bool bIsDragHovered = false;
    bool bIsEquipped = false;

    void UpdateDurabilityBar();
    void UpdateCooldownVisuals();
};