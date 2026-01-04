// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/LightActor.h"
#include "GameInstance/PowerSystemManager.h"

// Sets default values
ALightActor::ALightActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	
	LightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	LightComponent->SetupAttachment(MeshComponent);
	LightComponent->SetIntensity(OnIntensity);
	LightComponent->SetLightColor(LightColor);
	LightComponent->SetCastShadows(true);
	
	CurrentIntensity = OnIntensity;
	TargetIntensity = OnIntensity;
}

void ALightActor::BeginPlay()
{
	Super::BeginPlay();
	
	PowerSystemManager = GetGameInstance()->GetSubsystem<UPowerSystemManager>();
	if (PowerSystemManager)
	{
		PowerSystemManager->OnPowerStateChanged.AddDynamic(this, &ALightActor::OnPowerStateChanged);
		
		TargetIntensity = PowerSystemManager->IsPowerOn() ? OnIntensity : OffIntensity;
		CurrentIntensity = TargetIntensity;
		LightComponent->SetIntensity(CurrentIntensity);
	}
}

void ALightActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PowerSystemManager)
	{
		PowerSystemManager->OnPowerStateChanged.RemoveDynamic(this, &ALightActor::OnPowerStateChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void ALightActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!FMath::IsNearlyEqual(CurrentIntensity, TargetIntensity, 1.0f))
	{
		CurrentIntensity = FMath::FInterpTo(CurrentIntensity, TargetIntensity, DeltaTime, FadeSpeed);
		LightComponent->SetIntensity(CurrentIntensity);
	}
}

void ALightActor::OnPowerStateChanged(bool bIsPowerOn)
{
	TargetIntensity = bIsPowerOn ? OnIntensity : OffIntensity;
}



