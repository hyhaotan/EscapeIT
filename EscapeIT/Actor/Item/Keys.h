// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/ItemPickupActor.h"
#include "EscapeIT/Actor/Item/Keys.h"
#include "EscapeIT/Data/ItemData.h"
#include "Keys.generated.h"

class AChest;

UCLASS()
class ESCAPEIT_API AKeys : public AItemPickupActor
{
	GENERATED_BODY()
    
public:    
	AKeys();
    
	virtual void UseItem_Implementation() override;
    
protected:
	virtual void BeginPlay() override;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Settings")
	EKeyType KeyType;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Key Settings")
	float InteractionRange = 200.0f; // 2 meters
    
private:
	// Chest interaction
	bool TryUnlockChest();
	AChest* FindChestInRange();
	AChest* FindNearestChest();
    
	// Door interaction (TODO)
	bool TryUnlockDoor();
	AActor* FindDoorInRange();
};