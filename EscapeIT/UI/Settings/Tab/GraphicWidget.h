// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/SettingsTypes.h"
#include "GraphicWidget.generated.h"

class USelectionWidget;
class USettingsSubsystem;
class UButton;
class UCommonTextBlock;

UCLASS()
class ESCAPEIT_API UGraphicWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGraphicWidget(const FObjectInitializer& ObjectInitializer);

	void LoadSettings(const FS_GraphicsSettings& Settings);

	FS_GraphicsSettings GetCurrentSettings() const;

	TArray<FString> ValidateSettings() const;
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ===== WIDGET BINDINGS =====

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* QualityPresetSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* ResolutionSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* VSyncSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* FrameRateCapSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* RayTracingSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* RayTracingQualitySelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* ShadowQualitySelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* TextureQualitySelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* AntiAliasingSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* MotionBlurSelection;

	UPROPERTY(meta = (BindWidget))
	USelectionWidget* FieldOfViewSelection;

	UPROPERTY(meta = (BindWidgetOptional))
	UCommonTextBlock* FPSText;

	// ===== CALLBACKS =====

	UFUNCTION()
	void OnQualityPresetChanged(int32 NewIndex);

	UFUNCTION()
	void OnResolutionChanged(int32 NewIndex);

	UFUNCTION()
	void OnVSyncChanged(int32 NewIndex);

	UFUNCTION()
	void OnFrameRateCapChanged(int32 NewIndex);

	UFUNCTION()
	void OnRayTracingChanged(int32 NewIndex);

	UFUNCTION()
	void OnRayTracingQualityChanged(int32 NewIndex);

	UFUNCTION()
	void OnShadowQualityChanged(int32 NewIndex);

	UFUNCTION()
	void OnTextureQualityChanged(int32 NewIndex);

	UFUNCTION()
	void OnAntiAliasingChanged(int32 NewIndex);

	UFUNCTION()
	void OnMotionBlurChanged(int32 NewIndex);

	UFUNCTION()
	void OnFieldOfViewChanged(int32 NewIndex);

private:
	/** Reference to Settings Subsystem */
	UPROPERTY()
	USettingsSubsystem* SettingsSubsystem;

	FS_GraphicsSettings CurrentSettings;

	bool bIsLoadingSettings = false;

	bool bSettingsChanged;
	FTimerHandle FPSTimerHandle;

	void InitializeSelections();
	void LoadCurrentSettings();
	void AddToggleOptions(USelectionWidget* Selection);
	void AddPercentageOptions(USelectionWidget* Selection);

	UFUNCTION()
	void UpdateFPSCounter();

	TArray<FIntPoint> GetAvailableResolutions();
	FIntPoint IndexToResolution(int32 Index);
	int32 ResolutionToIndex(FIntPoint Resolution);
	int32 IndexToFrameRateCap(int32 Index);
	int32 FrameRateCapToIndex(int32 Value);
	float IndexToFOV(int32 Index);
	int32 FOVToIndex(float Value);
	void MarkAsCustomPreset();

	float IndexToPercentage(int32 Index);
	int32 PercentageToIndex(float Value);
};