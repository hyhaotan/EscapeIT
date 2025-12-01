#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/SettingsTypes.h"
#include "GraphicWidget.generated.h"

class USelectionSettingRow;
class USettingsSubsystem;
class UCommonTextBlock;

UCLASS()
class ESCAPEIT_API UGraphicWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UGraphicWidget(const FObjectInitializer& ObjectInitializer);

    // UUserWidget
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // Public API
    void LoadSettings(const FS_GraphicsSettings& Settings);
    FS_GraphicsSettings GetCurrentSettings() const;
    TArray<FString> ValidateSettings() const;

protected:
    // Selection rows (reusable SelectionSettingRow)
    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* QualityPresetRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* ResolutionRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* VSyncRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* FrameRateCapRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* RayTracingRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* RayTracingQualityRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* ShadowQualityRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* TextureQualityRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* AntiAliasingRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* MotionBlurRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* FieldOfViewRow;

    // Internal
    UPROPERTY()
    USettingsSubsystem* SettingsSubsystem;

    // Flags
    bool bIsLoadingSettings;

    // Current local settings (widget-level, not yet saved to subsystem)
    FS_GraphicsSettings CurrentSettings;

    // FPS counter timer
    FTimerHandle FPSTimerHandle;

    // Initialization
    void InitializeSelectionRows();

    // Helpers
    TArray<FText> MakeToggleOptions() const;
    TArray<FText> MakeQualityPresetOptions() const;
    TArray<FText> MakeResolutionOptions() const;
    TArray<FText> MakeFrameRateCapOptions() const;
    TArray<FText> MakeRayTracingQualityOptions() const;
    TArray<FText> MakeQualityLevelOptions() const;
    TArray<FText> MakeAntiAliasingOptions() const;
    TArray<FText> MakePercentageOptions() const;
    TArray<FText> MakeFieldOfViewOptions() const;

    // Conversion helpers
    TArray<FIntPoint> GetAvailableResolutions() const;
    FIntPoint IndexToResolution(int32 Index) const;
    int32 ResolutionToIndex(FIntPoint Resolution) const;
    int32 IndexToFrameRateCap(int32 Index) const;
    int32 FrameRateCapToIndex(int32 Value) const;
    float IndexToFOV(int32 Index) const;
    int32 FOVToIndex(float Value) const;
    float IndexToPercentage(int32 Index) const;
    int32 PercentageToIndex(float Value) const;

    // Helper to mark preset as custom when user changes individual settings
    void MarkAsCustomPreset();

    // Selection callbacks
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
};

