// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Sound/SoundBase.h"
#include "Animation/WidgetAnimation.h"
#include "EscapeIT/UI/Settings/Main/MainMenuSettingWidget.h"
#include "EscapeIT/GameInstance/EscapeITSubsystem.h"
#include "EscapeIT/UI/MainMenu/ConfirmExitWidget.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button events
	NewGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnNewGameButton);
	ContinueButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnContinueButton);
	OptionsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsButton);
	CreditsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreditsButton);
	ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitButton);

	if (NewGameButton)
	{
		NewGameButton->OnHovered.AddDynamic(this, &UMainMenuWidget::OnNewGameButtonHovered);
		NewGameButton->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnNewGameButtonUnhovered);
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UEscapeITSubsystem* SaveSubsystem = GI->GetSubsystem<UEscapeITSubsystem>())
		{
			if (!SaveSubsystem->DoesSaveExist())
			{
				if (ContinueButton) ContinueButton->SetIsEnabled(false);
			}
			else
			{
				if (ContinueButton) ContinueButton->SetIsEnabled(true);
			}
		}
	}

	// Setup hover effects
	SetupButtonHoverEffects();

	// Store original button texts for corruption effect
	TArray<UButton*> AllButtons = { NewGameButton, ContinueButton, OptionsButton, CreditsButton, ExitButton };
	for (UButton* Button : AllButtons)
	{
		if (Button)
		{
			UTextBlock* ButtonText = Cast<UTextBlock>(Button->GetChildAt(0));
			if (ButtonText)
			{
				OriginalButtonTexts.Add(ButtonText, ButtonText->GetText());
			}
		}
	}

	TimeElapsed = FMath::RandRange(0.0f, 100.0f);
}

void UMainMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	TimeElapsed += InDeltaTime;

	if (bEnableButtonGlitch)
		UpdateButtonGlitch(InDeltaTime);

	if (bEnableTextCorruption)
		UpdateTextCorruption(InDeltaTime);
}

// ============= Button Handlers =============
void UMainMenuWidget::OnNewGameButton()
{
	// Play click sound
	if (ButtonClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ButtonClickSound, 0.5f);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
		if (CameraManager)
		{
			CameraManager->StartCameraFade(0.0f, 1.0f, 1.5f, FLinearColor::Black, false, true);
		}
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			UGameplayStatics::OpenLevel(this, "Room1");
		}, 3.0f, false);
}

void UMainMenuWidget::OnContinueButton()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UEscapeITSubsystem* SaveSubsystem = GI->GetSubsystem<UEscapeITSubsystem>())
		{
			if (SaveSubsystem->DoesSaveExist())
			{
				// Optional: play animation / fade here
				SaveSubsystem->LoadGame();
				// After loading, open saved level if subsystem applies it, or let subsystem handle teleporting
			}
			else
			{
				// show feedback (toast) that no save exists
				UE_LOG(LogTemp, Warning, TEXT("No save to continue"));
			}
		}
	}
}

void UMainMenuWidget::OnOptionsButton()
{
	if (!MainMenuSettingWidgetClass) return;
	if (APlayerController* PC = GetOwningPlayer())
	{
		MainMenuSettingWidget = CreateWidget<UMainMenuSettingWidget>(PC, MainMenuSettingWidgetClass);
		if (MainMenuSettingWidget)
		{
			MainMenuSettingWidget->AddToViewport(1000);
			// Set input focus to new widget
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(MainMenuSettingWidget->TakeWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
	}
}

void UMainMenuWidget::OnCreditsButton()
{
	CreditsWidget = CreateWidget<UUserWidget>(this, CreditsWidgetClass);
	if (CreditsWidget)
	{
		CreditsWidget->AddToViewport(999);
	}
}

void UMainMenuWidget::OnExitButton()
{
	ConfirmExitWidget = CreateWidget<UConfirmExitWidget>(this, ConfirmExitWidgetClass);
	if (ConfirmExitWidget)
	{
		ConfirmExitWidget->AddToViewport(999);
	}
}

// ============= Horror Effects =============

void UMainMenuWidget::UpdateButtonGlitch(float DeltaTime)
{
	if (bIsButtonGlitching)
	{
		ButtonGlitchTimer += DeltaTime;

		if (ButtonGlitchTimer >= ButtonGlitchDuration)
		{
			bIsButtonGlitching = false;

			// Reset all button positions
			TArray<UButton*> AllButtons = { NewGameButton, ContinueButton, OptionsButton, CreditsButton, ExitButton };
			for (UButton* Button : AllButtons)
			{
				if (Button)
				{
					Button->SetRenderTranslation(FVector2D::ZeroVector);
				}
			}
		}
		return;
	}

	// Check for random glitch trigger
	if (FMath::RandRange(0.0f, 1.0f) < ButtonGlitchChance * DeltaTime)
	{
		TriggerButtonGlitch();
	}
}

void UMainMenuWidget::TriggerButtonGlitch()
{
	bIsButtonGlitching = true;
	ButtonGlitchTimer = 0.0f;

	// Glitch random buttons
	TArray<UButton*> AllButtons = { NewGameButton, ContinueButton, OptionsButton, CreditsButton, ExitButton };

	int32 NumButtonsToGlitch = FMath::RandRange(1, 3);
	for (int32 i = 0; i < NumButtonsToGlitch; i++)
	{
		int32 RandomIndex = FMath::RandRange(0, AllButtons.Num() - 1);
		UButton* Button = AllButtons[RandomIndex];

		if (Button)
		{
			FVector2D GlitchOffset(
				FMath::RandRange(-15.0f, 15.0f),
				FMath::RandRange(-8.0f, 8.0f)
			);
			Button->SetRenderTranslation(GlitchOffset);
		}
	}
}

void UMainMenuWidget::UpdateTextCorruption(float DeltaTime)
{
	if (FMath::RandRange(0.0f, 1.0f) < TextCorruptionChance * DeltaTime)
	{
		TriggerTextCorruption();
	}
}

void UMainMenuWidget::TriggerTextCorruption()
{
	if (CorruptedCharacters.Num() == 0) return;

	// Corrupt random button text temporarily
	for (auto& Pair : OriginalButtonTexts)
	{
		UTextBlock* TextBlock = Pair.Key;
		const FText& OriginalText = Pair.Value;

		if (TextBlock && FMath::RandRange(0.0f, 1.0f) < 0.3f)
		{
			FString OriginalString = OriginalText.ToString();
			FString CorruptedString = OriginalString;

			// Corrupt 1-2 random characters
			int32 NumCorruptions = FMath::RandRange(1, 2);
			for (int32 i = 0; i < NumCorruptions; i++)
			{
				if (OriginalString.Len() > 0)
				{
					int32 CharIndex = FMath::RandRange(0, OriginalString.Len() - 1);
					int32 CorruptIndex = FMath::RandRange(0, CorruptedCharacters.Num() - 1);
					CorruptedString[CharIndex] = CorruptedCharacters[CorruptIndex][0];
				}
			}

			TextBlock->SetText(FText::FromString(CorruptedString));

			// Restore original text after short delay
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, TextBlock, OriginalText]()
				{
					if (TextBlock)
					{
						TextBlock->SetText(OriginalText);
					}
				}, FMath::RandRange(0.1f, 0.3f), false);
		}
	}
}

void UMainMenuWidget::SetupButtonHoverEffects()
{
	// Setup each button individually with UFUNCTION delegates
	if (NewGameButton)
	{
		NewGameButton->OnHovered.AddDynamic(this, &UMainMenuWidget::OnNewGameButtonHovered);
		NewGameButton->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnNewGameButtonUnhovered);
	}

	if (ContinueButton)
	{
		ContinueButton->OnHovered.AddDynamic(this, &UMainMenuWidget::OnContinueButtonHovered);
		ContinueButton->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnContinueButtonUnhovered);
	}

	if (OptionsButton)
	{
		OptionsButton->OnHovered.AddDynamic(this, &UMainMenuWidget::OnOptionsButtonHovered);
		OptionsButton->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnOptionsButtonUnhovered);
	}

	if (CreditsButton)
	{
		CreditsButton->OnHovered.AddDynamic(this, &UMainMenuWidget::OnCreditsButtonHovered);
		CreditsButton->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnCreditsButtonUnhovered);
	}

	if (ExitButton)
	{
		ExitButton->OnHovered.AddDynamic(this, &UMainMenuWidget::OnExitButtonHovered);
		ExitButton->OnUnhovered.AddDynamic(this, &UMainMenuWidget::OnExitButtonUnhovered);
	}
}

// Generic hover handlers
void UMainMenuWidget::HandleButtonHover(UButton* Button)
{
	if (!Button) return;

	// Play hover sound
	if (ButtonHoverSound)
	{
		UGameplayStatics::PlaySound2D(this, ButtonHoverSound, 0.4f);
	}

	// Scale effect
	Button->SetRenderScale(FVector2D(1.05f, 1.05f));
}

void UMainMenuWidget::HandleButtonUnhover(UButton* Button)
{
	if (!Button) return;
	Button->SetRenderScale(FVector2D(1.0f, 1.0f));
}

// Individual button hover handlers
void UMainMenuWidget::OnNewGameButtonHovered()
{
	HandleButtonHover(NewGameButton);
}

void UMainMenuWidget::OnNewGameButtonUnhovered()
{
	HandleButtonUnhover(NewGameButton);
}

void UMainMenuWidget::OnContinueButtonHovered()
{
	HandleButtonHover(ContinueButton);
}

void UMainMenuWidget::OnContinueButtonUnhovered()
{
	HandleButtonUnhover(ContinueButton);
}

void UMainMenuWidget::OnOptionsButtonHovered()
{
	HandleButtonHover(OptionsButton);
}

void UMainMenuWidget::OnOptionsButtonUnhovered()
{
	HandleButtonUnhover(OptionsButton);
}

void UMainMenuWidget::OnCreditsButtonHovered()
{
	HandleButtonHover(CreditsButton);
}

void UMainMenuWidget::OnCreditsButtonUnhovered()
{
	HandleButtonUnhover(CreditsButton);
}

void UMainMenuWidget::OnExitButtonHovered()
{
	HandleButtonHover(ExitButton);
}

void UMainMenuWidget::OnExitButtonUnhovered()
{
	HandleButtonUnhover(ExitButton);
}