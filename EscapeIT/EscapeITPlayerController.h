// EscapeITPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EscapeITPlayerController.generated.h"

class UInventoryComponent;
class UInteractionPromptWidget;
class UInventoryWidget;
class UQuickbarWidget;
class UInputMappingContext;
class UInputAction;
class UFlashlightComponent;

UCLASS()
class ESCAPEIT_API AEscapeITPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void PlayerTick(float DeltaTime) override;

protected:
    // ========== Components ==========
    UPROPERTY()
    UInventoryComponent* InventoryComponent;

    UPROPERTY()
    UFlashlightComponent* FlashlightComponent;

    // ========== UI Widgets ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UInteractionPromptWidget> InteractionPromptWidgetClass;

    UPROPERTY()
    UInteractionPromptWidget* InteractionPromptWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UInventoryWidget> InventoryWidgetClass;

    UPROPERTY()
    UInventoryWidget* InventoryWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UQuickbarWidget> QuickbarWidgetClass;

    UPROPERTY()
    UQuickbarWidget* QuickbarWidget;

    // ========== Input Mapping ==========
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TArray<UInputMappingContext*> DefaultMappingContexts;

    // Input Actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* Quickbar1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* Quickbar2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* Quickbar3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* Quickbar4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ToggleInventory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* Interact;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* UseEquippedItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* DropItem;

    // ========== Interaction System ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bShowDebugTrace = false;

    UPROPERTY()
    AActor* CurrentInteractable;

    // ========== Equipped Item Tracking ==========
    UPROPERTY()
    int32 CurrentEquippedSlotIndex = -1; // -1 = không có item nào được equip

    // ========== Functions ==========
    void CheckForInteractables();
    void OnInteractableFound(AActor* Interactable);
    void OnInteractableLost();
    void OnInteract();
    void Inventory();

    // Equip functions
    void EquipQuickbarSlot1();
    void EquipQuickbarSlot2();
    void EquipQuickbarSlot3();
    void EquipQuickbarSlot4();
    void EquipQuickbarSlot(int32 SlotIndex);

    // Use equipped item
    void UseCurrentEquippedItem();

    // Giữ lại cho tương thích
    void UseQuickbarSlot1();
    void UseQuickbarSlot2();
    void UseQuickbarSlot3();
    void UseQuickbarSlot4();
    void UseQuickbarSlot(int32 SlotIndex);

    // Drop Item
    void DropCurrentItem();
};