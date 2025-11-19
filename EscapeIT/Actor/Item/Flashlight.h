// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EscapeIT/Actor/ItemPickupActor.h"
#include "Flashlight.generated.h"

class USpotLightComponent;
class UFlashlightComponent;

UCLASS()
class ESCAPEIT_API AFlashlight : public AItemPickupActor
{
	GENERATED_BODY()
	
protected:
	AFlashlight();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flashlight")
	USpotLightComponent* SpotLightComponent;
	
};
