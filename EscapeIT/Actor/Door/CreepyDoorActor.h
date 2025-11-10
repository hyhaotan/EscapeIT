// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EscapeIT/Actor/Door/Door.h"
#include "CreepyDoorActor.generated.h"

class UStaticMeshComponent;
class UTimelineComponent;
class UCurveFloat;
class USceneComponent;

UCLASS()
class ESCAPEIT_API ACreepyDoorActor : public ADoor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACreepyDoorActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
