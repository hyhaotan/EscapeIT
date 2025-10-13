// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EscapeIT/Actor/ItemPickupActor.h"
#include "Flashlight.generated.h"

UCLASS()
class ESCAPEIT_API AFlashlight : public AItemPickupActor
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sportlight")
	class USpotLightComponent* SpotLightComponent;

	AFlashlight();
};
