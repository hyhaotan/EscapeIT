// InventoryComponent.cpp - Horror Game Implementation

#include "InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EscapeIT/Actor/ItemPickupActor.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "EscapeIT/Actor/Components/SanityComponent.h"
#include "TimerManager.h"
#include "EscapeIT/Actor/Components/FlashlightComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize quickbar với empty slots
    QuickbarSlots.SetNum(QuickbarSize);
    for (int32 i = 0; i < QuickbarSize; i++)
    {
        QuickbarSlots[i] = FInventorySlot();
    }

    // Get hand socket reference
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        CharacterMesh = Character->GetMesh();
    }

    // HORROR: Reserve slot 0 for Flashlight (most important tool)
    // Slot 0: Flashlight
    // Slot 1-3: Other tools/items
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

    // HORROR: Quest items cannot be refused
    if (ItemData.ItemType == EItemType::QuestItem)
    {
        UE_LOG(LogTemp, Log, TEXT("AddItem: Force adding quest item '%s'"), *ItemData.ItemName.ToString());
    }

    int32 RemainingQuantity = Quantity;

    // Try to stack with existing items
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

                if (RemainingQuantity <= 0)
                {
                    break;
                }
            }
        }
    }

    // Create new slots for remaining items
    while (RemainingQuantity > 0)
    {
        if (IsInventoryFull())
        {
            // HORROR: Show "Inventory Full" warning
            UE_LOG(LogTemp, Warning, TEXT("AddItem: INVENTORY FULL! Cannot pick up '%s'"), *ItemData.ItemName.ToString());
            
            // Play inventory full sound
            PlayInventoryFullSound();

            // Partial add case
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
    }

    // Play pickup sound with slight audio variation for horror feel
    PlayItemSound(ItemData.PickupSound);

    // Broadcast events
    OnInventoryUpdated.Broadcast();
    OnItemAdded.Broadcast(ItemID, Quantity);
    
    if (ItemData.ItemType == EItemType::Tool || ItemData.ItemType == EItemType::Consumable)
    {
        // Check if item is already in quickbar
        bool bAlreadyInQuickbar = false;
        for (int32 i = 0; i < QuickbarSize; i++)
        {
            if (QuickbarSlots[i].ItemID == ItemID)
            {
                bAlreadyInQuickbar = true;
                break;
            }
        }

        // If not in quickbar, find first empty slot and add it
        if (!bAlreadyInQuickbar)
        {
            for (int32 i = 0; i < QuickbarSize; i++)
            {
                if (!QuickbarSlots[i].IsValid())
                {
                    AssignToQuickbar(ItemID, i);
                    UE_LOG(LogTemp, Log, TEXT("✅ AddItem: Auto-assigned '%s' to quickbar slot %d"), 
                        *ItemID.ToString(), i);
                    break;
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("✅ AddItem: Added %d x %s [%d/%d slots used]"), 
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
        UE_LOG(LogTemp, Warning, TEXT("RemoveItem: Not enough items to remove"));
        return false;
    }

    int32 RemainingToRemove = Quantity;

    // Remove from inventory slots (reverse iteration for safe removal)
    for (int32 i = InventorySlots.Num() - 1; i >= 0; i--)
    {
        if (InventorySlots[i].ItemID == ItemID)
        {
            int32 AmountToRemove = FMath::Min(InventorySlots[i].Quantity, RemainingToRemove);
            InventorySlots[i].Quantity -= AmountToRemove;
            RemainingToRemove -= AmountToRemove;

            if (InventorySlots[i].Quantity <= 0)
            {
                InventorySlots.RemoveAt(i);
            }

            if (RemainingToRemove <= 0)
            {
                break;
            }
        }
    }

    // Sync quickbar slots
    for (int32 i = 0; i < QuickbarSlots.Num(); ++i)
    {
        if (QuickbarSlots[i].IsValid() && QuickbarSlots[i].ItemID == ItemID)
        {
            SyncQuickbarSlot(i);
        }
    }

    // Unequip if equipped item is removed completely
    if (CurrentEquippedItemID == ItemID && !HasItem(ItemID, 1))
    {
        UnequipCurrentItem();
    }

    OnInventoryUpdated.Broadcast();
    OnItemRemoved.Broadcast(ItemID, Quantity);

    UE_LOG(LogTemp, Log, TEXT("RemoveItem: Removed %d x %s"), Quantity, *ItemID.ToString());
    return true;
}

// ============================================================================
// USE ITEMS
// ============================================================================

bool UInventoryComponent::UseItem(FName ItemID)
{
    if (!HasItem(ItemID, 1))
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: Item not found in inventory"));
        OnItemUsed.Broadcast(ItemID, false);
        return false;
    }

    FItemData ItemData;
    if (!GetItemData(ItemID, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("UseItem: Cannot get item data for %s"), *ItemID.ToString());
        OnItemUsed.Broadcast(ItemID, false);
        return false;
    }

    if (!ItemData.bCanBeUse)
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: Item '%s' cannot be used"), *ItemData.ItemName.ToString());
        OnItemUsed.Broadcast(ItemID, false);
        return false;
    }

    // Nếu là Pin thì chỉ cho dùng khi có Flashlight
    if (ItemData.ItemType == EItemType::Consumable)
    {
        if (ItemData.ItemConsumable == EItemConsumable::Battery)
        {
            UFlashlightComponent* FlashlightComp = GetOwner()->FindComponentByClass<UFlashlightComponent>();
            if (!FlashlightComp)
            {
                UE_LOG(LogTemp, Warning, TEXT("UseItem: Cannot use battery without flashlight!"));
                OnItemUsed.Broadcast(ItemID, false);
                return false;
            }

            // Replace battery + toggle light
            FlashlightComp->ReplaceBattery();
            FlashlightComp->ToggleLight();

            RemoveItem(ItemID, 1);
            PlayItemSound(ItemData.UseSound);

            OnItemUsed.Broadcast(ItemID, true);
            OnInventoryUpdated.Broadcast();

            UE_LOG(LogTemp, Log, TEXT("UseItem: Battery used with flashlight"));
            return true;
        }
    }

    // Các item khác xử lý như cũ
    bool bEffectApplied = ApplyItemEffect(ItemData);
    PlayItemSound(ItemData.UseSound);
    OnItemUsed.Broadcast(ItemID, bEffectApplied);

    if (ItemData.bIsConsumable)
    {
        if (CurrentEquippedItemID == ItemID)
        {
            UnequipCurrentItem();
        }
        for (int32 i = 0; i < QuickbarSlots.Num(); i++)
        {
            if (QuickbarSlots[i].ItemID == ItemID)
            {
                QuickbarSlots[i] = FInventorySlot();
            }
        }
        RemoveItem(ItemID, 1);
    }

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

    // Get item data
    FItemData ItemData;
    if (!GetItemData(CurrentEquippedItemID, ItemData))
    {
        return false;
    }

    if (ItemData.ItemType == EItemType::Tool)
    {
        if (ItemData.ItemCategory == EItemCategory::Flashlight)
        {
            if (UFlashlightComponent* FlashlightComp = GetOwner()->FindComponentByClass<UFlashlightComponent>())
            {
                bool bSuccess = FlashlightComp->ToggleLight();
            
                if (!bSuccess)
                {
                    UE_LOG(LogTemp, Warning, TEXT("UseEquippedItem: Cannot toggle flashlight"));
                }
            
                return bSuccess;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("UseEquippedItem: FlashlightComponent not found!"));
                return false;
            }
        }
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

    // Sync first
    SyncQuickbarSlot(QuickbarIndex);

    FInventorySlot& Slot = QuickbarSlots[QuickbarIndex];
    if (!Slot.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipQuickbarSlot: Slot %d is empty"), QuickbarIndex);
        return false;
    }

    // Check if item still exists
    if (!HasItem(Slot.ItemID, 1))
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipQuickbarSlot: Item not in inventory (clearing slot)"));
        Slot = FInventorySlot();
        OnInventoryUpdated.Broadcast();
        return false;
    }

    // Toggle equip/unequip
    if (CurrentEquippedItemID == Slot.ItemID && CurrentEquippedSlotIndex == QuickbarIndex)
    {
        UnequipCurrentItem();
        return true;
    }

    // Unequip current item first
    if (!CurrentEquippedItemID.IsNone())
    {
        UnequipCurrentItem();
    }

    // Get item data
    FItemData ItemData;
    if (!GetItemData(Slot.ItemID, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("EquipQuickbarSlot: ItemData not found"));
        return false;
    }

    // Attach to hand (visual only)
    bool bSuccess = AttachItemToHand(ItemData);

    if (bSuccess)
    {
        CurrentEquippedItemID = Slot.ItemID;
        CurrentEquippedSlotIndex = QuickbarIndex;

        // Play equip sound
        PlayItemSound(ItemData.PickupSound);

        // HORROR: Special handling for flashlight
        if (ItemData.ItemType == EItemType::Tool)
        {
            if (ItemData.ItemCategory == EItemCategory::Flashlight)
            {
                if (UFlashlightComponent* FlashlightComp = GetOwner()->FindComponentByClass<UFlashlightComponent>())
                {
                    // Tell FlashlightComponent to equip
                    FlashlightComp->EquipFlashlight();
                
                    // Auto-turn on flashlight when equipped
                    FlashlightComp->SetLightEnabled(true);
                
                    UE_LOG(LogTemp, Log, TEXT("EquipQuickbarSlot: Flashlight equipped and turned on"));
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("EquipQuickbarSlot: FlashlightComponent not found!"));
                }
            }
        }

        OnItemEquipped.Broadcast(Slot.ItemID);
        OnInventoryUpdated.Broadcast();

        UE_LOG(LogTemp, Log, TEXT("EquipQuickbarSlot: Equipped '%s' from slot %d"),
            *ItemData.ItemName.ToString(), QuickbarIndex);
    }

    return bSuccess;
}

bool UInventoryComponent::AttachItemToHand(const FItemData& ItemData)
{
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("AttachItemToHand: No character mesh!"));
        return false;
    }

    if (!ItemData.ItemMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("AttachItemToHand: Item has no mesh"));
        return false;
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
        UE_LOG(LogTemp, Error, TEXT("AttachItemToHand: Failed to create mesh component"));
        return false;
    }

    EquippedItemMesh->RegisterComponent();
    EquippedItemMesh->SetStaticMesh(ItemData.ItemMesh);
    EquippedItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Attach to hand socket
    FName SocketName = TEXT("Item"); // Or "hand_r", "RightHandSocket"

    if (!CharacterMesh->DoesSocketExist(SocketName))
    {
        UE_LOG(LogTemp, Warning, TEXT("AttachItemToHand: Socket '%s' not found"), *SocketName.ToString());
        SocketName = NAME_None;
    }

    EquippedItemMesh->AttachToComponent(
        CharacterMesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        SocketName
    );

    UE_LOG(LogTemp, Log, TEXT("AttachItemToHand: Attached '%s' to hand"), *ItemData.ItemName.ToString());
    return true;
}

void UInventoryComponent::UnequipCurrentItem()
{
    if (CurrentEquippedItemID.IsNone())
    {
        return;
    }

    // Get item data to check type
    FItemData ItemData;
    if (GetItemData(CurrentEquippedItemID, ItemData))
    {
        if (ItemData.ItemType == EItemType::Tool)
        {
            if (ItemData.ItemCategory == EItemCategory::Flashlight)
            {
                if (UFlashlightComponent* FlashlightComp = GetOwner()->FindComponentByClass<UFlashlightComponent>())
                {
                    // Tell FlashlightComponent to unequip (will auto turn off light)
                    FlashlightComp->UnequipFlashlight();
                
                    UE_LOG(LogTemp, Log, TEXT("UnequipCurrentItem: Flashlight unequipped"));
                }
            }
        }
    }

    // Destroy mesh (visual)
    if (EquippedItemMesh)
    {
        EquippedItemMesh->DestroyComponent();
        EquippedItemMesh = nullptr;
    }

    FName PreviousItemID = CurrentEquippedItemID;
    CurrentEquippedItemID = NAME_None;
    CurrentEquippedSlotIndex = -1;

    OnItemUnequipped.Broadcast(PreviousItemID);
    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("UnequipCurrentItem: Unequipped '%s'"), *PreviousItemID.ToString());
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

    // HORROR: Quest items cannot be dropped
    if (ItemData.ItemType == EItemType::QuestItem || !ItemData.bCanBeDropped)
    {
        UE_LOG(LogTemp, Warning, TEXT("DropItem: Cannot drop quest/important items!"));
        PlayCannotDropSound();
        return false;
    }

    if (!ItemData.PickupActorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("DropItem: No PickupActorClass"));
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    UWorld* World = Owner->GetWorld();
    if (!World)
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
    bool bRemoved = RemoveItem(ItemID, Quantity);
    if (!bRemoved)
    {
        return false;
    }

    // Play drop sound
    PlayItemSound(ItemData.UseSound);

    OnInventoryUpdated.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("DropItem: Dropped %d x %s"), Quantity, *ItemData.ItemName.ToString());
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
// QUICKBAR MANAGEMENT
// ============================================================================

bool UInventoryComponent::AssignToQuickbar(FName ItemID, int32 QuickbarIndex)
{
    if (QuickbarIndex < 0 || QuickbarIndex >= QuickbarSize)
    {
        return false;
    }

    if (!HasItem(ItemID, 1))
    {
        return false;
    }

    FInventorySlot* Slot = FindItemSlot(ItemID);
    if (Slot)
    {
        QuickbarSlots[QuickbarIndex] = FInventorySlot(ItemID, 1);
        QuickbarSlots[QuickbarIndex].RemainingUses = Slot->RemainingUses;

        OnInventoryUpdated.Broadcast();
        return true;
    }

    return false;
}

bool UInventoryComponent::RemoveFromQuickbar(int32 QuickbarIndex)
{
    if (QuickbarIndex < 0 || QuickbarIndex >= QuickbarSlots.Num())
    {
        return false;
    }

    FInventorySlot& QSlot = QuickbarSlots[QuickbarIndex];
    if (!QSlot.IsValid())
    {
        return true;
    }

    if (CurrentEquippedSlotIndex == QuickbarIndex)
    {
        UnequipCurrentItem();
    }

    QSlot = FInventorySlot();
    OnInventoryUpdated.Broadcast();
    return true;
}

FInventorySlot UInventoryComponent::GetQuickbarSlot(int32 Index) const
{
    if (Index >= 0 && Index < QuickbarSlots.Num())
    {
        FInventorySlot QSlot = QuickbarSlots[Index];
        
        if (QSlot.IsValid())
        {
            QSlot.Quantity = GetItemQuantity(QSlot.ItemID);
            if (QSlot.Quantity <= 0)
            {
                return FInventorySlot();
            }
        }
        
        return QSlot;
    }
    return FInventorySlot();
}

void UInventoryComponent::SyncQuickbarSlot(int32 QuickbarIndex)
{
    if (QuickbarIndex < 0 || QuickbarIndex >= QuickbarSlots.Num())
    {
        return;
    }

    FInventorySlot& QSlot = QuickbarSlots[QuickbarIndex];
    if (!QSlot.IsValid())
    {
        return;
    }

    int32 InvIndex = FindInventorySlotByItemID(QSlot.ItemID);

    if (InvIndex >= 0)
    {
        QSlot.Quantity = InventorySlots[InvIndex].Quantity;
        QSlot.RemainingUses = InventorySlots[InvIndex].RemainingUses;
    }
    else
    {
        // Item not in inventory, clear quickbar
        QuickbarSlots[QuickbarIndex] = FInventorySlot();
        if (CurrentEquippedSlotIndex == QuickbarIndex)
        {
            UnequipCurrentItem();
        }
    }
}

void UInventoryComponent::SyncAllQuickbarSlots()
{
    for (int32 i = 0; i < QuickbarSlots.Num(); ++i)
    {
        SyncQuickbarSlot(i);
    }
}

// ============================================================================
// DRAG & DROP SYSTEM
// ============================================================================

bool UInventoryComponent::SwapInventorySlots(int32 SlotA, int32 SlotB)
{
    if (SlotA < 0 || SlotA >= InventorySlots.Num() ||
        SlotB < 0 || SlotB >= InventorySlots.Num() ||
        SlotA == SlotB)
    {
        return false;
    }

    FInventorySlot TempA = InventorySlots[SlotA];
    FInventorySlot TempB = InventorySlots[SlotB];

    UE_LOG(LogTemp, Log, TEXT("✅ SwapInventorySlots: %d (%s) ↔ %d (%s)"),
        SlotA, TempA.IsValid() ? *TempA.ItemID.ToString() : TEXT("Empty"),
        SlotB, TempB.IsValid() ? *TempB.ItemID.ToString() : TEXT("Empty"));

    // Swap
    InventorySlots[SlotA] = TempB;
    InventorySlots[SlotB] = TempA;

    // Sync quickbar
    SyncAllQuickbarSlots();

    OnInventoryUpdated.Broadcast();
    return true;
}

void UInventoryComponent::MoveItemToSlot(int32 SourceIndex, int32 TargetIndex)
{
    // Validate
    if (!InventorySlots.IsValidIndex(SourceIndex) || !InventorySlots.IsValidIndex(TargetIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("MoveItemToSlot: Invalid indices %d → %d"), SourceIndex, TargetIndex);
        return;
    }

    if (SourceIndex == TargetIndex)
    {
        return;
    }

    // Check target is empty
    if (InventorySlots[TargetIndex].IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("MoveItemToSlot: Target slot %d is not empty!"), TargetIndex);
        return;
    }

    // Get source slot data
    FInventorySlot SourceSlot = InventorySlots[SourceIndex];
    
    if (!SourceSlot.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("MoveItemToSlot: Source slot %d is empty!"), SourceIndex);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("✅ MoveItemToSlot: %d → %d (%s, Qty: %d)"),
        SourceIndex, TargetIndex, *SourceSlot.ItemID.ToString(), SourceSlot.Quantity);

    // Move item
    InventorySlots[TargetIndex] = SourceSlot;
    InventorySlots[SourceIndex] = FInventorySlot(); // Clear source

    // ✅ CRITICAL: Sync quickbar slots to point to new location
    SyncAllQuickbarSlots();

    OnInventoryUpdated.Broadcast();
}

bool UInventoryComponent::SwapQuickbarSlots(int32 SlotA, int32 SlotB)
{
    if (SlotA < 0 || SlotA >= QuickbarSlots.Num() ||
        SlotB < 0 || SlotB >= QuickbarSlots.Num() ||
        SlotA == SlotB)
    {
        return false;
    }

    FInventorySlot TempA = QuickbarSlots[SlotA];
    FInventorySlot TempB = QuickbarSlots[SlotB];

    UE_LOG(LogTemp, Log, TEXT("✅ SwapQuickbarSlots: %d (%s) ↔ %d (%s)"),
        SlotA, TempA.IsValid() ? *TempA.ItemID.ToString() : TEXT("Empty"),
        SlotB, TempB.IsValid() ? *TempB.ItemID.ToString() : TEXT("Empty"));

    // Swap
    QuickbarSlots[SlotA] = TempB;
    QuickbarSlots[SlotB] = TempA;

    // Update equipped index if needed
    if (CurrentEquippedSlotIndex == SlotA)
    {
        CurrentEquippedSlotIndex = SlotB;
        UE_LOG(LogTemp, Log, TEXT("  → Equipped index moved: %d → %d"), SlotA, SlotB);
    }
    else if (CurrentEquippedSlotIndex == SlotB)
    {
        CurrentEquippedSlotIndex = SlotA;
        UE_LOG(LogTemp, Log, TEXT("  → Equipped index moved: %d → %d"), SlotB, SlotA);
    }

    OnInventoryUpdated.Broadcast();
    return true;
}

bool UInventoryComponent::MoveInventoryToQuickbar(int32 InventoryIndex, int32 QuickbarIndex)
{
    if (InventoryIndex < 0 || InventoryIndex >= InventorySlots.Num() ||
        QuickbarIndex < 0 || QuickbarIndex >= QuickbarSlots.Num())
    {
        return false;
    }

    FInventorySlot& InvSlot = InventorySlots[InventoryIndex];
    if (!InvSlot.IsValid())
    {
        return false;
    }

    // Simply create reference in quickbar
    QuickbarSlots[QuickbarIndex] = InvSlot;
    OnInventoryUpdated.Broadcast();
    return true;
}

bool UInventoryComponent::MoveQuickbarToInventory(int32 QuickbarIndex, int32 InventoryIndex)
{
    if (QuickbarIndex < 0 || QuickbarIndex >= QuickbarSlots.Num())
    {
        return false;
    }

    FInventorySlot& QSlot = QuickbarSlots[QuickbarIndex];
    if (!QSlot.IsValid())
    {
        return false;
    }

    // Just remove from quickbar (item stays in inventory)
    if (CurrentEquippedSlotIndex == QuickbarIndex)
    {
        UnequipCurrentItem();
    }

    QuickbarSlots[QuickbarIndex] = FInventorySlot();
    OnInventoryUpdated.Broadcast();
    return true;
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

bool UInventoryComponent::GetQuickbarSlotItem(int32 SlotIndex, FItemData& OutItemData) const
{
    if (SlotIndex >= 0 && SlotIndex < QuickbarSlots.Num())
    {
        const FInventorySlot& Slot = QuickbarSlots[SlotIndex];
        if (Slot.IsValid())
        {
            return GetItemData(Slot.ItemID, OutItemData);
        }
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
    for (int32 i = 0; i < QuickbarSize; i++)
    {
        QuickbarSlots[i] = FInventorySlot();
    }
    
    OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::PrintInventory()
{
    UE_LOG(LogTemp, Log, TEXT("========== INVENTORY =========="));
    UE_LOG(LogTemp, Log, TEXT("Slots: %d/%d"), InventorySlots.Num(), MaxInventorySlots);
    
    for (const FInventorySlot& Slot : InventorySlots)
    {
        FItemData ItemData;
        if (GetItemData(Slot.ItemID, ItemData))
        {
            UE_LOG(LogTemp, Log, TEXT("  %s x%d"), *ItemData.ItemName.ToString(), Slot.Quantity);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Equipped: %s"), 
        CurrentEquippedItemID.IsNone() ? TEXT("None") : *CurrentEquippedItemID.ToString());
    UE_LOG(LogTemp, Log, TEXT("==============================="));
}

// ============================================================================
// ITEM EFFECTS (HORROR-SPECIFIC)
// ============================================================================

bool UInventoryComponent::ApplyItemEffect(const FItemData& ItemData)
{
    if (!ItemData.bUseEffect)
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
                UE_LOG(LogTemp, Log, TEXT("ApplyItemEffect: Restored %.1f sanity"), 
                    ItemData.SanityRestoreAmount);
            }
        }
    }

    // Passive sanity drain reduction (Teddy Bear effect)
    if (ItemData.PassiveSanityDrainReduction > 0.0f)
    {
        // This should be handled by the item being in inventory
        // Not consumed on use
        UE_LOG(LogTemp, Log, TEXT("ApplyItemEffect: Passive effect active"));
        bEffectApplied = true;
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
// AUDIO SYSTEM
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
            FMath::RandRange(0.95f, 1.05f) // Slight pitch variation for horror
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
// SAVE/LOAD SUPPORT
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
            
            // Restore durability
            FInventorySlot* NewSlot = FindItemSlot(Slot.ItemID);
            if (NewSlot)
            {
                NewSlot->RemainingUses = Slot.RemainingUses;
            }
        }
    }
    
    OnInventoryUpdated.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("LoadInventorySlots: Loaded %d items"), SavedSlots.Num());
}

void UInventoryComponent::SaveQuickbarSetup(TArray<FInventorySlot>& OutQuickbarSlots) const
{
    OutQuickbarSlots = QuickbarSlots;
}

void UInventoryComponent::LoadQuickbarSetup(const TArray<FInventorySlot>& SavedQuickbar)
{
    if (SavedQuickbar.Num() != QuickbarSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("LoadQuickbarSetup: Quickbar size mismatch"));
        return;
    }
    
    for (int32 i = 0; i < QuickbarSize; i++)
    {
        if (SavedQuickbar[i].IsValid() && HasItem(SavedQuickbar[i].ItemID, 1))
        {
            AssignToQuickbar(SavedQuickbar[i].ItemID, i);
        }
    }
    
    OnInventoryUpdated.Broadcast();
}

// ============================================================================
// DEBUG / CHEAT COMMANDS
// ============================================================================

void UInventoryComponent::GiveAllItems()
{
    if (!ItemDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("GiveAllItems: No ItemDataTable set!"));
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