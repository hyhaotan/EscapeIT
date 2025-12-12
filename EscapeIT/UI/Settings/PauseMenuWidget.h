// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class UMainMenuSettingWidget;
class AWidgetManager;

UCLASS()
class ESCAPEIT_API UPauseMenuWidget : public UUserWidget
{
    GENERATED_BODY()
public:

    UPROPERTY()
    TObjectPtr<AWidgetManager> WidgetManager;

protected:
    virtual void NativeConstruct() override;

private:

    UFUNCTION()
    void OnClickResumeButton();

    UFUNCTION()
    void OnClickSettingButton();

    UFUNCTION()
    void OnClickBackButton();
    
    UFUNCTION()
    void OnSettingsMenuClosed();

    UPROPERTY(meta = (BindWidget))
    UButton* ResumeButton;

    UPROPERTY(meta = (BindWidget))
    UButton* SettingsButton;

    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UMainMenuSettingWidget> MainMenuSettingWidgetClass;
};