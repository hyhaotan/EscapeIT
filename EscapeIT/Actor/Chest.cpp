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
    
    RequiredKeyType = EKeyType::BasementKey;
}

void AChest::BeginPlay()
{
    Super::BeginPlay();
    
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
}

void AChest::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AChest::Interact_Implementation(AActor* Interactor)
{
    if (!Interactor)
    {
        return;
    }

    if (bIsOpen)
    {
        if (NotificationWidget)
        {
            NotificationWidget->ShowNotification(FText::FromString("Chest đã được mở rồi!"));
        }
        return;
    }
    
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
        return true;
    }
    
    FName RequiredKeyItemID = NAME_None;
    
    if (Inventory->ItemDataTable)
    {
        TArray<FName> RowNames = Inventory->ItemDataTable->GetRowNames();
        
        for (const FName& RowName : RowNames)
        {
            FItemData* ItemData = Inventory->ItemDataTable->FindRow<FItemData>(RowName, TEXT(""));
            if (ItemData && 
                ItemData->ItemType == EItemType::Key && 
                ItemData->KeyType == RequiredKeyType)
            {
                RequiredKeyItemID = ItemData->ItemID;
                break;
            }
        }
    }

    if (RequiredKeyItemID.IsNone())
    {
        OutFailReason = FString::Printf(TEXT("Không tìm thấy key phù hợp trong DataTable! (Cần: %s)"), 
            *UEnum::GetValueAsString(RequiredKeyType));
        return false;
    }
    
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
    
    return true;
}

void AChest::OpenChest(AActor* Interactor)
{
    UInventoryComponent* Inventory = GetInventoryFromActor(Interactor);
    if (!Inventory)
    {
        return;
    }
    
    PlayChestSound(UnlockSound);
    
    // Tiêu hao key nếu cần
    if (bConsumeKeyOnOpen && bRequiresKey)
    {
        FName RequiredKeyItemID = NAME_None;
        
        if (Inventory->ItemDataTable)
        {
            TArray<FName> RowNames = Inventory->ItemDataTable->GetRowNames();
            
            for (const FName& RowName : RowNames)
            {
                FItemData* ItemData = Inventory->ItemDataTable->FindRow<FItemData>(RowName, TEXT(""));
                if (ItemData && 
                    ItemData->ItemType == EItemType::Key && 
                    ItemData->KeyType == RequiredKeyType)
                {
                    RequiredKeyItemID = ItemData->ItemID;
                    break;
                }
            }
        }

        if (!RequiredKeyItemID.IsNone())
        {
            Inventory->RemoveItem(RequiredKeyItemID, 1);
            UE_LOG(LogTemp, Log, TEXT("Key đã bị tiêu hao: %s"), *RequiredKeyItemID.ToString());
        }
    }
    
    // Thêm items vào inventory
    int32 ItemsAdded = 0;
    for (const FName& ItemID : ChestItems)
    {
        if (!ItemID.IsNone())
        {
            bool bAdded = Inventory->AddItem(ItemID, 1);
            if (bAdded)
            {
                ItemsAdded++;
                UE_LOG(LogTemp, Log, TEXT("Đã nhận item: %s"), *ItemID.ToString());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Inventory đầy, không thể nhận item: %s"), *ItemID.ToString());
            }
        }
    }
    
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
        FString Message = FString::Printf(TEXT("Chest đã được mở! Nhận được %d item(s)"), ItemsAdded);
        NotificationWidget->ShowNotification(FText::FromString(Message));
    }
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
    UE_LOG(LogTemp, Log, TEXT("Animation mở chest hoàn tất!"));
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
    
    UInventoryComponent* Inventory = Actor->FindComponentByClass<UInventoryComponent>();
    
    if (!Inventory)
    {
        if (APawn* Pawn = Cast<ACharacter>(Actor))
        {
            Inventory = Pawn->FindComponentByClass<UInventoryComponent>();
        }
    }

    return Inventory;
}

bool AChest::CanBeOpenedWithKey(EKeyType InKeyType) const
{
    if (bIsOpen) return false;
    if (!bRequiresKey) return true;
    return RequiredKeyType == InKeyType;
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
        return;
    }
    
    OpenChest(Interactor);
}