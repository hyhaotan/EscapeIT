// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "LightActor.generated.h"

UCLASS()
class ESCAPEIT_API ALightActor : public AActor
{
	GENERATED_BODY()

public:
	ALightActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Components")
	UPointLightComponent* LightComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Components")
	UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Light Settings")
	float OnIntensity = 5000.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Light Settings")
	float OffIntensity = 0.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Light Settings")
	FLinearColor LightColor = FLinearColor::White;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Light Settings")
	float FadeSpeed = 5.0f;
	
private:
	UFUNCTION()
	void OnPowerStateChanged(bool bIsPowerOn);
	
	float CurrentIntensity;
	float TargetIntensity;
	
	virtual void Tick(float DeltaTime) override;
};
