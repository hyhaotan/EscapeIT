// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeIT/Actor/Item/Keys.h"
#include "EscapeIT/Actor/Chest.h"
#include "DrawDebugHelpers.h"
#include "Actor/Components/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"

AKeys::AKeys()
{
    PrimaryActorTick.bCanEverTick = false;
    KeyType = EKeyType::BasementKey;
    InteractionRange = 300.0f; // Default 3 meters
}

void AKeys::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Key '%s' initialized with KeyType: %s"), 
        *GetName(), *UEnum::GetValueAsString(KeyType));
}

void AKeys::UseItem_Implementation()
{
    Super::UseItem_Implementation();
    
    UInventoryComponent* InvComp = GetPlayerPawn()->FindComponentByClass<UInventoryComponent>();

    if (!InvComp->HasItem(ItemID,1)) return;
    
    UE_LOG(LogTemp, Log, TEXT("=== Using Key: %s ==="), *UEnum::GetValueAsString(KeyType));
    
    // Thử mở chest trước
    if (TryUnlockChest())
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Key used successfully - Chest unlocked!"));
        return;
    }
    
    // Nếu không có chest, thử mở door
    if (TryUnlockDoor())
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Key used successfully - Door unlocked!"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("✗ No matching chest or door found in range"));
    ShowNoTargetNotification();
}

bool AKeys::TryUnlockChest()
{
    AChest* ChestActor = FindChestInRange();
    
    if (!ChestActor)
    {
        UE_LOG(LogTemp, Log, TEXT("No chest found in range"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Found chest: %s"), *ChestActor->GetName());
    
    // Kiểm tra xem chest có thể mở bằng key này không
    if (!ChestActor->CanBeOpenedWithKey(KeyType))
    {
        UE_LOG(LogTemp, Warning, TEXT("✗ Wrong key type for this chest!"));
        ShowWrongKeyNotification();
        return false;
    }
    
    // Mở chest
    APawn* PlayerPawn = GetPlayerPawn();
    if (PlayerPawn)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Correct key! Opening chest..."));
        ChestActor->UnlockWithKey(PlayerPawn, KeyType);
        
        // Consume key sau khi dùng (nếu chest yêu cầu)
        // Logic tiêu hao key đã được xử lý trong ChestActor->UnlockWithKey
        
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("✗ No player pawn found!"));
    return false;
}

AChest* AKeys::FindChestInRange()
{
    APawn* PlayerPawn = GetPlayerPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("No player pawn!"));
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

    // Debug line (Green = hit, Red = no hit)
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
            UE_LOG(LogTemp, Log, TEXT("Line trace found chest: %s"), *Chest->GetName());
            
            // Debug point at chest location
            DrawDebugSphere(
                GetWorld(),
                Chest->GetActorLocation(),
                50.0f,
                12,
                FColor::Cyan,
                false,
                2.0f
            );
            
            return Chest;
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Line trace hit: %s (not a chest)"), 
                *HitResult.GetActor()->GetName());
        }
    }
    
    // Nếu line trace không hit chest, thử tìm chest gần nhất trong phạm vi
    UE_LOG(LogTemp, Log, TEXT("Line trace didn't hit chest, searching for nearest..."));
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
    
    if (FoundChests.Num() == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("No chests found in level"));
        return nullptr;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Found %d chest(s) in level, finding nearest..."), FoundChests.Num());
    
    AChest* NearestChest = nullptr;
    float NearestDistance = InteractionRange;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (AActor* Actor : FoundChests)
    {
        if (AChest* Chest = Cast<AChest>(Actor))
        {
            float Distance = FVector::Dist(PlayerLocation, Chest->GetActorLocation());
            
            UE_LOG(LogTemp, Log, TEXT("  - Chest '%s' at distance: %.2f cm"), 
                *Chest->GetName(), Distance);
            
            if (Distance <= NearestDistance)
            {
                NearestDistance = Distance;
                NearestChest = Chest;
            }
        }
    }
    
    if (NearestChest)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Found nearest chest: %s at distance: %.2f cm"), 
            *NearestChest->GetName(), NearestDistance);
        
        // Debug visualization
        DrawDebugSphere(
            GetWorld(),
            NearestChest->GetActorLocation(),
            50.0f,
            12,
            FColor::Green,
            false,
            2.0f
        );
        
        DrawDebugLine(
            GetWorld(),
            PlayerLocation,
            NearestChest->GetActorLocation(),
            FColor::Yellow,
            false,
            2.0f,
            0,
            3.0f
        );
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✗ No chest found within range (%.2f cm)"), InteractionRange);
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
    
    UE_LOG(LogTemp, Log, TEXT("Found door: %s"), *DoorActor->GetName());
    
    // TODO: Implement door interface
    // Example implementation:
    /*
    if (IDoorInterface* Door = Cast<IDoorInterface>(DoorActor))
    {
        if (Door->GetRequiredKeyType() == KeyType)
        {
            Door->Unlock();
            UE_LOG(LogTemp, Log, TEXT("✓ Door unlocked!"));
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("✗ Wrong key type for door!"));
            ShowWrongKeyNotification();
            return false;
        }
    }
    */
    
    UE_LOG(LogTemp, Warning, TEXT("Door interface not implemented yet"));
    return false;
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
        
        // TODO: Implement door detection
        // Example:
        // if (HitActor && HitActor->Implements<UDoorInterface>())
        // {
        //     return HitActor;
        // }
        
        // For now, check by class name (temporary)
        if (HitActor && HitActor->GetName().Contains(TEXT("Door")))
        {
            UE_LOG(LogTemp, Log, TEXT("Found potential door: %s"), *HitActor->GetName());
            return HitActor;
        }
    }

    return nullptr;
}

void AKeys::ShowWrongKeyNotification()
{
    // TODO: Implement notification system
    UE_LOG(LogTemp, Warning, TEXT(">> NOTIFICATION: Key không phù hợp!"));
    
    // Example:
    // if (UNotificationWidget* NotificationWidget = GetNotificationWidget())
    // {
    //     NotificationWidget->ShowNotification(
    //         FText::FromString("Key không phù hợp với chest/door này!"));
    // }
}

void AKeys::ShowNoTargetNotification()
{
    // TODO: Implement notification system
    UE_LOG(LogTemp, Warning, TEXT(">> NOTIFICATION: Không tìm thấy chest hoặc door nào trong tầm!"));
    
    // Example:
    /*
    if (UNotificationWidget* NotificationWidget = GetNotificationWidget())
    {
        NotificationWidget->ShowNotification(
            FText::FromString("Không có gì để mở trong tầm!"));
    }
    */
}