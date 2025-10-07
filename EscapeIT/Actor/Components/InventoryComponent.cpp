// InventoryComponent.cpp - Implementation

#include "InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EscapeIT/Actor/ItemPickupActor.h"
#include "Sound/SoundBase.h"

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
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update cooldowns
    UpdateCooldowns(DeltaTime);
}

// ============================================
// ADD ITEM
// ============================================
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

    // Check weight
    if (bUseWeightSystem)
    {
        float TotalWeight = ItemData.Weight * Quantity;
        if (CurrentWeight + TotalWeight > MaxWeight)
        {
            UE_LOG(LogTemp, Warning, TEXT("AddItem: Over weight limit"));
            return false;
        }
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
                RecalculateWeight();
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
    RecalculateWeight();
    OnInventoryUpdated.Broadcast();
    OnItemAdded.Broadcast(ItemID, Quantity);

    UE_LOG(LogTemp, Log, TEXT("AddItem: Added %d x %s"), Quantity, *ItemData.ItemName.ToString());
    return true;
}

// ============================================
// REMOVE ITEM
// ============================================
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

    for (int32 i = InventorySlots.Num() - 1; i >= 0; i--)
    {
        if (InventorySlots[i].ItemID == ItemID)
        {
            int32 AmountToRemove = FMath::Min(InventorySlots[i].Quantity, RemainingToRemove);
            InventorySlots[i].Quantity -= AmountToRemove;
            RemainingToRemove -= AmountToRemove;

            // Xóa slot nếu quantity = 0
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

    // Update weight và broadcast
    RecalculateWeight();
    OnInventoryUpdated.Broadcast();
    OnItemRemoved.Broadcast(ItemID, Quantity);

    return true;
}

// ============================================
// USE ITEM
// ============================================
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

// ============================================
// QUERY FUNCTIONS
// ============================================
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

// ============================================
// QUICKBAR
// ============================================
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
        QuickbarSlots[QuickbarIndex] = *Slot;
        OnInventoryUpdated.Broadcast();
        return true;
    }

    return false;
}

bool UInventoryComponent::UseQuickbarSlot(int32 QuickbarIndex)
{
    if (QuickbarIndex < 0 || QuickbarIndex >= QuickbarSize)
    {
        return false;
    }

    FInventorySlot& Slot = QuickbarSlots[QuickbarIndex];
    if (Slot.IsValid())
    {
        return UseItem(Slot.ItemID);
    }

    return false;
}

FInventorySlot UInventoryComponent::GetQuickbarSlot(int32 Index) const
{
    if (Index >= 0 && Index < QuickbarSlots.Num())
    {
        return QuickbarSlots[Index];
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

    // Get item data
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

    // Tính vị trí drop (trước mặt owner) + một chút jitter để các item không chồng lên nhau
    const FRotator OwnerRot = Owner->GetActorRotation();
    const FVector BaseLocation = Owner->GetActorLocation() + OwnerRot.Vector() * 150.0f + FVector(0, 0, 50.0f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // Spawn từng actor một (1 item per pickup). Nếu bạn muốn spawn 1 actor chứa nhiều quantity,
    // cần đảm bảo ItemPickupActor hỗ trợ quantity stack.
    for (int32 i = 0; i < Quantity; ++i)
    {
        FVector Jitter = FVector(FMath::RandRange(-30.0f, 30.0f), FMath::RandRange(-30.0f, 30.0f), i * 6.0f);
        FVector SpawnLocation = BaseLocation + Jitter;

        AItemPickupActor* Dropped = World->SpawnActor<AItemPickupActor>(
            ItemData.PickupActorClass,
            SpawnLocation,
            FRotator::ZeroRotator,
            SpawnParams
        );

        if (Dropped)
        {
            // Nếu ItemPickupActor có hàm để thiết lập thông tin item từ struct
            // (PlayerController trước của bạn đã gọi SetItemDataByStruct), gọi nó:
            // --- sửa tên hàm nếu khác ---
            //Dropped->SetItemDataByStruct(ItemData);

            // Nếu ItemPickupActor có property quantity, có thể set ở đây (tuỳ implementation)
            // Dropped->SetQuantity(1);

            // Bật vật lý và add impulse để item bay ra 1 chút
            if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Dropped->GetRootComponent()))
            {
                if (!RootPrim->IsSimulatingPhysics())
                {
                    RootPrim->SetSimulatePhysics(true);
                }
                FVector ImpulseDir = OwnerRot.Vector() + FVector(0.0f, 0.0f, 0.25f);
                ImpulseDir.Normalize();
                RootPrim->AddImpulse(ImpulseDir * 250.0f, NAME_None, true);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("DropItem: Failed to spawn pickup actor for %s"), *ItemID.ToString());
            // continue trying to spawn remaining ones (or you can early-return false)
        }
    }

    // Remove items from inventory (this will also handle stacks and slot removal)
    bool bRemoved = RemoveItem(ItemID, Quantity);
    if (!bRemoved)
    {
        UE_LOG(LogTemp, Warning, TEXT("DropItem: Failed to remove items from inventory after spawn"));
        // still proceed but inform user
    }

    // Update weight and broadcast (RemoveItem already broadcasts, but ensure weight correct)
    RecalculateWeight();
    OnInventoryUpdated.Broadcast();
    OnItemRemoved.Broadcast(ItemID, Quantity);

    UE_LOG(LogTemp, Log, TEXT("DropItem: Dropped %d x %s"), Quantity, *ItemData.ItemName.ToString());
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
        QSlot.Quantity = FMath::Min(RemainingTotal, QSlot.Quantity); // giữ nguyên kích thước hiển thị hoặc điều chỉnh tuỳ logic
    }

    OnInventoryUpdated.Broadcast();
    return true;
}


// ============================================
// UTILITY
// ============================================
bool UInventoryComponent::IsInventoryFull() const
{
    return InventorySlots.Num() >= MaxInventorySlots;
}

bool UInventoryComponent::IsOverWeight() const
{
    return bUseWeightSystem && CurrentWeight > MaxWeight;
}

float UInventoryComponent::GetWeightPercentage() const
{
    if (!bUseWeightSystem || MaxWeight <= 0.0f)
    {
        return 0.0f;
    }
    return (CurrentWeight / MaxWeight) * 100.0f;
}

void UInventoryComponent::ClearInventory()
{
    InventorySlots.Empty();
    for (int32 i = 0; i < QuickbarSize; i++)
    {
        QuickbarSlots[i] = FInventorySlot();
    }
    CurrentWeight = 0.0f;
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
    UE_LOG(LogTemp, Log, TEXT("Weight: %.1f / %.1f"), CurrentWeight, MaxWeight);
    UE_LOG(LogTemp, Log, TEXT("==============================="));
}

// ============================================
// INTERNAL FUNCTIONS
// ============================================
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

void UInventoryComponent::RecalculateWeight()
{
    if (!bUseWeightSystem)
    {
        return;
    }

    CurrentWeight = 0.0f;

    for (const FInventorySlot& Slot : InventorySlots)
    {
        FItemData ItemData;
        if (GetItemData(Slot.ItemID, ItemData))
        {
            CurrentWeight += ItemData.Weight * Slot.Quantity;
        }
    }
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
            // Giả sử có SanityComponent
            // USanityComponent* SanityComp = Owner->FindComponentByClass<USanityComponent>();
            // if (SanityComp)
            // {
            //     SanityComp->RestoreSanity(ItemData.SanityRestoreAmount);
            // }

            UE_LOG(LogTemp, Log, TEXT("ApplyItemEffect: Restore Sanity +%.1f"), ItemData.SanityRestoreAmount);
        }
    }

    // TODO: Apply passive effects (Teddy Bear)
    // TODO: Apply special effects (Decoy, Stun)
}

void UInventoryComponent::UpdateCooldowns(float DeltaTime)
{
    bool bNeedUpdate = false;

    for (FInventorySlot& Slot : InventorySlots)
    {
        if (Slot.CooldownRemaining > 0.0f)
        {
            Slot.CooldownRemaining -= DeltaTime;
            if (Slot.CooldownRemaining < 0.0f)
            {
                Slot.CooldownRemaining = 0.0f;
            }
            bNeedUpdate = true;
        }
    }

    // Update quickbar cooldowns
    for (FInventorySlot& Slot : QuickbarSlots)
    {
        if (Slot.CooldownRemaining > 0.0f)
        {
            Slot.CooldownRemaining -= DeltaTime;
            if (Slot.CooldownRemaining < 0.0f)
            {
                Slot.CooldownRemaining = 0.0f;
            }
            bNeedUpdate = true;
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