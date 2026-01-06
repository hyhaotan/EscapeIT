#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UMenuButton;
class UConfirmExitWidget;
class UCreditsWidget;
class UMainMenuSettingWidget;
class UWidgetAnimation;

UCLASS()
class ESCAPEIT_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    UFUNCTION()
    void ShowMainMenuWidget() {PlayAnimation(ShowMainMenuAnim);}
    
    UFUNCTION()
    void HideMainMenuWidget() {PlayAnimation(HiddenMainMenuAnim);}

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(EditAnywhere, Category = "Audio")
    TObjectPtr<USoundBase> ButtonHoverSound;
    
    UPROPERTY(EditAnywhere,Category="UI")
    TSubclassOf<UConfirmExitWidget> ConfirmExitWidgetClass; 
    
    UPROPERTY(EditAnywhere,Category="UI")
    TSubclassOf<UUserWidget> CreditsWidgetClass; 
    
    UPROPERTY(EditAnywhere,Category="UI")
    TSubclassOf<UMainMenuSettingWidget> MainMenuSettingWidgetClass;

private:
    UFUNCTION()
    void OnNewGameClicked();

    UFUNCTION()
    void OnContinueClicked();

    UFUNCTION()
    void OnOptionsClicked();

    UFUNCTION()
    void OnCreditsClicked();

    UFUNCTION()
    void OnExitClicked();
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UMenuButton> NewGameButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UMenuButton> ContinueButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UMenuButton> OptionsButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UMenuButton> CreditsButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UMenuButton> ExitButton;
    
    UPROPERTY(meta=(BindWidgetAnim),Transient)
    UWidgetAnimation* ShowMainMenuAnim;
    
    UPROPERTY(meta=(BindWidgetAnim),Transient)
    UWidgetAnimation* HiddenMainMenuAnim;
    
    UPROPERTY()
    UConfirmExitWidget* ConfirmExitWidget;
    
    UPROPERTY()
    UUserWidget* CreditsWidget;
    
    UPROPERTY()
    UMainMenuSettingWidget* MainMenuSettingWidget;
};