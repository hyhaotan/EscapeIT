// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SanityComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSanityChangedDelegate, float, CurrentSanity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSanityDepletedDelegate);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ESCAPEIT_API USanityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USanityComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Sanity", meta = (AllowPrivateAccess = "true"))
	float Sanity;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity", meta = (AllowPrivateAccess = "true"))
	float MinSanity;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity", meta = (AllowPrivateAccess = "true"))
	float MaxSanity;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity", meta = (AllowPrivateAccess = "true"))
	float SanityDecayRate;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity", meta = (AllowPrivateAccess = "true"))
	bool bAutoDecay;

public:
	UPROPERTY(BlueprintAssignable, Category = "Sanity")
	FOnSanityChangedDelegate OnSanityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Sanity")
	FOnSanityDepletedDelegate OnSanityDepleted;

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	float GetSanity() const;

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	float GetMinSanity() const;

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	float GetMaxSanity() const;

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	float GetSanityPercent() const;

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	void SetSanity(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	void SetMinSanity(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	void SetMaxSanity(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	void ModifySanity(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	void RestoreSanity(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	void ReduceSanity(float Amount);

	void CalculatorSanity(float Amount);
	void UpdateSanity();
};