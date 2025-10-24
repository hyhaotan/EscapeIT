// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AudioWidget.generated.h"

class USelectionWidget;
class USettingsSubsystem;
class UButton;
class UTextBlock;
class USlider;

/**
 * Widget for Audio Settings
 */
UCLASS()
class ESCAPEIT_API UAudioWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UAudioWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ===== WIDGET BINDINGS =====

	UPROPERTY(meta = (BindWidget))
	USlider* MasterVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MasterVolumeText;

	UPROPERTY(meta = (BindWidget))
	USlider* SFXVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SFXVolumeText;

	UPROPERTY(meta = (BindWidget))
	USlider* MusicVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MusicVolumeText;

	UPROPERTY(meta = (BindWidget))
	USlider* AmbientVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmbientVolumeText;

	UPROPERTY(meta = (BindWidget))
	USlider* DialogueVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DialogueVolumeText;

	UPROPERTY(meta = (BindWidget))
	USlider* UIVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* UIVolumeText;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* AudioLanguageSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* AudioOutputSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* ClosedCaptionsSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* SubtitlesSelection;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* TestAudioButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ResetButton;

	// ===== CALLBACKS =====

	UFUNCTION()
	void OnMasterVolumeChanged(float Value);

	UFUNCTION()
	void OnSFXVolumeChanged(float Value);

	UFUNCTION()
	void OnMusicVolumeChanged(float Value);

	UFUNCTION()
	void OnAmbientVolumeChanged(float Value);

	UFUNCTION()
	void OnDialogueVolumeChanged(float Value);

	UFUNCTION()
	void OnUIVolumeChanged(float Value);

	UFUNCTION()
	void OnAudioLanguageChanged(int32 NewIndex);

	UFUNCTION()
	void OnAudioOutputChanged(int32 NewIndex);

	UFUNCTION()
	void OnClosedCaptionsChanged(int32 NewIndex);

	UFUNCTION()
	void OnSubtitlesChanged(int32 NewIndex);

	UFUNCTION()
	void OnTestAudioButtonClicked();

	UFUNCTION()
	void OnResetButtonClicked();

private:
	/** Reference to Settings Subsystem */
	UPROPERTY()
	USettingsSubsystem* SettingsSubsystem;

	/** Test sound to play */
	UPROPERTY()
	class USoundBase* TestSound;

	/** Track if we're currently updating sliders (prevent feedback loop) */
	bool bUpdatingSliders;

	/** Initialize all selection widgets with options */
	void InitializeSelections();

	/** Initialize all sliders */
	void InitializeSliders();

	/** Load current settings from subsystem */
	void LoadCurrentSettings();

	/** Helper to create toggle options (On/Off) */
	void AddToggleOptions(USelectionWidget* Selection);

	/** Update text displays to match sliders */
	void UpdateVolumeTexts();

	/** Bind slider events */
	void BindSliderEvents();

	/** Unbind slider events */
	void UnbindSliderEvents();
};