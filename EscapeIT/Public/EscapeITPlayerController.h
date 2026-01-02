#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Data/ItemData.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "EscapeITPlayerController.generated.h"

// Forward declarations
class UInputMappingContext;
class UInputAction;
class UInventoryComponent;
class UFlashlightComponent;
class AWidgetManager;
class UInteractionPromptWidget;
class ULevelSequence;
class UCameraShakeBase;

USTRUCT(BlueprintType)
struct FBatterySearchResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bFound = false;

    UPROPERTY(BlueprintReadOnly)
    FName ItemID;

    UPROPERTY(BlueprintReadOnly)
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

public:
    virtual void PlayerTick(float DeltaTime) override;

    // ============================================
    // INPUT SENSITIVITY
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    float MouseSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    float GamepadSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bInvertPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Input")
    bool bLastInputWasGamepad;

    UPROPERTY(BlueprintReadOnly, Category = "Input")
    double LastInputNotifyTime;

    virtual void AddYawInput(float Val) override;
    virtual void AddPitchInput(float Val) override;

    UFUNCTION(BlueprintCallable, Category = "Input")
    void NotifyMouseInput();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void NotifyGamepadInput();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetMouseSensitivity(float InSensitivity);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetGamepadSensitivity(float InSensitivity);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetInvertPitch(bool bInvert);

    // ============================================
    // INTRO SEQUENCE
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro")
    TObjectPtr<ULevelSequence> IntroSequence;

    UPROPERTY(BlueprintReadOnly, Category = "Intro")
    TObjectPtr<ULevelSequencePlayer> SequencePlayer;

    UPROPERTY(BlueprintReadOnly, Category = "Intro")
    ALevelSequenceActor* SequenceActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro")
    float FadeInDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro")
    bool bSkippale = true;

    UPROPERTY(BlueprintReadOnly, Category = "Intro")
    bool bIntroPlaying = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intro")
    TSubclassOf<UCameraShakeBase> IntroCameraShake;

    UFUNCTION(BlueprintCallable, Category = "Intro")
    void PlayIntroSequence();

    UFUNCTION(BlueprintCallable, Category = "Intro")
    void OnIntroFinished();

    UFUNCTION(BlueprintCallable, Category = "Intro")
    void SkipIntro();

    UFUNCTION(BlueprintCallable, Category = "Intro")
    void ApplyWakeupEffects();

    // ============================================
    // INTERACTION SYSTEM
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnEnterInteractableRange(AActor* Interactable);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnLeaveInteractableRange(AActor* Interactable);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void OnInteractableDestroyed(AActor* Interactable);

    // ============================================
    // INTERACTION PROPERTIES
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float HoldInteractDuration = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    TObjectPtr<AActor> CurrentInteractable;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    bool bIsHoldingInteract = false;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    float HoldInteractProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    TObjectPtr<AActor> HoldingInteractable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Debug")
    bool bShowDebugTrace = false;

    // ============================================
    // INTERACTION EXECUTION (STILL USED)
    // ============================================

    void ResetHoldInteraction();
    void ExecuteHoldInteraction();
    void OnInteractCanceled();
    void OnInteractOngoing(float DeltaTime);

    void OnInteract();
    void OnInteractReleased();

    // ============================================
    // INPUT ACTIONS
    // ============================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Mapping")
    TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

    // Quickbar
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions|Quickbar")
    TObjectPtr<UInputAction> Quickbar1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions|Quickbar")
    TObjectPtr<UInputAction> Quickbar2;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions|Quickbar")
    TObjectPtr<UInputAction> Quickbar3;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions|Quickbar")
    TObjectPtr<UInputAction> Quickbar4;

    // Flashlight
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions|Flashlight")
    TObjectPtr<UInputAction> ToggleFlashlight;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions|Flashlight")
    TObjectPtr<UInputAction> ChargeBatteryFlashlight;

    // Item Actions
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions|Items")
    TObjectPtr<UInputAction> UseEquippedItem;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions|Items")
    TObjectPtr<UInputAction> DropItem;

    // UI & Interaction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions|UI")
    TObjectPtr<UInputAction> ToggleInventory;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions|UI")
    TObjectPtr<UInputAction> Interact;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions|UI")
    TObjectPtr<UInputAction> PauseMenu;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions|Intro")
    TObjectPtr<UInputAction> SkipIntroLS;

    // ============================================
    // COMPONENTS
    // ============================================

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<UInventoryComponent> InventoryComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    TObjectPtr<UFlashlightComponent> FlashlightComponent;

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    TObjectPtr<AWidgetManager> WidgetManagerHUD;

    // ============================================
    // QUICKBAR SYSTEM
    // ============================================

    UPROPERTY(BlueprintReadOnly, Category = "Quickbar")
    int32 CurrentEquippedSlotIndex;

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

    UFUNCTION(BlueprintCallable, Category = "Quickbar")
    void UpdateCharacterFlashlightState(const FItemData& ItemData);

    // ============================================
    // ITEM USAGE
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Items")
    void UseCurrentEquippedItem();

    UFUNCTION(BlueprintCallable, Category = "Items")
    void DropCurrentItem();

    // ============================================
    // FLASHLIGHT SYSTEM
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    void OnToggleFlashlight();

    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    bool ValidateFlashlightComponents();

    UFUNCTION(BlueprintCallable, Category = "Flashlight")
    bool IsFlashlightEquipped();

    // ============================================
    // BATTERY CHARGING
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Battery")
    void ChargeBattery();

    UFUNCTION(BlueprintCallable, Category = "Battery")
    FBatterySearchResult FindBatteryInInventory();

    UFUNCTION(BlueprintCallable, Category = "Battery")
    void ApplyBatteryCharge(const FBatterySearchResult& BatteryResult);

    // ============================================
    // INVENTORY UI
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "UI")
    void Inventory();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void OpenInventory();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseInventory();

    // ============================================
    // PAUSE MENU
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "UI")
    void OnPauseMenu();

    // ============================================
    // UI & FEEDBACK
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowNotification(const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void InitWidget();

    // ============================================
    // UTILITY FUNCTIONS
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Components")
    void FindComponentClass();

    UFUNCTION(BlueprintCallable, Category = "Feedback")
    void PlayBatteryDepletedFeedback();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Equipment")
    bool IsPerformingEquipAction();

    void PerformEquipAction(int32 SlotIndex);

    // ============================================
    // DEPRECATED FUNCTIONS
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
    void SetupInputMappingContext();
    void BindInputActions();
};