// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Door/DoorActor.h"
#include "UI/ElectricCabinetWidget.h"
#include "ElectricCabinetActor.generated.h"

UCLASS()
class ESCAPEIT_API AElectricCabinetActor : public ADoorActor
{
	GENERATED_BODY()
	
public:	
	AElectricCabinetActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="UI")
	TSubclassOf<UElectricCabinetWidget> ElectricCabinetWidgetClass;
	
	UPROPERTY()
	UElectricCabinetWidget* ElectricCabinetWidget;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Cabinet")
	bool bCanInteract = true;
	
	UPROPERTY(BlueprintReadOnly,Category="Cabinet")
	bool bIsRepaired = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cabinet")
	float WidgetShowDelay = 1.5f;

private:
	virtual void CalculateDoorOpenDirection_Implementation(AActor* Interactor) override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	
	UFUNCTION()
	void OnPuzzleCompleted();
	
	void ShowPuzzleWidget(APlayerController* PlayerController);
	
	void HidePuzzleWidget();
	
	void OnDoorOpenFinished();
	
	UPROPERTY()
	APlayerController* CachedPlayerController;
	
	FTimerHandle WidgetShowTimerHandle;
};
