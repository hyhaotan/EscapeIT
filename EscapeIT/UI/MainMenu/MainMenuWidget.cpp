// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Sound/SoundBase.h"
#include "Animation/WidgetAnimation.h"
#include "EscapeIT/UI/Settings/Main/MainMenuSettingWidget.h"
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

	// Update horror effects
	if (bEnableStaticNoise)
		UpdateStaticNoise(InDeltaTime);

	if (bEnableButtonGlitch)
		UpdateButtonGlitch(InDeltaTime);

	if (bEnableTextCorruption)
		UpdateTextCorruption(InDeltaTime);

	if (bEnableScreenFlash)
		UpdateScreenFlash(InDeltaTime);

	if (bEnableVignettePulse)
		UpdateVignettePulse(InDeltaTime);
}

// ============= Button Handlers =============
void UMainMenuWidget::OnNewGameButton()
{
	// Play click sound
	if (ButtonClickSound)
	{
		UGameplayStatics::PlaySound2D(this, ButtonClickSound, 0.5f);
	}

	// Add slight delay with screen flash for dramatic effect
	if (FlashImage)
	{
		FlashImage->SetColorAndOpacity(FLinearColor::White);
		FlashImage->SetOpacity(1.0f);
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			UGameplayStatics::OpenLevel(this, "Room1");
		}, 0.5f, false);
}

void UMainMenuWidget::OnContinueButton()
{
	// Implement continue logic
}

void UMainMenuWidget::OnOptionsButton()
{
	MainMenuSettingWidget = CreateWidget<UMainMenuSettingWidget>(GetWorld(), MainMenuSettingWidgetClass);
	if (MainMenuSettingWidget)
	{
		MainMenuSettingWidget->AddToViewport(999);
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
void UMainMenuWidget::UpdateStaticNoise(float DeltaTime)
{
	if (!StaticNoiseOverlay) return;

	// Animated static noise effect
	float NoiseValue = FMath::PerlinNoise1D(TimeElapsed * StaticNoiseSpeed);
	float Opacity = FMath::Clamp(NoiseValue * StaticNoiseIntensity, 0.0f, StaticNoiseIntensity);

	StaticNoiseOverlay->SetOpacity(Opacity);

	// Random position shift for more chaotic effect
	if (FMath::RandRange(0.0f, 1.0f) < 0.1f)
	{
		FVector2D RandomOffset(
			FMath::RandRange(-2.0f, 2.0f),
			FMath::RandRange(-2.0f, 2.0f)
		);
		StaticNoiseOverlay->SetRenderTranslation(RandomOffset);
	}
}

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

void UMainMenuWidget::UpdateScreenFlash(float DeltaTime)
{
	if (!FlashImage) return;

	if (bIsFlashing)
	{
		FlashTimer += DeltaTime;

		// Fade out flash
		float Alpha = 1.0f - (FlashTimer / FlashDuration);
		FlashImage->SetOpacity(FMath::Max(0.0f, Alpha));

		if (FlashTimer >= FlashDuration)
		{
			bIsFlashing = false;
			FlashImage->SetOpacity(0.0f);
		}
		return;
	}

	// Random flash trigger
	if (FMath::RandRange(0.0f, 1.0f) < FlashChance * DeltaTime)
	{
		TriggerScreenFlash();
	}
}

void UMainMenuWidget::TriggerScreenFlash()
{
	if (!FlashImage) return;

	bIsFlashing = true;
	FlashTimer = 0.0f;

	FlashImage->SetColorAndOpacity(FlashColor);
	FlashImage->SetOpacity(FMath::RandRange(0.3f, 0.7f));
}

void UMainMenuWidget::UpdateVignettePulse(float DeltaTime)
{
	if (!VignetteOverlay) return;

	// Breathing vignette effect
	float PulseCycle = FMath::Sin(TimeElapsed * VignettePulseSpeed);
	float Opacity = FMath::Lerp(VignetteMinOpacity, VignetteMaxOpacity, (PulseCycle + 1.0f) * 0.5f);

	VignetteOverlay->SetOpacity(Opacity);
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