// GameplayWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/SettingsTypes.h"
#include "GameplayWidget.generated.h"

class USelectionSettingRow;
class USettingsSubsystem;

UCLASS()
class ESCAPEIT_API UGameplayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UGameplayWidget(const FObjectInitializer& ObjectInitializer);

    // UUserWidget
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // Public API
    void LoadSettings(const FS_GameplaySettings& Settings);
    FS_GameplaySettings GetCurrentSettings() const;
    TArray<FString> ValidateSettings() const;

protected:
    // Selection rows (reusable SelectionSettingRow)
    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* DifficultyRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* SanityDrainRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* EntityDetectionRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* PuzzleHintRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* HintTimeRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* SkipPuzzlesRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* ObjectiveMarkersRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* InteractionIndicatorsRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* AutoPickupRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* CameraShakeRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* ScreenBlurRow;

    // Internal
    UPROPERTY()
    USettingsSubsystem* SettingsSubsystem;

    // Flags
    bool bIsLoadingSettings;

    // Current local settings (widget-level, not yet saved to subsystem)
    FS_GameplaySettings CurrentSettings;

    // Initialization
    void InitializeSelectionRows();

    // Helpers
    TArray<FText> MakeToggleOptions() const;
    TArray<FText> MakeDifficultyOptions() const;
    TArray<FText> MakeMultiplierOptions() const;
    TArray<FText> MakeHintTimeOptions() const;
    TArray<FText> MakePercentageOptions() const;

    // Conversion helpers
    float IndexToMultiplier(int32 Index) const;
    int32 MultiplierToIndex(float Value) const;
    float IndexToPercentage(int32 Index) const;
    int32 PercentageToIndex(float Value) const;
    float IndexToHintTime(int32 Index) const;
    int32 HintTimeToIndex(float Value) const;

    // Selection callbacks
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
};