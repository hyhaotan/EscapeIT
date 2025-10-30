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

        this->RemoveFromParent();
    }
}

void UPauseMenuWidget::OnClickSettingButton()
{
    APlayerController* PC = GetOwningPlayer();
    if (PC && MainMenuSettingWidgetClass)
    {
        UMainMenuSettingWidget* SettingWidget = CreateWidget<UMainMenuSettingWidget>(PC, MainMenuSettingWidgetClass);
        if (SettingWidget)
        {
            SettingWidget->AddToPlayerScreen(999); // an toàn hơn AddToViewport
            PC->bShowMouseCursor = true;

            // set input mode and focus widget:
            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(SettingWidget->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(InputMode);
        }
    }
    this->RemoveFromParent();
}

void UPauseMenuWidget::OnClickBackButton()
{
    this->RemoveFromParent();
    UGameplayStatics::OpenLevel(GetWorld(), TEXT("Lobby"));
}