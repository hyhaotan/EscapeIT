// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuHUD.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

#include "EscapeIT/UI/MainMenu/MainMenuWidget.h"

void AMainMenuHUD::BeginPlay()
{
	Super::BeginPlay();

	InitializeWidget();
}


void AMainMenuHUD::InitializeWidget()
{
	TObjectPtr<APlayerController> PlayerCon = UGameplayStatics::GetPlayerController(this, 0);
	PlayerCon->SetInputMode(FInputModeUIOnly());

	MainMenuWidget = CreateWidget<UMainMenuWidget>(GetWorld(), MainMenuWidgetClass);
	if (MainMenuWidget)
	{
		MainMenuWidget->AddToViewport();
	}
}

