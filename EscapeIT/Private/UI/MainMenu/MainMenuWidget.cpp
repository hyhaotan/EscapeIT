#include "UI/MainMenu/MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Sound/SoundBase.h"
#include "Animation/WidgetAnimation.h"
#include "GameInstance/EscapeITSubsystem.h"
#include "UI/MainMenu/ConfirmExitWidget.h"
#include "UI/MainMenu/MenuButton.h"
#include "UI/Settings/Main/MainMenuSettingWidget.h"

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    TArray<UMenuButton*> AllButtons = {
        NewGameButton,ContinueButton,OptionsButton,
        CreditsButton,ExitButton
    };
    
    for (UMenuButton* Button : AllButtons)
    {
        if (Button)
        {
            Button->SetHoverSound(ButtonHoverSound);
        }
    }
    
    if (NewGameButton)
    {
        NewGameButton->OnButtonClicked.AddDynamic(this, &UMainMenuWidget::OnNewGameClicked);
        NewGameButton->SetButtonText(FText::FromString(TEXT("New Game")));
    }
    
    if (ContinueButton)
    {
        ContinueButton->OnButtonClicked.AddDynamic(this, &UMainMenuWidget::OnContinueClicked);
        ContinueButton->SetButtonText(FText::FromString(TEXT("Continue")));
    }
    
    if (OptionsButton)
    {
        OptionsButton->OnButtonClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsClicked);
        OptionsButton->SetButtonText(FText::FromString(TEXT("Options")));
    }
    
    if (CreditsButton)
    {
        CreditsButton->OnButtonClicked.AddDynamic(this, &UMainMenuWidget::OnCreditsClicked);
        CreditsButton->SetButtonText(FText::FromString(TEXT("Credits")));
    }
    
    if (ExitButton)
    {
        ExitButton->OnButtonClicked.AddDynamic(this, &UMainMenuWidget::OnExitClicked);
        ExitButton->SetButtonText(FText::FromString(TEXT("Exit Game")));
    }

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UEscapeITSubsystem* SaveSubsystem = GI->GetSubsystem<UEscapeITSubsystem>())
        {
            if (ContinueButton)
            {
                ContinueButton->SetEnabled(SaveSubsystem->DoesSaveExist());
            }
        }
    }
}

void UMainMenuWidget::OnNewGameClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
        if (CameraManager)
        {
            CameraManager->StartCameraFade(0.0f, 1.0f, 3.0f, FLinearColor::Black, false, true);
        }
    }

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            UGameplayStatics::OpenLevel(this, "Room1");
        }, 1.5f, false);
	
    this->HideMainMenuWidget();
}

void UMainMenuWidget::OnContinueClicked()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UEscapeITSubsystem* SaveSubsystem = GI->GetSubsystem<UEscapeITSubsystem>())
        {
            if (SaveSubsystem->DoesSaveExist())
            {
                SaveSubsystem->LoadGame();
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("No save to continue"));
            }
        }
    }
}
void UMainMenuWidget::OnOptionsClicked()
{
    if (!MainMenuSettingWidgetClass) return;
    if (APlayerController* PC = GetOwningPlayer())
    {
        MainMenuSettingWidget = CreateWidget<UMainMenuSettingWidget>(PC, MainMenuSettingWidgetClass);
        if (MainMenuSettingWidget)
        {
            MainMenuSettingWidget->AddToViewport(1000);
            MainMenuSettingWidget->ShowSettingsMenu(true);
        }
    }
}

void UMainMenuWidget::OnCreditsClicked()
{
    CreditsWidget = CreateWidget<UUserWidget>(this, CreditsWidgetClass);
    if (CreditsWidget)
    {
        CreditsWidget->AddToViewport(999);
    }
}

void UMainMenuWidget::OnExitClicked()
{
    ConfirmExitWidget = CreateWidget<UConfirmExitWidget>(this, ConfirmExitWidgetClass);
    if (ConfirmExitWidget)
    {
        ConfirmExitWidget->AddToViewport(999);
    }
}