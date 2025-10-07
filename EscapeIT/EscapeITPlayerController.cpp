#include "EscapeITPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Actor/ItemPickupActor.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "EscapeIT/UI/HUD/WidgetManager.h"
#include "InputMappingContext.h"
#include "EscapeIT/UI/Inventory/QuickbarWidget.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "EscapeIT/Actor/Components/FlashlightComponent.h"

AEscapeITPlayerController::AEscapeITPlayerController()
{
    bShowMouseCursor = false;
    bEnableClickEvents = false;

    InteractionRange = 500.0f; // default tương đối
    CurrentInteractableActor = nullptr;
}

void AEscapeITPlayerController::BeginPlay()
{
    Super::BeginPlay();

    InitializeComponents();

    // Lấy WidgetManager từ HUD (kiểm tra kiểu cast)
    WidgetManager = Cast<AWidgetManager>(GetHUD());
    if (!WidgetManager)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController: WidgetManager not found!"));
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
            if (Quickbar1) EnhancedInputComponent->BindAction(Quickbar1, ETriggerEvent::Completed, this, &AEscapeITPlayerController::UseQuickbarSlot1);
            if (Quickbar2) EnhancedInputComponent->BindAction(Quickbar2, ETriggerEvent::Completed, this, &AEscapeITPlayerController::UseQuickbarSlot2);
            if (Quickbar3) EnhancedInputComponent->BindAction(Quickbar3, ETriggerEvent::Completed, this, &AEscapeITPlayerController::UseQuickbarSlot3);
            if (Quickbar4) EnhancedInputComponent->BindAction(Quickbar4, ETriggerEvent::Completed, this, &AEscapeITPlayerController::UseQuickbarSlot4);
            if (ToggleInventory) EnhancedInputComponent->BindAction(ToggleInventory, ETriggerEvent::Completed, this, &AEscapeITPlayerController::DisplayInventory);
            if (Interact) EnhancedInputComponent->BindAction(Interact, ETriggerEvent::Completed, this, &AEscapeITPlayerController::InteractItem);
            if (ToggleFlashlight) EnhancedInputComponent->BindAction(ToggleFlashlight, ETriggerEvent::Completed, this, &AEscapeITPlayerController::Flashlight);
            if (DropItem) EnhancedInputComponent->BindAction(DropItem, ETriggerEvent::Completed, this, &AEscapeITPlayerController::DropCurrentItem);
        }
    }
}

// ============================================
// INITIALIZATION
// ============================================
void AEscapeITPlayerController::InitializeComponents()
{
    APawn* PlayerPawn = GetPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController: No pawn found!"));
        return;
    }

    // Get components từ pawn
    InventoryComponent = PlayerPawn->FindComponentByClass<UInventoryComponent>();
    FlashlightComponent = PlayerPawn->FindComponentByClass<UFlashlightComponent>();

    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController: InventoryComponent not found on pawn!"));
    }

    if (!FlashlightComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController: FlashlightComponent not found on pawn!"));
    }
}

// ============================================
// QUICKBAR INPUT
// ============================================
void AEscapeITPlayerController::UseQuickbarSlot1() { UseQuickbarSlot(0); }
void AEscapeITPlayerController::UseQuickbarSlot2() { UseQuickbarSlot(1); }
void AEscapeITPlayerController::UseQuickbarSlot3() { UseQuickbarSlot(2); }
void AEscapeITPlayerController::UseQuickbarSlot4() { UseQuickbarSlot(3); }

void AEscapeITPlayerController::UseQuickbarSlot(int32 SlotIndex)
{
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UseQuickbarSlot: InventoryComponent null"));
        return;
    }

    // Slot 0 = Flashlight (special handling)
    if (SlotIndex == 0)
    {
        Flashlight();
        return;
    }

    // Other slots = use item
    bool bSuccess = InventoryComponent->UseQuickbarSlot(SlotIndex);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Used quickbar slot %d"), SlotIndex);

        // Highlight slot trong UI nếu có WidgetManager
        if (WidgetManager && WidgetManager->QuickbarWidget)
        {
            WidgetManager->QuickbarWidget->HighlightSlot(SlotIndex);
        }
    }
}

void AEscapeITPlayerController::DisplayInventory()
{
    if (WidgetManager)
    {
        WidgetManager->Inventory();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DisplayInventory: WidgetManager is null"));
    }
}

// ============================================
// INTERACTION SYSTEM
// ============================================
void AEscapeITPlayerController::InteractItem()
{
    AActor* InteractableActor = GetLookingAtActor();

    if (InteractableActor)
    {
        // Check nếu là item pickup
        AItemPickupActor* PickupActor = Cast<AItemPickupActor>(InteractableActor);
        if (PickupActor)
        {
            APawn* Pawns = GetPawn();
            if (Pawns)
            {
                PickupActor->PickupItem(Pawns);
                UE_LOG(LogTemp, Log, TEXT("Interacted with pickup: %s"), *PickupActor->GetName());
            }
            return;
        }

        // TODO: Handle other interactable objects (doors, puzzles, etc.)
        UE_LOG(LogTemp, Log, TEXT("Interacted with: %s"), *InteractableActor->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No interactable object in range"));
    }
}

AActor* AEscapeITPlayerController::GetLookingAtActor()
{
    APawn* PlayerPawn = GetPawn();
    if (!PlayerPawn)
    {
        return nullptr;
    }

    // Get camera location và direction
    FVector CameraLocation;
    FRotator CameraRotation;
    GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector TraceStart = CameraLocation;
    FVector TraceEnd = TraceStart + (CameraRotation.Vector() * InteractionRange);

    // Line trace
    FHitResult HitResult;
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(InteractionTrace), true);
    QueryParams.AddIgnoredActor(PlayerPawn);
    QueryParams.bTraceComplex = true;

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        TraceStart,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    // Debug draw (chỉ trong development)
#if WITH_EDITOR
    DrawDebugLine(GetWorld(), TraceStart, TraceEnd, bHit ? FColor::Green : FColor::Red, false, 0.1f, 0, 1.0f);
#endif

    if (bHit)
    {
        AActor* HitActor = HitResult.GetActor();

        // Check nếu actor có tag "Interactable" hoặc là AItemPickupActor
        if (HitActor && (HitActor->ActorHasTag(FName("Interactable")) || Cast<AItemPickupActor>(HitActor)))
        {
            return HitActor;
        }
    }

    return nullptr;
}

void AEscapeITPlayerController::UpdateInteractionPrompt()
{
    AActor* LookingAt = GetLookingAtActor();

    if (LookingAt != CurrentInteractableActor)
    {
        CurrentInteractableActor = LookingAt;

        if (CurrentInteractableActor)
        {
            // TODO: Show interaction prompt UI
            // "Press E to Pick Up [ItemName]"
            UE_LOG(LogTemp, Log, TEXT("Looking at: %s"), *CurrentInteractableActor->GetName());
        }
        else
        {
            // TODO: Hide interaction prompt
        }
    }
}

// ============================================
// FLASHLIGHT
// ============================================
void AEscapeITPlayerController::Flashlight()
{
    if (!FlashlightComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("ToggleFlashlight: FlashlightComponent is null"));
        return;
    }

    // Check xem có flashlight trong quickbar không (nếu InventoryComponent được implement)
    if (InventoryComponent)
    {
        FInventorySlot FlashlightSlot = InventoryComponent->GetQuickbarSlot(0);
        if (!FlashlightSlot.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("ToggleFlashlight: No flashlight in quickbar slot 0"));
            return;
        }
    }

    FlashlightComponent->ToggleLight();
}

// ============================================
// DROP ITEM
// ============================================
void AEscapeITPlayerController::DropCurrentItem()
{
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("DropItem: InventoryComponent is null"));
        return;
    }

    APawn* PlayerPawn = GetPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("DropItem: No pawn found"));
        return;
    }

    // Lấy slot để drop (ở đây mặc định slot 1, có thể thay đổi)
    int32 SlotToDrop = 1; // Slot 1, 2, hoặc 3 (không drop flashlight ở slot 0)

    // Kiểm tra xem slot có item không
    FInventorySlot SlotData = InventoryComponent->GetQuickbarSlot(SlotToDrop);
    if (!SlotData.IsValid() || SlotData.Quantity <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DropItem: No item in slot %d to drop"), SlotToDrop);
        return;
    }

    // Tính vị trí drop (trước mặt player)
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FRotator PlayerRotation = PlayerPawn->GetActorRotation();
    FVector DropLocation = PlayerLocation + (PlayerRotation.Vector() * 150.0f); // 150cm trước mặt
    DropLocation.Z += 50.0f;

    // Spawn item pickup actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    if (SlotData.ItemData && SlotData.ItemData->PickupActorClass)
    {
        AItemPickupActor* DroppedItem = GetWorld()->SpawnActor<AItemPickupActor>(
            SlotData.ItemData->PickupActorClass,
            DropLocation,
            FRotator::ZeroRotator,
            SpawnParams
        );

        if (DroppedItem)
        {
            // Set item data cho dropped item (hàm SetItemData cần implement trong ItemPickupActor)
            //DroppedItem->SetItemDataByStruct(*SlotData.ItemData);

            // Apply physics impulse để item bay ra một chút nếu root là primitive
            if (UPrimitiveComponent* ItemMesh = Cast<UPrimitiveComponent>(DroppedItem->GetRootComponent()))
            {
                if (!ItemMesh->IsSimulatingPhysics())
                {
                    ItemMesh->SetSimulatePhysics(true);
                }
                FVector ImpulseDirection = PlayerRotation.Vector() + FVector(0, 0, 0.3f);
                ImpulseDirection.Normalize();
                ItemMesh->AddImpulse(ImpulseDirection * 300.0f, NAME_None, true);
            }

            if (InventoryComponent->DropItem(SlotData.ItemID, 1))
            {
                UE_LOG(LogTemp, Log, TEXT("Dropped item via InventoryComponent from slot %d"), SlotToDrop);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to drop item via InventoryComponent"));
            }

            UE_LOG(LogTemp, Log, TEXT("Dropped item: %s from slot %d"), *SlotData.ItemData->ItemName.ToString(), SlotToDrop);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn dropped item"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DropItem: ItemData or PickupActorClass is null"));
    }
}
