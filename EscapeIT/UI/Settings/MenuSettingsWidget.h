// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuSettingsWidget.generated.h"

class UButton;
class UMainMenuSettingWidget;

UCLASS()
class ESCAPEIT_API UMenuSettingsWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	UButton* ResumeButton;

	UPROPERTY(meta = (BindWidget))
	UButton* SettingsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	virtual void NativeConstruct() override;

protected:
	UPROPERTY()
	TObjectPtr<UMainMenuSettingWidget> MainMenuSettingWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MainMenuSetting")
	TSubclassOf<UMainMenuSettingWidget> MainMenuSettingWidgetClass;

private:
	void OnClickResumeButton();
	void OnClickSettingButton();
	void OnClickBackButton();
};
