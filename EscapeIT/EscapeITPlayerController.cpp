#include "EscapeITPlayerController.h"
#include "EscapeIT/Actor/ItemPickupActor.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "EscapeIT/Actor/Components/FlashlightComponent.h"
#include "EscapeIT/UI/Inventory/InteractionPromptWidget.h"
#include "EscapeIT/UI/Inventory/InventoryWidget.h"
#include "EscapeIT/UI/Inventory/QuickbarWidget.h"
#include "Blueprint/UserWidget.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/LocalPlayer.h"
#include "EscapeIT/UI/HUD/WidgetManager.h"
#include <EnhancedInputSubsystems.h>
#include "EscapeITCharacter.h"
#include "Actor/Item/Flashlight.h"
#include "EscapeIT/UI/NotificationWidget.h"

AEscapeITPlayerController::AEscapeITPlayerController()
    : MouseSensitivity(1.0f)
    , GamepadSensitivity(1.0f)
    , bInvertPitch(false)
    , bLastInputWasGamepad(false)
    , LastInputNotifyTime(0.0)
    , CurrentEquippedSlotIndex(-1)
{
    bShowMouseCursor = false;
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

void AEscapeITPlayerController::BeginPlay()
{
    Super::BeginPlay();

    LastInputNotifyTime = FPlatformTime::Seconds();
    
    InitWidget();
    FindComponentClass();
}

void AEscapeITPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (IsLocalPlayerController())
    {
        // Add Input Mapping Context
        if (ULocalPlayer* LP = GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
            {
                for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
                {
                    if (CurrentContext)
                    {
                        Subsystem->AddMappingContext(CurrentContext, 0);
                    }
                }
            }
        }

        if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
        {
            // Bind quickbar keys to EQUIP
            if (Quickbar1) EnhancedInputComponent->BindAction(Quickbar1, ETriggerEvent::Completed, this, &AEscapeITPlayerController::EquipQuickbarSlot1);
            if (Quickbar2) EnhancedInputComponent->BindAction(Quickbar2, ETriggerEvent::Completed, this, &AEscapeITPlayerController::EquipQuickbarSlot2);
            if (Quickbar3) EnhancedInputComponent->BindAction(Quickbar3, ETriggerEvent::Completed, this, &AEscapeITPlayerController::EquipQuickbarSlot3);
            if (Quickbar4) EnhancedInputComponent->BindAction(Quickbar4, ETriggerEvent::Completed, this, &AEscapeITPlayerController::EquipQuickbarSlot4);

            // Toggle flashlight (F key)
            if (ToggleFlashlight) EnhancedInputComponent->BindAction(ToggleFlashlight, ETriggerEvent::Completed, this, &AEscapeITPlayerController::OnToggleFlashlight);

            // Use equipped item (Right-click)
            if (UseEquippedItem) EnhancedInputComponent->BindAction(UseEquippedItem, ETriggerEvent::Completed, this, &AEscapeITPlayerController::UseCurrentEquippedItem);
            
            if (ChargeBatteryFlashlight) EnhancedInputComponent->BindAction(ChargeBatteryFlashlight, ETriggerEvent::Completed, this, &AEscapeITPlayerController::ChargeBattery);

            // Other actions
            if (ToggleInventory) EnhancedInputComponent->BindAction(ToggleInventory, ETriggerEvent::Completed, this, &AEscapeITPlayerController::Inventory);
            if (Interact) EnhancedInputComponent->BindAction(Interact, ETriggerEvent::Completed, this, &AEscapeITPlayerController::OnInteract);
            if (PauseMenu) EnhancedInputComponent->BindAction(PauseMenu, ETriggerEvent::Completed, this, &AEscapeITPlayerController::OnPauseMenu);
            if (DropItem) EnhancedInputComponent->BindAction(DropItem, ETriggerEvent::Completed, this, &AEscapeITPlayerController::DropCurrentItem);
        }
    }
}

void AEscapeITPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);

    // Update interaction detection
    CheckForInteractables();
}

// ============================================
// INTERACTION SYSTEM
// ============================================

void AEscapeITPlayerController::CheckForInteractables()
{
    if (!GetPawn())
    {
        return;
    }

    // Line trace from camera
    FVector CameraLocation;
    FRotator CameraRotation;
    GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * InteractionDistance);

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetPawn());
    QueryParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        CameraLocation,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    // Debug line
    if (bShowDebugTrace)
    {
        DrawDebugLine(
            GetWorld(),
            CameraLocation,
            TraceEnd,
            bHit ? FColor::Green : FColor::Red,
            false,
            0.1f,
            0,
            2.0f
        );
    }

    AActor* HitActor = bHit ? HitResult.GetActor() : nullptr;
    
    if (HitActor != CurrentInteractable)
    {
        AActor* OldInteractable = CurrentInteractable;
        
        // Update sang actor mới
        CurrentInteractable = HitActor;
        
        if (OldInteractable)
        {
            OnInteractableLost(OldInteractable);
        }
        
        if (CurrentInteractable)
        {
            OnInteractableFound(CurrentInteractable);
        }
    }
}

void AEscapeITPlayerController::OnInteractableFound(AActor* Interactable)
{
    if (!Interactable || !WidgetManagerHUD->InteractionPromptWidget)
    {
        return;
    }

    // Check if it's an item pickup
    if (AItemPickupActor* PickupActor = Cast<AItemPickupActor>(Interactable))
    {
        FItemData ItemData;
        if (PickupActor->GetItemData(ItemData))
        {
            FText ActionText = FText::FromString(TEXT("Press"));
            FText TargetText = FText::Format(
                FText::FromString(TEXT("to Pick Up {0}")),
                ItemData.ItemName
            );

            WidgetManagerHUD->InteractionPromptWidget->ShowPrompt(ActionText, TargetText, nullptr);
            
            if (PickupActor->MeshComponent)
            {
                PickupActor->MeshComponent->SetRenderCustomDepth(true);
                PickupActor->MeshComponent->SetCustomDepthStencilValue(1);
            }
        }
        return;
    }

    // Check for other interactable types
    WidgetManagerHUD->InteractionPromptWidget->UpdatePromptForActor(Interactable);
}

void AEscapeITPlayerController::OnInteractableLost(AActor* OldInteractable)
{
    if (WidgetManagerHUD->InteractionPromptWidget)
    {
        WidgetManagerHUD->InteractionPromptWidget->HidePrompt();
    }
    
    if (OldInteractable)
    {
        if (AItemPickupActor* PickupActor = Cast<AItemPickupActor>(OldInteractable))
        {
            if (PickupActor->MeshComponent)
            {
                PickupActor->MeshComponent->SetRenderCustomDepth(false);
                PickupActor->MeshComponent->SetCustomDepthStencilValue(0);
            }
        }
    }
}

void AEscapeITPlayerController::OnInteract()
{
    if (!CurrentInteractable)
    {
        return;
    }

    if (CurrentInteractable->Implements<UInteract>())
    {
        IInteract::Execute_Interact(CurrentInteractable, GetPawn());

        if (WidgetManagerHUD->InteractionPromptWidget)
        {
            WidgetManagerHUD->InteractionPromptWidget->HidePrompt();
        }

        CurrentInteractable = nullptr;
    }
}

// ============================================
// INVENTORY SYSTEM
// ============================================

void AEscapeITPlayerController::Inventory()
{
    if (!WidgetManagerHUD->InventoryWidgetClass)
    {
        return;
    }

    if (WidgetManagerHUD->InventoryWidget && WidgetManagerHUD->InventoryWidget->IsInViewport())
    {
        // Close inventory
        WidgetManagerHUD->InventoryWidget->RemoveFromParent();
        WidgetManagerHUD->InventoryWidget = nullptr;

        SetInputMode(FInputModeGameOnly());
        bShowMouseCursor = false;
    }
    else
    {
        // Open inventory
        WidgetManagerHUD->InventoryWidget = CreateWidget<UInventoryWidget>(this, WidgetManagerHUD->InventoryWidgetClass);
        if (WidgetManagerHUD->InventoryWidget)
        {
            WidgetManagerHUD->InventoryWidget->AddToViewport(20);

            // Initialize inventory widget
            if (UInventoryWidget* InvWidget = Cast<UInventoryWidget>(WidgetManagerHUD->InventoryWidget))
            {
                if (InventoryComponent)
                {
                    InvWidget->InitInventory(InventoryComponent);
                }
            }

            FInputModeGameAndUI InputMode;
            InputMode.SetWidgetToFocus(WidgetManagerHUD->InventoryWidget->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            SetInputMode(InputMode);

            bShowMouseCursor = true;
        }
    }
}

// ============================================
// FLASHLIGHT SYSTEM (CONTROLLED BY PLAYER)
// ============================================

void AEscapeITPlayerController::OnToggleFlashlight()
{
  if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnToggleFlashlight: InventoryComponent is null!"));
        return;
    }

    // Check if any item is equipped
    if (!InventoryComponent->IsItemEquipped())
    {
        UE_LOG(LogTemp, Warning, TEXT("OnToggleFlashlight: No item equipped!"));
        return;
    }

    // Get currently equipped item data
    FItemData EquippedItemData;
    if (!InventoryComponent->GetEquippedItem(EquippedItemData))
    {
        UE_LOG(LogTemp, Warning, TEXT("OnToggleFlashlight: Cannot get equipped item data!"));
        return;
    }

    // Check if equipped item is a flashlight
    if (EquippedItemData.ItemType != EItemType::Tool || EquippedItemData.ToolType != EToolType::Flashlight)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnToggleFlashlight: Equipped item is not a flashlight!"));
        return;
    }
    
    APawn* PlayerPawn = GetPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("OnToggleFlashlight: PlayerPawn is null!"));
        return;
    }

    UFlashlightComponent* FlashlightComp = PlayerPawn->FindComponentByClass<UFlashlightComponent>();
    if (!FlashlightComp)
    {
        UE_LOG(LogTemp, Error, TEXT("OnToggleFlashlight: FlashlightComponent not found on character!"));
        return;
    }
    
    // Check if flashlight can be toggled (has battery)
    if (!FlashlightComp->CanToggleLight())
    {
        UE_LOG(LogTemp, Warning, TEXT("OnToggleFlashlight: Cannot toggle (no battery or not equipped)"));
        return;
    }

    // Toggle the flashlight
    bool bSuccess = FlashlightComp->ToggleLight();

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("✅ Flashlight toggled: %s (Battery: %.1f%%)"),
            FlashlightComp->IsLightOn() ? TEXT("ON") : TEXT("OFF"),
            FlashlightComp->GetBatteryPercentage());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ Failed to toggle flashlight (battery depleted)"));
    }
}

// ============================================
// QUICKBAR SYSTEM
// ============================================

void AEscapeITPlayerController::EquipQuickbarSlot1()
{
    EquipQuickbarSlot(0);
}

void AEscapeITPlayerController::EquipQuickbarSlot2()
{
    EquipQuickbarSlot(1);
}

void AEscapeITPlayerController::EquipQuickbarSlot3()
{
    EquipQuickbarSlot(2);
}

void AEscapeITPlayerController::EquipQuickbarSlot4()
{
    EquipQuickbarSlot(3);
}

void AEscapeITPlayerController::EquipQuickbarSlot(int32 SlotIndex)
{
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipQuickbarSlot: InventoryComponent is null!"));
        return;
    }

    if (CurrentEquippedSlotIndex == SlotIndex)
    {
        UE_LOG(LogTemp, Log, TEXT("Toggling OFF slot %d"), SlotIndex + 1);
        UnequipCurrentItem();
        return;
    }
    
    FInventorySlot QuickbarSlot = InventoryComponent->GetQuickbarSlot(SlotIndex);
    
    if (!QuickbarSlot.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipQuickbarSlot: Slot %d is empty"), SlotIndex + 1);
        return;
    }

    // Get item data for logging/animation purposes
    FItemData ItemData;
    if (!InventoryComponent->GetItemData(QuickbarSlot.ItemID, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("EquipQuickbarSlot: Failed to get ItemData"));
        return;
    }

    if (CurrentEquippedSlotIndex >= 0)
    {
        UnequipCurrentItem();
    }
    
    bool bSuccess = InventoryComponent->EquipQuickbarSlot(SlotIndex);

    if (bSuccess)
    {
        CurrentEquippedSlotIndex = SlotIndex;
        
        if (AEscapeITCharacter* Char = GetPawn<AEscapeITCharacter>())
        {
            Char->bIsHoldingFlashlight = (ItemData.ItemType == EItemType::Tool && 
                                           ItemData.ToolType == EToolType::Flashlight);
        }

        UE_LOG(LogTemp, Log, TEXT("✅ Equipped '%s' from quickbar slot %d"),
            *ItemData.ItemName.ToString(), SlotIndex + 1);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ Failed to equip item from slot %d"), SlotIndex + 1);
        CurrentEquippedSlotIndex = -1;
    }
}

void AEscapeITPlayerController::UnequipCurrentItem()
{
    if (!InventoryComponent || CurrentEquippedSlotIndex < 0)
    {
        return;
    }

    // Get current equipped item data
    FItemData CurrentItemData;
    bool bHasEquippedItem = InventoryComponent->GetEquippedItem(CurrentItemData);

    // Unequip through InventoryComponent (will handle FlashlightComponent automatically)
    InventoryComponent->UnequipCurrentItem();

    // Update character animation state
    if (AEscapeITCharacter* Characters = GetPawn<AEscapeITCharacter>())
    {
        Characters->bIsHoldingFlashlight = false;
    }

    CurrentEquippedSlotIndex = -1;

    UE_LOG(LogTemp, Log, TEXT("Unequipped item"));
}

// ============================================
// USE EQUIPPED ITEM
// ============================================

void AEscapeITPlayerController::UseCurrentEquippedItem()
{
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UseCurrentEquippedItem: InventoryComponent is null!"));
        return;
    }

    if (CurrentEquippedSlotIndex < 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("UseCurrentEquippedItem: No item equipped!"));
        return;
    }

    // Get equipped item data first
    FItemData ItemData;
    if (!InventoryComponent->GetEquippedItem(ItemData))
    {
        UE_LOG(LogTemp, Warning, TEXT("UseCurrentEquippedItem: Cannot get equipped item data!"));
        return;
    }

    // Block flashlight from being used with left-click
    if (ItemData.ItemType == EItemType::Tool && ItemData.ToolType == EToolType::Flashlight)
    {
        UE_LOG(LogTemp, Warning, TEXT("UseCurrentEquippedItem: Cannot use flashlight with left-click! Press F instead."));
        return;
    }

    // Only allow consumable items to be used
    if (ItemData.ItemType != EItemType::Consumable)
    {
        UE_LOG(LogTemp, Warning, TEXT("UseCurrentEquippedItem: Item '%s' is not consumable!"), *ItemData.ItemName.ToString());
        return;
    }

    // Use the consumable item
    bool bSuccess = InventoryComponent->UseEquippedItem();

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Used consumable item '%s' from slot %d"), 
            *ItemData.ItemName.ToString(), CurrentEquippedSlotIndex + 1);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to use consumable item '%s'"), *ItemData.ItemName.ToString());
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
        UE_LOG(LogTemp, Warning, TEXT("DropCurrentItem: No item equipped!"));
        return;
    }

    // Get current equipped item before dropping
    FItemData CurrentItemData;
    bool bHasEquippedItem = InventoryComponent->GetEquippedItem(CurrentItemData);

    // Drop through InventoryComponent (will handle FlashlightComponent automatically)
    bool bSuccess = InventoryComponent->DropEquippedItem();

    if (bSuccess)
    {
        // Update character animation state
        if (AEscapeITCharacter* Charac = GetPawn<AEscapeITCharacter>())
        {
            Charac->bIsHoldingFlashlight = false;
        }

        CurrentEquippedSlotIndex = -1;

        UE_LOG(LogTemp, Log, TEXT("Dropped equipped item"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to drop item"));
    }
}

void AEscapeITPlayerController::ChargeBattery()
{
    if (!InventoryComponent || !FlashlightComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("ChargeBattery: Missing components!"));
        return;
    }
    
    if (!FlashlightComponent->IsEquipped())
    {
        UE_LOG(LogTemp, Warning, TEXT("ChargeBattery: Flashlight not equipped!"));
        FText Message = FText::FromString(TEXT("Flashlight must be equipped first"));
        WidgetManagerHUD->NotificationWidget->ShowNotification(Message);
        return;
    }
    
    FName BatteryItemID = NAME_None;
    FItemData BatteryData;
    
    TArray<FInventorySlot> AllSlots = InventoryComponent->GetAllInventorySlots();
    for (const FInventorySlot& Slot : AllSlots)
    {
        FItemData ItemData;
        if (InventoryComponent->GetItemData(Slot.ItemID, ItemData))
        {
            if (ItemData.ItemType == EItemType::Consumable && 
                ItemData.ConsumableType == EConsumableType::Battery)
            {
                BatteryItemID = Slot.ItemID;
                BatteryData = ItemData;
                break;
            }
        }
    }
    
    if (BatteryItemID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("ChargeBattery: No battery found!"));
        FText Message = FText::FromString(TEXT("You need a battery to charge the flashlight"));
        WidgetManagerHUD->NotificationWidget->ShowNotification(Message);
        return;
    }
   
    if (FlashlightComponent->CurrentBattery <= 100.0f)
    {
        FlashlightComponent->AddBatteryCharge(BatteryData.BatteryChargePercent);
        
        FText Message = FText::FromString(FString::Printf(TEXT("Battery replaced (+%d%% charge)"),
        static_cast<int32>(BatteryData.BatteryChargePercent)));
        WidgetManagerHUD->NotificationWidget->ShowNotification(Message);
    }
    else
    {
        FText Message = FText::FromString(FString::Printf(TEXT("Your batterty flashlight must have less 100 percent then you can charge battery")));
        WidgetManagerHUD->NotificationWidget->ShowNotification(Message);
        return;
    }
    
    InventoryComponent->RemoveItem(BatteryItemID, 1);
}

void AEscapeITPlayerController::InitWidget()
{
    WidgetManagerHUD = Cast<AWidgetManager>(GetHUD());
    WidgetManagerHUD->InitializeWidgets();
}

void AEscapeITPlayerController::FindComponentClass()
{
    if (GetPawn())
    {
        InventoryComponent = GetPawn()->FindComponentByClass<UInventoryComponent>();
        FlashlightComponent = GetPawn()->FindComponentByClass<UFlashlightComponent>();
    }
}

// ============================================
// PAUSE MENU
// ============================================

void AEscapeITPlayerController::OnPauseMenu()
{
    if (AHUD* HUD = GetHUD())
    {
        if (AWidgetManager* WidgetManager = Cast<AWidgetManager>(HUD))
        {
            WidgetManager->TogglePauseMenu();
        }
    }
}

// ============================================
// DEPRECATED FUNCTIONS (For backward compatibility)
// ============================================

void AEscapeITPlayerController::UseQuickbarSlot1()
{
    EquipQuickbarSlot1();
}

void AEscapeITPlayerController::UseQuickbarSlot2()
{
    EquipQuickbarSlot2();
}

void AEscapeITPlayerController::UseQuickbarSlot3()
{
    EquipQuickbarSlot3();
}

void AEscapeITPlayerController::UseQuickbarSlot4()
{
    EquipQuickbarSlot4();
}

void AEscapeITPlayerController::UseQuickbarSlot(int32 SlotIndex)
{
    EquipQuickbarSlot(SlotIndex);
}