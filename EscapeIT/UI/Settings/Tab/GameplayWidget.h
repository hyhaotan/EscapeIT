// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/EscapeITEnums.h"
#include "GameplayWidget.generated.h"

class USelectionWidget;
class USettingsSubsystem;
class UCommonTextBlock;
class UButton;

UCLASS()
class ESCAPEIT_API UGameplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGameplayWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ===== WIDGET BINDINGS =====

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* DifficultySelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* SanityDrainSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* EntityDetectionSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* PuzzleHintSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* HintTimeSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* SkipPuzzlesSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* ObjectiveMarkersSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* InteractionIndicatorsSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* AutoPickupSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* CameraShakeSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* ScreenBlurSelection;

	UPROPERTY(meta = (BindWidget))
	UButton* ResetButton;

	// ===== CALLBACKS =====

	UFUNCTION()
	void OnDifficultyChanged(int32 NewIndex);

	UFUNCTION()
	void OnSanityDrainChanged(int32 NewIndex);

	UFUNCTION()
	void OnEntityDetectionChanged(int32 NewIndex);

	UFUNCTION()
	void OnPuzzleHintChanged(int32 NewIndex);

	UFUNCTION()
	void OnHintTimeChanged(int32 NewIndex);

	UFUNCTION()
	void OnSkipPuzzlesChanged(int32 NewIndex);

	UFUNCTION()
	void OnObjectiveMarkersChanged(int32 NewIndex);

	UFUNCTION()
	void OnInteractionIndicatorsChanged(int32 NewIndex);

	UFUNCTION()
	void OnAutoPickupChanged(int32 NewIndex);

	UFUNCTION()
	void OnCameraShakeChanged(int32 NewIndex);

	UFUNCTION()
	void OnScreenBlurChanged(int32 NewIndex);

	UFUNCTION()
	void OnResetButtonClicked();

	// ===== PROPERTIES =====

	/** Current settings being edited (not yet applied) */
	UPROPERTY()
	FS_GameplaySettings CurrentSettings;

	/** Reference to Settings Subsystem */
	UPROPERTY()
	USettingsSubsystem* SettingsSubsystem;

	/** Flag to prevent callbacks during loading */
	UPROPERTY()
	bool bIsLoadingSettings = false;

public:
	// ===== PUBLIC API =====

	/**
	 * Load settings into the widget UI
	 * @param Settings - The settings to load
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadSettings(const FS_GameplaySettings& Settings);

	/**
	 * Get current settings from the widget (pending changes)
	 * @return The current settings being edited
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	FS_GameplaySettings GetCurrentSettings() const;

	/**
	 * Validate current settings
	 * @return Array of error messages (empty if valid)
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	TArray<FString> ValidateSettings() const;

private:
	// ===== INITIALIZATION =====

	/** Initialize all selection widgets with options */
	void InitializeSelections();

	/** Load settings from subsystem into CurrentSettings and UI */
	void LoadCurrentSettings();

	// ===== HELPER FUNCTIONS =====

	/** Add On/Off options to a selection widget */
	void AddToggleOptions(USelectionWidget* Selection);

	/** Add percentage options (0%, 25%, 50%, 75%, 100%) to a selection widget */
	void AddPercentageOptions(USelectionWidget* Selection);

	/** Add multiplier options (0.5x, 0.75x, 1.0x, 1.5x, 2.0x) to a selection widget */
	void AddMultiplierOptions(USelectionWidget* Selection);

	/** Convert selection index to multiplier value */
	float IndexToMultiplier(int32 Index);

	/** Convert multiplier value to selection index */
	int32 MultiplierToIndex(float Value);

	/** Convert selection index to percentage (0.0 to 1.0) */
	float IndexToPercentage(int32 Index);

	/** Convert percentage to selection index */
	int32 PercentageToIndex(float Value);
};