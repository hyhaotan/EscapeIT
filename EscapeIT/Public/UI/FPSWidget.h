// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FPSWidget.generated.h"

class UTextBlock;

UCLASS()
class ESCAPEIT_API UFPSWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFPSWidget(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
public:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* FPSText;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="FPS")
	float UpdateRate = 1.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="FPS")
	bool bShowDetailedStats = true;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="FPS")
	int32 MaxFPSHistorySize = 60;
	
private:
	UFUNCTION()
	void EnableAdvancedFPSMonitoring(bool bEnable);
	
	UFUNCTION(BlueprintPure,Category="FPS Counter")
	float GetAverageFPS() const;
	
	UFUNCTION(BlueprintPure,Category="FPS Counter")
	float GetCurrentFPS() const;
	
	UFUNCTION(BlueprintPure,Category="FPS Counter")
	float GetMaxFPS() const;
	
	UFUNCTION(BlueprintPure,Category="FPS Counter")
	float GetMinFPS() const;
	
	void UpdateFPSCounter();
	void UpdateFPSHistory(float CurrentFPS);
	
	FTimerHandle FPSTimerHandle;
	TArray<float> FPSHistory;
	float MinFPS = 999.0f;
	float MaxFPS = 0.0f;
};
