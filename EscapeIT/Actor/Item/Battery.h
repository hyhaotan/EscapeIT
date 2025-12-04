// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemPickupActor.h"
#include "Battery.generated.h"

UCLASS()
class ESCAPEIT_API ABattery : public AItemPickupActor
{
	GENERATED_BODY()
	
public:
	ABattery();

protected:
	virtual void BeginPlay() override;
};
