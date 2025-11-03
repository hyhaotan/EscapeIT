// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UMainMenuSettingWidget;
class UUserWidget;
class UConfirmExitWidget;
class UUniformGridPanel;

UCLASS()
class ESCAPEIT_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
private:
	//================= Button Menu ==============
	UPROPERTY(meta = (BindWidget))
	UButton* NewGameButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ContinueButton;

	UPROPERTY(meta = (BindWidget))
	UButton* OptionsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CreditsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	UPROPERTY()
	UUniformGridPanel* MainMenuUniform;

	//================= WIDGET ===================
	UPROPERTY()
	TObjectPtr<UMainMenuSettingWidget> MainMenuSettingWidget;

	UPROPERTY()
	TObjectPtr<UConfirmExitWidget> ConfirmExitWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> CreditsWidget;

	//================= FUNCTION =================

	UFUNCTION()
	void OnNewGameButton();

	UFUNCTION()
	void OnContinueButton();

	UFUNCTION()
	void OnOptionsButton();

	UFUNCTION()
	void OnCreditsButton();

	UFUNCTION()
	void OnExitButton();

public:
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, Category = "Setting")
	TSubclassOf<UMainMenuSettingWidget> MainMenuSettingWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Setting")
	TSubclassOf<UConfirmExitWidget> ConfirmExitWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Setting")
	TSubclassOf<UUserWidget> CreditsWidgetClass;
};
