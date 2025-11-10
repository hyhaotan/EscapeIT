// Fill out your copyright notice in the Description page of Project Settings.


#include "CreepyDoorActor.h"

// Sets default values
ACreepyDoorActor::ACreepyDoorActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	OpenAngle = 45.0f;
	bIsOpen = false;
}

// Called when the game starts or when spawned
void ACreepyDoorActor::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this,&ACreepyDoorActor::OpenDoor, 3.0f,false);
}

// Called every frame
void ACreepyDoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

