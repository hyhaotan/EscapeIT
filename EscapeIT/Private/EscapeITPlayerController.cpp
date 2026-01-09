#include "EscapeITPlayerController.h"
#include "Actor/ItemPickupActor.h"
#include "Actor/Components/InventoryComponent.h"
#include "Actor/Components/FlashlightComponent.h"
#include "UI/Inventory/InteractionPromptWidget.h"
#include "UI/Inventory/InventoryWidget.h"
#include "UI/Inventory/QuickbarWidget.h"
#include "Blueprint/UserWidget.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/LocalPlayer.h"
#include "UI/HUD/WidgetManager.h"
#include <EnhancedInputSubsystems.h>

#include "EscapeITCameraManager.h"
#include "EscapeITCharacter.h"
#include "UI/NotificationWidget.h"
#include "Camera/CameraComponent.h"
#include "MovieSceneSequencePlayer.h"
#include "EscapeITCameraManager.h"
#include "Actor/ElectricCabinetActor.h"
#include "Actor/Door/Door.h"
#include "Actor/Door/DoorActor.h"

AEscapeITPlayerController::AEscapeITPlayerController()
    : MouseSensitivity(1.0f)
    , GamepadSensitivity(1.0f)
    , bInvertPitch(false)
    , bLastInputWasGamepad(false)
    , LastInputNotifyTime(0.0)
    , CurrentEquippedSlotIndex(-1)
{
    bShowMouseCursor = false;
    
    PlayerCameraManagerClass = AEscapeITCameraManager::StaticClass();
}

void AEscapeITPlayerController::BeginPlay()
{
    Super::BeginPlay();

    LastInputNotifyTime = FPlatformTime::Seconds();
    DisableInput(this);
    SetShowMouseCursor(false);
    
    PlayerCameraManager->StartCameraFade(1.0f,0.0f,FadeInDuration,
                                        FLinearColor::Black,false,true);
    
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle,this,
                                            &AEscapeITPlayerController::PlayIntroSequence,
                                        FadeInDuration,false);
    
    InitWidget();
    FindComponentClass();
}

void AEscapeITPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (IsLocalPlayerController())
    {
        SetupInputMappingContext();
        BindInputActions();
    }
}

void AEscapeITPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    
    if (bIsHoldingInteract)
    {
        OnInteractOngoing(DeltaTime);
    }
}

void AEscapeITPlayerController::PlayIntroSequence()
{
    if (!IntroSequence)
    {
        UE_LOG(LogTemp,Warning,TEXT("The Intro Level Sequence is null!"));
        OnIntroFinished();
        return;
    }
    
    bIntroPlaying = true;
    
    FMovieSceneSequencePlaybackSettings PlaybackSettings;
    PlaybackSettings.bAutoPlay = true;
    PlaybackSettings.bPauseAtEnd = true;
    
    SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
        GetWorld(),
        IntroSequence,
        PlaybackSettings,
        SequenceActor);

    if (SequencePlayer)
    {
        SequencePlayer->OnFinished.AddDynamic(this,&AEscapeITPlayerController::OnIntroFinished);
    
        SequencePlayer->Play();
    }
}

void AEscapeITPlayerController::OnIntroFinished()
{
    bIntroPlaying = false;
    
    EnableInput(this);
    
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);

    if (SequencePlayer)
    {
        SequencePlayer = nullptr;
    }
}

void AEscapeITPlayerController::SkipIntro()
{
    if (bIntroPlaying && bSkippale && SequencePlayer)
    {
        SequencePlayer->Stop();
        OnIntroFinished();
    }
}

void AEscapeITPlayerController::ApplyWakeupEffects()
{
    AEscapeITCameraManager* CameraManager = Cast<AEscapeITCameraManager>(PlayerCameraManager);

    if (CameraManager)
    {
        CameraManager->StartCameraShake(IntroCameraShake,1.0f);
    
        FPostProcessSettings PostProcessSettings;
        PostProcessSettings.bOverride_MotionBlurAmount = true;
        PostProcessSettings.MotionBlurAmount = 0.8f;
    
        PostProcessSettings.bOverride_VignetteIntensity = true;
        PostProcessSettings.VignetteIntensity = 0.6f;
    
        PostProcessSettings.bOverride_SceneFringeIntensity = true;
        PostProcessSettings.SceneFringeIntensity = 2.0f;
    
        CameraManager->AddCachedPPBlend(PostProcessSettings,1.0f);
    
        FTimerHandle EffectTimer;
        GetWorldTimerManager().SetTimer(EffectTimer, [CameraManager]()
        {
            if (CameraManager)
            {
                CameraManager->ClearPostProcessEffects();
            }
        }, 3.0f, false);
    }
}

// ============================================
// INPUT SYSTEM
// ============================================

void AEscapeITPlayerController::SetupInputMappingContext()
{
    ULocalPlayer* LP = GetLocalPlayer();
    if (!LP) return;

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP);
    if (!Subsystem) return;

    // Add all default mapping contexts
    for (UInputMappingContext* Context : DefaultMappingContexts)
    {
        if (Context)
        {
            Subsystem->AddMappingContext(Context, 0);
        }
    }
}

void AEscapeITPlayerController::BindInputActions()
{
    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EnhancedInput) return;

    // Quickbar equipment
    if (Quickbar1) EnhancedInput->BindAction(Quickbar1, ETriggerEvent::Completed, this, &AEscapeITPlayerController::EquipQuickbarSlot1);
    if (Quickbar2) EnhancedInput->BindAction(Quickbar2, ETriggerEvent::Completed, this, &AEscapeITPlayerController::EquipQuickbarSlot2);
    if (Quickbar3) EnhancedInput->BindAction(Quickbar3, ETriggerEvent::Completed, this, &AEscapeITPlayerController::EquipQuickbarSlot3);
    if (Quickbar4) EnhancedInput->BindAction(Quickbar4, ETriggerEvent::Completed, this, &AEscapeITPlayerController::EquipQuickbarSlot4);

    // Flashlight control
    if (ToggleFlashlight) EnhancedInput->BindAction(ToggleFlashlight, ETriggerEvent::Completed, this, &AEscapeITPlayerController::OnToggleFlashlight);
    if (ChargeBatteryFlashlight) EnhancedInput->BindAction(ChargeBatteryFlashlight, ETriggerEvent::Completed, this, &AEscapeITPlayerController::ChargeBattery);

    // Item usage
    if (UseEquippedItem) EnhancedInput->BindAction(UseEquippedItem, ETriggerEvent::Completed, this, &AEscapeITPlayerController::UseCurrentEquippedItem);
    if (DropItem) EnhancedInput->BindAction(DropItem, ETriggerEvent::Completed, this, &AEscapeITPlayerController::DropCurrentItem);

    // UI & Interaction
    if (ToggleInventory) EnhancedInput->BindAction(ToggleInventory, ETriggerEvent::Completed, this, &AEscapeITPlayerController::Inventory);
    if (Interact) 
    {
        EnhancedInput->BindAction(Interact, ETriggerEvent::Started, this, &AEscapeITPlayerController::OnHoldInteract);
        EnhancedInput->BindAction(Interact, ETriggerEvent::Completed, this, &AEscapeITPlayerController::OnInteractReleased);
        EnhancedInput->BindAction(Interact, ETriggerEvent::Completed, this, &AEscapeITPlayerController::OnPressInteract);
    }
    
    if (PauseMenu) EnhancedInput->BindAction(PauseMenu, ETriggerEvent::Completed, this, &AEscapeITPlayerController::OnPauseMenu);
    
    // Skip Intro
    if (SkipIntroLS) EnhancedInput->BindAction(SkipIntroLS, ETriggerEvent::Completed, this, &AEscapeITPlayerController::SkipIntro);
}

void AEscapeITPlayerController::AddYawInput(float Val)
{
    if (FMath::IsNearlyZero(Val)) return;

    const float Sensitivity = bLastInputWasGamepad ? GamepadSensitivity : MouseSensitivity;
    const float Scaled = Val * Sensitivity;

    Super::AddYawInput(Scaled);
}

void AEscapeITPlayerController::AddPitchInput(float Val)
{
    if (FMath::IsNearlyZero(Val)) return;

    const float Sensitivity = bLastInputWasGamepad ? GamepadSensitivity : MouseSensitivity;
    float Effective = Val * Sensitivity;

    if (bInvertPitch)
    {
        Effective *= -1.0f;
    }

    Super::AddPitchInput(Effective);
}

void AEscapeITPlayerController::NotifyMouseInput()
{
    bLastInputWasGamepad = false;
    LastInputNotifyTime = FPlatformTime::Seconds();
}

void AEscapeITPlayerController::NotifyGamepadInput()
{
    bLastInputWasGamepad = true;
    LastInputNotifyTime = FPlatformTime::Seconds();
}

void AEscapeITPlayerController::SetMouseSensitivity(float InSensitivity)
{
    MouseSensitivity = FMath::Clamp(InSensitivity, 0.01f, 10.0f);
}

void AEscapeITPlayerController::SetGamepadSensitivity(float InSensitivity)
{
    GamepadSensitivity = FMath::Clamp(InSensitivity, 0.01f, 10.0f);
}

void AEscapeITPlayerController::SetInvertPitch(bool bInvert)
{
    bInvertPitch = bInvert;
}

// ============================================
// INTERACTION SYSTEM
// ============================================


void AEscapeITPlayerController::OnEnterInteractableRange(AActor* Interactable)
{
    if (!Interactable) return;

    if (bIsHoldingInteract && HoldingInteractable && HoldingInteractable != Interactable)
    {
        OnInteractCanceled();
    }
    
    CurrentInteractable = Interactable;
}

void AEscapeITPlayerController::OnLeaveInteractableRange(AActor* Interactable)
{
    if (!Interactable) return;

    if (bIsHoldingInteract && HoldingInteractable == Interactable)
    {
        OnInteractCanceled();
    }

    if (CurrentInteractable == Interactable)
    {
        CurrentInteractable = nullptr;
    }
}

void AEscapeITPlayerController::OnInteractableDestroyed(AActor* Interactable)
{
    if (!Interactable) return;

    if (bIsHoldingInteract && HoldingInteractable == Interactable)
    {
        ResetHoldInteraction();
    }

    if (CurrentInteractable == Interactable)
    {
        CurrentInteractable = nullptr;
    }
}

void AEscapeITPlayerController::ResetHoldInteraction()
{
    if (AItemPickupActor* PickupActor = Cast<AItemPickupActor>(HoldingInteractable))
    {
        PickupActor->CancelHoldInteraction();
    }
    
    bIsHoldingInteract = false;
    HoldInteractProgress = 0.0f;
    HoldingInteractable = nullptr;
}

void AEscapeITPlayerController::ExecuteHoldInteraction()
{
    if (!HoldingInteractable)
    {
        UE_LOG(LogTemp, Error, TEXT("ExecuteHoldInteraction: HoldingInteractable is null!"));
        ResetHoldInteraction();
        return;
    }

    if (!IsValid(HoldingInteractable))
    {
        UE_LOG(LogTemp, Warning, TEXT("ExecuteHoldInteraction: HoldingInteractable is no longer valid"));
        ResetHoldInteraction();
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Hold interaction completed! Executing interaction on: %s"), 
        *HoldingInteractable->GetName());

    if (AItemPickupActor* PickupActor = Cast<AItemPickupActor>(HoldingInteractable))
    {
        PickupActor->CompleteHoldInteraction();
    }

    if (HoldingInteractable->Implements<UInteract>())
    {
        IInteract::Execute_Interact(HoldingInteractable, GetPawn());
    }

    ResetHoldInteraction();
}

void AEscapeITPlayerController::ExecutePressInteraction()
{
    if (!CurrentInteractable) return;
    if (!IsValid(CurrentInteractable)) return;

    if (AInteractableActor  * InteractableActor = Cast<AInteractableActor>(CurrentInteractable))
    {
        InteractableActor->OnPressInteraction();
        InteractableActor->ExecutePressInteraction();
    }

    if (CurrentInteractable->Implements<UInteract>())
    {
        IInteract::Execute_Interact(CurrentInteractable,GetPawn());
    }
}

void AEscapeITPlayerController::OnInteractCanceled()
{
    if (!bIsHoldingInteract)
    {
        return;
    }

    if (AItemPickupActor* PickupActor = Cast<AItemPickupActor>(HoldingInteractable))
    {
        PickupActor->CancelHoldInteraction();
    }

    ResetHoldInteraction();
}

void AEscapeITPlayerController::OnInteractOngoing(float DeltaTime)
{
    if (!bIsHoldingInteract)
    {
        return;
    }

    if (!HoldingInteractable)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnInteractOngoing: HoldingInteractable is null!"));
        ResetHoldInteraction();
        return;
    }

    if (CurrentInteractable != HoldingInteractable)
    {
        UE_LOG(LogTemp, Log, TEXT("OnInteractOngoing: Looking away from interactable"));
        OnInteractCanceled();
        return;
    }

    if (HoldInteractDuration <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("OnInteractOngoing: HoldInteractDuration is invalid (%.2f)"), 
            HoldInteractDuration);
        HoldInteractDuration = 1.0f; // Fallback value
    }

    HoldInteractProgress += DeltaTime / HoldInteractDuration;
    HoldInteractProgress = FMath::Clamp(HoldInteractProgress, 0.0f, 1.0f);

    if (AItemPickupActor* PickupActor = Cast<AItemPickupActor>(HoldingInteractable))
    {
        PickupActor->UpdateHoldProgress(HoldInteractProgress);
    }

    if (HoldInteractProgress >= 1.0f)
    {
        ExecuteHoldInteraction();
    }
}

void AEscapeITPlayerController::OnHoldInteract()
{
    if (!CurrentInteractable)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnHoldInteract: No interactable found"));
        return;
    }

    if (!CurrentInteractable->Implements<UInteract>())
    {
        UE_LOG(LogTemp, Warning, TEXT("OnHoldInteract: Actor doesn't implement IInteract interface"));
        return;
    }

    AInteractableActor* InteractableActor = Cast<AInteractableActor>(CurrentInteractable);
    if (!InteractableActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnHoldInteract: Not an InteractableActor"));
        return;
    }

    EInteractionType InteractionType = InteractableActor->GetInteractionType();

    if (InteractionType == EInteractionType::Press)
    {
        UE_LOG(LogTemp, Log, TEXT("OnHoldInteract: This is a Press interaction, ignoring hold"));
        return;
    }

    if (bIsHoldingInteract)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnHoldInteract: Already holding interact"));
        return;
    }

    bIsHoldingInteract = true;
    HoldInteractProgress = 0.0f;
    HoldingInteractable = CurrentInteractable;

    HoldInteractDuration = InteractableActor->GetHoldDuration();

    InteractableActor->StartHoldInteraction();

    UE_LOG(LogTemp, Log, TEXT("Started holding interact on: %s (Duration: %.2f seconds, Type: %s)"), 
        *CurrentInteractable->GetName(), 
        HoldInteractDuration,
        InteractionType == EInteractionType::Hold ? TEXT("Hold") : TEXT("Both"));
}

void AEscapeITPlayerController::OnInteractReleased()
{
    if (!bIsHoldingInteract)
    {
        return;
    }
    
    AInteractableActor* InteractableActor = Cast<AInteractableActor>(CurrentInteractable);
    if (!InteractableActor) return;
    
    EInteractionType InteractionType = InteractableActor->GetInteractionType();

    if (InteractionType == EInteractionType::Both && !bIsHoldingInteract)
    {
        ExecuteHoldInteraction();
        return;
    }

    if (bIsHoldingInteract)
    {
        if (HoldInteractProgress < 1.0f)
        {
            UE_LOG(LogTemp, Log, TEXT("OnInteractReleased: Interaction cancelled at %.1f%%"), 
                   HoldInteractProgress * 100.0f);
            OnInteractCanceled();
        }
    }
}

void AEscapeITPlayerController::OnPressInteract()
{
    if (!CurrentInteractable) return;

    if (!CurrentInteractable->Implements<UInteract>()) return;

    AInteractableActor* InteractableActor = Cast<AInteractableActor>(CurrentInteractable);
    if (!InteractableActor) return;

    EInteractionType InteractionType = InteractableActor->GetInteractionType();

    if (InteractionType == EInteractionType::Press)
    {
        ExecutePressInteraction();
    }
    else if (InteractionType == EInteractionType::Both && !bIsHoldingInteract)
    {
        UE_LOG(LogTemp, Log, TEXT("OnPressInteract: Both type - waiting to see if hold or press"));
    }
    else if (InteractionType == EInteractionType::Hold)
    {
        UE_LOG(LogTemp, Log, TEXT("OnPressInteract: Hold type - handled by OnHoldInteract"));
    }
}

// ============================================
// INVENTORY SYSTEM
// ============================================

void AEscapeITPlayerController::Inventory()
{
    if (!WidgetManagerHUD->InventoryWidgetClass) return;

    bool bIsInventoryOpen = WidgetManagerHUD->GetInventoryWidget() && WidgetManagerHUD->GetInventoryWidget()->IsInViewport();

    if (bIsInventoryOpen)
    {
        CloseInventory();
    }
    else
    {
        OpenInventory();
    }
}

void AEscapeITPlayerController::OpenInventory()
{
    UInventoryWidget* InventoryWidget = CreateWidget<UInventoryWidget>(this, WidgetManagerHUD->InventoryWidgetClass);
    WidgetManagerHUD->SetInventoryWidget(InventoryWidget);
    
    if (!InventoryWidget) return;

    InventoryWidget->AddToViewport(20);
    
    if (InventoryComponent)
    {
        InventoryWidget->InitInventory(InventoryComponent);
    }

    // Set input mode
    FInputModeGameAndUI InputMode;
    InputMode.SetWidgetToFocus(WidgetManagerHUD->GetInventoryWidget()->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);

    bShowMouseCursor = true;

    UE_LOG(LogTemp, Log, TEXT("Inventory: Opened"));
}

void AEscapeITPlayerController::CloseInventory()
{
    if (!WidgetManagerHUD->GetInventoryWidget()) return;

    WidgetManagerHUD->GetInventoryWidget()->RemoveFromParent();
    WidgetManagerHUD->SetInventoryWidget(nullptr);

    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;

    UE_LOG(LogTemp, Log, TEXT("Inventory: Closed"));
}

// ============================================
// FLASHLIGHT SYSTEM
// ============================================

void AEscapeITPlayerController::OnToggleFlashlight()
{
    // Validate components
    if (!ValidateFlashlightComponents())
    {
        return;
    }

    // Check if flashlight is equipped (use proper state check)
    if (!FlashlightComponent->IsEquipped())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot toggle: Flashlight not equipped"));
        ShowNotification(TEXT("Flashlight not equipped!"));
        return;
    }

    // Check if we're in the middle of equip/unequip animation
    EFlashlightState CurrentState = FlashlightComponent->GetCurrentState();
    if (CurrentState == EFlashlightState::Equipping || CurrentState == EFlashlightState::Unequipping)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot toggle: Flashlight is being equipped/unequipped"));
        return;
    }

    // Check battery
    if (!FlashlightComponent->CanToggleLight())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot toggle: Battery depleted"));
        ShowNotification(TEXT("Battery is depleted!"));
        PlayBatteryDepletedFeedback();
        return;
    }

    // Toggle the light
    bool bSuccess = FlashlightComponent->ToggleLight();

    if (bSuccess)
    {
        bool bIsOn = FlashlightComponent->IsLightOn();
        float BatteryPercent = FlashlightComponent->GetBatteryPercentage();
        
        UE_LOG(LogTemp, Log, TEXT("Flashlight toggled: %s (Battery: %.1f%%)"),
            bIsOn ? TEXT("ON") : TEXT("OFF"), BatteryPercent);

        // Optional: Show battery percentage when turning on
        if (bIsOn)
        {
            FString Message = FString::Printf(TEXT("Flashlight ON (%.0f%% battery)"), BatteryPercent);
            ShowNotification(Message);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to toggle flashlight"));
    }
}

bool AEscapeITPlayerController::ValidateFlashlightComponents()
{
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("InventoryComponent is NULL"));
        return false;
    }

    if (!FlashlightComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("FlashlightComponent not found on character"));
        FindComponentClass();
        
        if (!FlashlightComponent)
        {
            return false;
        }
    }

    return true;
}

bool AEscapeITPlayerController::IsFlashlightEquipped()
{
    if (!ValidateFlashlightComponents())
    {
        return false;
    }

    return FlashlightComponent->IsEquipped();
}

// ============================================
// BATTERY CHARGING SYSTEM
// ============================================

void AEscapeITPlayerController::ChargeBattery()
{
    // Validate components
    if (!InventoryComponent || !FlashlightComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Missing components!"));
        return;
    }
    
    // Check if flashlight is equipped (proper state check)
    if (!FlashlightComponent->IsEquipped())
    {
        UE_LOG(LogTemp, Warning, TEXT("Flashlight must be equipped first"));
        ShowNotification(TEXT("Equip flashlight first!"));
        return;
    }
    
    // Check if in the middle of animation
    if (IsPerformingEquipAction())
    {
        UE_LOG(LogTemp, Warning, TEXT("Wait for equip/unequip to complete"));
        return;
    }
    
    // Check if battery is full
    float CurrentBatteryPercent = FlashlightComponent->GetBatteryPercentage();
    if (CurrentBatteryPercent >= 100.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Battery is already full"));
        ShowNotification(TEXT("Battery is already full!"));
        return;
    }

    // Find battery in inventory
    FBatterySearchResult BatteryResult = FindBatteryInInventory();
    
    if (!BatteryResult.bFound)
    {
        UE_LOG(LogTemp, Warning, TEXT("No battery found in inventory"));
        ShowNotification(TEXT("You need a battery!"));
        return;
    }

    // Apply battery charge
    ApplyBatteryCharge(BatteryResult);
}

FBatterySearchResult AEscapeITPlayerController::FindBatteryInInventory()
{
    FBatterySearchResult Result;
    Result.bFound = false;

    TArray<FInventorySlot> AllSlots = InventoryComponent->GetAllInventorySlots();
    
    for (const FInventorySlot& Slot : AllSlots)
    {
        FItemData ItemData;
        if (InventoryComponent->GetItemData(Slot.ItemID, ItemData))
        {
            if (ItemData.ItemType == EItemType::Consumable && 
                ItemData.ConsumableType == EConsumableType::Battery)
            {
                Result.bFound = true;
                Result.ItemID = Slot.ItemID;
                Result.ItemData = ItemData;
                break;
            }
        }
    }

    return Result;
}

void AEscapeITPlayerController::ApplyBatteryCharge(const FBatterySearchResult& BatteryResult)
{
    // Calculate charge amount
    float ChargePercent = BatteryResult.ItemData.BatteryChargePercent;
    float MaxDuration = FlashlightComponent->GetMaxBatteryDuration();
    float ChargeSeconds = (ChargePercent / 100.0f) * MaxDuration;

    // Apply charge
    FlashlightComponent->AddBatteryCharge(ChargeSeconds);

    // Remove battery from inventory
    InventoryComponent->RemoveItem(BatteryResult.ItemID, 1);

    // Show feedback
    FString Message = FString::Printf(TEXT("Battery charged: +%d%%"), static_cast<int32>(ChargePercent));
    ShowNotification(Message);

    UE_LOG(LogTemp, Log, TEXT("Battery charged: +%.1f%% (New: %.1f%%)"), 
        ChargePercent, 
        FlashlightComponent->GetBatteryPercentage());
}

// ============================================
// QUICKBAR SYSTEM
// ============================================

void AEscapeITPlayerController::EquipQuickbarSlot1() { EquipQuickbarSlot(0); }
void AEscapeITPlayerController::EquipQuickbarSlot2() { EquipQuickbarSlot(1); }
void AEscapeITPlayerController::EquipQuickbarSlot3() { EquipQuickbarSlot(2); }
void AEscapeITPlayerController::EquipQuickbarSlot4() { EquipQuickbarSlot(3); }

void AEscapeITPlayerController::EquipQuickbarSlot(int32 SlotIndex)
{
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryComponent is NULL"));
        return;
    }

    // Toggle off if same slot
    if (CurrentEquippedSlotIndex == SlotIndex)
    {
        UE_LOG(LogTemp, Log, TEXT("Toggling OFF slot %d"), SlotIndex + 1);
        UnequipCurrentItem();
        return;
    }
    
    // Get quickbar slot data
    FInventorySlot QuickbarSlot = InventoryComponent->GetQuickbarSlot(SlotIndex);
    
    if (!QuickbarSlot.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Slot %d is empty"), SlotIndex + 1);
        ShowNotification(FString::Printf(TEXT("Slot %d is empty"), SlotIndex + 1));
        return;
    }

    // Get item data
    FItemData ItemData;
    if (!InventoryComponent->GetItemData(QuickbarSlot.ItemID, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get ItemData for slot %d"), SlotIndex + 1);
        return;
    }

    // Check if we're already equipping/unequipping something
    if (IsPerformingEquipAction())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot equip: Already performing equip/unequip action"));
        return;
    }

    // Unequip current item first (if any)
    if (CurrentEquippedSlotIndex >= 0)
    {
        // For flashlight, wait for unequip to complete before equipping new item
        if (FlashlightComponent && FlashlightComponent->IsEquipped())
        {
            UE_LOG(LogTemp, Log, TEXT("Unequipping flashlight before equipping new item..."));
            UnequipCurrentItem();
            
            // Queue the new equip action
            FTimerHandle EquipDelayTimer;
            GetWorld()->GetTimerManager().SetTimer(
                EquipDelayTimer,
                [this, SlotIndex]()
                {
                    PerformEquipAction(SlotIndex);
                },
                0.5f,
                false
            );
            return;
        }
        else
        {
            UnequipCurrentItem();
        }
    }
    
    // Perform equip
    PerformEquipAction(SlotIndex);
}

void AEscapeITPlayerController::UnequipCurrentItem()
{
    if (!InventoryComponent || CurrentEquippedSlotIndex < 0)
    {
        return;
    }

    // Check if already unequipping
    if (IsPerformingEquipAction())
    {
        UE_LOG(LogTemp, Warning, TEXT("Already performing equip/unequip action"));
        return;
    }

    // Get current item data before unequipping
    FItemData CurrentItemData;
    bool bHasEquippedItem = InventoryComponent->GetEquippedItem(CurrentItemData);

    // Unequip (InventoryComponent will handle flashlight state machine)
    InventoryComponent->UnequipCurrentItem();

    // Update character state
    if (AEscapeITCharacter* Characters = GetPawn<AEscapeITCharacter>())
    {
        Characters->bIsHoldingFlashlight = false;
        // Animation is handled by FlashlightComponent's state machine
    }

    CurrentEquippedSlotIndex = -1;

    UE_LOG(LogTemp, Log, TEXT("Unequipped item"));
}

void AEscapeITPlayerController::UpdateCharacterFlashlightState(const FItemData& ItemData)
{
    AEscapeITCharacter* Chars = GetPawn<AEscapeITCharacter>();
    if (!Chars) return;

    bool bIsFlashlight = (ItemData.ItemType == EItemType::Tool && 
                          ItemData.ToolType == EToolType::Flashlight);
    
    Chars->bIsHoldingFlashlight = bIsFlashlight;
    
    if (bIsFlashlight)
    {
        Chars->AnimState = EAnimState::Flashlight;
    }
    else
    {
        Chars->AnimState = EAnimState::Unarmed;
    }
}

// ============================================
// USE EQUIPPED ITEM
// ============================================

void AEscapeITPlayerController::UseCurrentEquippedItem()
{
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryComponent is NULL"));
        return;
    }

    if (CurrentEquippedSlotIndex < 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No item equipped"));
        return;
    }

    // Check if in the middle of animation
    if (IsPerformingEquipAction())
    {
        UE_LOG(LogTemp, Warning, TEXT("Wait for equip/unequip to complete"));
        return;
    }

    // Get equipped item data
    FItemData ItemData;
    if (!InventoryComponent->GetEquippedItem(ItemData))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot get equipped item data"));
        return;
    }

    // Block flashlight from being used with left-click
    if (ItemData.ItemType == EItemType::Tool && ItemData.ToolType == EToolType::Flashlight)
    {
        UE_LOG(LogTemp, Warning, TEXT("Use 'F' key to toggle flashlight"));
        ShowNotification(TEXT("Press F to toggle flashlight"));
        return;
    }

    // Only allow consumable items
    if (ItemData.ItemType != EItemType::Consumable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Item '%s' is not consumable"), *ItemData.ItemName.ToString());
        return;
    }

    // Use consumable item
    bool bSuccess = InventoryComponent->UseEquippedItem();

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Used consumable: '%s'"), *ItemData.ItemName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to use: '%s'"), *ItemData.ItemName.ToString());
    }
}

// ============================================
// DROP ITEM
// ============================================

void AEscapeITPlayerController::DropCurrentItem()
{
    if (!InventoryComponent)
    {
        return;
    }

    if (CurrentEquippedSlotIndex < 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No item equipped to drop"));
        return;
    }

    // Get current equipped item data
    FItemData CurrentItemData;
    bool bHasEquippedItem = InventoryComponent->GetEquippedItem(CurrentItemData);

    // Drop item
    bool bSuccess = InventoryComponent->DropEquippedItem();

    if (bSuccess)
    {
        // Update character state
        if (AEscapeITCharacter* Characters = GetPawn<AEscapeITCharacter>())
        {
            Characters->bIsHoldingFlashlight = false;
        }

        CurrentEquippedSlotIndex = -1;

        UE_LOG(LogTemp, Log, TEXT("Dropped equipped item"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to drop item"));
    }
}

// ============================================
// UI & FEEDBACK
// ============================================

void AEscapeITPlayerController::ShowNotification(const FString& Message)
{
    if (!WidgetManagerHUD || !WidgetManagerHUD->GetNotificationWidget()) return;

    FText NotificationText = FText::FromString(Message);
    WidgetManagerHUD->GetNotificationWidget()->ShowNotification(NotificationText);
}

void AEscapeITPlayerController::InitWidget()
{
    WidgetManagerHUD = Cast<AWidgetManager>(GetHUD());
    if (WidgetManagerHUD)
    {
        WidgetManagerHUD->InitializeWidgets();
    }
}

void AEscapeITPlayerController::FindComponentClass()
{
    if (GetPawn())
    {
        InventoryComponent = GetPawn()->FindComponentByClass<UInventoryComponent>();
        FlashlightComponent = GetPawn()->FindComponentByClass<UFlashlightComponent>();
    }
}

void AEscapeITPlayerController::PlayBatteryDepletedFeedback()
{
    if (bLastInputWasGamepad)
    {
        PlayHapticEffect(nullptr,EControllerHand::Right);
    }
}

bool AEscapeITPlayerController::IsPerformingEquipAction()
{
    if (!FlashlightComponent)
    {
        return false;
    }

    EFlashlightState State = FlashlightComponent->GetCurrentState();
    return (State == EFlashlightState::Equipping || State == EFlashlightState::Unequipping);
}

void AEscapeITPlayerController::PerformEquipAction(int32 SlotIndex)
{
    // Get slot data again (in case it changed)
    FInventorySlot QuickbarSlot = InventoryComponent->GetQuickbarSlot(SlotIndex);
    
    if (!QuickbarSlot.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Slot %d is no longer valid"), SlotIndex + 1);
        return;
    }

    FItemData ItemData;
    if (!InventoryComponent->GetItemData(QuickbarSlot.ItemID, ItemData))
    {
        return;
    }

    // Equip item
    bool bSuccess = InventoryComponent->EquipQuickbarSlot(SlotIndex);

    if (bSuccess)
    {
        CurrentEquippedSlotIndex = SlotIndex;
        
        // Update character state
        UpdateCharacterFlashlightState(ItemData);
        
        // No need to play animation here - FlashlightComponent handles it via state machine
        
        UE_LOG(LogTemp, Log, TEXT("Equipped '%s' from slot %d"),
            *ItemData.ItemName.ToString(), SlotIndex + 1);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to equip item from slot %d"), SlotIndex + 1);
        CurrentEquippedSlotIndex = -1;
    }
}

// ============================================
// PAUSE MENU
// ============================================

void AEscapeITPlayerController::OnPauseMenu()
{
    AHUD* HUD = GetHUD();
    if (!HUD) return;

    AWidgetManager* WidgetManager = Cast<AWidgetManager>(HUD);
    if (WidgetManager)
    {
        WidgetManager->TogglePauseMenu();
    }
}

// ============================================
// DEPRECATED FUNCTIONS - Backward compatibility
// ============================================

void AEscapeITPlayerController::UseQuickbarSlot1() { EquipQuickbarSlot1(); }
void AEscapeITPlayerController::UseQuickbarSlot2() { EquipQuickbarSlot2(); }
void AEscapeITPlayerController::UseQuickbarSlot3() { EquipQuickbarSlot3(); }
void AEscapeITPlayerController::UseQuickbarSlot4() { EquipQuickbarSlot4(); }
void AEscapeITPlayerController::UseQuickbarSlot(int32 SlotIndex) { EquipQuickbarSlot(SlotIndex); }