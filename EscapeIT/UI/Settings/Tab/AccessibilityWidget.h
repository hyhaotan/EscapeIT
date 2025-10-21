// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AccessibilityWidget.generated.h"

class USelectionWidget;
class USettingsSubsystem;
class UButton;

UCLASS()
class ESCAPEIT_API UAccessibilityWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UAccessibilityWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ===== WIDGET BINDINGS =====

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* TextSizeSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* TextContrastSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* DyslexiaFontSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* ColorBlindModeSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* HighContrastUISelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* ReducedMotionSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* PhotosensitivityModeSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* ScreenReaderSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* SoundCuesVisualizationSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* HapticFeedbackSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* SingleHandedModeSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* HoldToActivateSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* HoldActivationTimeSelection;

	UPROPERTY(meta = (BindWidget))
	UButton* ResetButton;

	// ===== CALLBACKS =====

	UFUNCTION()
	void OnTextSizeChanged(int32 NewIndex);

	UFUNCTION()
	void OnTextContrastChanged(int32 NewIndex);

	UFUNCTION()
	void OnDyslexiaFontChanged(int32 NewIndex);

	UFUNCTION()
	void OnColorBlindModeChanged(int32 NewIndex);

	UFUNCTION()
	void OnHighContrastUIChanged(int32 NewIndex);

	UFUNCTION()
	void OnReducedMotionChanged(int32 NewIndex);

	UFUNCTION()
	void OnPhotosensitivityModeChanged(int32 NewIndex);

	UFUNCTION()
	void OnScreenReaderChanged(int32 NewIndex);

	UFUNCTION()
	void OnSoundCuesVisualizationChanged(int32 NewIndex);

	UFUNCTION()
	void OnHapticFeedbackChanged(int32 NewIndex);

	UFUNCTION()
	void OnSingleHandedModeChanged(int32 NewIndex);

	UFUNCTION()
	void OnHoldToActivateChanged(int32 NewIndex);

	UFUNCTION()
	void OnHoldActivationTimeChanged(int32 NewIndex);

	UFUNCTION()
	void OnResetButtonClicked();

private:
	/** Reference to Settings Subsystem */
	UPROPERTY()
	USettingsSubsystem* SettingsSubsystem;

	void InitializeSelections();
	void LoadCurrentSettings();
	void AddToggleOptions(USelectionWidget* Selection);
	float IndexToActivationTime(int32 Index);
	int32 ActivationTimeToIndex(float Value);
};