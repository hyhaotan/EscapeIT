// EscapeITPlayerController.h - Controller tích hợp inventory system

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "EscapeIT/Actor/Components/FlashlightComponent.h"
#include "EscapeITPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class AWidgetManager;
class UQuickbarWidget;

UCLASS()
class ESCAPEIT_API AEscapeITPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AEscapeITPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    /** Input Mapping Contexts */
    UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
    TArray<UInputMappingContext*> DefaultMappingContexts;

    /*Input Component*/
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* Quickbar1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* Quickbar2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* Quickbar3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* Quickbar4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* ToggleInventory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* Interact;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* ToggleFlashlight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* DropItem;
public:
    // ============================================
    // WIDGET REFERENCES
    // ============================================
    UPROPERTY()
    AWidgetManager* WidgetManager;

    // ============================================
    // COMPONENT REFERENCES
    // ============================================

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UInventoryComponent* InventoryComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UFlashlightComponent* FlashlightComponent;

    // ============================================
    // INPUT ACTIONS
    // ============================================

    // Quickbar (1-4)
    UFUNCTION()
    void UseQuickbarSlot1();

    UFUNCTION()
    void UseQuickbarSlot2();

    UFUNCTION()
    void UseQuickbarSlot3();

    UFUNCTION()
    void UseQuickbarSlot4();

    // Interaction (E key)
    UFUNCTION()
    void InteractItem();

    // Flashlight toggle (F key)
    UFUNCTION()
    void Flashlight();

    // Drop item (G key - optional)
    UFUNCTION()
    void DropCurrentItem();

    // ============================================
    // INTERACTION SYSTEM
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    AActor* GetLookingAtActor();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void UpdateInteractionPrompt();

protected:
    void InitializeComponents();
    void UseQuickbarSlot(int32 SlotIndex);

private:
    void DisplayInventory();
    AActor* CurrentInteractableActor = nullptr;
    float InteractionRange = 200.0f;

    UPROPERTY()
    UQuickbarWidget* QuickbarWidget;
};