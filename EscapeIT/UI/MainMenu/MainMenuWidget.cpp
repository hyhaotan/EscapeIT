// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"
#include "EscapeIT/UI/Settings/Main/MainMenuSettingWidget.h"
#include "EscapeIT/UI/MainMenu/ConfirmExitWidget.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	NewGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnNewGameButton);
	ContinueButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnContinueButton);
	OptionsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsButton);
	CreditsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreditsButton);
	ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitButton);
}

void UMainMenuWidget::OnNewGameButton()
{
	UGameplayStatics::OpenLevel(this,"Room1");
}

void UMainMenuWidget::OnContinueButton()
{
}

void UMainMenuWidget::OnOptionsButton()
{
	MainMenuSettingWidget = CreateWidget<UMainMenuSettingWidget>(GetWorld(), MainMenuSettingWidgetClass);
	if (MainMenuSettingWidget)
	{
		MainMenuSettingWidget->AddToViewport(999);
	}
}

void UMainMenuWidget::OnCreditsButton()
{
	CreditsWidget = CreateWidget<UUserWidget>(this, CreditsWidgetClass);
	if (CreditsWidget)
	{
		CreditsWidget->AddToViewport(999);
	}
}

void UMainMenuWidget::OnExitButton()
{
	ConfirmExitWidget = CreateWidget<UConfirmExitWidget>(this, ConfirmExitWidgetClass);
	if (ConfirmExitWidget)
	{
		ConfirmExitWidget->AddToViewport(999);
	}
}

