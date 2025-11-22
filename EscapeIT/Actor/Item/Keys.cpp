// Fill out your copyright notice in the Description page of Project Settings.

#include "EscapeIT/Actor/Item/Keys.h"

AKeys::AKeys()
{
	PrimaryActorTick.bCanEverTick = false;
	
	KeyType = EKeysType::Key1;
}

void AKeys::UseItem_Implementation()
{
	Super::UseItem_Implementation();
    
	UE_LOG(LogTemp, Log, TEXT("Using Key: %d"), (int32)KeyType);
    
	if (TryUnlockDoor())
	{
		UE_LOG(LogTemp, Log, TEXT("Key used successfully - Door unlocked!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No matching door found in range"));
	}
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
	return true;
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
	FVector End = Start + (ForwardVector * 200.0f); // 2m range

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PlayerPawn);

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
		return HitResult.GetActor();
	}

	return nullptr;
}

void AKeys::BeginPlay()
{
	Super::BeginPlay();
}
