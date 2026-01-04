// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PowerSystemManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPowerStateChanged,bool,bIsPowerOn);

UCLASS()
class ESCAPEIT_API UPowerSystemManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// ========================== FUNCTION ==========================
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UFUNCTION(BlueprintCallable,Category="Power")
	void SetPowerState(bool bNewState);
	
	UFUNCTION(BlueprintCallable,Category="Power")
	bool IsPowerOn() const {return bIsPowerOn;}
	
	UFUNCTION(BlueprintCallable,Category="Power")
	void CausePowerFailure();
	
	// ========================== PROPERTIES =========================
	// ================ AVAIABLE ==============
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Power")
	float PowerOffDuration;
	
	UPROPERTY(BlueprintReadOnly,Category="Power")
	bool bIsPowerOn;
	
	// =============== TIMER =================
		
	FTimerHandle DelayPowerEvent;
	
	// =============== EVENT =================
	UPROPERTY(BlueprintAssignable,Category="Power")
	FOnPowerStateChanged OnPowerStateChanged;
	
	// ========================== SOUND ==============================
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Sound")
	TObjectPtr<USoundBase> PowerOffSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TSoftObjectPtr<USoundBase> PowerOffSoundSoft;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Sound")
	TObjectPtr<USoundBase> PowerOnSound;
};
