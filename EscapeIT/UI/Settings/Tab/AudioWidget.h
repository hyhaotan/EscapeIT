// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AudioWidget.generated.h"

class USelectionWidget;
class USettingsSubsystem;
class UButton;
class UProgressBar;
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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ===== WIDGET BINDINGS =====

	/** Master Volume Slider */
	UPROPERTY(meta = (BindWidget))
	USlider* MasterVolumeSlider;

	/** Master Volume Progress Bar (Visual feedback) */
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* MasterVolumeBar;

	/** SFX Volume Slider */
	UPROPERTY(meta = (BindWidget))
	USlider* SFXVolumeSlider;

	/** SFX Volume Progress Bar */
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* SFXVolumeBar;

	/** Music Volume Slider */
	UPROPERTY(meta = (BindWidget))
	USlider* MusicVolumeSlider;

	/** Music Volume Progress Bar */
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* MusicVolumeBar;

	/** Ambient Volume Slider */
	UPROPERTY(meta = (BindWidget))
	USlider* AmbientVolumeSlider;

	/** Ambient Volume Progress Bar */
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* AmbientVolumeBar;

	/** Dialogue Volume Slider */
	UPROPERTY(meta = (BindWidget))
	USlider* DialogueVolumeSlider;

	/** Dialogue Volume Progress Bar */
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* DialogueVolumeBar;

	/** UI Volume Slider */
	UPROPERTY(meta = (BindWidget))
	USlider* UIVolumeSlider;

	/** UI Volume Progress Bar */
	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* UIVolumeBar;

	/** Audio Language Selection */
	UPROPERTY(meta = (BindWidget))
	USelectionWidget* AudioLanguageSelection;

	/** Audio Output Selection */
	UPROPERTY(meta = (BindWidget))
	USelectionWidget* AudioOutputSelection;

	/** Closed Captions Toggle */
	UPROPERTY(meta = (BindWidget))
	USelectionWidget* ClosedCaptionsSelection;

	/** Subtitles Toggle */
	UPROPERTY(meta = (BindWidget))
	USelectionWidget* SubtitlesSelection;

	/** Test Audio Button */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* TestAudioButton;

	/** Reset to Default Button */
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

	/** Update progress bars to match sliders */
	void UpdateProgressBars();

	/** Bind slider events */
	void BindSliderEvents();

	/** Unbind slider events */
	void UnbindSliderEvents();
};