// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WindowJumpscareActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UTimelineComponent;
class UCurveFloat;
class USceneComponent;
class USanityComponent;
class UCameraComponent;
class AEscapeITCharacter;
class ACameraActor;
class USpringArmComponent;
class AWidgetManager;

UCLASS()
class ESCAPEIT_API AWindowJumpscareActor : public AActor
{
	GENERATED_BODY()

public:
	AWindowJumpscareActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, Category = "Window | Frame")
	TObjectPtr<UStaticMeshComponent> WindowFrameMesh;

	// Scene components để làm pivot point cho bản lề
	UPROPERTY(EditAnywhere, Category = "Window | Left")
	TObjectPtr<USceneComponent> LeftWindowHinge;

	UPROPERTY(EditAnywhere, Category = "Window | Left")
	TObjectPtr<UStaticMeshComponent> LeftWindowMesh;

	UPROPERTY(EditAnywhere, Category = "Window | Right")
	TObjectPtr<USceneComponent> RightWindowHinge;

	UPROPERTY(EditAnywhere, Category = "Window | Right")
	TObjectPtr<UStaticMeshComponent> RightWindowMesh;

	UPROPERTY(EditAnywhere, Category = "Actor")
	TObjectPtr<UStaticMeshComponent> GhostMesh;

	UPROPERTY(EditAnywhere, Category = "Box Collission")
	TObjectPtr<UBoxComponent> BoxCollission;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	TObjectPtr<UTimelineComponent> WindowTimeline;

	UPROPERTY(EditAnywhere, Category = "Curve")
	TObjectPtr<UCurveFloat> WindowCurve;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> GhostCamera;

	UPROPERTY()
	TObjectPtr<AActor> OriginalViewTarget;

	UPROPERTY(EditAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> GhostSpringArm;

	UPROPERTY(EditAnywhere, Category = "Window | Settings")
	float LeftWindowOpenAngle = -90.0f;

	UPROPERTY(EditAnywhere, Category = "Window | Settings")
	float RightWindowOpenAngle = -90.0f;

	UPROPERTY(EditAnywhere, Category = "Window | Settings")
	float CloseDelayTime = 3.0f;

	// Audio
	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<USoundBase> WindowOpenSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<USoundBase> JumpscareSound;

	// Internal state
	FRotator LeftHingeInitialRot;
	FRotator RightHingeInitialRot;
	FTimerHandle CloseWindowTimer;
	bool bHasTriggered = false;

	UPROPERTY()
	TObjectPtr<USanityComponent> SanityComponent;

	UPROPERTY()
	TObjectPtr<AWidgetManager> WidgetManager;

	UPROPERTY()
	TObjectPtr<AEscapeITCharacter> OwningCharacter;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void UpdateWindowRotation(float Value);

	void TriggerJumpscare();
	void CloseWindows();
};