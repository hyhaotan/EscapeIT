// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UMainMenuSettingWidget;
class UConfirmExitWidget;
class USoundBase;

UCLASS()
class ESCAPEIT_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ============= Original Buttons =============
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NewGameButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ContinueButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> OptionsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CreditsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

	// ============= Widget Classes =============
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UMainMenuSettingWidget> MainMenuSettingWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> CreditsWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UConfirmExitWidget> ConfirmExitWidgetClass;

	UPROPERTY()
	TObjectPtr<UMainMenuSettingWidget> MainMenuSettingWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> CreditsWidget;

	UPROPERTY()
	TObjectPtr<UConfirmExitWidget> ConfirmExitWidget;

	// ============= Horror Effect Settings =============
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Static Noise")
	bool bEnableStaticNoise = true;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Static Noise", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StaticNoiseIntensity = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Static Noise")
	float StaticNoiseSpeed = 50.0f;

	// Button Glitch Effect
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Button Glitch")
	bool bEnableButtonGlitch = true;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Button Glitch")
	float ButtonGlitchChance = 0.02f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Button Glitch")
	float ButtonGlitchDuration = 0.1f;

	// Text Corruption
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Text")
	bool bEnableTextCorruption = true;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Text")
	float TextCorruptionChance = 0.01f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Text")
	TArray<FString> CorruptedCharacters = { TEXT("█"), TEXT("▓"), TEXT("▒"), TEXT("░"), TEXT("Ⱥ"), TEXT("Ɇ"), TEXT("Ø") };

	// Screen Flash
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Flash")
	bool bEnableScreenFlash = true;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Flash")
	float FlashChance = 0.005f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Flash")
	float FlashDuration = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Flash")
	FLinearColor FlashColor = FLinearColor::White;

	// Vignette Pulse
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Vignette")
	bool bEnableVignettePulse = true;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Vignette")
	float VignettePulseSpeed = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Vignette", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VignetteMinOpacity = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Vignette", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VignetteMaxOpacity = 0.7f;

	// Audio (Menu-specific sounds only, ambient handled by LobbyCamera)
	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	TObjectPtr<USoundBase> ButtonHoverSound;

	UPROPERTY(EditAnywhere, Category = "Horror Effects|Audio")
	TObjectPtr<USoundBase> ButtonClickSound;

private:
	// Button Click Handlers
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

	// Horror Effect Functions
	void UpdateButtonGlitch(float DeltaTime);
	void UpdateTextCorruption(float DeltaTime);

	void TriggerButtonGlitch();
	void TriggerTextCorruption();

	void SetupButtonHoverEffects();

	// Button Hover Handlers
	UFUNCTION()
	void OnNewGameButtonHovered();
	UFUNCTION()
	void OnNewGameButtonUnhovered();
	UFUNCTION()
	void OnContinueButtonHovered();
	UFUNCTION()
	void OnContinueButtonUnhovered();
	UFUNCTION()
	void OnOptionsButtonHovered();
	UFUNCTION()
	void OnOptionsButtonUnhovered();
	UFUNCTION()
	void OnCreditsButtonHovered();
	UFUNCTION()
	void OnCreditsButtonUnhovered();
	UFUNCTION()
	void OnExitButtonHovered();
	UFUNCTION()
	void OnExitButtonUnhovered();

	// Helper functions
	void HandleButtonHover(UButton* Button);
	void HandleButtonUnhover(UButton* Button);

	// State Variables
	float TimeElapsed = 0.0f;
	bool bIsButtonGlitching = false;
	float ButtonGlitchTimer = 0.0f;

	TMap<UButton*, FVector2D> OriginalButtonPositions;
	TMap<UTextBlock*, FText> OriginalButtonTexts;
};