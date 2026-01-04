// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/PowerSystemManager.h"

#include "StaticMeshComponentHelper.h"

void UPowerSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bIsPowerOn = true;
	PowerOffDuration = 5.0f;

	if (PowerOffSoundSoft.IsNull())
	{
		PowerOffSoundSoft = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/Sound/PowerOff.PowerOff")));
	}
	
	PowerOffSound = PowerOffSoundSoft.LoadSynchronous();
	
	if (!PowerOffSound)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load PowerOff sound!"));
	}
}

void UPowerSystemManager::SetPowerState(bool bNewState)
{
	if (bIsPowerOn != bNewState)
	{
		bIsPowerOn = bNewState;
		OnPowerStateChanged.Broadcast(bIsPowerOn);
	}
}

void UPowerSystemManager::CausePowerFailure()
{
	SetPowerState(false);
}
