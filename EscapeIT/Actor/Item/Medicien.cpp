// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Item/Medicien.h"

#include  "EscapeIT/Actor/Components/SanityComponent.h"

AMedicien::AMedicien()
{
}

void AMedicien::UseItem_Implementation()
{
	Super::UseItem_Implementation();
	RestoreSanity();
}

void AMedicien::BeginPlay()
{
	Super::BeginPlay();
}

void AMedicien::RestoreSanity()
{
	FItemData ItemData;
	if (!GetItemData(ItemData))
	{
		return;
	}
	
	APawn* Pawn = GetPlayerPawn();
	if (!Pawn)
	{
		return;
	}

	if (const auto Sanity = Pawn->FindComponentByClass<USanityComponent>())
	{
		Sanity->RestoreSanity(ItemData.SanityRestoreAmount);
		UE_LOG(LogTemp, Log, TEXT("Restored %.1f sanity"), ItemData.SanityRestoreAmount);
	} 
	UE_LOG(LogTemp, Log, TEXT("Medicine used - Sanity restored: %.1f"), 
	ItemData.SanityRestoreAmount);
}
