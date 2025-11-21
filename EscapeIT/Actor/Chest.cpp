// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeIT/Actor/Chest.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "Components/TimelineComponent.h"

AChest::AChest()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create chest base mesh
    ChestBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestBaseMesh"));
    ChestBaseMesh->SetupAttachment(RootComponent);
    ChestBaseMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ChestBaseMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create chest lid mesh
    ChestLidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestLidMesh"));
    ChestLidMesh->SetupAttachment(ChestBaseMesh);
    ChestLidMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create interaction sphere
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(150.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create prompt widget
    PromptWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
    PromptWidget->SetupAttachment(RootComponent);
    PromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
    PromptWidget->SetDrawSize(FVector2D(200.0f, 50.0f));
    PromptWidget->SetVisibility(false);
    
    OpenTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("OpenTimeline"));

    // Default settings
    bRequiresKey = true;
    RequiredKeyType = EKeyType::RustyKey;
    RequiredKeyID = FName("Key_Rusty");
    bConsumeKeyOnUse = false;
    bIsOpen = false;
    bIsLocked = true;
    
    // Animation settings
    OpenAngle = 90.0f;
    OpenSpeed = 2.0f;
    
    // Internal state
    bIsAnimating = false;
    CurrentLidAngle = 0.0f;
    AnimationProgress = 0.0f;
    bPlayerNearby = false;
    NearbyPlayer = nullptr;
}

void AChest::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeChest();
    
    if (OpenCurve)
    {
        FOnTimelineFloat ProgressFunction;
        ProgressFunction.BindUFunction(this, FName("HandleOpenTimelineProgress"));
        OpenTimeline->AddInterpFloat(OpenCurve, ProgressFunction);
        
        FOnTimelineEvent FinishedEvent;
        FinishedEvent.BindUFunction(this, FName("OnOpenTimelineFinished"));
        OpenTimeline->SetTimelineFinishedFunc(FinishedEvent);
        OpenTimeline->SetLooping(false);
        OpenTimeline->SetPlayRate(OpenSpeed > 0.f ? OpenSpeed : 1.f);
    }
}

void AChest::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Handle chest opening animation
    if (bIsAnimating)
    {
        AnimationProgress += DeltaTime * OpenSpeed;
        
        if (AnimationProgress >= 1.0f)
        {
            AnimationProgress = 1.0f;
            bIsAnimating = false;
        }

        // Calculate rotation with curve if available
        float Alpha = AnimationProgress;
        if (OpenCurve)
        {
            Alpha = OpenCurve->GetFloatValue(AnimationProgress);
        }

        CurrentLidAngle = FMath::Lerp(0.0f, OpenAngle, Alpha);
        
        // Apply rotation to lid
        FRotator NewRotation = InitialLidRotation;
        NewRotation.Pitch += CurrentLidAngle;
        ChestLidMesh->SetRelativeRotation(NewRotation);
    }
}

void AChest::InitializeChest()
{
    // Store initial lid rotation
    if (ChestLidMesh)
    {
        InitialLidRotation = ChestLidMesh->GetRelativeRotation();
    }

    // Set initial lock state
    if (!bRequiresKey)
    {
        bIsLocked = false;
    }
}

void AChest::Interact_Implementation(AActor* Interactor)
{
    if (!Interactor)
    {
        return;
    }

    TryOpenChest(Interactor);
}

bool AChest::TryOpenChest(AActor* Opener)
{
    // Already open
    if (bIsOpen)
    {
        return false;
    }

    // Check if locked and requires key
    if (bIsLocked && bRequiresKey)
    {
        if (HasRequiredKey(Opener))
        {
            // Unlock and open
            UnlockChest();
            
            // Play unlock sound
            if (UnlockSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, UnlockSound, GetActorLocation());
            }
            
            // Consume key if needed
            if (bConsumeKeyOnUse)
            {
                ConsumeKeyFromInventory(Opener);
            }
            
            OpenChest(Opener);
            return true;
        }
        else
        {
            // Play locked sound and broadcast event
            if (LockedSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, LockedSound, GetActorLocation());
            }
            
            OnChestLocked.Broadcast(this, Opener);
            
            // Show message to player (you can implement UI feedback here)
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                    FString::Printf(TEXT("Chest is locked! Requires: %s"), 
                    *RequiredKeyID.ToString()));
            }
            
            return false;
        }
    }

    // Not locked, open directly
    OpenChest(Opener);
    return true;
}

bool AChest::HasRequiredKey(AActor* Interactor) const
{
    if (!Interactor)
    {
        return false;
    }

    // Get inventory component from interactor
    UInventoryComponent* InventoryComp = Interactor->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComp)
    {
        return false;
    }

    // Check if player has the required key
    return InventoryComp->HasItem(RequiredKeyID, 1);
}

void AChest::UnlockChest()
{
    bIsLocked = false;
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Chest Unlocked!"));
    }
}

void AChest::OpenChest(AActor* Opener)
{
    if (bIsOpen || bIsAnimating)
    {
        return;
    }

    bIsOpen = true;
    bIsAnimating = true;

    if (OpenTimeline)
    {
        OpenTimeline->PlayFromStart();
    }

    if (OpenSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, OpenSound, GetActorLocation());
    }

    GiveLootToPlayer(Opener);
    OnChestOpened.Broadcast(this, Opener);
    ShowPrompt(false);
}

void AChest::SpawnLoot()
{
    // This method spawns loot in the world
    // You can implement this if you want items to physically drop
    
    for (const FName& ItemID : LootItems)
    {
        // Spawn item pickup actor
        // Implementation depends on your game's needs
    }
}

void AChest::GiveLootToPlayer(AActor* Player)
{
    if (!Player)
    {
        return;
    }

    UInventoryComponent* InventoryComp = Player->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComp)
    {
        return;
    }

    // Add items from LootItemsWithQuantity map
    for (const TPair<FName, int32>& LootPair : LootItemsWithQuantity)
    {
        bool bSuccess = InventoryComp->AddItem(LootPair.Key, LootPair.Value);
        
        if (bSuccess)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                    FString::Printf(TEXT("Received: %s x%d"), 
                    *LootPair.Key.ToString(), LootPair.Value));
            }
        }
    }

    // Add items from LootItems array (quantity = 1)
    for (const FName& ItemID : LootItems)
    {
        InventoryComp->AddItem(ItemID, 1);
    }
}

void AChest::ConsumeKeyFromInventory(AActor* Player)
{
    if (!Player)
    {
        return;
    }

    UInventoryComponent* InventoryComp = Player->FindComponentByClass<UInventoryComponent>();
    if (InventoryComp)
    {
        InventoryComp->RemoveItem(RequiredKeyID, 1);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
                FString::Printf(TEXT("Used key: %s"), *RequiredKeyID.ToString()));
        }
    }
}

void AChest::ShowPrompt(bool bShow)
{
    if (PromptWidget)
    {
        PromptWidget->SetVisibility(bShow);
    }
}

FText AChest::GetInteractionPrompt() const
{
    if (bIsOpen)
    {
        return FText::FromString(TEXT("Chest (Empty)"));
    }
    
    if (bIsLocked && bRequiresKey)
    {
        return FText::FromString(FString::Printf(TEXT("Locked Chest [Requires: %s]"), 
            *RequiredKeyID.ToString()));
    }
    
    return FText::FromString(TEXT("Press E to Open Chest"));
}

void AChest::OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor)
    {
        return;
    }

    // Check if it's the player (you might want to add a player tag check)
    if (OtherActor->ActorHasTag(FName("Player")))
    {
        bPlayerNearby = true;
        NearbyPlayer = OtherActor;
        
        if (!bIsOpen)
        {
            ShowPrompt(true);
        }
    }
}

void AChest::OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor == NearbyPlayer)
    {
        bPlayerNearby = false;
        NearbyPlayer = nullptr;
        ShowPrompt(false);
    }
}

void AChest::HandleOpenTimelineProgress(float Value)
{
    float Angle = FMath::Lerp(0.0f, OpenAngle, Value);

    if (ChestLidMesh)
    {
        FRotator Rot = InitialLidRotation;
        Rot.Pitch += Angle;
        ChestLidMesh->SetRelativeRotation(Rot);
    }
}

void AChest::OnOpenTimelineFinished()
{
    bIsAnimating = false;

    // Đảm bảo set góc cuối cùng chính xác
    FRotator Rot = InitialLidRotation;
    Rot.Pitch += OpenAngle;
    ChestLidMesh->SetRelativeRotation(Rot);
}
