
#include "InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EscapeIT/Actor/ItemPickupActor.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "EscapeIT/Actor/Components/SanityComponent.h"
#include "EscapeIT/Actor/Components/FlashlightComponent.h"
#include "EscapeIT/Actor/Item/Flashlight.h"
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
// ADD/REMOVE ITEMS
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

    // ========================================================================
    // STEP 1: Try stacking with existing items
    // ========================================================================
    if (ItemData.MaxStackSize > 1)
    {
        for (FInventorySlot& Slot : InventorySlots)
        {
            if (Slot.ItemID == ItemID && Slot.Quantity < ItemData.MaxStackSize)
            {
                int32 SpaceAvailable = ItemData.MaxStackSize - Slot.Quantity;
                int32 AmountToAdd = FMath::Min(SpaceAvailable, RemainingQuantity);

                Slot.Quantity += AmountToAdd;
                RemainingQuantity -= AmountToAdd;

                UE_LOG(LogTemp, Log, TEXT("  → Stacked %d into existing slot (now %d)"), AmountToAdd, Slot.Quantity);

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

            // Partial add
            if (RemainingQuantity < Quantity)
            {
                OnInventoryUpdated.Broadcast();
                OnItemAdded.Broadcast(ItemID, Quantity - RemainingQuantity);
            }
            return false;
        }

        int32 AmountToAdd = FMath::Min(RemainingQuantity, ItemData.MaxStackSize);
        FInventorySlot NewSlot(ItemID, AmountToAdd);

        InventorySlots.Add(NewSlot);
        RemainingQuantity -= AmountToAdd;

        UE_LOG(LogTemp, Log, TEXT("  → Created new slot with %d items"), AmountToAdd);
    }

    // ========================================================================
    // STEP 3: Play sound & broadcast events
    // ========================================================================
    PlayItemSound(ItemData.PickupSound);
    OnInventoryUpdated.Broadcast();
    OnItemAdded.Broadcast(ItemID, Quantity);

    // ========================================================================
    // STEP 4: Auto-assign to quickbar (if tool/consumable)
    // ========================================================================
    TryAutoAssignToQuickbar(ItemID);

    UE_LOG(LogTemp, Log, TEXT("Added %d x %s [%d/%d slots]"), 
        Quantity, *ItemData.ItemName.ToString(), InventorySlots.Num(), MaxInventorySlots);
    
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

    // Remove from inventory (reverse iteration for safe removal)
    for (int32 i = InventorySlots.Num() - 1; i >= 0 && RemainingToRemove > 0; i--)
    {
        if (InventorySlots[i].ItemID == ItemID)
        {
            int32 AmountToRemove = FMath::Min(InventorySlots[i].Quantity, RemainingToRemove);
            InventorySlots[i].Quantity -= AmountToRemove;
            RemainingToRemove -= AmountToRemove;

            if (InventorySlots[i].Quantity <= 0)
            {
                // Update quickbar references BEFORE removing slot
                for (int32 q = 0; q < QuickbarSlotIndices.Num(); q++)
                {
                    if (QuickbarSlotIndices[q] == i)
                    {
                        // Item removed, clear quickbar slot
                        QuickbarSlotIndices[q] = -1;
                        
                        // If this was equipped, unequip it
                        if (CurrentEquippedSlotIndex == q)
                        {
                            UnequipCurrentItem();
                        }
                    }
                    else if (QuickbarSlotIndices[q] > i)
                    {
                        // Adjust indices after removed slot
                        QuickbarSlotIndices[q]--;
                    }
                }

                InventorySlots.RemoveAt(i);
                UE_LOG(LogTemp, Log, TEXT("  → Removed slot at index %d"), i);
            }
        }
    }

    // Validate quickbar references
    ValidateQuickbarReferences();

    OnInventoryUpdated.Broadcast();
    OnItemRemoved.Broadcast(ItemID, Quantity);

    UE_LOG(LogTemp, Log, TEXT("Removed %d x %s"), Quantity, *ItemID.ToString());
    return true;
}

// ============================================================================
// USE ITEMS
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
    
    const auto Pawn = UGameplayStatics::GetPlayerPawn(this,0);

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
            UE_LOG(LogTemp, Warning, TEXT("UseItem: Flashlight not equipped!"));
            OnItemUsed.Broadcast(ItemID, false);
            return false;
        }
        if (FlashlightComp->CurrentBattery < 100.0f)
        {
            FlashlightComp->AddBatteryCharge(ItemData.BatteryChargePercent);
        }
        else
        {
            return false;
        }

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

        // Apply effect
        bool bEffectApplied = ApplyItemEffect(ItemData);
        PlayItemSound(ItemData.UseSound);
        OnItemUsed.Broadcast(ItemID, bEffectApplied);

        // Remove from inventory
        if (CurrentEquippedItemID == ItemID)
        {
            UnequipCurrentItem();
        }
        
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
// EQUIP/UNEQUIP
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
    
    if (CurrentEquippedItemID == Slot.ItemID && CurrentEquippedSlotIndex == QuickbarIndex)
    {
        UE_LOG(LogTemp, Log, TEXT("Toggling OFF equipped item"));
        UnequipCurrentItem();
        return true;
    }
    
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
        const auto Pawn = UGameplayStatics::GetPlayerPawn(this,0);
        UFlashlightComponent* FlashlightComp = Pawn->FindComponentByClass<UFlashlightComponent>();
        
        if (!FlashlightComp)
        {
            UE_LOG(LogTemp, Error, TEXT("FlashlightComponent not found on character!"));
            return false;
        }
        
        if (!SpawnFlashlightActor(ItemData))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn flashlight visual actor!"));
            return false;
        }
        
        if (SpawnedFlashlightActor && SpawnedFlashlightActor->GetSpotLight())
        {
            FlashlightComp->InitializeFlashlight(SpawnedFlashlightActor, SpawnedFlashlightActor->SpotLightComponent);
            FlashlightComp->SetEquipped(true);
            
            UE_LOG(LogTemp, Log, TEXT("FlashlightComponent connected to spawned flashlight"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("SpawnedFlashlightActor or SpotLight is NULL!"));
            return false;
        }
    }
    // ========================================================================
    // NORMAL ITEMS: Attach visual mesh to hand
    // ========================================================================
    else
    {
        if (!AttachItemToHand(ItemData))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to attach item to hand"));
            return false;
        }
    }

    // ========================================================================
    // FINALIZE EQUIPPING
    // ========================================================================
    CurrentEquippedItemID = Slot.ItemID;
    CurrentEquippedSlotIndex = QuickbarIndex;

    PlayItemSound(ItemData.PickupSound);

    // Broadcast events
    OnItemEquipped.Broadcast(Slot.ItemID);
    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("Equipped '%s' from quickbar slot %d"), 
        *ItemData.ItemName.ToString(), QuickbarIndex);

    return true;
}

void UInventoryComponent::UnequipCurrentItem()
{
    if (CurrentEquippedItemID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("  → No item to unequip"));
        return;
    }

    FItemData ItemData;
    if (!GetItemData(CurrentEquippedItemID, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("UnequipCurrentItem: Failed to get item data"));
        return;
    }

    FName PreviousItemID = CurrentEquippedItemID;

    // ========================================================================
    // HANDLE FLASHLIGHT UNEQUIP
    // ========================================================================
    if (ItemData.ItemType == EItemType::Tool && ItemData.ToolType == EToolType::Flashlight)
    {
        const auto Pawn = UGameplayStatics::GetPlayerPawn(this,0);
        
        if (UFlashlightComponent* FlashlightComp = Pawn->FindComponentByClass<UFlashlightComponent>())
        {
            FlashlightComp->SetLightEnabled(false);
            FlashlightComp->SetEquipped(false);
            
            UE_LOG(LogTemp, Log, TEXT("  → FlashlightComponent unequipped"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("FlashlightComponent not found during unequip"));
        }
        
        if (SpawnedFlashlightActor)
        {
            SpawnedFlashlightActor->Destroy();
            SpawnedFlashlightActor = nullptr;
            UE_LOG(LogTemp, Log, TEXT("  → Flashlight visual actor destroyed"));
        }
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

        // Destroy visual mesh
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
// QUICKBAR MANAGEMENT (IMPROVED)
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
// DRAG & DROP (IMPROVED)
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
    if (!InventorySlots.IsValidIndex(SourceIndex) || !InventorySlots.IsValidIndex(TargetIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("MoveItemToSlot: Invalid indices"));
        return false;
    }

    if (SourceIndex == TargetIndex)
    {
        return true;
    }

    if (InventorySlots[TargetIndex].IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Target slot not empty! Use SwapInventorySlots instead"));
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("MoveItemToSlot: %d → %d"), SourceIndex, TargetIndex);

    // Move item
    InventorySlots[TargetIndex] = InventorySlots[SourceIndex];
    InventorySlots[SourceIndex] = FInventorySlot();

    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        if (QuickbarSlotIndices[i] == SourceIndex)
        {
            QuickbarSlotIndices[i] = TargetIndex;
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

    if (!ItemData.ItemMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠ AttachItemToHand: Item has no mesh"));
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

    // Attach to hand socket
    FName SocketName = TEXT("Item");

    if (!CharacterMesh->DoesSocketExist(SocketName))
    {
        UE_LOG(LogTemp, Warning, TEXT("⚠ Socket '%s' not found, attaching to root"), *SocketName.ToString());
        SocketName = NAME_None;
    }

    EquippedItemMesh->AttachToComponent(
        CharacterMesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        SocketName
    );

    UE_LOG(LogTemp, Log, TEXT("  → Attached '%s' to hand"), *ItemData.ItemName.ToString());
    return true;
}

bool UInventoryComponent::SpawnFlashlightActor(const FItemData& ItemData)
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("Owner is not a character!"));
        return false;
    }

    if (!FlashlightClass)
    {
        UE_LOG(LogTemp, Error, TEXT("FlashlightClass not set in Blueprint!"));
        return false;
    }
    
    // Destroy existing flashlight
    if (SpawnedFlashlightActor)
    {
        SpawnedFlashlightActor->Destroy();
        SpawnedFlashlightActor = nullptr;
    }

    // Spawn new flashlight
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Character;
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
    
    // Make sure actor is visible
    SpawnedFlashlightActor->SetActorHiddenInGame(false);
    SpawnedFlashlightActor->SetActorEnableCollision(false); // No collision when equipped
    
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
    
    // Attach to socket
    FName SocketName = FName("Item");

    if (CharacterMesh && CharacterMesh->DoesSocketExist(SocketName))
    {
        bool bAttached = SpawnedFlashlightActor->AttachToComponent(
            CharacterMesh,
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            SocketName
        );
        
        if (bAttached)
        {
            UE_LOG(LogTemp, Log, TEXT("Flashlight attached to socket '%s'"), *SocketName.ToString());
            UE_LOG(LogTemp, Log, TEXT("   Location: %s"), 
                *SpawnedFlashlightActor->GetActorLocation().ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to attach to socket!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Socket '%s' not found!"), *SocketName.ToString());
        
        // Try alternative attachment
        SpawnedFlashlightActor->AttachToActor(
            Character,
            FAttachmentTransformRules::KeepRelativeTransform
        );
        
        // Manually position near hand
        SpawnedFlashlightActor->SetActorRelativeLocation(FVector(50, 10, 0));
        SpawnedFlashlightActor->SetActorRelativeRotation(FRotator(0, 90, 0));
    }

    UE_LOG(LogTemp, Log, TEXT("Flashlight spawned and attached successfully"));
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
// AUTO-ASSIGN & VALIDATION
// ============================================================================

void UInventoryComponent::TryAutoAssignToQuickbar(FName ItemID)
{
    FItemData ItemData;
    if (!GetItemData(ItemID, ItemData))
    {
        return;
    }

    // Only auto-assign tools and consumables
    if (ItemData.ItemType != EItemType::Tool && ItemData.ItemType != EItemType::Consumable)
    {
        return;
    }

    // Check if already in quickbar
    int32 InventoryIndex = FindInventorySlotByItemID(ItemID);
    if (InventoryIndex < 0)
    {
        return;
    }

    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        if (QuickbarSlotIndices[i] == InventoryIndex)
        {
            // Already in quickbar
            return;
        }
    }

    // Find first empty quickbar slot
    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        if (QuickbarSlotIndices[i] < 0)
        {
            AssignToQuickbar(InventoryIndex, i);
            UE_LOG(LogTemp, Log, TEXT("  → Auto-assigned to quickbar slot %d"), i);
            return;
        }
    }
}

void UInventoryComponent::ValidateQuickbarReferences()
{
    for (int32 i = 0; i < QuickbarSlotIndices.Num(); i++)
    {
        int32 InvIndex = QuickbarSlotIndices[i];
        
        if (InvIndex < 0)
        {
            continue; // Empty slot
        }

        // Check if index is still valid
        if (!InventorySlots.IsValidIndex(InvIndex) || !InventorySlots[InvIndex].IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("⚠ Quickbar[%d] had invalid reference, clearing"), i);
            QuickbarSlotIndices[i] = -1;
            
            if (CurrentEquippedSlotIndex == i)
            {
                UnequipCurrentItem();
            }
        }
    }
}

// ============================================================================
// AUDIO
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
// SAVE/LOAD
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
// DEBUG
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