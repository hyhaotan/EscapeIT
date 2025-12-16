// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MainMenuHUD.generated.h"

class UMainMenuWidget;

UCLASS()
class ESCAPEIT_API AMainMenuHUD : public AHUD
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UMainMenuWidget> MainMenuWidget;

	UFUNCTION()
	void InitializeWidget();

	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere,Category="Sound")
	USoundBase* AmbientSound;

public:
	UPROPERTY(EditAnywhere,Category = "Menu")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;
};
