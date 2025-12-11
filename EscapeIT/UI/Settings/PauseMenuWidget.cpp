// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenuWidget.h"
#include "EscapeIT/UI/Settings/Main/MainMenuSettingWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "EscapeIT/UI/HUD/WidgetManager.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResumeButton) ResumeButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnClickResumeButton);

	if (SettingsButton) SettingsButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnClickSettingButton);

	if (BackButton) BackButton->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnClickBackButton);
}

void UPauseMenuWidget::OnClickResumeButton()
{
    if (WidgetManager)
    {
        WidgetManager->HidePauseMenu();
    }
    else
    {
        APlayerController* PC = GetOwningPlayer();
        if (PC)
        {
            PC->bShowMouseCursor = false;
            PC->SetInputMode(FInputModeGameOnly());
        }

        SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UPauseMenuWidget::OnClickSettingButton()
{
    APlayerController* PC = GetOwningPlayer();
    if (PC && MainMenuSettingWidgetClass)
    {
        if (UMainMenuSettingWidget* SettingWidget = CreateWidget<UMainMenuSettingWidget>(PC, MainMenuSettingWidgetClass))
        {
            SettingWidget->AddToPlayerScreen(999);
            SettingWidget->ShowSettingsMenu(false);
        }
    }
    SetVisibility(ESlateVisibility::Collapsed);
}

void UPauseMenuWidget::OnClickBackButton()
{
    SetVisibility(ESlateVisibility::Collapsed);
    UGameplayStatics::OpenLevel(GetWorld(), TEXT("Lobby"));
}