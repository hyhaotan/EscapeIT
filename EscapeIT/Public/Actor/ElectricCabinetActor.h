// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Door/DoorActor.h"
#include "ElectricCabinetActor.generated.h"

UCLASS()
class ESCAPEIT_API AElectricCabinetActor : public ADoorActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AElectricCabinetActor();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	virtual void CalculateDoorOpenDirection_Implementation(AActor* Interactor) override;
	
	virtual void Interact_Implementation(AActor* Interactor) override;
};
