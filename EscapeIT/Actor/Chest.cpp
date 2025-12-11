// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeIT/Actor/Chest.h"
#include "Components/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TimelineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/Character.h"
#include "EscapeIT/UI/NotificationWidget.h"

AChest::AChest()
{
    PrimaryActorTick.bCanEverTick = true;
    
    ChestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestMesh"));
    RootComponent = ChestMesh;
    
    LidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LidMesh"));
    LidMesh->SetupAttachment(ChestMesh);
    
    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(ChestMesh);
    InteractionBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
    
    InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    OpenTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("OpenTimeline"));
    
    // Default settings
    ChestType = EKeyType::BasementKey;
    RequiredKeyType = EKeyType::BasementKey;
}

void AChest::BeginPlay()
{
    Super::BeginPlay();
    
    // Setup timeline animation
    if (ChestCurve && OpenTimeline)
    {
        FOnTimelineFloat TimelineCallback;
        TimelineCallback.BindUFunction(this, FName("UpdateLidRotation"));
        OpenTimeline->AddInterpFloat(ChestCurve, TimelineCallback);

        FOnTimelineEvent TimelineFinishedCallback;
        TimelineFinishedCallback.BindUFunction(this, FName("OnOpenFinished"));
        OpenTimeline->SetTimelineFinishedFunc(TimelineFinishedCallback);
    }

    if (NotificationWidgetClass)
    {
        NotificationWidget = CreateWidget<UNotificationWidget>(GetWorld(), NotificationWidgetClass);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Chest '%s' initialized - Type: %s, RequiresKey: %s"), 
        *GetName(),
        *UEnum::GetValueAsString(ChestType),
        bRequiresKey ? TEXT("Yes") : TEXT("No"));
}

void AChest::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AChest::Interact_Implementation(AActor* Interactor)
{
    if (!Interactor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Chest Interact: No Interactor!"));
        return;
    }

    // Chest đã mở rồi
    if (bIsOpen)
    {
        if (NotificationWidget)
        {
            NotificationWidget->ShowNotification(FText::FromString("Chest đã được mở rồi!"));
        }
        UE_LOG(LogTemp, Log, TEXT("Chest '%s' already opened"), *GetName());
        return;
    }
    
    // Kiểm tra điều kiện mở chest
    FString FailReason;
    if (CheckCanOpenChest(Interactor, FailReason))
    {
        OpenChest(Interactor);
    }
    else
    {
        PlayChestSound(LockedSound);
        if (NotificationWidget && !FailReason.IsEmpty())
        {
            NotificationWidget->ShowNotification(FText::FromString(FailReason));
        }
        UE_LOG(LogTemp, Warning, TEXT("Cannot open chest '%s': %s"), *GetName(), *FailReason);
    }
}

bool AChest::CheckCanOpenChest(AActor* Interactor, FString& OutFailReason)
{
    UInventoryComponent* Inventory = GetInventoryFromActor(Interactor);
    if (!Inventory)
    {
        OutFailReason = TEXT("Không tìm thấy Inventory!");
        return false;
    }
    
    // Nếu chest không cần key thì mở luôn
    if (!bRequiresKey)
    {
        UE_LOG(LogTemp, Log, TEXT("Chest '%s' does not require key"), *GetName());
        return true;
    }
    
    // Tìm key phù hợp trong inventory
    FName RequiredKeyItemID = FindMatchingKeyItemID(Inventory, RequiredKeyType);
    
    if (RequiredKeyItemID.IsNone())
    {
        OutFailReason = FString::Printf(TEXT("Không tìm thấy key phù hợp trong inventory! (Cần: %s)"), 
            *UEnum::GetValueAsString(RequiredKeyType));
        return false;
    }
    
    // Kiểm tra xem có key trong inventory không
    if (!Inventory->HasItem(RequiredKeyItemID, 1))
    {
        OutFailReason = FString::Printf(TEXT("Bạn không có key phù hợp! (Cần: %s)"), 
            *UEnum::GetValueAsString(RequiredKeyType));
        return false;
    }
    
    // Kiểm tra xem có yêu cầu equip key không
    if (bRequireKeyEquipped)
    {
        FName EquippedItemID = Inventory->GetCurrentEquippedItemID();
        
        if (EquippedItemID != RequiredKeyItemID)
        {
            OutFailReason = TEXT("Bạn cần trang bị key vào tay để mở chest!");
            return false;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Chest '%s' can be opened with key: %s"), 
        *GetName(), *RequiredKeyItemID.ToString());
    return true;
}

void AChest::OpenChest(AActor* Interactor)
{
    UInventoryComponent* Inventory = GetInventoryFromActor(Interactor);
    if (!Inventory)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot open chest: No inventory found!"));
        return;
    }
    
    PlayChestSound(UnlockSound);
    
    // Tiêu hao key nếu cần
    if (bConsumeKeyOnOpen && bRequiresKey)
    {
        FName RequiredKeyItemID = FindMatchingKeyItemID(Inventory, RequiredKeyType);
        
        if (!RequiredKeyItemID.IsNone())
        {
            Inventory->RemoveItem(RequiredKeyItemID, 1);
            UE_LOG(LogTemp, Log, TEXT("Key consumed: %s"), *RequiredKeyItemID.ToString());
        }
    }
    
    // Thêm items vào inventory
    int32 ItemsAdded = 0;
    TArray<FString> ItemNames;
    
    for (const FName& ItemIDs : ChestItems)
    {
        if (!ItemIDs.IsNone())
        {
            bool bAdded = Inventory->AddItem(ItemIDs, 1);
            if (bAdded)
            {
                ItemsAdded++;
                
                // Lấy tên item để hiển thị
                if (Inventory->ItemDataTable)
                {
                    FItemData* ItemData = Inventory->ItemDataTable->FindRow<FItemData>(ItemIDs, TEXT(""));
                    if (ItemData)
                    {
                        ItemNames.Add(ItemData->ItemName.ToString());
                    }
                }
                
                UE_LOG(LogTemp, Log, TEXT("Item added to inventory: %s"), *ItemIDs.ToString());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Inventory full, cannot add item: %s"), *ItemIDs.ToString());
            }
        }
    }
    
    // Cập nhật trạng thái chest
    bIsOpen = true;
    bIsLocked = false;
    
    // Animation mở nắp
    if (OpenTimeline && ChestCurve)
    {
        OpenTimeline->PlayFromStart();
    }
    
    PlayChestSound(OpenSound);

    // Hiển thị thông báo
    if (NotificationWidget)
    {
        FString Message;
        if (ItemsAdded > 0)
        {
            Message = FString::Printf(TEXT("Chest đã được mở! Nhận được %d item(s)"), ItemsAdded);
            
            if (ItemNames.Num() > 0)
            {
                Message += TEXT("\n");
                for (int32 i = 0; i < ItemNames.Num(); i++)
                {
                    Message += ItemNames[i];
                    if (i < ItemNames.Num() - 1)
                    {
                        Message += TEXT(", ");
                    }
                }
            }
        }
        else
        {
            Message = TEXT("Chest đã được mở! (Rỗng)");
        }
        
        NotificationWidget->ShowNotification(FText::FromString(Message));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Chest '%s' opened successfully! Items added: %d"), 
        *GetName(), ItemsAdded);
}

void AChest::UpdateLidRotation(float Value)
{
    if (LidMesh)
    {
        FRotator NewRotation = FRotator(Value * MaxLidRotation, 0.0f, 0.0f);
        LidMesh->SetRelativeRotation(NewRotation);
    }
}

void AChest::OnOpenFinished()
{
    UE_LOG(LogTemp, Log, TEXT("Chest '%s' open animation finished!"), *GetName());
}

void AChest::PlayChestSound(USoundBase* Sound)
{
    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
    }
}

UInventoryComponent* AChest::GetInventoryFromActor(AActor* Actor) const
{
    if (!Actor)
    {
        return nullptr;
    }
    
    // Thử tìm component trực tiếp
    UInventoryComponent* Inventory = Actor->FindComponentByClass<UInventoryComponent>();
    
    // Nếu không có, thử cast sang Character/Pawn
    if (!Inventory)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            Inventory = Character->FindComponentByClass<UInventoryComponent>();
        }
        else if (APawn* Pawn = Cast<APawn>(Actor))
        {
            Inventory = Pawn->FindComponentByClass<UInventoryComponent>();
        }
    }

    return Inventory;
}

FName AChest::FindMatchingKeyItemID(UInventoryComponent* Inventory, EKeyType InKeyType) const
{
    if (!Inventory || !Inventory->ItemDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("No inventory or DataTable!"));
        return NAME_None;
    }
    
    // Duyệt qua tất cả items trong DataTable
    TArray<FName> RowNames = Inventory->ItemDataTable->GetRowNames();
    
    for (const FName& RowName : RowNames)
    {
        FItemData* ItemData = Inventory->ItemDataTable->FindRow<FItemData>(RowName, TEXT(""));
        
        // Tìm item có type là Key và KeyType khớp
        if (ItemData && 
            ItemData->ItemType == EItemType::Key && 
            ItemData->KeyType == InKeyType)
        {
            UE_LOG(LogTemp, Log, TEXT("Found matching key: %s for KeyType: %s"), 
                *ItemData->ItemID.ToString(),
                *UEnum::GetValueAsString(InKeyType));
            return ItemData->ItemID;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("No matching key found in DataTable for KeyType: %s"), 
        *UEnum::GetValueAsString(InKeyType));
    return NAME_None;
}

bool AChest::CanBeOpenedWithKey(EKeyType InKeyType) const
{
    // Chest đã mở rồi
    if (bIsOpen)
    {
        UE_LOG(LogTemp, Log, TEXT("Chest '%s' is already open"), *GetName());
        return false;
    }
    
    // Chest không cần key
    if (!bRequiresKey)
    {
        UE_LOG(LogTemp, Log, TEXT("Chest '%s' does not require key"), *GetName());
        return true;
    }
    
    // So sánh KeyType
    bool bCanOpen = (RequiredKeyType == InKeyType);
    
    UE_LOG(LogTemp, Log, TEXT("Chest '%s' CanBeOpenedWithKey(%s): %s (Required: %s)"), 
        *GetName(),
        *UEnum::GetValueAsString(InKeyType),
        bCanOpen ? TEXT("YES") : TEXT("NO"),
        *UEnum::GetValueAsString(RequiredKeyType));
    
    return bCanOpen;
}

void AChest::UnlockWithKey(AActor* Interactor, EKeyType InKeyType)
{
    if (!CanBeOpenedWithKey(InKeyType))
    {
        PlayChestSound(LockedSound);
        if (NotificationWidget)
        {
            NotificationWidget->ShowNotification(FText::FromString("Key không phù hợp!"));
        }
        UE_LOG(LogTemp, Warning, TEXT("Wrong key type for chest '%s'!"), *GetName());
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Unlocking chest '%s' with key type: %s"), 
        *GetName(), *UEnum::GetValueAsString(InKeyType));
    
    OpenChest(Interactor);
}