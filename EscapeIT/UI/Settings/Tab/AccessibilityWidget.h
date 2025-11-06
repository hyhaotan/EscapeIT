// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/SettingsTypes.h"
#include "AccessibilityWidget.generated.h"

class USelectionSettingRow;

UCLASS()
class ESCAPEIT_API UAccessibilityWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UAccessibilityWidget(const FObjectInitializer& ObjectInitializer);

    // UUserWidget
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Public API
    void LoadSettings(const FS_AccessibilitySettings& Settings);
    FS_AccessibilitySettings GetCurrentSettings() const;
    TArray<FString> ValidateSettings() const;

protected:
    // Selection rows (reusable SelectionSettingRow)
    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* TextSizeRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* TextContrastRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* DyslexiaFontRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* ColorBlindModeRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* HighContrastUIRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* ReducedMotionRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* PhotosensitivityModeRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* ScreenReaderRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* SoundCuesVisualizationRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* HapticFeedbackRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* SingleHandedModeRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* HoldToActivateRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* HoldActivationTimeRow;

    // Internal
    UPROPERTY()
    class USettingsSubsystem* SettingsSubsystem;

    // Flags
    bool bIsLoadingSettings;

    // Current local settings (widget-level, not yet saved to subsystem)
    FS_AccessibilitySettings CurrentSettings;

    // Initialization
    void InitializeSelectionRows();

    // Helpers
    TArray<FText> MakeToggleOptions() const;
    TArray<FText> MakeTextSizeOptions() const;
    TArray<FText> MakeTextContrastOptions() const;
    TArray<FText> MakeColorBlindModeOptions() const;
    TArray<FText> MakePhotosensitivityModeOptions() const;
    TArray<FText> MakeSingleHandedModeOptions() const;
    TArray<FText> MakeHoldActivationTimeOptions() const;

    float IndexToActivationTime(int32 Index);
    int32 ActivationTimeToIndex(float Value);

    // Selection callbacks
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
};