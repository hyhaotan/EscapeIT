
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "EscapeIT/Data/ItemData.h"
#include "InventorySlotWidget.generated.h"

UCLASS()
class ESCAPEIT_API UInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    // Drag & Drop support
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

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
    UTextBlock* HotkeyText; // For quickbar slots

    UPROPERTY(meta = (BindWidget))
    UImage* QualityBorder; // Rarity indicator

    // ============================================
    // PROPERTIES
    // ============================================

    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    int32 SlotIndex = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    FInventorySlot SlotData;

    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    FItemData ItemData;

    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    bool bIsQuickbarSlot = false;

    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    bool bIsEmpty = true;

    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    bool bIsSelected = false;

    // Reference to parent inventory widget
    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    class UInventoryWidget* ParentInventoryWidget;

    // Style colors
    UPROPERTY(EditDefaultsOnly, Category = "Slot|Style")
    FLinearColor NormalColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.9f);

    UPROPERTY(EditDefaultsOnly, Category = "Slot|Style")
    FLinearColor HoverColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

    UPROPERTY(EditDefaultsOnly, Category = "Slot|Style")
    FLinearColor SelectedColor = FLinearColor(0.8f, 0.6f, 0.2f, 1.0f);

    UPROPERTY(EditDefaultsOnly, Category = "Slot|Style")
    FLinearColor EmptyColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.5f);

    // Rarity colors
    UPROPERTY(EditDefaultsOnly, Category = "Slot|Style")
    FLinearColor CommonColor = FLinearColor::Gray;

    UPROPERTY(EditDefaultsOnly, Category = "Slot|Style")
    FLinearColor UncommonColor = FLinearColor::Green;

    UPROPERTY(EditDefaultsOnly, Category = "Slot|Style")
    FLinearColor RareColor = FLinearColor::Blue;

    UPROPERTY(EditDefaultsOnly, Category = "Slot|Style")
    FLinearColor UniqueColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange

    // ============================================
    // FUNCTIONS
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Slot")
    void UpdateSlot(const FInventorySlot& NewSlotData);

    UFUNCTION(BlueprintCallable, Category = "Slot")
    void SetSelected(bool bSelected);

    UFUNCTION(BlueprintCallable, Category = "Slot")
    void OnSlotClicked();

    UFUNCTION(BlueprintCallable, Category = "Slot")
    void OnSlotDoubleClicked();

protected:
    void UpdateVisuals();
    void ShowTooltip();
    void HideTooltip();

    FLinearColor GetRarityColor(EItemRarity Rarity) const;

private:
    bool bIsHovered = false;
    FTimerHandle DoubleClickTimerHandle;
    int32 ClickCount = 0;
};