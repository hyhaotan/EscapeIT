// Fill out your copyright notice in the Description page of Project Settings.

#include "SanityComponent.h"

USanityComponent::USanityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	MaxSanity = 100.0f;
	MinSanity = 0.0f;
	Sanity = 100.0f;
	SanityDecayRate = 1.0f;
	bAutoDecay = false;
}

void USanityComponent::BeginPlay()
{
	Super::BeginPlay();

	Sanity = MaxSanity;
}

void USanityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAutoDecay && SanityDecayRate > 0.0f)
	{
		CalculatorSanity(-SanityDecayRate * DeltaTime);
	}
}

float USanityComponent::GetSanity() const
{
	return Sanity;
}

float USanityComponent::GetMinSanity() const
{
	return MinSanity;
}

float USanityComponent::GetMaxSanity() const
{
	return MaxSanity;
}

float USanityComponent::GetSanityPercent() const
{
	if (MaxSanity <= MinSanity)
	{
		return 0.0f;
	}

	return (Sanity - MinSanity) / (MaxSanity - MinSanity);
}

void USanityComponent::SetSanity(float Amount)
{
	Sanity = FMath::Clamp(Amount, MinSanity, MaxSanity);
	UpdateSanity();
}

void USanityComponent::SetMinSanity(float Amount)
{
	MinSanity = Amount;
}

void USanityComponent::SetMaxSanity(float Amount)
{
	MaxSanity = Amount;
}

void USanityComponent::ModifySanity(float Amount)
{
	CalculatorSanity(Amount);
}

void USanityComponent::RestoreSanity(float Amount)
{
	if (Amount > 0.0f)
	{
		CalculatorSanity(Amount);
	}
}

void USanityComponent::ReduceSanity(float Amount)
{
	if (Amount > 0.0f)
	{
		CalculatorSanity(-Amount);
	}
}

void USanityComponent::CalculatorSanity(float Amount)
{
	Sanity = FMath::Clamp(Sanity + Amount, MinSanity, MaxSanity);
	UpdateSanity();
}

void USanityComponent::UpdateSanity()
{
	OnSanityChanged.Broadcast(Sanity);

	if (Sanity <= MinSanity)
	{
		OnSanityDepleted.Broadcast();
	}
}