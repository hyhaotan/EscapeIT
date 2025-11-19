// InventoryComponent.cpp - Implementation

#include "InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EscapeIT/Actor/ItemPickupActor.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Character.h"
#include "EscapeIT/Actor/Components/SanityComponent.h"
#include "EscapeIT/EscapeITCharacter.h"
#include "Components/SkeletalMeshComponent.h"

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
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update cooldowns
    UpdateCooldowns(DeltaTime);
}

bool UInventoryComponent::AddItem(FName ItemID, int32 Quantity)
{
    if (ItemID.IsNone() || Quantity <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("AddItem: Invalid ItemID or Quantity"));
        return false;
    }

    // Lấy item data
    FItemData ItemData;
    if (!GetItemData(ItemID, ItemData))
    {
        UE_LOG(LogTemp, Warning, TEXT("AddItem: ItemID '%s' not found in DataTable"), *ItemID.ToString());
        return false;
    }

    int32 RemainingQuantity = Quantity;

    // Nếu item có thể stack, tìm slot hiện có
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

    // Nếu còn dư, tạo slot mới
    while (RemainingQuantity > 0)
    {
        if (IsInventoryFull())
        {
            UE_LOG(LogTemp, Warning, TEXT("AddItem: Inventory is full"));

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

        // Set remaining uses cho items có durability
        if (ItemData.bHasDurability)
        {
            NewSlot.RemainingUses = ItemData.MaxUses;
        }

        InventorySlots.Add(NewSlot);
        RemainingQuantity -= AmountToAdd;
    }

    // Play pickup sound
    PlayItemSound(ItemData.PickupSound);

    // Update weight và broadcast
    OnInventoryUpdated.Broadcast();
    OnItemAdded.Broadcast(ItemID, Quantity);

    bool bAlreadyInQuickbar = false;
    int32 ExistingSlotIndex = -1;

    // Check if item already in quickbar
    for (int32 i = 0; i < QuickbarSize; i++)
    {
        if (QuickbarSlots[i].ItemID == ItemID)
        {
            bAlreadyInQuickbar = true;
            ExistingSlotIndex = i;
            UE_LOG(LogTemp, Log, TEXT("AddItem: Item '%s' already in quickbar slot %d"),
                *ItemID.ToString(), i);
            break;
        }
    }

    if (!bAlreadyInQuickbar)
    {
        // SPECIAL CASE: Flashlight always goes to slot 0
        if (ItemData.ItemType == EItemType::Tool)
        {
            if (ItemData.ItemCategory == EItemCategory::Flashlight)
            {
                bool bAssigned = AssignToQuickbar(ItemID, 0);
                if (bAssigned)
                {
                    UE_LOG(LogTemp, Log, TEXT("AddItem: Auto-assigned Flashlight '%s' to quickbar slot 0"),
                        *ItemID.ToString());
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("AddItem: Failed to assign Flashlight to slot 0"));
                }
            }
        }
        // Other items: auto-assign to slots 1-3
        else
        {
            for (int32 i = 1; i < QuickbarSize; i++)
            {
                if (!QuickbarSlots[i].IsValid())
                {
                    bool bAssigned = AssignToQuickbar(ItemID, i);
                    if (bAssigned)
                    {
                        UE_LOG(LogTemp, Log, TEXT("AddItem: Auto-assigned '%s' to quickbar slot %d"),
                            *ItemID.ToString(), i);
                    }
                    break;
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("AddItem: Added %d x %s"), Quantity, *ItemData.ItemName.ToString());
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

    // Remove from inventory slots
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

    // Sync quickbar slots - update or clear if item is gone
    for (int32 i = 0; i < QuickbarSlots.Num(); ++i)
    {
        if (QuickbarSlots[i].IsValid() && QuickbarSlots[i].ItemID == ItemID)
        {
            SyncQuickbarSlot(i);
        }
    }

    // If equipped item is removed, unequip
    if (CurrentEquippedItemID == ItemID && !HasItem(ItemID, 1))
    {
        UnequipCurrentItem();
    }

    OnInventoryUpdated.Broadcast();
    OnItemRemoved.Broadcast(ItemID, Quantity);

    UE_LOG(LogTemp, Log, TEXT("RemoveItem: Removed %d x %s"), Quantity, *ItemID.ToString());
    return true;
}

bool UInventoryComponent::UseItem(FName ItemID)
{
    if (!HasItem(ItemID, 1))
    {
        OnItemUsed.Broadcast(ItemID, false);
        return false;
    }

    // Lấy item data
    FItemData ItemData;
    if (!GetItemData(ItemID, ItemData))
    {
        OnItemUsed.Broadcast(ItemID, false);
        return false;
    }

    // Check cooldown
    FInventorySlot* Slot = FindItemSlot(ItemID);
    if (Slot && Slot->CooldownRemaining > 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("UseItem: Item on cooldown (%.1fs remaining)"), Slot->CooldownRemaining);
        OnItemUsed.Broadcast(ItemID, false);
        return false;
    }

    // Apply effect
    ApplyItemEffect(ItemData);

    // Play sound
    PlayItemSound(ItemData.UseSound);

    // Set cooldown
    if (Slot && ItemData.UsageCooldown > 0.0f)
    {
        Slot->CooldownRemaining = ItemData.UsageCooldown;

        OnItemCooldownUpdated.Broadcast(ItemID, ItemData.UsageCooldown, ItemData.UsageCooldown);

        SyncAllQuickbarSlots();
    }

    // Handle durability/uses
    if (ItemData.bHasDurability && Slot)
    {
        Slot->RemainingUses--;
        if (Slot->RemainingUses <= 0)
        {
            RemoveItem(ItemID, 1);
            UE_LOG(LogTemp, Log, TEXT("UseItem: Item '%s' durability depleted"), *ItemData.ItemName.ToString());
        }
    }
    // Remove consumable
    else if (ItemData.bIsConsumable)
    {
        RemoveItem(ItemID, 1);
    }

    OnItemUsed.Broadcast(ItemID, true);
    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("UseItem: Used '%s'"), *ItemData.ItemName.ToString());
    return true;
}

bool UInventoryComponent::EquipQuickbarSlot(int32 QuickbarIndex)
{
    if (QuickbarIndex < 0 || QuickbarIndex >= QuickbarSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipQuickbarSlot: Invalid index %d"), QuickbarIndex);
        return false;
    }

    // Sync first to ensure data is current
    SyncQuickbarSlot(QuickbarIndex);

    FInventorySlot& Slot = QuickbarSlots[QuickbarIndex];
    if (!Slot.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipQuickbarSlot: Slot %d is empty"), QuickbarIndex);
        return false;
    }

    // Double-check item exists in inventory
    if (!HasItem(Slot.ItemID, 1))
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipQuickbarSlot: Item '%s' not in inventory (clearing slot %d)"),
            *Slot.ItemID.ToString(), QuickbarIndex);
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

    // Unequip current item
    if (!CurrentEquippedItemID.IsNone())
    {
        UnequipCurrentItem();
    }

    // Get item data
    FItemData ItemData;
    if (!GetItemData(Slot.ItemID, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("EquipQuickbarSlot: ItemData not found for %s"), *Slot.ItemID.ToString());
        return false;
    }

    // Attach to hand
    bool bSuccess = AttachItemToHand(ItemData);

    if (bSuccess)
    {
        CurrentEquippedItemID = Slot.ItemID;
        CurrentEquippedSlotIndex = QuickbarIndex;

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
        UE_LOG(LogTemp, Error, TEXT("AttachItemToHand: No character mesh found!"));
        return false;
    }

    if (!ItemData.ItemMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("AttachItemToHand: Item has no mesh"));
        return false;
    }

    // Destroy existing equipped mesh
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

    // Disable collision khi đang cầm
    EquippedItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Attach to hand socket
    FName SocketName = TEXT("Item");

    if (!CharacterMesh->DoesSocketExist(SocketName))
    {
        UE_LOG(LogTemp, Warning, TEXT("AttachItemToHand: Socket '%s' not found, using default"), *SocketName.ToString());
        SocketName = NAME_None;
    }

    EquippedItemMesh->AttachToComponent(
       CharacterMesh,
       FAttachmentTransformRules::SnapToTargetNotIncludingScale,
       SocketName
   );

    UE_LOG(LogTemp, Log, TEXT("AttachItemToHand: Successfully attached item to hand"));
    return true;
}

void UInventoryComponent::UnequipCurrentItem()
{
    if (CurrentEquippedItemID.IsNone())
    {
        return;
    }

    // Destroy mesh
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

bool UInventoryComponent::UseEquippedItem()
{
    if (CurrentEquippedItemID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("UseEquippedItem: No item equipped"));
        return false;
    }

    if (!HasItem(CurrentEquippedItemID, 1))
    {
        UE_LOG(LogTemp, Warning, TEXT("UseEquippedItem: Item no longer in inventory"));
        UnequipCurrentItem();
        return false;
    }

    // Lấy item data
    FItemData ItemData;
    if (!GetItemData(CurrentEquippedItemID, ItemData))
    {
        return false;
    }

    // Check cooldown
    FInventorySlot* Slot = FindItemSlot(CurrentEquippedItemID);
    if (Slot && Slot->CooldownRemaining > 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("UseEquippedItem: Item on cooldown (%.1fs remaining)"),
            Slot->CooldownRemaining);
        return false;
    }

    // Apply effect
    ApplyItemEffect(ItemData);

    // Play sound
    PlayItemSound(ItemData.UseSound);

    // Set cooldown
    if (Slot && ItemData.UsageCooldown > 0.0f)
    {
        Slot->CooldownRemaining = ItemData.UsageCooldown;

        // Sync cooldown to quickbar
        if (CurrentEquippedSlotIndex >= 0 && CurrentEquippedSlotIndex < QuickbarSlots.Num())
        {
            QuickbarSlots[CurrentEquippedSlotIndex].CooldownRemaining = ItemData.UsageCooldown;
        }
    }

    // Handle durability/uses
    if (ItemData.bHasDurability && Slot)
    {
        Slot->RemainingUses--;
        if (Slot->RemainingUses <= 0)
        {
            UE_LOG(LogTemp, Log, TEXT("UseEquippedItem: Item '%s' durability depleted"),
                *ItemData.ItemName.ToString());

            // Unequip trước khi remove
            UnequipCurrentItem();
            RemoveItem(ItemData.ItemID, 1);
            return true;
        }
    }
    // Remove consumable
    else if (ItemData.bIsConsumable)
    {
        // Unequip trước khi remove
        UnequipCurrentItem();
        RemoveItem(ItemData.ItemID, 1);
        return true;
    }

    OnItemUsed.Broadcast(CurrentEquippedItemID, true);
    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("UseEquippedItem: Used '%s'"), *ItemData.ItemName.ToString());
    return true;
}

bool UInventoryComponent::DropEquippedItem()
{
    if (CurrentEquippedItemID.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("DropEquippedItem: No item equipped"));
        return false;
    }

    FName ItemToDrop = CurrentEquippedItemID;

    // Unequip trước
    UnequipCurrentItem();

    // Drop item
    return DropItem(ItemToDrop, 1);
}

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

FName UInventoryComponent::GetCurrentEquippedItemID() const
{
    return CurrentEquippedItemID;
}

bool UInventoryComponent::IsItemEquipped() const
{
    return !CurrentEquippedItemID.IsNone();
}

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

    // Tìm item trong inventory
    FInventorySlot* Slot = FindItemSlot(ItemID);
    if (Slot)
    {
        // Tạo quickbar slot reference
        QuickbarSlots[QuickbarIndex] = FInventorySlot(ItemID, 1);

        // Sync cooldown nếu có
        QuickbarSlots[QuickbarIndex].CooldownRemaining = Slot->CooldownRemaining;
        QuickbarSlots[QuickbarIndex].RemainingUses = Slot->RemainingUses;

        OnInventoryUpdated.Broadcast();

        UE_LOG(LogTemp, Log, TEXT("AssignToQuickbar: Assigned %s to slot %d"),
            *ItemID.ToString(), QuickbarIndex);
        return true;
    }

    return false;
}

bool UInventoryComponent::UseQuickbarSlot(int32 QuickbarIndex)
{
    // Legacy support - giờ gọi Equip thay vì Use
    return EquipQuickbarSlot(QuickbarIndex);
}

FInventorySlot UInventoryComponent::GetQuickbarSlot(int32 Index) const
{
    if (Index >= 0 && Index < QuickbarSlots.Num())
    {
        FInventorySlot QSlot = QuickbarSlots[Index];

        if (QSlot.IsValid())
        {
            // Cập nhật quantity thực tế từ inventory
            QSlot.Quantity = GetItemQuantity(QSlot.ItemID);

            // Nếu hết item trong inventory, clear slot
            if (QSlot.Quantity <= 0)
            {
                return FInventorySlot();
            }
        }

        return QSlot;
    }
    return FInventorySlot();
}

bool UInventoryComponent::DropItem(FName ItemID, int32 Quantity)
{
    if (ItemID.IsNone() || Quantity <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("DropItem: Invalid params"));
        return false;
    }

    if (!HasItem(ItemID, Quantity))
    {
        UE_LOG(LogTemp, Warning, TEXT("DropItem: Not enough items to drop"));
        return false;
    }

    FItemData ItemData;
    if (!GetItemData(ItemID, ItemData))
    {
        UE_LOG(LogTemp, Error, TEXT("DropItem: ItemData not found for %s"), *ItemID.ToString());
        return false;
    }

    if (!ItemData.PickupActorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("DropItem: No PickupActorClass set for %s"), *ItemID.ToString());
        return false;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("DropItem: Component has no owner"));
        return false;
    }

    UWorld* World = Owner->GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("DropItem: No world"));
        return false;
    }

    // Calculate drop location (in front of owner)
    const FRotator OwnerRot = Owner->GetActorRotation();
    const FVector BaseLocation = Owner->GetActorLocation() + OwnerRot.Vector() * 150.0f + FVector(0, 0, 50.0f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // Spawn pickup actors
    for (int32 i = 0; i < Quantity; ++i)
    {
        // Add jitter so items don't stack on top of each other
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
            // IMPORTANT: Set ItemID and DataTable
            Dropped->ItemID = ItemID;
            Dropped->ItemDataTable = ItemDataTable;
            Dropped->Quantity = 1;

            // Initialize from DataTable to load mesh and properties
            Dropped->SetItemID(ItemID);

            // Enable physics and add impulse
            if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Dropped->GetRootComponent()))
            {
                RootPrim->SetSimulatePhysics(true);

                // Add impulse in forward direction with slight upward arc
                FVector ImpulseDir = OwnerRot.Vector() + FVector(0.0f, 0.0f, 0.5f);
                ImpulseDir.Normalize();
                RootPrim->AddImpulse(ImpulseDir * 300.0f, NAME_None, true);
            }

            UE_LOG(LogTemp, Log, TEXT("DropItem: Spawned pickup for '%s'"), *ItemData.ItemName.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("DropItem: Failed to spawn pickup actor for %s"), *ItemID.ToString());
        }
    }

    // Remove items from inventory
    bool bRemoved = RemoveItem(ItemID, Quantity);
    if (!bRemoved)
    {
        UE_LOG(LogTemp, Warning, TEXT("DropItem: Failed to remove items from inventory after spawn"));
        return false;
    }

    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("DropItem: Successfully dropped %d x %s"), Quantity, *ItemData.ItemName.ToString());
    return true;
}

bool UInventoryComponent::RemoveItemFromQuickbar(int32 QuickbarIndex, int32 Quantity)
{
    if (QuickbarIndex < 0 || QuickbarIndex >= QuickbarSlots.Num())
    {
        return false;
    }
    if (Quantity <= 0)
    {
        return false;
    }

    FInventorySlot& QSlot = QuickbarSlots[QuickbarIndex];
    if (!QSlot.IsValid())
    {
        return false;
    }

    // Nếu item này đang equipped, unequip trước
    if (CurrentEquippedItemID == QSlot.ItemID)
    {
        UnequipCurrentItem();
    }

    // Số lượng thực tế cần remove = min(request, current quickbar quantity, total in inventory)
    int32 TotalInInventory = GetItemQuantity(QSlot.ItemID);
    int32 ToRemove = FMath::Min(Quantity, TotalInInventory);

    if (ToRemove <= 0)
    {
        // nothing to remove
        QSlot = FInventorySlot();
        OnInventoryUpdated.Broadcast();
        return true;
    }

    // Remove khỏi inventory (RemoveItem sẽ cập nhật weight + broadcast)
    bool bOk = RemoveItem(QSlot.ItemID, ToRemove);
    if (!bOk)
    {
        UE_LOG(LogTemp, Warning, TEXT("RemoveItemFromQuickbar: RemoveItem failed for %s"), *QSlot.ItemID.ToString());
        return false;
    }

    // Cập nhật quickbar slot: nếu trong inventory đã hết, clear quickbar; nếu còn, điều chỉnh quantity hiển thị
    int32 RemainingTotal = GetItemQuantity(QSlot.ItemID);
    if (RemainingTotal <= 0)
    {
        QSlot = FInventorySlot(); // clear
    }
    else
    {
        // Set quickbar quantity to min(remaining, original max stack you want to show)
        QSlot.Quantity = FMath::Min(RemainingTotal, QSlot.Quantity);
    }

    OnInventoryUpdated.Broadcast();
    return true;
}

bool UInventoryComponent::IsInventoryFull() const
{
    return InventorySlots.Num() >= MaxInventorySlots;
}


void UInventoryComponent::ClearInventory()
{
    // Unequip trước khi clear
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
    for (const FInventorySlot& Slot : InventorySlots)
    {
        FItemData ItemData;
        if (GetItemData(Slot.ItemID, ItemData))
        {
            UE_LOG(LogTemp, Log, TEXT("  %s x%d"), *ItemData.ItemName.ToString(), Slot.Quantity);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Equipped: %s"), *CurrentEquippedItemID.ToString());
    UE_LOG(LogTemp, Log, TEXT("==============================="));
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

int32 UInventoryComponent::FindEmptySlot() const
{
    return InventorySlots.Num() < MaxInventorySlots ? InventorySlots.Num() : -1;
}

void UInventoryComponent::ApplyItemEffect(const FItemData& ItemData)
{
    // Áp dụng sanity restore
    if (ItemData.SanityRestoreAmount > 0.0f)
    {
        // Cần reference tới SanityComponent
        AActor* Owner = GetOwner();
        if (Owner)
        {
             USanityComponent* SanityComp = Owner->FindComponentByClass<USanityComponent>();
             if (SanityComp)
             {
                 SanityComp->RestoreSanity(ItemData.SanityRestoreAmount);
             }

            UE_LOG(LogTemp, Log, TEXT("ApplyItemEffect: Restore Sanity +%.1f"), ItemData.SanityRestoreAmount);
        }
    }

    // TODO: Apply passive effects (Teddy Bear)
    // TODO: Apply special effects (Decoy, Stun)
}

void UInventoryComponent::UpdateCooldowns(float DeltaTime)
{
    bool bNeedUpdate = false;

    // Update inventory cooldowns
    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.CooldownRemaining > 0.0f)
        {
            float OldCooldown = Slot.CooldownRemaining;
            Slot.CooldownRemaining -= DeltaTime;

            if (Slot.CooldownRemaining < 0.0f)
            {
                Slot.CooldownRemaining = 0.0f;
            }

            // Broadcast cooldown update
            FItemData ItemData;
            if (GetItemData(Slot.ItemID, ItemData))
            {
                OnItemCooldownUpdated.Broadcast(Slot.ItemID, Slot.CooldownRemaining, ItemData.UsageCooldown);
            }

            bNeedUpdate = true;
        }
    }

    // Sync quickbar cooldowns from inventory
    for (int32 i = 0; i < QuickbarSlots.Num(); ++i)
    {
        if (QuickbarSlots[i].IsValid())
        {
            FInventorySlot* InvSlot = FindItemSlot(QuickbarSlots[i].ItemID);
            if (InvSlot)
            {
                QuickbarSlots[i].CooldownRemaining = InvSlot->CooldownRemaining;
                QuickbarSlots[i].RemainingUses = InvSlot->RemainingUses;
            }
        }
    }

    if (bNeedUpdate)
    {
        OnInventoryUpdated.Broadcast();
    }
}

void UInventoryComponent::PlayItemSound(USoundBase* Sound)
{
    if (Sound)
    {
        UGameplayStatics::PlaySound2D(this, Sound);
    }
}

bool UInventoryComponent::TryAutoAssignToQuickbar(FName ItemID)
{
    // Tìm slot trống đầu tiên (bỏ qua slot 0 - flashlight)
    for (int32 i = 1; i < QuickbarSize; i++)
    {
        if (!QuickbarSlots[i].IsValid())
        {
            return AssignToQuickbar(ItemID, i);
        }
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
            // Get ItemData from DataTable using ItemID
            return GetItemData(Slot.ItemID, OutItemData);
        }
    }
    return false;
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

    // Tìm item trong inventory
    int32 InvIndex = FindInventorySlotByItemID(QSlot.ItemID);

    if (InvIndex >= 0)
    {
        // Sync data từ inventory (source of truth)
        QSlot.Quantity = InventorySlots[InvIndex].Quantity;
        QSlot.RemainingUses = InventorySlots[InvIndex].RemainingUses;
        QSlot.CooldownRemaining = InventorySlots[InvIndex].CooldownRemaining;

        UE_LOG(LogTemp, Verbose, TEXT("SyncQuickbarSlot: Synced QB[%d] with Inv[%d] - %s"),
            QuickbarIndex, InvIndex, *QSlot.ItemID.ToString());
    }
    else
    {
        // Item không còn trong inventory, clear quickbar slot
        UE_LOG(LogTemp, Warning, TEXT("SyncQuickbarSlot: Item '%s' not in inventory, clearing QB[%d]"),
            *QSlot.ItemID.ToString(), QuickbarIndex);

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

bool UInventoryComponent::SwapInventorySlots(int32 SlotA, int32 SlotB)
{
    if (SlotA < 0 || SlotA >= InventorySlots.Num() ||
        SlotB < 0 || SlotB >= InventorySlots.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("SwapInventorySlots: Invalid indices"));
        return false;
    }

    if (SlotA == SlotB)
    {
        return true;
    }

    // Simple swap within inventory
    FInventorySlot Temp = InventorySlots[SlotA];
    InventorySlots[SlotA] = InventorySlots[SlotB];
    InventorySlots[SlotB] = Temp;

    // Sync tất cả quickbar slots vì có thể có reference tới items này
    SyncAllQuickbarSlots();

    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("SwapInventorySlots: Swapped slots %d <-> %d"), SlotA, SlotB);
    return true;
}

bool UInventoryComponent::SwapQuickbarSlots(int32 SlotA, int32 SlotB)
{
    if (SlotA < 0 || SlotA >= QuickbarSlots.Num() ||
        SlotB < 0 || SlotB >= QuickbarSlots.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("SwapQuickbarSlots: Invalid indices"));
        return false;
    }

    if (SlotA == SlotB)
    {
        return true;
    }

    // Simple swap within quickbar (chỉ swap reference, không ảnh hưởng inventory)
    FInventorySlot Temp = QuickbarSlots[SlotA];
    QuickbarSlots[SlotA] = QuickbarSlots[SlotB];
    QuickbarSlots[SlotB] = Temp;

    // Update equipped index
    if (CurrentEquippedSlotIndex == SlotA)
    {
        CurrentEquippedSlotIndex = SlotB;
    }
    else if (CurrentEquippedSlotIndex == SlotB)
    {
        CurrentEquippedSlotIndex = SlotA;
    }

    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("SwapQuickbarSlots: Swapped slots %d <-> %d"), SlotA, SlotB);
    return true;
}

bool UInventoryComponent::MoveInventoryToQuickbar(int32 InventoryIndex, int32 QuickbarIndex)
{
    if (InventoryIndex < 0 || InventoryIndex >= InventorySlots.Num() ||
        QuickbarIndex < 0 || QuickbarIndex >= QuickbarSlots.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("MoveInventoryToQuickbar: Invalid indices"));
        return false;
    }

    FInventorySlot& InvSlot = InventorySlots[InventoryIndex];
    FInventorySlot& QSlot = QuickbarSlots[QuickbarIndex];

    if (!InvSlot.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("MoveInventoryToQuickbar: Source slot is empty"));
        return false;
    }

    // Case 1: Quickbar slot is empty - simple move
    if (!QSlot.IsValid())
    {
        // Move item to quickbar (reference only, không xóa khỏi inventory)
        QuickbarSlots[QuickbarIndex] = InvSlot;

        UE_LOG(LogTemp, Log, TEXT("MoveInventoryToQuickbar: Added '%s' to quickbar %d"),
            *InvSlot.ItemID.ToString(), QuickbarIndex);
    }
    // Case 2: Quickbar has different item - swap
    else if (QSlot.ItemID != InvSlot.ItemID)
    {
        // Tìm item trong quickbar có tồn tại trong inventory không
        int32 QuickbarItemInvIndex = FindInventorySlotByItemID(QSlot.ItemID);

        // Swap references
        FInventorySlot TempInv = InvSlot;
        FInventorySlot TempQuick = QSlot;

        // Update inventory
        InventorySlots[InventoryIndex] = TempQuick;

        // Update quickbar
        QuickbarSlots[QuickbarIndex] = TempInv;

        // Nếu item từ quickbar có trong inventory ở slot khác, cập nhật lại quickbar slot đó
        if (QuickbarItemInvIndex >= 0 && QuickbarItemInvIndex != InventoryIndex)
        {
            // Item vẫn tồn tại trong inventory ở vị trí khác, không làm gì
        }

        UE_LOG(LogTemp, Log, TEXT("MoveInventoryToQuickbar: SWAPPED '%s' <-> '%s'"),
            *TempInv.ItemID.ToString(), *TempQuick.ItemID.ToString());
    }
    // Case 3: Quickbar has same item - just update reference
    else
    {
        QuickbarSlots[QuickbarIndex] = InvSlot;
        UE_LOG(LogTemp, Log, TEXT("MoveInventoryToQuickbar: Updated quickbar reference"));
    }

    OnInventoryUpdated.Broadcast();
    return true;
}

bool UInventoryComponent::MoveQuickbarToInventory(int32 QuickbarIndex, int32 InventoryIndex)
{
    if (QuickbarIndex < 0 || QuickbarIndex >= QuickbarSlots.Num() ||
        InventoryIndex < 0 || InventoryIndex >= MaxInventorySlots)
    {
        UE_LOG(LogTemp, Warning, TEXT("MoveQuickbarToInventory: Invalid indices"));
        return false;
    }

    FInventorySlot& QSlot = QuickbarSlots[QuickbarIndex];

    if (!QSlot.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("MoveQuickbarToInventory: Source quickbar slot is empty"));
        return false;
    }

    // Expand inventory if needed
    while (InventorySlots.Num() <= InventoryIndex)
    {
        InventorySlots.Add(FInventorySlot());
    }

    FInventorySlot& InvSlot = InventorySlots[InventoryIndex];

    // Tìm xem item trong quickbar có tồn tại ở đâu trong inventory
    int32 SourceInventoryIndex = FindInventorySlotByItemID(QSlot.ItemID);

    // Case 1: Target inventory slot is empty
    if (!InvSlot.IsValid())
    {
        if (SourceInventoryIndex >= 0 && SourceInventoryIndex != InventoryIndex)
        {
            // Item đã tồn tại ở slot khác trong inventory
            // Chỉ cần move vị trí trong inventory
            FInventorySlot TempItem = InventorySlots[SourceInventoryIndex];
            InventorySlots[SourceInventoryIndex] = FInventorySlot(); // Clear old slot
            InventorySlots[InventoryIndex] = TempItem; // Move to new slot

            // Clear quickbar slot
            QuickbarSlots[QuickbarIndex] = FInventorySlot();

            UE_LOG(LogTemp, Log, TEXT("MoveQuickbarToInventory: Moved '%s' from inv[%d] to inv[%d], cleared quickbar"),
                *TempItem.ItemID.ToString(), SourceInventoryIndex, InventoryIndex);
        }
        else
        {
            // Item không tồn tại trong inventory (lỗi logic, quickbar phải reference từ inventory)
            // Hoặc đang move về đúng vị trí cũ của nó
            InventorySlots[InventoryIndex] = QSlot;
            QuickbarSlots[QuickbarIndex] = FInventorySlot();

            UE_LOG(LogTemp, Log, TEXT("MoveQuickbarToInventory: Moved '%s' to inv[%d], cleared quickbar"),
                *QSlot.ItemID.ToString(), InventoryIndex);
        }

        // Unequip if needed
        if (CurrentEquippedSlotIndex == QuickbarIndex)
        {
            UnequipCurrentItem();
        }
    }
    // Case 2: Target has different item - swap
    else if (InvSlot.ItemID != QSlot.ItemID)
    {
        FInventorySlot TempQuick = QSlot;
        FInventorySlot TempInv = InvSlot;

        // Nếu quickbar item tồn tại trong inventory, update vị trí
        if (SourceInventoryIndex >= 0 && SourceInventoryIndex != InventoryIndex)
        {
            InventorySlots[SourceInventoryIndex] = TempInv; // Item từ target inv slot
        }

        // Update target inventory slot
        InventorySlots[InventoryIndex] = TempQuick;

        // Update quickbar với item từ inventory
        QuickbarSlots[QuickbarIndex] = TempInv;

        UE_LOG(LogTemp, Log, TEXT("MoveQuickbarToInventory: SWAPPED '%s' <-> '%s'"),
            *TempQuick.ItemID.ToString(), *TempInv.ItemID.ToString());
    }
    // Case 3: Same item - just update position
    else
    {
        // Moving item to its own inventory slot
        QuickbarSlots[QuickbarIndex] = FInventorySlot();

        if (CurrentEquippedSlotIndex == QuickbarIndex)
        {
            UnequipCurrentItem();
        }

        UE_LOG(LogTemp, Log, TEXT("MoveQuickbarToInventory: Cleared quickbar, item already in inventory"));
    }

    // Sync all quickbar slots
    SyncAllQuickbarSlots();

    OnInventoryUpdated.Broadcast();
    return true;
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

bool UInventoryComponent::RemoveFromQuickbar(int32 QuickbarIndex)
{
    if (QuickbarIndex < 0 || QuickbarIndex >= QuickbarSlots.Num())
    {
        return false;
    }

    FInventorySlot& QSlot = QuickbarSlots[QuickbarIndex];

    if (!QSlot.IsValid())
    {
        return true; // Already empty
    }

    // If equipped, unequip first
    if (CurrentEquippedSlotIndex == QuickbarIndex)
    {
        UnequipCurrentItem();
    }

    // Clear quickbar slot (item remains in inventory)
    QSlot = FInventorySlot();

    OnInventoryUpdated.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("RemoveFromQuickbar: Cleared quickbar slot %d"), QuickbarIndex);
    return true;
}

bool UInventoryComponent::GetEquippedItem(FItemData& OutItemData) const
{
    if (CurrentEquippedSlotIndex >= 0 && CurrentEquippedSlotIndex < QuickbarSlots.Num())
    {
        const FInventorySlot& Slot = QuickbarSlots[CurrentEquippedSlotIndex];
        if (Slot.IsValid())
        {
            // Get ItemData from DataTable using ItemID
            return GetItemData(Slot.ItemID, OutItemData);
        }
    }
    return false;
}