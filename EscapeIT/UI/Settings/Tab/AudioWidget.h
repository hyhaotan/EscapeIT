// AudioWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/EscapeITSettingsStructs.h"
#include "AudioWidget.generated.h"

class USelectionWidget;
class USlider;
class UTextBlock;
class UButton;
class USoundBase;

UCLASS()
class ESCAPEIT_API UAudioWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UAudioWidget(const FObjectInitializer& ObjectInitializer);

	// UUserWidget
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Public API
	void LoadSettings(const FS_AudioSettings& Settings);
	FS_AudioSettings GetCurrentSettings() const;
	TArray<FString> ValidateSettings() const;

protected:
	// UI refs (bind in UMG)
	UPROPERTY(meta = (BindWidgetOptional))
	USelectionWidget* AudioLanguageSelection;

	UPROPERTY(meta = (BindWidgetOptional))
	USelectionWidget* AudioOutputSelection;

	UPROPERTY(meta = (BindWidgetOptional))
	USelectionWidget* ClosedCaptionsSelection;

	UPROPERTY(meta = (BindWidgetOptional))
	USelectionWidget* SubtitlesSelection;

	UPROPERTY(meta = (BindWidgetOptional))
	USlider* MasterVolumeSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* MasterVolumeText;

	UPROPERTY(meta = (BindWidgetOptional))
	USlider* SFXVolumeSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* SFXVolumeText;

	UPROPERTY(meta = (BindWidgetOptional))
	USlider* MusicVolumeSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* MusicVolumeText;

	UPROPERTY(meta = (BindWidgetOptional))
	USlider* AmbientVolumeSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* AmbientVolumeText;

	UPROPERTY(meta = (BindWidgetOptional))
	USlider* DialogueVolumeSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* DialogueVolumeText;

	UPROPERTY(meta = (BindWidgetOptional))
	USlider* UIVolumeSlider;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* UIVolumeText;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* TestAudioButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* ResetButton;

	// Internal
	UPROPERTY()
	class USettingsSubsystem* SettingsSubsystem;

	// Test sound (optional)
	UPROPERTY()
	USoundBase* TestSound;

	// Flags to avoid feedback loops
	bool bUpdatingSliders;
	bool bIsLoadingSettings;

	// Current local settings (widget-level, not yet saved to subsystem)
	FS_AudioSettings CurrentSettings;

	// Initialization helpers
	void InitializeSelections();
	void InitializeSliders();
	void BindSliderEvents();
	void UnbindSliderEvents();

	// UI helpers
	void AddToggleOptions(USelectionWidget* Selection);
	void UpdateVolumeTexts();

	// Callbacks (update CurrentSettings only; do NOT call subsystem here)
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

	// Buttons
	UFUNCTION()
	void OnTestAudioButtonClicked();

	UFUNCTION()
	void OnResetButtonClicked();
};
