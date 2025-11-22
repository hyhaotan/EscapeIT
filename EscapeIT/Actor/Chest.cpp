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
    
    // ✅ Setup collision cho interaction
    InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    OpenTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("OpenTimeline"));
    
    RequiredKeyType = EKeysType::Key1;
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

    // ✅ Tạo và thêm widget vào viewport
    if (NotificationWidgetClass)
    {
        NotificationWidget = CreateWidget<UNotificationWidget>(GetWorld(), NotificationWidgetClass);
        if (NotificationWidget)
        {
            NotificationWidget->AddToViewport();
            NotificationWidget->SetVisibility(ESlateVisibility::Hidden);
        }
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
        // ✅ Hiển thị lý do fail
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
                ItemData->ItemType == EItemType::Tool && 
                ItemData->ItemCategory == EItemCategory::MasterKey &&
                ItemData->KeysType == RequiredKeyType)
            {
                RequiredKeyItemID = ItemData->ItemID;
                break;
            }
        }
    }

    if (RequiredKeyItemID.IsNone())
    {
        OutFailReason = TEXT("Không tìm thấy dữ liệu key trong DataTable!");
        return false;
    }
    
    if (!Inventory->HasItem(RequiredKeyItemID, 1))
    {
        OutFailReason = TEXT("Bạn không có key phù hợp để mở chest này!");
        return false;
    }
    
    if (bRequireKeyEquipped)
    {
        FName EquippedItemID = Inventory->GetCurrentEquippedItemID();
        
        if (EquippedItemID != RequiredKeyItemID)
        {
            OutFailReason = TEXT("Bạn cần trang bị key để mở chest!");
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
                    ItemData->ItemType == EItemType::Tool && 
                    ItemData->ItemCategory == EItemCategory::MasterKey &&
                    ItemData->KeysType == RequiredKeyType)
                {
                    RequiredKeyItemID = ItemData->ItemID;
                    break;
                }
            }
        }

        if (!RequiredKeyItemID.IsNone())
        {
            Inventory->RemoveItem(RequiredKeyItemID, 1);
            UE_LOG(LogTemp, Log, TEXT("Key đã bị tiêu hao!"));
        }
    }
    
    for (const FName& ItemID : ChestItems)
    {
        if (!ItemID.IsNone())
        {
            bool bAdded = Inventory->AddItem(ItemID, 1);
            if (bAdded)
            {
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
    
    if (OpenTimeline && ChestCurve)
    {
        OpenTimeline->PlayFromStart();
    }
    
    PlayChestSound(OpenSound);

    if (NotificationWidget)
    {
        NotificationWidget->ShowNotification(FText::FromString("Chest đã được mở!"));
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
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            Inventory = Character->FindComponentByClass<UInventoryComponent>();
        }
    }

    return Inventory;
}