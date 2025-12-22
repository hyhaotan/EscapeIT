// ============================================
// PLAYER CONTROLLER HEADER - IMPROVED VERSION
// ============================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Data/ItemData.h"
#include "EscapeITPlayerController.generated.h"

// Forward declarations
class UInventoryComponent;
class UFlashlightComponent;
class AWidgetManager;
class UInputMappingContext;
class UInputAction;
struct FItemData;

USTRUCT(BlueprintType)
struct FBatterySearchResult
{
    GENERATED_BODY()

    UPROPERTY()
    bool bFound = false;

    UPROPERTY()
    FName ItemID = NAME_None;

    UPROPERTY()
    FItemData ItemData;
};

UCLASS()
class ESCAPEIT_API AEscapeITPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AEscapeITPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void PlayerTick(float DeltaTime) override;

public:
    // ============================================
    // INPUT SETTINGS
    // ============================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Sensitivity")
    float MouseSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Sensitivity")
    float GamepadSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Settings")
    bool bInvertPitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Context")
    TArray<UInputMappingContext*> DefaultMappingContexts;
    
    void SetMouseSensitivity(float InSensitivity);
    void SetGamepadSensitivity(float InSensitivity);
    void SetInvertPitch(bool bInvert);

    // ============================================
    // INPUT ACTIONS
    // ============================================

    // Quickbar
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Actions|Quickbar")
    UInputAction* Quickbar1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Actions|Quickbar")
    UInputAction* Quickbar2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Actions|Quickbar")
    UInputAction* Quickbar3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Actions|Quickbar")
    UInputAction* Quickbar4;

    // Flashlight
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Actions|Flashlight")
    UInputAction* ToggleFlashlight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Actions|Flashlight")
    UInputAction* ChargeBatteryFlashlight;

    // Items
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Actions|Items")
    UInputAction* UseEquippedItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Actions|Items")
    UInputAction* DropItem;

    // UI & Interaction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Actions|UI")
    UInputAction* ToggleInventory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Actions|UI")
    UInputAction* Interact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|Actions|UI")
    UInputAction* PauseMenu;

    // ============================================
    // INTERACTION SYSTEM
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Debug")
    bool bShowDebugTrace = false;

    // ============================================
    // COMPONENT REFERENCES
    // ============================================

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UInventoryComponent* InventoryComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UFlashlightComponent* FlashlightComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    AWidgetManager* WidgetManagerHUD;

    // ============================================
    // INPUT OVERRIDES
    // ============================================

    virtual void AddYawInput(float Val) override;
    virtual void AddPitchInput(float Val) override;

    void NotifyMouseInput();
    void NotifyGamepadInput();

    // ============================================
    // PUBLIC API - INTERACTION
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnInteract();

    // ============================================
    // PUBLIC API - INVENTORY
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void Inventory();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void DropCurrentItem();

    // ============================================
    // PUBLIC API - FLASHLIGHT
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    void OnToggleFlashlight();

    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    void ChargeBattery();

    // ============================================
    // PUBLIC API - QUICKBAR
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void EquipQuickbarSlot1();

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void EquipQuickbarSlot2();

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void EquipQuickbarSlot3();

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void EquipQuickbarSlot4();

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void EquipQuickbarSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void UnequipCurrentItem();

    // ============================================
    // PUBLIC API - ITEM USAGE
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Items")
    void UseCurrentEquippedItem();

    // ============================================
    // PUBLIC API - UI
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "UI")
    void OnPauseMenu();

    // ============================================
    // DEPRECATED - Backward compatibility
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Quickbar", meta = (DeprecatedFunction, DeprecationMessage = "Use EquipQuickbarSlot1 instead"))
    void UseQuickbarSlot1();

    UFUNCTION(BlueprintCallable, Category = "Quickbar", meta = (DeprecatedFunction, DeprecationMessage = "Use EquipQuickbarSlot2 instead"))
    void UseQuickbarSlot2();

    UFUNCTION(BlueprintCallable, Category = "Quickbar", meta = (DeprecatedFunction, DeprecationMessage = "Use EquipQuickbarSlot3 instead"))
    void UseQuickbarSlot3();

    UFUNCTION(BlueprintCallable, Category = "Quickbar", meta = (DeprecatedFunction, DeprecationMessage = "Use EquipQuickbarSlot4 instead"))
    void UseQuickbarSlot4();

    UFUNCTION(BlueprintCallable, Category = "Quickbar", meta = (DeprecatedFunction, DeprecationMessage = "Use EquipQuickbarSlot instead"))
    void UseQuickbarSlot(int32 SlotIndex);

private:
    // ============================================
    // INTERNAL STATE
    // ============================================

    AActor* CurrentInteractable = nullptr;
    
    bool bLastInputWasGamepad;
    double LastInputNotifyTime;
    
    int32 CurrentEquippedSlotIndex;

    // ============================================
    // INTERNAL FUNCTIONS - INPUT
    // ============================================

    void SetupInputMappingContext();
    void BindInputActions();

    // ============================================
    // INTERNAL FUNCTIONS - INTERACTION
    // ============================================

    void CheckForInteractables();
    void HandleInteractableChange(AActor* OldInteractable, AActor* NewInteractable);
    void OnInteractableFound(AActor* Interactable);
    void OnInteractableLost(AActor* OldInteractable);
    void HandleItemPickupFound(class AItemPickupActor* PickupActor);

    // ============================================
    // INTERNAL FUNCTIONS - INVENTORY
    // ============================================

    void OpenInventory();
    void CloseInventory();

    // ============================================
    // INTERNAL FUNCTIONS - FLASHLIGHT
    // ============================================

    bool ValidateFlashlightComponents();
    bool IsFlashlightEquipped();
    
    // Battery management
    FBatterySearchResult FindBatteryInInventory();
    void ApplyBatteryCharge(const FBatterySearchResult& BatteryResult);

    // ============================================
    // INTERNAL FUNCTIONS - QUICKBAR
    // ============================================
    
    void UpdateCharacterFlashlightState(const FItemData& ItemData);

    // ============================================
    // INTERNAL FUNCTIONS - UI
    // ============================================

    void ShowNotification(const FString& Message);
    void InitWidget();
    void FindComponentClass();
    
    void PlayBatteryDepletedFeedback();
    bool IsPerformingEquipAction();
    void PerformEquipAction(int32 SlotIndex);
};