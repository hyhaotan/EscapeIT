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
class AFlashlight;
class AWidgetManager;
class UNotificationWidget;

UCLASS()
class ESCAPEIT_API AEscapeITPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AEscapeITPlayerController();

    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void PlayerTick(float DeltaTime) override;

    void SetMouseSensitivity(float InSensitivity) { MouseSensitivity = FMath::Max(0.001f, InSensitivity); }
    void SetGamepadSensitivity(float InSensitivity) { GamepadSensitivity = FMath::Max(0.001f, InSensitivity); }
    void SetInvertPitch(bool bInvert) { bInvertPitch = bInvert; }

    float GetMouseSensitivity() const { return MouseSensitivity; }
    float GetGamepadSensitivity() const { return GamepadSensitivity; }
    bool IsInvertPitch() const { return bInvertPitch; }

    UFUNCTION(BlueprintCallable, Category = "Input")
    void NotifyMouseInput();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void NotifyGamepadInput();

protected:
    // ========== Components ==========
    UPROPERTY()
    TObjectPtr<UInventoryComponent> InventoryComponent;

    UPROPERTY()
    TObjectPtr<UFlashlightComponent> FlashlightComponent;

    // ========== UI Widgets ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UInteractionPromptWidget> InteractionPromptWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UQuickbarWidget> QuickbarWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UInventoryWidget> InventoryWidgetClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UNotificationWidget> NotificationWidgetClass;

    UPROPERTY()
    TObjectPtr<UInteractionPromptWidget> InteractionPromptWidget;

    UPROPERTY()
    TObjectPtr<UInventoryWidget> InventoryWidget;

    UPROPERTY()
    TObjectPtr<UQuickbarWidget> QuickbarWidget;

    UPROPERTY()
    TObjectPtr<AWidgetManager> WidgetManagerHUD;
    
    UPROPERTY()
    TObjectPtr<UNotificationWidget> NotificationWidget;

    // ========== Input Mapping ==========
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TArray<UInputMappingContext*> DefaultMappingContexts;

    // Input Actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> Quickbar1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> Quickbar2;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> Quickbar3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> Quickbar4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> ToggleInventory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> Interact;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> UseEquippedItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> DropItem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> ToggleFlashlight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> PauseMenu; 
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputAction> ChargeBatteryFlashlight;

    // ========== Interaction System ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bShowDebugTrace = false;

    UPROPERTY()
    TObjectPtr<AActor> CurrentInteractable;

    // ========== Equipped Item Tracking ==========
    UPROPERTY()
    int32 CurrentEquippedSlotIndex = -1; // -1 = không có item nào được equip

    float MouseSensitivity;
    bool bInvertPitch;
    float GamepadSensitivity;
    bool bLastInputWasGamepad;
    double LastInputNotifyTime;
    
    UPROPERTY()
    AFlashlight* Flashlight;

    // ========== Functions ==========
    void CheckForInteractables();
    void OnInteractableFound(AActor* Interactable);
    void OnInteractableLost(AActor* OldInteractable);
    void OnInteract();
    void Inventory();
    void OnToggleFlashlight();
    void OnPauseMenu();
    void UnequipCurrentItem();

    virtual void AddYawInput(float Val) override;
    virtual void AddPitchInput(float Val) override;

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
    
    void ChargeBattery();
    
    void InitWidget();
    void FindComponentClass();
};