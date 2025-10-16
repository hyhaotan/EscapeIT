
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
#include <EnhancedInputSubsystems.h>

void AEscapeITPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!FlashlightComponent)
    {
        // Nếu không có trong Controller, thử tìm từ Pawn
        if (GetPawn())
        {
            FlashlightComponent = GetPawn()->FindComponentByClass<UFlashlightComponent>();
            if (FlashlightComponent)
            {
                UE_LOG(LogTemp, Log, TEXT("FlashlightComponent found on Pawn"));
            }
        }
    }

    FlashlightComponent->SetLightEnabled(true);

    // Get inventory component
    if (GetPawn())
    {
        InventoryComponent = GetPawn()->FindComponentByClass<UInventoryComponent>();
    }

    // Create interaction prompt widget
    if (InteractionPromptWidgetClass)
    {
        InteractionPromptWidget = CreateWidget<UInteractionPromptWidget>(this, InteractionPromptWidgetClass);
        if (InteractionPromptWidget)
        {
            InteractionPromptWidget->AddToViewport(10); // High Z-order
            InteractionPromptWidget->HidePrompt();
        }
    }

    if (QuickbarWidgetClass)
    {
        QuickbarWidget = CreateWidget<UQuickbarWidget>(this, QuickbarWidgetClass);
        if (QuickbarWidget)
        {
            QuickbarWidget->AddToViewport(5); // Lower than interaction prompt
            QuickbarWidget->Initialize(InventoryComponent, FlashlightComponent);

            UE_LOG(LogTemp, Log, TEXT("QuickbarWidget created and initialized"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create QuickbarWidget"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("QuickbarWidgetClass not set!"));
    }
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
            // Bind quickbar keys to EQUIP instead of USE
            if (Quickbar1) EnhancedInputComponent->BindAction(Quickbar1, ETriggerEvent::Completed, this, &AEscapeITPlayerController::EquipQuickbarSlot1);
            if (Quickbar2) EnhancedInputComponent->BindAction(Quickbar2, ETriggerEvent::Completed, this, &AEscapeITPlayerController::EquipQuickbarSlot2);
            if (Quickbar3) EnhancedInputComponent->BindAction(Quickbar3, ETriggerEvent::Completed, this, &AEscapeITPlayerController::EquipQuickbarSlot3);
            if (Quickbar4) EnhancedInputComponent->BindAction(Quickbar4, ETriggerEvent::Completed, this, &AEscapeITPlayerController::EquipQuickbarSlot4);
            if (ToggleFlashlight)
                EnhancedInputComponent->BindAction(ToggleFlashlight, ETriggerEvent::Completed, this, &AEscapeITPlayerController::OnFlashlight);
            if (ToggleInventory) EnhancedInputComponent->BindAction(ToggleInventory, ETriggerEvent::Completed, this, &AEscapeITPlayerController::Inventory);
            if (Interact) EnhancedInputComponent->BindAction(Interact, ETriggerEvent::Completed, this, &AEscapeITPlayerController::OnInteract);

            // Add right-click to USE equipped item
            if (UseEquippedItem) EnhancedInputComponent->BindAction(UseEquippedItem, ETriggerEvent::Completed, this, &AEscapeITPlayerController::UseCurrentEquippedItem);

            // Drop equipped item
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

    // Update current interactable
    if (HitActor != CurrentInteractable)
    {
        CurrentInteractable = HitActor;

        if (CurrentInteractable)
        {
            OnInteractableFound(CurrentInteractable);
        }
        else
        {
            OnInteractableLost();
        }
    }
}

void AEscapeITPlayerController::OnInteractableFound(AActor* Interactable)
{
    if (!Interactable || !InteractionPromptWidget)
    {
        return;
    }

    // Check if it's an item pickup
    AItemPickupActor* PickupActor = Cast<AItemPickupActor>(Interactable);
    if (PickupActor)
    {
        FItemData ItemData;
        if (PickupActor->GetItemData(ItemData))
        {
            FText ActionText = FText::FromString(TEXT("Press"));
            FText TargetText = FText::Format(
                FText::FromString(TEXT("to Pick Up {0}")),
                ItemData.ItemName
            );

            InteractionPromptWidget->ShowPrompt(ActionText, TargetText, nullptr);
        }
        return;
    }

    // Check for other interactable types
    InteractionPromptWidget->UpdatePromptForActor(Interactable);
}

void AEscapeITPlayerController::OnInteractableLost()
{
    if (InteractionPromptWidget)
    {
        InteractionPromptWidget->HidePrompt();
    }
}

void AEscapeITPlayerController::OnInteract()
{
    if (!CurrentInteractable)
    {
        return;
    }

    // Handle item pickup
    AItemPickupActor* PickupActor = Cast<AItemPickupActor>(CurrentInteractable);
    if (PickupActor)
    {
        PickupActor->PickupItem(GetPawn());
        CurrentInteractable = nullptr;

        if (InteractionPromptWidget)
        {
            InteractionPromptWidget->HidePrompt();
        }
        return;
    }

    // Handle other interactables
    // TODO: Add door, puzzle, etc.
}

void AEscapeITPlayerController::Inventory()
{
    if (!InventoryWidgetClass)
    {
        return;
    }

    if (InventoryWidget && InventoryWidget->IsInViewport())
    {
        // Close inventory
        InventoryWidget->RemoveFromParent();
        InventoryWidget = nullptr;

        SetInputMode(FInputModeGameOnly());
        bShowMouseCursor = false;
    }
    else
    {
        // Open inventory
        InventoryWidget = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);
        if (InventoryWidget)
        {
            InventoryWidget->AddToViewport(20);

            // Initialize if it has Initialize function
            if (UInventoryWidget* InvWidget = Cast<UInventoryWidget>(InventoryWidget))
            {
                if (InventoryComponent)
                {
                    InvWidget->Initialize(InventoryComponent);
                }
            }

            FInputModeGameAndUI InputMode;
            InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            SetInputMode(InputMode);

            bShowMouseCursor = true;
        }
    }
}

void AEscapeITPlayerController::OnFlashlight()
{
    if (!FlashlightComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot toggle flashlight - FlashlightComponent is null!"));
        return;
    }

    // Check if flashlight is currently equipped
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot toggle flashlight - InventoryComponent is null!"));
        return;
    }

    // Get currently equipped item
    FItemData EquippedItem;
    bool bHasEquippedItem = InventoryComponent->GetEquippedItem(EquippedItem);

    if (!bHasEquippedItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("No item equipped!"));
        return;
    }

    // Check if equipped item is a flashlight using ItemCategory
    if (EquippedItem.ItemType == EItemType::Tool)
    {
        if (EquippedItem.ItemCategory == EItemCategory::Flashlight)
        {
            FlashlightComponent->ToggleLight();
            UE_LOG(LogTemp, Log, TEXT("Flashlight toggled"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Equipped item is not a flashlight! Current item: %s"),
            *EquippedItem.ItemName.ToString());
    }
}

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
        return;
    }

    // Get item data before equipping
    FItemData ItemData;
    bool bHasItem = InventoryComponent->GetQuickbarSlotItem(SlotIndex, ItemData);

    // Unequip flashlight if currently equipped
    if (CurrentEquippedSlotIndex >= 0 && FlashlightComponent)
    {
        FItemData CurrentItem;
        if (InventoryComponent->GetEquippedItem(CurrentItem))
        {
            if (CurrentItem.ItemName.ToString().Contains(TEXT("Flashlight")))
            {
                FlashlightComponent->UnequipFlashlight();
            }
        }
    }

    // Equip new item
    bool bSuccess = InventoryComponent->EquipQuickbarSlot(SlotIndex);

    if (bSuccess)
    {
        CurrentEquippedSlotIndex = SlotIndex;

        // If equipped item is flashlight, mark it as equipped in FlashlightComponent
        if (bHasItem && ItemData.ItemName.ToString().Contains(TEXT("Flashlight")))
        {
            if (FlashlightComponent)
            {
                FlashlightComponent->EquipFlashlight();
                UE_LOG(LogTemp, Log, TEXT("Flashlight equipped and ready to use"));
            }
        }

        UE_LOG(LogTemp, Log, TEXT("Equipped item from quickbar slot %d"), SlotIndex + 1);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to equip item from slot %d"), SlotIndex + 1);
    }
}

// ========== USE EQUIPPED ITEM (Right-click) ==========

void AEscapeITPlayerController::UseCurrentEquippedItem()
{
    if (!InventoryComponent)
    {
        return;
    }

    if (CurrentEquippedSlotIndex < 0)
    {
        return;
    }

    // Sử dụng item đang equipped
    bool bSuccess = InventoryComponent->UseEquippedItem();

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Used equipped item from slot %d"), CurrentEquippedSlotIndex + 1);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to use equipped item"));
    }
}

// ========== DEPRECATED (Giữ lại cho tương thích) ==========

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

void AEscapeITPlayerController::DropCurrentItem()
{
    if (InventoryComponent)
    {
        // Unequip flashlight before dropping
        if (CurrentEquippedSlotIndex >= 0 && FlashlightComponent)
        {
            FItemData CurrentItem;
            if (InventoryComponent->GetEquippedItem(CurrentItem))
            {
                if (CurrentItem.ItemName.ToString().Contains(TEXT("Flashlight")))
                {
                    FlashlightComponent->UnequipFlashlight();
                }
            }
        }

        InventoryComponent->DropEquippedItem();
        CurrentEquippedSlotIndex = -1;
    }
}
