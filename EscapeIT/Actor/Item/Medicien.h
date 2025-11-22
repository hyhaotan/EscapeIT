// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemPickupActor.h"
#include "Medicien.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPEIT_API AMedicien : public AItemPickupActor
{
	GENERATED_BODY()
public:
	AMedicien();
	
	virtual void UseItem_Implementation() override;
	
protected:
	virtual  void BeginPlay() override;
	
private:
	void RestoreSanity();
};
