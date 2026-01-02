// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/PowerSystemManager.h"

void UPowerSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bIsPowerOn = true;
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
