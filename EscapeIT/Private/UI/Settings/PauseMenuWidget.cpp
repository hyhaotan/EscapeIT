// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Settings/PauseMenuWidget.h"
#include "UI/Settings/Main/MainMenuSettingWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/HUD/WidgetManager.h"

void UPauseMenuWidget::InitializeWithWidgetManager()
{
    if (SettingsWidget && WidgetManager)
    {
        SettingsWidget->SetWidgetManager(WidgetManager);
    }
}

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
            SettingWidget->AddToViewport(999);
            SettingWidget->ShowSettingsMenu(false);
            SettingWidget->OnBackClicked.AddDynamic(this,&UPauseMenuWidget::OnSettingsMenuClosed);
        }
    }
    SetVisibility(ESlateVisibility::Collapsed);
}

void UPauseMenuWidget::OnClickBackButton()
{
    SetVisibility(ESlateVisibility::Collapsed);
    UGameplayStatics::OpenLevel(GetWorld(), TEXT("Lobby"));
}

void UPauseMenuWidget::OnSettingsMenuClosed()
{
    SetVisibility(ESlateVisibility::Visible);
    const auto PC = GetOwningPlayer();
    if (PC)
    {
        FInputModeUIOnly InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
}
