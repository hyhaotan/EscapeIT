// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/ElectricCabinetActor.h"

// Sets default values
AElectricCabinetActor::AElectricCabinetActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AElectricCabinetActor::Interact_Implementation(AActor* Interactor)
{
	IInteract::Interact_Implementation(Interactor);
}

// Called when the game starts or when spawned
void AElectricCabinetActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AElectricCabinetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AElectricCabinetActor::CalculateDoorOpenDirection_Implementation(AActor* Interactor)
{
	Super::CalculateDoorOpenDirection_Implementation(Interactor);
}

