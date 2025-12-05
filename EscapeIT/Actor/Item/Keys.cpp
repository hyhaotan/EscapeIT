// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeIT/Actor/Item/Keys.h"
#include "EscapeIT/Actor/Chest.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

AKeys::AKeys()
{
    PrimaryActorTick.bCanEverTick = false;
    KeyType = EKeyType::BasementKey;
}

void AKeys::BeginPlay()
{
    Super::BeginPlay();
}

void AKeys::UseItem_Implementation()
{
    Super::UseItem_Implementation();
    
    UE_LOG(LogTemp, Log, TEXT("Using Key: %s"), *UEnum::GetValueAsString(KeyType));
    
    // Thử mở chest trước
    if (TryUnlockChest())
    {
        UE_LOG(LogTemp, Log, TEXT("Key used successfully - Chest unlocked!"));
        return;
    }
    
    // Nếu không có chest, thử mở door
    if (TryUnlockDoor())
    {
        UE_LOG(LogTemp, Log, TEXT("Key used successfully - Door unlocked!"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("No matching chest or door found in range"));
}

bool AKeys::TryUnlockChest()
{
    AChest* ChestActor = FindChestInRange();
    
    if (!ChestActor)
    {
        return false;
    }
    
    // Kiểm tra xem chest có thể mở bằng key này không
    if (!ChestActor->CanBeOpenedWithKey(KeyType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Wrong key type for this chest!"));
        return false;
    }
    
    // Mở chest
    APawn* PlayerPawn = GetPlayerPawn();
    if (PlayerPawn)
    {
        ChestActor->UnlockWithKey(PlayerPawn, KeyType);
        return true;
    }
    
    return false;
}

AChest* AKeys::FindChestInRange()
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return nullptr;
    }

    FVector Start = PlayerPawn->GetActorLocation();
    FVector ForwardVector = PlayerPawn->GetActorForwardVector();
    FVector End = Start + (ForwardVector * InteractionRange);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(PlayerPawn);
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        Params
    );

    // Debug line
    DrawDebugLine(
        GetWorld(),
        Start,
        End,
        bHit ? FColor::Green : FColor::Red,
        false,
        2.0f,
        0,
        2.0f
    );

    if (bHit)
    {
        AChest* Chest = Cast<AChest>(HitResult.GetActor());
        if (Chest)
        {
            UE_LOG(LogTemp, Log, TEXT("Found chest: %s"), *Chest->GetName());
            return Chest;
        }
    }
    
    // Nếu line trace không hit, thử tìm chest gần nhất trong phạm vi
    return FindNearestChest();
}

AChest* AKeys::FindNearestChest()
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return nullptr;
    }

    TArray<AActor*> FoundChests;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AChest::StaticClass(), FoundChests);
    
    AChest* NearestChest = nullptr;
    float NearestDistance = InteractionRange;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (AActor* Actor : FoundChests)
    {
        if (AChest* Chest = Cast<AChest>(Actor))
        {
            float Distance = FVector::Dist(PlayerLocation, Chest->GetActorLocation());
            
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestChest = Chest;
            }
        }
    }
    
    if (NearestChest)
    {
        UE_LOG(LogTemp, Log, TEXT("Found nearest chest: %s at distance: %.2f"), 
            *NearestChest->GetName(), NearestDistance);
    }
    
    return NearestChest;
}

bool AKeys::TryUnlockDoor()
{
    AActor* DoorActor = FindDoorInRange();
    
    if (!DoorActor)
    {
        return false;
    }
    
    // TODO: Implement door unlock interface
    // Example:
    // if (IDoorInterface* Door = Cast<IDoorInterface>(DoorActor))
    // {
    //     if (Door->GetRequiredKeyType() == KeyType)
    //     {
    //         Door->Unlock();
    //         return true;
    //     }
    //     else
    //     {
    //         UE_LOG(LogTemp, Warning, TEXT("Wrong key type!"));
    //         return false;
    //     }
    // }
    
    UE_LOG(LogTemp, Log, TEXT("Found door: %s"), *DoorActor->GetName());
    return false; // Chưa implement
}

AActor* AKeys::FindDoorInRange()
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        return nullptr;
    }

    FVector Start = PlayerPawn->GetActorLocation();
    FVector ForwardVector = PlayerPawn->GetActorForwardVector();
    FVector End = Start + (ForwardVector * InteractionRange);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(PlayerPawn);
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        Params
    );

    if (bHit)
    {
        AActor* HitActor = HitResult.GetActor();
        // TODO: Check if it's a door
        // if (HitActor && HitActor->IsA(ADoor::StaticClass()))
        // {
        //     return HitActor;
        // }
    }

    return nullptr;
}