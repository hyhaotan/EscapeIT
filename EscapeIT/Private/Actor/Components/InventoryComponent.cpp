
#include "Actor/Components/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Actor/ItemPickupActor.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Actor/Components/SanityComponent.h"
#include "Actor/Components/FlashlightComponent.h"
#include "Actor/Item/Flashlight.h"
#include "TimerManager.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize quickbar indices (all empty)
    QuickbarSlotIndices.Init(-1, QuickbarSize);

    // Get character mesh for attaching items
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        CharacterMesh = Character->GetMesh();
    }

    UE_LOG(LogTemp, Log, TEXT("InventoryComponent initialized - Max:%d slots, Quickbar:%d"), 
        MaxInventorySlots, QuickbarSize);
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// ============================================================================
// ADD/REMOVE ITEMS - FIXED
// ============================================================================

bool UInventoryComponent::AddItem(FName ItemID, int32 Quantity)
{
    if (ItemID.IsNone() || Quantity <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AddItem: Invalid ItemID or Quantity"));
        return false;
    }

    FItemData ItemData;
    if (!GetItemData(ItemID, ItemData))
    {
        UE_LOG(LogTemp, Warning, TEXT("AddItem: ItemID '%s' not found in DataTable"), *ItemID.ToString());
        return false;
    }

    int32 RemainingQuantity = Quantity;
    int32 AddedToSlotIndex = -1;

    // ========================================================================
    // STEP 1: Try stacking with existing items
    // ========================================================================
    if (ItemData.MaxStackSize > 1)
    {
        for (int32 i = 0; i < InventorySlots.Num(); i++)
        {
            FInventorySlot& Slot = InventorySlots[i];
            if (Slot.ItemID == ItemID && Slot.Quantity < ItemData.MaxStackSize)
            {
                int32 SpaceAvailable = ItemData.MaxStackSize - Slot.Quantity;
                int32 AmountToAdd = FMath::Min(SpaceAvailable, RemainingQuantity);

                Slot.Quantity += AmountToAdd;
                RemainingQuantity -= AmountToAdd;
                
                if (AddedToSlotIndex == -1)
                {
                    AddedToSlotIndex = i;
                }

                UE_LOG(LogTemp, Log, TEXT("  → Stacked %d into slot[%d] (now %d)"), AmountToAdd, i, Slot.Quantity);

                if (RemainingQuantity <= 0)
                {
                    break;
                }
            }
        }
    }

    // ========================================================================
    // STEP 2: Create new slots for remaining items
    // ========================================================================
    while (RemainingQuantity > 0)
    {
        if (IsInventoryFull())
        {
            UE_LOG(LogTemp, Warning, TEXT("INVENTORY FULL! Cannot add '%s'"), *ItemData.ItemName.ToString());
            PlayInventoryFullSound();

            // Partial add success
            if (RemainingQuantity < Quantity)
            {
                OnInventoryUpdated.Broadcast();
                OnItemAdded.Broadcast(ItemID, Quantity - RemainingQuantity);
            }
            return false;
        }

        int32 AmountToAdd = FMath::Min(RemainingQuantity, ItemData.MaxStackSize);
        FInventorySlot NewSlot(ItemID, AmountToAdd);

        int32 NewSlotIndex = InventorySlots.Add(NewSlot);
        RemainingQuantity -= AmountToAdd;
        
        if (AddedToSlotIndex == -1)
        {
            AddedToSlotIndex = NewSlotIndex;
        }

        UE_LOG(LogTemp, Log, TEXT("  → Created new slot[%d] with %d items"), NewSlotIndex, AmountToAdd);
    }

    // ========================================================================
    // STEP 3: Play sound & broadcast events
    // ========================================================================
    PlayItemSound(ItemData.PickupSound);
    OnInventoryUpdated.Broadcast();
    OnItemAdded.Broadcast(ItemID, Quantity);

    // ========================================================================
    // STEP 4: Auto-assign to quickbar (IMPROVED LOGIC)
    // ========================================================================
    if (AddedToSlotIndex != -1)
    {
        TryAutoAssignToQuickbar(ItemID, AddedToSlotIndex);
    }

    UE_LOG(LogTemp, Log, TEXT("Added %d x %s [%d/%d slots]"), 
        Quantity, *ItemData.ItemName.ToString(), InventorySlots.Num(), MaxInventorySlots);
    
    return true;
}

bool UInventoryComponent::CanAddItem(FName ItemID, int32 Quantity)
{
    if (IsInventoryFull()) return false;
    
    return true;
}

bool UInventoryComponent::RemoveItem(FName ItemID, int32 Quantity)
{
    if (ItemID.IsNone() || Quantity <= 0)
    {
        return false;
    }

    if (!HasItem(ItemID, Quantity))
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveItem: Not enough '%s' (need %d, have %d)"), 
            *ItemID.ToString(), Quantity, GetItemQuantity(ItemID));
        return false;
    }

    int32 RemainingToRemove = Quantity;
    TArray<int32> SlotsToRemove;

    // ========================================================================
    // STEP 1: Mark slots for removal and reduce quantities
    // ========================================================================
    for (int32 i = InventorySlots.Num() - 1; i >= 0 && RemainingToRemove > 0; i--)
    {
        if (InventorySlots[i].ItemID == ItemID)
        {
            int32 AmountToRemove = FMath::Min(InventorySlots[i].Quantity, RemainingToRemove);
            InventorySlots[i].Quantity -= AmountToRemove;
            RemainingToRemove -= AmountToRemove;

            if (InventorySlots[i].Quantity <= 0)
            {
                SlotsToRemove.Add(i);
            }
        }
    }

    // ========================================================================
    // STEP 2: Handle equipped item if being removed
    // ========================================================================
    if (CurrentEquippedSlotIndex >= 0)
    {
        int32 EquippedInventoryIndex = GetQuickbarInventoryIndex(CurrentEquippedSlotIndex);
        
        if (EquippedInventoryIndex >= 0 && InventorySlots.IsValidIndex(EquippedInventoryIndex))
        {
            if (InventorySlots[EquippedInventoryIndex].ItemID == ItemID)
            {
                // Check if this slot will be removed
                if (SlotsToRemove.Contains(EquippedInventoryIndex))
                {
                    UE_LOG(LogTemp, Warning, TEXT("  → Equipped item being removed, unequipping..."));
                    UnequipCurrentItem();
                }
            }
        }
    }

    // ========================================================================
    // STEP 3: Remove empty slots and update quickbar indices
    // ========================================================================
    for (int32 SlotIndexToRemove : SlotsToRemove)
    {
        RemoveSlotAndUpdateReferences(SlotIndexToRemove);
    }

    // ========================================================================
    // STEP 4: Validate and broadcast
    // ========================================================================
    ValidateQuickbarReferences();
    OnInventoryUpdated.Broadcast();
    OnItemRemoved.Broadcast(ItemID, Quantity);

    UE_LOG(LogTemp, Log, TEXT("Removed %d x %s"), Quantity, *ItemID.ToString());
    return true;
}

// ============================================================================
// NEW HELPER: Remove slot and update all references
// ============================================================================
void UInventoryComponent::RemoveSlotAndUpdateReferences(int32 SlotIndexToRemove)
{
    if (!InventorySlots.IsValidIndex(SlotIndexToRemove))
    {
        return;
    }

    // Update quickbar references BEFORE removing
    for (int32 q = 0; q < QuickbarSlotIndices.Num(); q++)
    {
        if (QuickbarSlotIndices[q] == SlotIndexToRemove)
        {
            // Clear this quickbar slot
            QuickbarSlotIndices[q] = -1;
            UE_LOG(LogTemp, Log, TEXT("  → Cleared quickbar[%d]"), q);
        }
        else if (QuickbarSlotIndices[q] > SlotIndexToRemove)
        {
            // Shift down indices after removed slot
            QuickbarSlotIndices[q]--;
            UE_LOG(LogTemp, Log, TEXT("  → Shifted quickbar[%d]: %d -> %d"), 
                q, QuickbarSlotIndices[q] + 1, QuickbarSlotIndices[q]);
        }
    }

    // Remove the slot
    InventorySlots.RemoveAt(SlotIndexToRemove);
    UE_LOG(LogTemp, Log, TEXT("  → Removed inventory slot[%d]"), SlotIndexToRemove);
}

// ============================================================================
// USE ITEMS - IMPROVED VALIDATION
// ============================================================================

bool UInventoryComponent::UseItem(FName ItemID)
{
    if (!HasItem(ItemID, 1))
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: Item not in inventory"));
        OnItemUsed.Broadcast(ItemID, false);
        return false;
    }

    FItemData ItemData;
    if (!GetItemData(ItemID, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("UseItem: Cannot get item data"));
        OnItemUsed.Broadcast(ItemID, false);
        return false;
    }

    if (!ItemData.bCanBeUsed)
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: Item cannot be used"));
        OnItemUsed.Broadcast(ItemID, false);
        return false;
    }
    
    const auto Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Pawn)
    {
        UE_LOG(LogTemp, Error, TEXT("UseItem: Player pawn not found"));
        return false;
    }

    // ========================================================================
    // BATTERY REPLACEMENT
    // ========================================================================
    if (ItemData.ItemType == EItemType::Consumable && ItemData.ConsumableType == EConsumableType::Battery)
    {
        UFlashlightComponent* FlashlightComp = Pawn->FindComponentByClass<UFlashlightComponent>();
        
        if (!FlashlightComp)
        {
            UE_LOG(LogTemp, Warning, TEXT("UseItem: FlashlightComponent not found!"));
            OnItemUsed.Broadcast(ItemID, false);
            return false;
        }
        
        if (!FlashlightComp->IsEquipped())
        {
            UE_LOG(LogTemp, Warning, TEXT("UseItem: Flashlight must be equipped to replace battery!"));
            OnItemUsed.Broadcast(ItemID, false);
            return false;
        }

        float CurrentBattery = FlashlightComp->GetCurrentBattery();
        if (CurrentBattery >= 100.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("UseItem: Battery is already full!"));
            OnItemUsed.Broadcast(ItemID, false);
            return false;
        }

        FlashlightComp->AddBatteryCharge(ItemData.BatteryChargePercent);
        RemoveItem(ItemID, 1);
        PlayItemSound(ItemData.UseSound);
        OnItemUsed.Broadcast(ItemID, true);
        OnInventoryUpdated.Broadcast();

        UE_LOG(LogTemp, Log, TEXT("Battery replaced (+%.0f%%)"), ItemData.BatteryChargePercent);
        return true;
    }

    // ========================================================================
    // CONSUMABLE ITEMS (Sanity restore)
    // ========================================================================
    if (ItemData.ItemType == EItemType::Consumable)
    {
        USanityComponent* Sanity = Pawn->FindComponentByClass<USanityComponent>();
        
        if (!Sanity)
        {
            UE_LOG(LogTemp, Warning, TEXT("UseItem: SanityComponent not found!"));
            OnItemUsed.Broadcast(ItemID, false);
            return false;
        }

        // Check if sanity is already full
        if (Sanity->GetSanity() >= 100.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("UseItem: Sanity is already full!"));
            OnItemUsed.Broadcast(ItemID, false);
            return false;
        }

        // Apply effect BEFORE removing
        bool bEffectApplied = ApplyItemEffect(ItemData);
        PlayItemSound(ItemData.UseSound);
        OnItemUsed.Broadcast(ItemID, bEffectApplied);

        // Unequip if currently equipped
        if (CurrentEquippedItemID == ItemID)
        {
            UnequipCurrentItem();
        }
        
        // Remove from inventory AFTER all operations
        RemoveItem(ItemID, 1);
        OnInventoryUpdated.Broadcast();
        
        UE_LOG(LogTemp, Log, TEXT("Used consumable: %s"), *ItemData.ItemName.ToString());
        return true;
    }

    // ========================================================================
    // OTHER ITEMS
    // ========================================================================
    bool bEffectApplied = ApplyItemEffect(ItemData);
    PlayItemSound(ItemData.UseSound);
    OnItemUsed.Broadcast(ItemID, bEffectApplied);
    OnInventoryUpdated.Broadcast();
    
    return true;
}

bool UInventoryComponent::UseEquippedItem()
{
    if (CurrentEquippedItemID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("UseEquippedItem: No item equipped"));
        return false;
    }

    FItemData ItemData;
    if (!GetItemData(CurrentEquippedItemID, ItemData))
    {
        return false;
    }

    // ========================================================================
    // FLASHLIGHT TOGGLE
    // ========================================================================
    if (ItemData.ItemType == EItemType::Tool && ItemData.ToolType == EToolType::Flashlight)
    {
        UFlashlightComponent* FlashlightComp = GetOwner()->FindComponentByClass<UFlashlightComponent>();
        
        if (!FlashlightComp)
        {
            UE_LOG(LogTemp, Error, TEXT("UseEquippedItem: FlashlightComponent not found!"));
            return false;
        }
        
        bool bSuccess = FlashlightComp->ToggleLight();
        
        if (!bSuccess)
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot toggle flashlight"));
        }
        
        return bSuccess;
    }
    
    return UseItem(CurrentEquippedItemID);
}

// ============================================================================
// EQUIP/UNEQUIP - IMPROVED
// ============================================================================

bool UInventoryComponent::EquipQuickbarSlot(int32 QuickbarIndex)
{
    if (QuickbarIndex < 0 || QuickbarIndex >= QuickbarSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipQuickbarSlot: Invalid index %d"), QuickbarIndex);
        return false;
    }
    
    int32 InventoryIndex = QuickbarSlotIndices[QuickbarIndex];
    
    if (InventoryIndex < 0 || !InventorySlots.IsValidIndex(InventoryIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipQuickbarSlot: Slot %d is empty"), QuickbarIndex);
        return false;
    }

    FInventorySlot& Slot = InventorySlots[InventoryIndex];
    
    if (!Slot.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipQuickbarSlot: Invalid slot data"));
        QuickbarSlotIndices[QuickbarIndex] = -1;
        OnInventoryUpdated.Broadcast();
        return false;
    }
    
    // Toggle off if already equipped
    if (CurrentEquippedItemID == Slot.ItemID && CurrentEquippedSlotIndex == QuickbarIndex)
    {
        UE_LOG(LogTemp, Log, TEXT("Toggling OFF equipped item"));
        UnequipCurrentItem();
        return true;
    }
    
    // Unequip current item first
    if (!CurrentEquippedItemID.IsNone())
    {
        UnequipCurrentItem();
    }
    
    FItemData ItemData;
    if (!GetItemData(Slot.ItemID, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("EquipQuickbarSlot: ItemData not found"));
        return false;
    }

    // ========================================================================
    // SPECIAL HANDLING: FLASHLIGHT
    // ========================================================================
    if (ItemData.ItemType == EItemType::Tool && ItemData.ToolType == EToolType::Flashlight)
    {
        if (!EquipFlashlight(ItemData, QuickbarIndex))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to equip flashlight"));
            return false;
        }
        
        return true;
    }

    // ========================================================================
    // NORMAL ITEMS: Attach to hand
    // ========================================================================
    if (!AttachItemToHand(ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to attach item"));
        return false;
    }

    // ========================================================================
    // FINALIZE EQUIPPING
    // ========================================================================
    CurrentEquippedItemID = Slot.ItemID;
    CurrentEquippedSlotIndex = QuickbarIndex;

    PlayItemSound(ItemData.PickupSound);
    OnItemEquipped.Broadcast(Slot.ItemID);
    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("Equipped '%s' from quickbar slot %d"), 
        *ItemData.ItemName.ToString(), QuickbarIndex);

    return true;
}

bool UInventoryComponent::EquipFlashlight(const FItemData& ItemData, int32 QuickbarIndex)
{
    // Validate flashlight class
    if (!FlashlightClass)
    {
        UE_LOG(LogTemp, Error, TEXT("FlashlightClass not set in InventoryComponent!"));
        return false;
    }

    // Get FlashlightComponent from owner
    const auto Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Pawn)
    {
        UE_LOG(LogTemp, Error, TEXT("Player pawn not found!"));
        return false;
    }

    UFlashlightComponent* FlashlightComp = Pawn->FindComponentByClass<UFlashlightComponent>();
    if (!FlashlightComp)
    {
        UE_LOG(LogTemp, Error, TEXT("FlashlightComponent not found on player!"));
        return false;
    }

    if (FlashlightComp->IsEquipped() || FlashlightComp->GetCurrentState() != EFlashlightState::Unequipped)
    {
        UE_LOG(LogTemp, Warning, TEXT("Forcing flashlight cleanup before re-equip"));
        FlashlightComp->UnequipFlashlight();
        
        // Wait for cleanup to complete
        FTimerHandle WaitTimer;
        GetWorld()->GetTimerManager().SetTimer(
            WaitTimer,
            [this, ItemData, QuickbarIndex]()
            {
                // Retry equip after cleanup
                EquipFlashlight(ItemData, QuickbarIndex);
            },
            0.5f,
            false
        );
        return false;
    }

    // Clean up existing flashlight if any
    if (SpawnedFlashlightActor)
    {
        SpawnedFlashlightActor->Destroy();
        SpawnedFlashlightActor = nullptr;
    }

    // Spawn flashlight actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    SpawnedFlashlightActor = GetWorld()->SpawnActor<AFlashlight>(
        FlashlightClass,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams
    );

    if (!SpawnedFlashlightActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn flashlight actor!"));
        return false;
    }

    // Configure visibility
    SpawnedFlashlightActor->SetActorHiddenInGame(false);
    SpawnedFlashlightActor->SetActorEnableCollision(false);

    // Make all components visible
    TArray<UActorComponent*> Components;
    SpawnedFlashlightActor->GetComponents(Components);
    for (UActorComponent* Comp : Components)
    {
        if (USceneComponent* SceneComp = Cast<USceneComponent>(Comp))
        {
            SceneComp->SetVisibility(true, true);
        }
    }

    // Attach to character mesh socket
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("CharacterMesh is NULL!"));
        SpawnedFlashlightActor->Destroy();
        SpawnedFlashlightActor = nullptr;
        return false;
    }

    FName SocketName = TEXT("Item");
    
    if (!CharacterMesh->DoesSocketExist(SocketName))
    {
        UE_LOG(LogTemp, Error, TEXT("Socket '%s' not found on character mesh!"), *SocketName.ToString());
        SpawnedFlashlightActor->Destroy();
        SpawnedFlashlightActor = nullptr;
        return false;
    }

    bool bAttached = SpawnedFlashlightActor->AttachToComponent(
        CharacterMesh,
        FAttachmentTransformRules(EAttachmentRule::SnapToTarget, 
                                 EAttachmentRule::SnapToTarget, 
                                 EAttachmentRule::KeepWorld, 
                                 false),
        SocketName
    );

    if (!bAttached)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to attach flashlight to socket!"));
        SpawnedFlashlightActor->Destroy();
        SpawnedFlashlightActor = nullptr;
        return false;
    }

    // Tell FlashlightComponent to equip (this starts the equip animation and state machine)
    bool bEquipSuccess = FlashlightComp->EquipFlashlight(SpawnedFlashlightActor);
    
    if (!bEquipSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("FlashlightComponent failed to equip!"));
        SpawnedFlashlightActor->Destroy();
        SpawnedFlashlightActor = nullptr;
        return false;
    }

    // Update inventory state
    CurrentEquippedItemID = ItemData.ItemID;
    CurrentEquippedSlotIndex = QuickbarIndex;

    // Broadcast events
    PlayItemSound(ItemData.PickupSound);
    OnItemEquipped.Broadcast(ItemData.ItemID);
    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("Flashlight equipped successfully from slot %d"), QuickbarIndex);
    return true;
}

void UInventoryComponent::UnequipFlashlight()
{
    const auto Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!Pawn)
    {
        UE_LOG(LogTemp, Error, TEXT("Player pawn not found!"));
        return;
    }

    UFlashlightComponent* FlashlightComp = Pawn->FindComponentByClass<UFlashlightComponent>();
    if (!FlashlightComp)
    {
        UE_LOG(LogTemp, Error, TEXT("FlashlightComponent not found!"));
        return;
    }

    // Tell FlashlightComponent to unequip (this starts the unequip animation and state machine)
    FlashlightComp->UnequipFlashlight();

    // Note: The flashlight actor will be destroyed by FlashlightComponent's cleanup
    // after the unequip animation completes, but we need to clear our reference
    // We use a timer to ensure cleanup happens after the animation
    if (GetWorld())
    {
        FTimerHandle CleanupTimer;
        GetWorld()->GetTimerManager().SetTimer(
            CleanupTimer,
            [this]()
            {
                if (SpawnedFlashlightActor)
                {
                    SpawnedFlashlightActor->Destroy();
                    SpawnedFlashlightActor = nullptr;
                }
            },
            1.0f, // Wait 1 second to ensure animation completes
            false
        );
    }

    UE_LOG(LogTemp, Log, TEXT("Flashlight unequip initiated"));
}

void UInventoryComponent::UnequipCurrentItem()
{
    if (CurrentEquippedItemID.IsNone())
    {
        UE_LOG(LogTemp, Log, TEXT("UnequipCurrentItem: Nothing equipped"));
        return;
    }

    FItemData ItemData;
    if (!GetItemData(CurrentEquippedItemID, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("UnequipCurrentItem: Failed to get item data"));
    }

    FName PreviousItemID = CurrentEquippedItemID;

    // ========================================================================
    // HANDLE FLASHLIGHT UNEQUIP
    // ========================================================================
    if (ItemData.ItemType == EItemType::Tool && ItemData.ToolType == EToolType::Flashlight)
    {
        UnequipFlashlight();
    }
    // ========================================================================
    // HANDLE OTHER ITEMS
    // ========================================================================
    else
    {
        if (CurrentAttachedItemActor)
        {
            CurrentAttachedItemActor->Destroy();
            CurrentAttachedItemActor = nullptr;
        }

        if (EquippedItemMesh)
        {
            EquippedItemMesh->DestroyComponent();
            EquippedItemMesh = nullptr;
        }
    }

    // ========================================================================
    // FINALIZE UNEQUIPPING
    // ========================================================================
    CurrentEquippedItemID = NAME_None;
    CurrentEquippedSlotIndex = -1;

    OnItemUnequipped.Broadcast(PreviousItemID);
    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("Unequipped '%s'"), *PreviousItemID.ToString());
}


// ============================================================================
// DROP ITEMS
// ============================================================================

bool UInventoryComponent::DropItem(FName ItemID, int32 Quantity)
{
    if (ItemID.IsNone() || Quantity <= 0)
    {
        return false;
    }

    if (!HasItem(ItemID, Quantity))
    {
        UE_LOG(LogTemp, Warning, TEXT("DropItem: Not enough items"));
        return false;
    }

    FItemData ItemData;
    if (!GetItemData(ItemID, ItemData))
    {
        return false;
    }
    
    if (ItemData.ItemType == EItemType::QuestItem || !ItemData.bCanBeDropped)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot drop quest/important items!"));
        PlayCannotDropSound();
        return false;
    }

    if (!ItemData.PickupActorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("DropItem: No PickupActorClass"));
        return false;
    }

    AActor* Owner = GetOwner();
    UWorld* World = Owner ? Owner->GetWorld() : nullptr;
    
    if (!Owner || !World)
    {
        return false;
    }

    // Unequip if currently equipped
    if (CurrentEquippedItemID == ItemID)
    {
        UnequipCurrentItem();
    }

    // Calculate drop location
    const FRotator OwnerRot = Owner->GetActorRotation();
    const FVector BaseLocation = Owner->GetActorLocation() + 
        OwnerRot.Vector() * 150.0f + FVector(0, 0, 50.0f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // Spawn pickup actors
    for (int32 i = 0; i < Quantity; ++i)
    {
        FVector Jitter = FVector(
            FMath::RandRange(-30.0f, 30.0f),
            FMath::RandRange(-30.0f, 30.0f),
            i * 10.0f
        );
        FVector SpawnLocation = BaseLocation + Jitter;

        AItemPickupActor* Dropped = World->SpawnActor<AItemPickupActor>(
            ItemData.PickupActorClass,
            SpawnLocation,
            FRotator::ZeroRotator,
            SpawnParams
        );

        if (Dropped)
        {
            Dropped->ItemID = ItemID;
            Dropped->ItemDataTable = ItemDataTable;
            Dropped->Quantity = 1;
            Dropped->SetItemID(ItemID);

            // Add physics impulse
            if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Dropped->GetRootComponent()))
            {
                RootPrim->SetSimulatePhysics(true);
                FVector ImpulseDir = OwnerRot.Vector() + FVector(0.0f, 0.0f, 0.5f);
                ImpulseDir.Normalize();
                RootPrim->AddImpulse(ImpulseDir * 300.0f, NAME_None, true);
            }
        }
    }

    // Remove from inventory
    RemoveItem(ItemID, Quantity);
    PlayItemSound(ItemData.UseSound);

    UE_LOG(LogTemp, Log, TEXT("Dropped %d x %s"), Quantity, *ItemData.ItemName.ToString());
    return true;
}

bool UInventoryComponent::DropEquippedItem()
{
    if (CurrentEquippedItemID.IsNone())
    {
        return false;
    }

    FName ItemToDrop = CurrentEquippedItemID;
    UnequipCurrentItem();
    return DropItem(ItemToDrop, 1);
}

// ============================================================================
// QUICKBAR MANAGEMENT - IMPROVED
// ============================================================================

bool UInventoryComponent::AssignToQuickbar(int32 InventoryIndex, int32 QuickbarIndex)
{
    if (QuickbarIndex < 0 || QuickbarIndex >= QuickbarSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("AssignToQuickbar: Invalid quickbar index %d"), QuickbarIndex);
        return false;
    }

    if (InventoryIndex < 0 || !InventorySlots.IsValidIndex(InventoryIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("AssignToQuickbar: Invalid inventory index %d"), InventoryIndex);
        return false;
    }

    if (!InventorySlots[InventoryIndex].IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("AssignToQuickbar: Empty inventory slot"));
        return false;
    }

    // Unequip if this quickbar slot was equipped
    if (CurrentEquippedSlotIndex == QuickbarIndex)
    {
        UnequipCurrentItem();
    }

    QuickbarSlotIndices[QuickbarIndex] = InventoryIndex;
    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("Assigned inventory[%d] to quickbar[%d]"), InventoryIndex, QuickbarIndex);
    return true;
}

bool UInventoryComponent::RemoveFromQuickbar(int32 QuickbarIndex)
{
    if (QuickbarIndex < 0 || QuickbarIndex >= QuickbarSlotIndices.Num())
    {
        return false;
    }

    // Unequip if this slot is equipped
    if (CurrentEquippedSlotIndex == QuickbarIndex)
    {
        UnequipCurrentItem();
    }

    QuickbarSlotIndices[QuickbarIndex] = -1;
    OnInventoryUpdated.Broadcast();
    
    UE_LOG(LogTemp, Log, TEXT("Removed quickbar slot %d"), QuickbarIndex);
    return true;
}

FInventorySlot UInventoryComponent::GetQuickbarSlot(int32 Index) const
{
    if (Index < 0 || Index >= QuickbarSlotIndices.Num())
    {
        return FInventorySlot();
    }

    int32 InventoryIndex = QuickbarSlotIndices[Index];
    
    if (InventoryIndex < 0 || !InventorySlots.IsValidIndex(InventoryIndex))
    {
        return FInventorySlot();
    }

    return InventorySlots[InventoryIndex];
}

int32 UInventoryComponent::GetQuickbarInventoryIndex(int32 QuickbarIndex) const
{
    if (QuickbarIndex >= 0 && QuickbarIndex < QuickbarSlotIndices.Num())
    {
        return QuickbarSlotIndices[QuickbarIndex];
    }
    return -1;
}

// ============================================================================
// DRAG & DROP - IMPROVED
// ============================================================================

bool UInventoryComponent::SwapInventorySlots(int32 SlotA, int32 SlotB)
{
    if (SlotA < 0 || SlotA >= InventorySlots.Num() ||
        SlotB < 0 || SlotB >= InventorySlots.Num() ||
        SlotA == SlotB)
    {
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("SwapInventorySlots: %d ↔ %d"), SlotA, SlotB);

    // Swap slots
    FInventorySlot TempA = InventorySlots[SlotA];
    InventorySlots[SlotA] = InventorySlots[SlotB];
    InventorySlots[SlotB] = TempA;

    // Update quickbar references
    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        if (QuickbarSlotIndices[i] == SlotA)
        {
            QuickbarSlotIndices[i] = SlotB;
        }
        else if (QuickbarSlotIndices[i] == SlotB)
        {
            QuickbarSlotIndices[i] = SlotA;
        }
    }

    OnInventoryUpdated.Broadcast();
    return true;
}

bool UInventoryComponent::MoveItemToSlot(int32 SourceIndex, int32 TargetIndex)
{
    if (!InventorySlots.IsValidIndex(SourceIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("MoveItemToSlot: Invalid source index %d"), SourceIndex);
        return false;
    }

    if (TargetIndex < 0 || TargetIndex >= MaxInventorySlots)
    {
        UE_LOG(LogTemp, Error, TEXT("MoveItemToSlot: Target index %d out of bounds (max: %d)"), 
            TargetIndex, MaxInventorySlots);
        return false;
    }

    if (SourceIndex == TargetIndex)
    {
        return true;
    }

    UE_LOG(LogTemp, Log, TEXT("MoveItemToSlot: %d → %d"), SourceIndex, TargetIndex);

    if (TargetIndex >= InventorySlots.Num())
    {
        int32 SlotsToAdd = (TargetIndex - InventorySlots.Num()) + 1;
        for (int32 i = 0; i < SlotsToAdd; i++)
        {
            InventorySlots.Add(FInventorySlot());
        }
        UE_LOG(LogTemp, Log, TEXT("Expanded array to %d slots"), InventorySlots.Num());
    }

    if (InventorySlots[TargetIndex].IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Target slot occupied! Use SwapInventorySlots instead"));
        return SwapInventorySlots(SourceIndex, TargetIndex);
    }

    InventorySlots[TargetIndex] = InventorySlots[SourceIndex];
    InventorySlots[SourceIndex] = FInventorySlot();

    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        if (QuickbarSlotIndices[i] == SourceIndex)
        {
            QuickbarSlotIndices[i] = TargetIndex;
            UE_LOG(LogTemp, Log, TEXT("  → Updated quickbar[%d]: %d → %d"), i, SourceIndex, TargetIndex);
        }
    }

    OnInventoryUpdated.Broadcast();
    return true;
}

bool UInventoryComponent::SwapQuickbarSlots(int32 SlotA, int32 SlotB)
{
    if (SlotA < 0 || SlotA >= QuickbarSlotIndices.Num() ||
        SlotB < 0 || SlotB >= QuickbarSlotIndices.Num() ||
        SlotA == SlotB)
    {
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("SwapQuickbarSlots: %d ↔ %d"), SlotA, SlotB);

    // Swap indices
    int32 TempIndex = QuickbarSlotIndices[SlotA];
    QuickbarSlotIndices[SlotA] = QuickbarSlotIndices[SlotB];
    QuickbarSlotIndices[SlotB] = TempIndex;

    // Update equipped slot index if needed
    if (CurrentEquippedSlotIndex == SlotA)
    {
        CurrentEquippedSlotIndex = SlotB;
    }
    else if (CurrentEquippedSlotIndex == SlotB)
    {
        CurrentEquippedSlotIndex = SlotA;
    }

    OnInventoryUpdated.Broadcast();
    return true;
}

bool UInventoryComponent::MoveInventoryToQuickbar(int32 InventoryIndex, int32 QuickbarIndex)
{
    return AssignToQuickbar(InventoryIndex, QuickbarIndex);
}

bool UInventoryComponent::RemoveQuickbarToInventory(int32 QuickbarIndex)
{
    return RemoveFromQuickbar(QuickbarIndex);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

bool UInventoryComponent::HasItem(FName ItemID, int32 Quantity) const
{
    return GetItemQuantity(ItemID) >= Quantity;
}

int32 UInventoryComponent::GetItemQuantity(FName ItemID) const
{
    int32 TotalQuantity = 0;
    for (const FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.ItemID == ItemID)
        {
            TotalQuantity += Slot.Quantity;
        }
    }
    return TotalQuantity;
}

bool UInventoryComponent::GetItemData(FName ItemID, FItemData& OutItemData) const
{
    if (!ItemDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("GetItemData: ItemDataTable is not set!"));
        return false;
    }

    FItemData* Data = ItemDataTable->FindRow<FItemData>(ItemID, TEXT("GetItemData"));
    if (Data)
    {
        OutItemData = *Data;
        return true;
    }

    return false;
}

FInventorySlot* UInventoryComponent::FindItemSlot(FName ItemID)
{
    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.ItemID == ItemID)
        {
            return &Slot;
        }
    }
    return nullptr;
}

int32 UInventoryComponent::FindInventorySlotByItemID(const FName& ItemID) const
{
    for (int32 i = 0; i < InventorySlots.Num(); i++)
    {
        if (InventorySlots[i].IsValid() && InventorySlots[i].ItemID == ItemID)
        {
            return i;
        }
    }
    return -1;
}

bool UInventoryComponent::IsInventoryFull() const
{
    return InventorySlots.Num() >= MaxInventorySlots;
}

FName UInventoryComponent::GetCurrentEquippedItemID() const
{
    return CurrentEquippedItemID;
}

bool UInventoryComponent::IsItemEquipped() const
{
    return !CurrentEquippedItemID.IsNone();
}

bool UInventoryComponent::GetEquippedItem(FItemData& OutItemData) const
{
    if (!CurrentEquippedItemID.IsNone())
    {
        return GetItemData(CurrentEquippedItemID, OutItemData);
    }
    return false;
}

void UInventoryComponent::ClearInventory()
{
    if (!CurrentEquippedItemID.IsNone())
    {
        UnequipCurrentItem();
    }

    CleanupSpawnedActors();
    
    InventorySlots.Empty();
    QuickbarSlotIndices.Init(-1, QuickbarSize);
    
    OnInventoryUpdated.Broadcast();
    
    UE_LOG(LogTemp, Log, TEXT("Inventory cleared"));
}

void UInventoryComponent::PrintInventory()
{
    UE_LOG(LogTemp, Log, TEXT("========== INVENTORY =========="));
    UE_LOG(LogTemp, Log, TEXT("Slots: %d/%d"), InventorySlots.Num(), MaxInventorySlots);
    
    for (int32 i = 0; i < InventorySlots.Num(); i++)
    {
        const FInventorySlot& Slot = InventorySlots[i];
        FItemData ItemData;
        if (GetItemData(Slot.ItemID, ItemData))
        {
            UE_LOG(LogTemp, Log, TEXT("  [%d] %s x%d"), i, *ItemData.ItemName.ToString(), Slot.Quantity);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT(""));
    UE_LOG(LogTemp, Log, TEXT("Quickbar:"));
    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        int32 InvIndex = QuickbarSlotIndices[i];
        if (InvIndex >= 0 && InventorySlots.IsValidIndex(InvIndex))
        {
            FItemData ItemData;
            if (GetItemData(InventorySlots[InvIndex].ItemID, ItemData))
            {
                UE_LOG(LogTemp, Log, TEXT("  [%d] → Inv[%d] %s"), i, InvIndex, *ItemData.ItemName.ToString());
            }
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("  [%d] → EMPTY"), i);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT(""));
    UE_LOG(LogTemp, Log, TEXT("Equipped: %s (Slot %d)"), 
        CurrentEquippedItemID.IsNone() ? TEXT("None") : *CurrentEquippedItemID.ToString(),
        CurrentEquippedSlotIndex);
    UE_LOG(LogTemp, Log, TEXT("==============================="));
}

// ============================================================================
// ITEM EFFECTS
// ============================================================================

bool UInventoryComponent::ApplyItemEffect(const FItemData& ItemData)
{
    if (ItemData.ItemType != EItemType::Consumable)
    {
        return false;
    }

    bool bEffectApplied = false;

    // Restore sanity
    if (ItemData.SanityRestoreAmount > 0.0f)
    {
        if (AActor* Owner = GetOwner())
        {
            if (USanityComponent* Sanity = Owner->FindComponentByClass<USanityComponent>())
            {
                Sanity->RestoreSanity(ItemData.SanityRestoreAmount);
                bEffectApplied = true;
                UE_LOG(LogTemp, Log, TEXT("  → Restored %.1f sanity"), ItemData.SanityRestoreAmount);
            }
        }
    }

    return bEffectApplied;
}

float UInventoryComponent::GetPassiveSanityDrainReduction() const
{
    float TotalReduction = 0.0f;

    for (const FInventorySlot& Slot : InventorySlots)
    {
        FItemData ItemData;
        if (GetItemData(Slot.ItemID, ItemData))
        {
            if (ItemData.PassiveSanityDrainReduction > 0.0f)
            {
                TotalReduction += ItemData.PassiveSanityDrainReduction;
            }
        }
    }

    return TotalReduction;
}

// ============================================================================
// SPAWNING & ATTACHMENT
// ============================================================================

bool UInventoryComponent::AttachItemToHand(const FItemData& ItemData)
{
   if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("AttachItemToHand: No character mesh!"));
        return false;
    }

    FName SocketName = TEXT("Item");
    
    if (!CharacterMesh->DoesSocketExist(SocketName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Socket '%s' not found!"), *SocketName.ToString());
        return false;
    }

    // ========================================================================
    // FLASHLIGHT: Should not reach here - handled by EquipFlashlight()
    // ========================================================================
    if (ItemData.ItemType == EItemType::Tool && ItemData.ToolType == EToolType::Flashlight)
    {
        UE_LOG(LogTemp, Error, TEXT("AttachItemToHand: Flashlight should use EquipFlashlight() instead!"));
        return false;
    }

    // ========================================================================
    // NORMAL ITEMS: Static mesh component
    // ========================================================================
    if (!ItemData.ItemMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("AttachItemToHand: Item has no mesh"));
        return true;
    }

    // Destroy existing mesh
    if (EquippedItemMesh)
    {
        EquippedItemMesh->DestroyComponent();
        EquippedItemMesh = nullptr;
    }

    // Create new static mesh component
    EquippedItemMesh = NewObject<UStaticMeshComponent>(GetOwner());
    if (!EquippedItemMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create mesh component"));
        return false;
    }

    EquippedItemMesh->RegisterComponent();
    EquippedItemMesh->SetStaticMesh(ItemData.ItemMesh);
    EquippedItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    EquippedItemMesh->AttachToComponent(
        CharacterMesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        SocketName
    );

    UE_LOG(LogTemp, Log, TEXT("Attached '%s' to socket '%s'"), 
        *ItemData.ItemName.ToString(), *SocketName.ToString());
    
    return true;
}

void UInventoryComponent::CleanupSpawnedActors()
{
    if (SpawnedFlashlightActor)
    {
        SpawnedFlashlightActor->Destroy();
        SpawnedFlashlightActor = nullptr;
    }

    if (CurrentAttachedItemActor)
    {
        CurrentAttachedItemActor->Destroy();
        CurrentAttachedItemActor = nullptr;
    }

    if (EquippedItemMesh)
    {
        EquippedItemMesh->DestroyComponent();
        EquippedItemMesh = nullptr;
    }
}
// ============================================================================
// AUTO-ASSIGN & VALIDATION - IMPROVED VERSION
// ============================================================================

void UInventoryComponent::TryAutoAssignToQuickbar(FName ItemID, int32 PreferredInventoryIndex)
{
    FItemData ItemData;
    if (!GetItemData(ItemID, ItemData))
    {
        return;
    }

    // Only auto-assign tools and consumables
    if (ItemData.ItemType != EItemType::Tool && ItemData.ItemType != EItemType::Consumable)
    {
        UE_LOG(LogTemp, Log, TEXT("Skipping auto-assign (not tool/consumable)"));
        return;
    }

    // Validate preferred index
    if (PreferredInventoryIndex < 0 || !InventorySlots.IsValidIndex(PreferredInventoryIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("  → Invalid PreferredInventoryIndex: %d"), PreferredInventoryIndex);
        return;
    }

    // Check if this SPECIFIC slot is already in quickbar
    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        if (QuickbarSlotIndices[i] == PreferredInventoryIndex)
        {
            UE_LOG(LogTemp, Log, TEXT("  → Inventory slot %d already in quickbar[%d]"), 
                PreferredInventoryIndex, i);
            return;
        }
    }

    // Check if the SAME ITEM TYPE is already in quickbar (optional - comment out if you want duplicates)
    bool bSameItemAlreadyInQuickbar = false;
    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        int32 QuickbarInvIndex = QuickbarSlotIndices[i];
        if (QuickbarInvIndex >= 0 && InventorySlots.IsValidIndex(QuickbarInvIndex))
        {
            if (InventorySlots[QuickbarInvIndex].ItemID == ItemID)
            {
                bSameItemAlreadyInQuickbar = true;
                UE_LOG(LogTemp, Log, TEXT("  → Same item type already in quickbar[%d]"), i);
                break;
            }
        }
    }

    // If same item exists in quickbar, don't auto-assign another stack
    if (bSameItemAlreadyInQuickbar)
    {
        return;
    }

    // Find first empty quickbar slot
    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        if (QuickbarSlotIndices[i] < 0)
        {
            AssignToQuickbar(PreferredInventoryIndex, i);
            UE_LOG(LogTemp, Log, TEXT("  → Auto-assigned inventory[%d] to quickbar[%d]"), 
                PreferredInventoryIndex, i);
            return;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("  → Quickbar full, cannot auto-assign"));
}

void UInventoryComponent::ValidateQuickbarReferences()
{
    bool bNeedUpdate = false;

    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        int32 InvIndex = QuickbarSlotIndices[i];
        
        if (InvIndex < 0)
        {
            continue; // Empty slot is valid
        }

        // Check if index is out of bounds
        if (InvIndex >= InventorySlots.Num())
        {
            UE_LOG(LogTemp, Warning, TEXT("⚠ Quickbar[%d] has invalid index %d (out of bounds), clearing"), 
                i, InvIndex);
            QuickbarSlotIndices[i] = -1;
            bNeedUpdate = true;

            // Unequip if this was equipped
            if (CurrentEquippedSlotIndex == i)
            {
                UnequipCurrentItem();
            }
            continue;
        }

        // Check if slot is empty/invalid
        if (!InventorySlots[InvIndex].IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("⚠ Quickbar[%d] points to empty slot %d, clearing"), 
                i, InvIndex);
            QuickbarSlotIndices[i] = -1;
            bNeedUpdate = true;

            // Unequip if this was equipped
            if (CurrentEquippedSlotIndex == i)
            {
                UnequipCurrentItem();
            }
        }
    }

    if (bNeedUpdate)
    {
        OnInventoryUpdated.Broadcast();
    }
}

// ============================================================================
// ADDITIONAL HELPER FUNCTIONS
// ============================================================================

bool UInventoryComponent::IsQuickbarFull() const
{
    for (int32 Index : QuickbarSlotIndices)
    {
        if (Index < 0)
        {
            return false; // Found empty slot
        }
    }
    return true;
}

int32 UInventoryComponent::GetFirstEmptyQuickbarSlot() const
{
    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        if (QuickbarSlotIndices[i] < 0)
        {
            return i;
        }
    }
    return -1;
}

int32 UInventoryComponent::FindQuickbarSlotByInventoryIndex(int32 InventoryIndex) const
{
    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        if (QuickbarSlotIndices[i] == InventoryIndex)
        {
            return i;
        }
    }
    return -1;
}

bool UInventoryComponent::IsItemInQuickbar(FName ItemID) const
{
    for (int32 QuickbarIndex : QuickbarSlotIndices)
    {
        if (QuickbarIndex >= 0 && InventorySlots.IsValidIndex(QuickbarIndex))
        {
            if (InventorySlots[QuickbarIndex].ItemID == ItemID)
            {
                return true;
            }
        }
    }
    return false;
}

void UInventoryComponent::CompactInventory()
{
    // Remove all empty slots
    for (int32 i = InventorySlots.Num() - 1; i >= 0; i--)
    {
        if (!InventorySlots[i].IsValid())
        {
            RemoveSlotAndUpdateReferences(i);
        }
    }

    ValidateQuickbarReferences();
    OnInventoryUpdated.Broadcast();
    
    UE_LOG(LogTemp, Log, TEXT("Inventory compacted: %d slots"), InventorySlots.Num());
}

void UInventoryComponent::SortInventoryByType()
{
    if (InventorySlots.Num() <= 1)
    {
        return;
    }

    // Create copy with item data
    struct FSortableSlot
    {
        FInventorySlot Slot;
        EItemType Type;
        FText ItemName;
        int32 OriginalIndex;
    };

    TArray<FSortableSlot> SortableSlots;
    for (int32 i = 0; i < InventorySlots.Num(); i++)
    {
        if (InventorySlots[i].IsValid())
        {
            FSortableSlot SortSlot;
            SortSlot.Slot = InventorySlots[i];
            SortSlot.OriginalIndex = i;

            FItemData ItemData;
            if (GetItemData(InventorySlots[i].ItemID, ItemData))
            {
                SortSlot.Type = ItemData.ItemType;
                SortSlot.ItemName = ItemData.ItemName;
            }

            SortableSlots.Add(SortSlot);
        }
    }

    // Sort by type, then by name
    SortableSlots.Sort([](const FSortableSlot& A, const FSortableSlot& B)
    {
        if (A.Type != B.Type)
        {
            return A.Type < B.Type;
        }
        return A.ItemName.ToString() < B.ItemName.ToString();
    });

    // Build mapping from old to new indices
    TMap<int32, int32> IndexMapping;
    for (int32 i = 0; i < SortableSlots.Num(); i++)
    {
        IndexMapping.Add(SortableSlots[i].OriginalIndex, i);
    }

    // Rebuild inventory
    TArray<FInventorySlot> NewInventory;
    for (const FSortableSlot& SortSlot : SortableSlots)
    {
        NewInventory.Add(SortSlot.Slot);
    }
    InventorySlots = NewInventory;

    // Update quickbar references
    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        int32 OldIndex = QuickbarSlotIndices[i];
        if (OldIndex >= 0 && IndexMapping.Contains(OldIndex))
        {
            QuickbarSlotIndices[i] = IndexMapping[OldIndex];
        }
        else
        {
            QuickbarSlotIndices[i] = -1;
        }
    }

    ValidateQuickbarReferences();
    OnInventoryUpdated.Broadcast();
    
    UE_LOG(LogTemp, Log, TEXT("Inventory sorted by type"));
}

// ============================================================================
// AUDIO - Keep original implementations
// ============================================================================

void UInventoryComponent::PlayItemSound(USoundBase* Sound)
{
    if (Sound && GetOwner())
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            Sound,
            GetOwner()->GetActorLocation(),
            1.0f,
            FMath::RandRange(0.95f, 1.05f)
        );
    }
}

void UInventoryComponent::PlayInventoryFullSound()
{
    if (InventoryFullSound)
    {
        UGameplayStatics::PlaySound2D(this, InventoryFullSound);
    }
}

void UInventoryComponent::PlayItemBreakSound()
{
    if (ItemBreakSound)
    {
        UGameplayStatics::PlaySound2D(this, ItemBreakSound);
    }
}

void UInventoryComponent::PlayCannotDropSound()
{
    if (CannotDropSound)
    {
        UGameplayStatics::PlaySound2D(this, CannotDropSound);
    }
}

// ============================================================================
// SAVE/LOAD - Keep original implementations
// ============================================================================

TArray<FInventorySlot> UInventoryComponent::GetAllInventorySlots() const
{
    return InventorySlots;
}

void UInventoryComponent::LoadInventorySlots(const TArray<FInventorySlot>& SavedSlots)
{
    ClearInventory();
    
    for (const FInventorySlot& Slot : SavedSlots)
    {
        if (Slot.IsValid())
        {
            AddItem(Slot.ItemID, Slot.Quantity);
            
            FInventorySlot* NewSlot = FindItemSlot(Slot.ItemID);
            if (NewSlot)
            {
                NewSlot->RemainingUses = Slot.RemainingUses;
            }
        }
    }
    
    OnInventoryUpdated.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("Loaded %d items"), SavedSlots.Num());
}

void UInventoryComponent::SaveQuickbarSetup(TArray<int32>& OutQuickbarIndices) const
{
    OutQuickbarIndices = QuickbarSlotIndices;
}

void UInventoryComponent::LoadQuickbarSetup(const TArray<int32>& SavedQuickbar)
{
    if (SavedQuickbar.Num() != QuickbarSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠ LoadQuickbarSetup: Size mismatch"));
        return;
    }
    
    QuickbarSlotIndices = SavedQuickbar;
    ValidateQuickbarReferences();
    
    OnInventoryUpdated.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("Quickbar setup loaded"));
}

// ============================================================================
// DEBUG FUNCTIONS
// ============================================================================

void UInventoryComponent::GiveAllItems()
{
    if (!ItemDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("GiveAllItems: No ItemDataTable!"));
        return;
    }

    TArray<FName> RowNames = ItemDataTable->GetRowNames();
    
    for (const FName& RowName : RowNames)
    {
        FItemData* ItemData = ItemDataTable->FindRow<FItemData>(RowName, TEXT("GiveAllItems"));
        if (ItemData)
        {
            AddItem(RowName, ItemData->MaxStackSize);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("GiveAllItems: Added %d item types"), RowNames.Num());
}

void UInventoryComponent::RemoveAllItems()
{
    ClearInventory();
    UE_LOG(LogTemp, Log, TEXT("RemoveAllItems: Inventory cleared"));
}

void UInventoryComponent::DebugPrintQuickbarState() const
{
    UE_LOG(LogTemp, Log, TEXT("========== QUICKBAR STATE =========="));
    
    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        int32 InvIndex = QuickbarSlotIndices[i];
        
        if (InvIndex < 0)
        {
            UE_LOG(LogTemp, Log, TEXT("  [%d] → EMPTY"), i);
        }
        else if (!InventorySlots.IsValidIndex(InvIndex))
        {
            UE_LOG(LogTemp, Warning, TEXT("  [%d] → INVALID INDEX %d"), i, InvIndex);
        }
        else if (!InventorySlots[InvIndex].IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("  [%d] → EMPTY SLOT AT INDEX %d"), i, InvIndex);
        }
        else
        {
            FItemData ItemData;
            if (GetItemData(InventorySlots[InvIndex].ItemID, ItemData))
            {
                FString EquipMarker = (CurrentEquippedSlotIndex == i) ? TEXT(" [EQUIPPED]") : TEXT("");
                UE_LOG(LogTemp, Log, TEXT("  [%d] → Inv[%d] %s x%d%s"), 
                    i, 
                    InvIndex, 
                    *ItemData.ItemName.ToString(),
                    InventorySlots[InvIndex].Quantity,
                    *EquipMarker);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("===================================="));
}

void UInventoryComponent::ValidateInventoryIntegrity()
{
    UE_LOG(LogTemp, Log, TEXT("========== VALIDATING INVENTORY =========="));
    
    int32 ErrorCount = 0;
    
    // Check for invalid slots
    for (int32 i = 0; i < InventorySlots.Num(); i++)
    {
        if (!InventorySlots[i].IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid slot at index %d"), i);
            ErrorCount++;
        }
        else
        {
            FItemData ItemData;
            if (!GetItemData(InventorySlots[i].ItemID, ItemData))
            {
                UE_LOG(LogTemp, Error, TEXT("Unknown ItemID '%s' at index %d"), 
                    *InventorySlots[i].ItemID.ToString(), i);
                ErrorCount++;
            }
        }
    }
    
    // Check quickbar references
    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        int32 InvIndex = QuickbarSlotIndices[i];
        
        if (InvIndex >= 0)
        {
            if (!InventorySlots.IsValidIndex(InvIndex))
            {
                UE_LOG(LogTemp, Error, TEXT("Quickbar[%d] references invalid index %d"), i, InvIndex);
                ErrorCount++;
            }
            else if (!InventorySlots[InvIndex].IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("Quickbar[%d] references empty slot %d"), i, InvIndex);
                ErrorCount++;
            }
        }
    }
    
    // Check equipped item
    if (!CurrentEquippedItemID.IsNone())
    {
        if (CurrentEquippedSlotIndex < 0 || CurrentEquippedSlotIndex >= QuickbarSlotIndices.Num())
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid CurrentEquippedSlotIndex: %d"), CurrentEquippedSlotIndex);
            ErrorCount++;
        }
        else
        {
            int32 InvIndex = QuickbarSlotIndices[CurrentEquippedSlotIndex];
            if (InvIndex < 0 || !InventorySlots.IsValidIndex(InvIndex))
            {
                UE_LOG(LogTemp, Error, TEXT("Equipped slot %d references invalid inventory index"), 
                    CurrentEquippedSlotIndex);
                ErrorCount++;
            }
            else if (InventorySlots[InvIndex].ItemID != CurrentEquippedItemID)
            {
                UE_LOG(LogTemp, Error, TEXT("Equipped ItemID mismatch: %s vs %s"), 
                    *CurrentEquippedItemID.ToString(), 
                    *InventorySlots[InvIndex].ItemID.ToString());
                ErrorCount++;
            }
        }
    }
    
    if (ErrorCount == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Inventory integrity check PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Inventory integrity check FAILED - %d errors found"), ErrorCount);
    }
    
    UE_LOG(LogTemp, Log, TEXT("=========================================="));
}