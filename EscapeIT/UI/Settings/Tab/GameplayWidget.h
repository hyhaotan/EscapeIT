// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/SettingsTypes.h"
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

	// ===== PROPERTIES =====

	UPROPERTY()
	FS_GameplaySettings CurrentSettings;

	UPROPERTY()
	TObjectPtr<USettingsSubsystem> SettingsSubsystem;

	UPROPERTY()
	bool bIsLoadingSettings = false;

public:
	// ===== PUBLIC API =====

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadSettings(const FS_GameplaySettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	FS_GameplaySettings GetCurrentSettings() const;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	TArray<FString> ValidateSettings() const;

private:
	// ===== INITIALIZATION =====

	void InitializeSelections();

	void LoadCurrentSettings();

	// ===== HELPER FUNCTIONS =====

	void AddToggleOptions(USelectionWidget* Selection);

	void AddPercentageOptions(USelectionWidget* Selection);

	void AddMultiplierOptions(USelectionWidget* Selection);

	float IndexToMultiplier(int32 Index);

	int32 MultiplierToIndex(float Value);

	float IndexToPercentage(int32 Index);

	int32 PercentageToIndex(float Value);
};