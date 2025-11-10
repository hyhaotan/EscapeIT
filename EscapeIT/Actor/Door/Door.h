// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Door.generated.h"

class UTimelineComponent;
class UStaticMeshComponent;
class USceneComponent;
class UCurveFloat;

UCLASS()
class ESCAPEIT_API ADoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<USceneComponent> DoorPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<UStaticMeshComponent> DoorFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<UStaticMeshComponent> Door;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	TObjectPtr<UTimelineComponent> DoorTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Animation")
	TObjectPtr<UCurveFloat> DoorCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Settings")
	float OpenAngle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|State")
	bool bIsOpen;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door|State")
	FRotator DoorRotationTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Animation")
	float AnimationDuration;

public:
	UFUNCTION()
	void UpdateDoorRotation(float Value);

	UFUNCTION()
	void OpenDoor();

	UFUNCTION()
	void CloseDoor();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Door")
	bool IsOpen() const { return bIsOpen; }
};
