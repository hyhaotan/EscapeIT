// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuSettingsWidget.h"
#include "EscapeIT/UI/Settings/Main/MainMenuSettingWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UMenuSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResumeButton) ResumeButton->OnClicked.AddDynamic(this, &UMenuSettingsWidget::OnClickResumeButton);

	if (SettingsButton) SettingsButton->OnClicked.AddDynamic(this, &UMenuSettingsWidget::OnClickSettingButton);

	if (BackButton) BackButton->OnClicked.AddDynamic(this, &UMenuSettingsWidget::OnClickBackButton);
}

void UMenuSettingsWidget::OnClickResumeButton()
{
	UGameplayStatics::SetGamePaused(GetWorld(),false);
}

void UMenuSettingsWidget::OnClickSettingButton()
{
	FInputModeUIOnly Input;
	MainMenuSettingWidget = CreateWidget<UMainMenuSettingWidget>(GetWorld(), MainMenuSettingWidgetClass);
	if (MainMenuSettingWidget)
	{
		MainMenuSettingWidget->AddToViewport(999);
		UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx();
	}

	this->RemoveFromParent();
}

void UMenuSettingsWidget::OnClickBackButton()
{
}
