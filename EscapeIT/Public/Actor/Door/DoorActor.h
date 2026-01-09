// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Door/Door.h"
#include "DoorActor.generated.h"

class UTimelineComponent;
class USoundBase;

UCLASS()
class ESCAPEIT_API ADoorActor : public ADoor
{
	GENERATED_BODY()
	
public:
	ADoorActor();

	// Interaction Interface
	virtual void Interact_Implementation(AActor* Interactor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bIsLocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	FName RequiredKeyID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
	USoundBase* LockedSound;

	// Door Functions
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Door")
	void CalculateDoorOpenDirection(AActor* Interactor);
	virtual void CalculateDoorOpenDirection_Implementation(AActor* Interactor);
	
	virtual void OpenDoor_Implementation() override;
	virtual void CloseDoor_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Door")
	void SetLocked(bool bLocked);

	UFUNCTION(BlueprintCallable, Category = "Door")
	void UnlockWithKey(FName KeyID);

protected:
	virtual void BeginPlay() override;

	void OnDoorLocked(AActor* Interactor);
};