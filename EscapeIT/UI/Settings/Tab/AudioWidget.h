// AudioWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/Data/SettingsTypes.h"
#include "AudioWidget.generated.h"

class USelectionSettingRow;
class UNumericSettingRow;
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
    // Selection rows (reusable SelectionSettingRow)
    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* AudioLanguageRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* AudioOutputRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* ClosedCaptionsRow;

    UPROPERTY(meta = (BindWidgetOptional))
    USelectionSettingRow* SubtitlesRow;

    // Reusable numeric rows (Bind instances of your NumericSettingRow UMG)
    UPROPERTY(meta = (BindWidgetOptional))
    UNumericSettingRow* MasterVolumeRow;

    UPROPERTY(meta = (BindWidgetOptional))
    UNumericSettingRow* SFXVolumeRow;

    UPROPERTY(meta = (BindWidgetOptional))
    UNumericSettingRow* MusicVolumeRow;

    UPROPERTY(meta = (BindWidgetOptional))
    UNumericSettingRow* AmbientVolumeRow;

    UPROPERTY(meta = (BindWidgetOptional))
    UNumericSettingRow* DialogueVolumeRow;

    UPROPERTY(meta = (BindWidgetOptional))
    UNumericSettingRow* UIVolumeRow;

    UPROPERTY(meta = (BindWidgetOptional))
    UButton* TestAudioButton;

    // Internal
    UPROPERTY()
    class USettingsSubsystem* SettingsSubsystem;

    // Test sound (optional)
    UPROPERTY()
    USoundBase* TestSound;

    // Flags
    bool bIsLoadingSettings;

    // Current local settings (widget-level, not yet saved to subsystem)
    FS_AudioSettings CurrentSettings;

    // Initialization
    void InitializeSelectionRows();

    // Helpers
    TArray<FText> MakeToggleOptions() const;
    TArray<FText> MakeAudioLanguageOptions() const;
    TArray<FText> MakeAudioOutputOptions() const;

    // Selection callbacks
    UFUNCTION()
    void OnAudioLanguageChanged(int32 NewIndex);

    UFUNCTION()
    void OnAudioOutputChanged(int32 NewIndex);

    UFUNCTION()
    void OnClosedCaptionsChanged(int32 NewIndex);

    UFUNCTION()
    void OnSubtitlesChanged(int32 NewIndex);

    // Numeric row handlers
    UFUNCTION()
    void HandleMasterVolumeRowChanged(float NewValue);

    UFUNCTION()
    void HandleSFXVolumeRowChanged(float NewValue);

    UFUNCTION()
    void HandleMusicVolumeRowChanged(float NewValue);

    UFUNCTION()
    void HandleAmbientVolumeRowChanged(float NewValue);

    UFUNCTION()
    void HandleDialogueVolumeRowChanged(float NewValue);

    UFUNCTION()
    void HandleUIVolumeRowChanged(float NewValue);

    // Button
    UFUNCTION()
    void OnTestAudioButtonClicked();
};