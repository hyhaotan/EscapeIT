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
    TObjectPtr<APlayerController> PC = GetOwningPlayer();
    if (!PC) return;

    if (MainMenuSettingWidgetClass)
    {
        UMainMenuSettingWidget* SettingWidget = CreateWidget<UMainMenuSettingWidget>(GetWorld(), MainMenuSettingWidgetClass);
        if (SettingWidget)
        {
            SettingWidget->AddToViewport(999);
            PC->bShowMouseCursor = true;
            PC->SetInputMode(FInputModeUIOnly());

            this->RemoveFromParent();
        }
    }
}

void UPauseMenuWidget::OnClickBackButton()
{
    this->RemoveFromParent();
    UGameplayStatics::OpenLevel(GetWorld(), TEXT("Lobby"));
}