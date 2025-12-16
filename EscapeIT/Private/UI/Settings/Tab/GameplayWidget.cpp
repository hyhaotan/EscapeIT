
#include "UI/Settings/Tab/GameplayWidget.h"
#include "UI/Settings/Row/SelectionSettingRow.h"
#include "Settings/Core/SettingsSubsystem.h"

UGameplayWidget::UGameplayWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , bIsLoadingSettings(false)
    , SettingsSubsystem(nullptr)
{
}

void UGameplayWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get Settings Subsystem (optional)
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        SettingsSubsystem = GameInstance->GetSubsystem<USettingsSubsystem>();
    }

    if (!SettingsSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplayWidget: SettingsSubsystem not found; widget will still work locally"));
    }

    // Initialize selection rows and bind their delegates
    InitializeSelectionRows();

    // Load current settings either from subsystem (if available) or defaults
    if (SettingsSubsystem)
    {
        LoadSettings(SettingsSubsystem->GetAllSettings().GameplaySettings);
    }
    else
    {
        FS_GameplaySettings DefaultSettings;
        LoadSettings(DefaultSettings);
    }
}

void UGameplayWidget::NativeDestruct()
{
    // Unbind selection row delegates
    if (DifficultyRow)
        DifficultyRow->OnSelectionChanged.RemoveDynamic(this, &UGameplayWidget::OnDifficultyChanged);
    if (SanityDrainRow)
        SanityDrainRow->OnSelectionChanged.RemoveDynamic(this, &UGameplayWidget::OnSanityDrainChanged);
    if (EntityDetectionRow)
        EntityDetectionRow->OnSelectionChanged.RemoveDynamic(this, &UGameplayWidget::OnEntityDetectionChanged);
    if (PuzzleHintRow)
        PuzzleHintRow->OnSelectionChanged.RemoveDynamic(this, &UGameplayWidget::OnPuzzleHintChanged);
    if (HintTimeRow)
        HintTimeRow->OnSelectionChanged.RemoveDynamic(this, &UGameplayWidget::OnHintTimeChanged);
    if (SkipPuzzlesRow)
        SkipPuzzlesRow->OnSelectionChanged.RemoveDynamic(this, &UGameplayWidget::OnSkipPuzzlesChanged);
    if (ObjectiveMarkersRow)
        ObjectiveMarkersRow->OnSelectionChanged.RemoveDynamic(this, &UGameplayWidget::OnObjectiveMarkersChanged);
    if (InteractionIndicatorsRow)
        InteractionIndicatorsRow->OnSelectionChanged.RemoveDynamic(this, &UGameplayWidget::OnInteractionIndicatorsChanged);
    if (AutoPickupRow)
        AutoPickupRow->OnSelectionChanged.RemoveDynamic(this, &UGameplayWidget::OnAutoPickupChanged);
    if (CameraShakeRow)
        CameraShakeRow->OnSelectionChanged.RemoveDynamic(this, &UGameplayWidget::OnCameraShakeChanged);
    if (ScreenBlurRow)
        ScreenBlurRow->OnSelectionChanged.RemoveDynamic(this, &UGameplayWidget::OnScreenBlurChanged);

    Super::NativeDestruct();
}

// Initialize selection rows with options and bind delegates
void UGameplayWidget::InitializeSelectionRows()
{
    const TArray<FText> ToggleOptions = MakeToggleOptions();
    const TArray<FText> DifficultyOptions = MakeDifficultyOptions();
    const TArray<FText> MultiplierOptions = MakeMultiplierOptions();
    const TArray<FText> HintTimeOptions = MakeHintTimeOptions();
    const TArray<FText> PercentageOptions = MakePercentageOptions();

    if (DifficultyRow)
    {
        DifficultyRow->InitializeRow(DifficultyOptions, static_cast<int32>(CurrentSettings.DifficultyLevel), FText::FromString(TEXT("Difficulty Level")));
        DifficultyRow->OnSelectionChanged.AddDynamic(this, &UGameplayWidget::OnDifficultyChanged);
    }

    if (SanityDrainRow)
    {
        SanityDrainRow->InitializeRow(MultiplierOptions, MultiplierToIndex(CurrentSettings.SanityDrainMultiplier), FText::FromString(TEXT("Sanity Drain")));
        SanityDrainRow->OnSelectionChanged.AddDynamic(this, &UGameplayWidget::OnSanityDrainChanged);
    }

    if (EntityDetectionRow)
    {
        EntityDetectionRow->InitializeRow(MultiplierOptions, MultiplierToIndex(CurrentSettings.EntityDetectionRangeMultiplier), FText::FromString(TEXT("Entity Detection Range")));
        EntityDetectionRow->OnSelectionChanged.AddDynamic(this, &UGameplayWidget::OnEntityDetectionChanged);
    }

    if (PuzzleHintRow)
    {
        PuzzleHintRow->InitializeRow(ToggleOptions, CurrentSettings.bPuzzleHintSystemEnabled ? 1 : 0, FText::FromString(TEXT("Puzzle Hints")));
        PuzzleHintRow->OnSelectionChanged.AddDynamic(this, &UGameplayWidget::OnPuzzleHintChanged);
    }

    if (HintTimeRow)
    {
        HintTimeRow->InitializeRow(HintTimeOptions, HintTimeToIndex(CurrentSettings.AutoRevealHintTime), FText::FromString(TEXT("Hint Time")));
        HintTimeRow->OnSelectionChanged.AddDynamic(this, &UGameplayWidget::OnHintTimeChanged);
    }

    if (SkipPuzzlesRow)
    {
        SkipPuzzlesRow->InitializeRow(ToggleOptions, CurrentSettings.bAllowSkipPuzzles ? 1 : 0, FText::FromString(TEXT("Skip Puzzles")));
        SkipPuzzlesRow->OnSelectionChanged.AddDynamic(this, &UGameplayWidget::OnSkipPuzzlesChanged);
    }

    if (ObjectiveMarkersRow)
    {
        ObjectiveMarkersRow->InitializeRow(ToggleOptions, CurrentSettings.bShowObjectiveMarkers ? 1 : 0, FText::FromString(TEXT("Objective Markers")));
        ObjectiveMarkersRow->OnSelectionChanged.AddDynamic(this, &UGameplayWidget::OnObjectiveMarkersChanged);
    }

    if (InteractionIndicatorsRow)
    {
        InteractionIndicatorsRow->InitializeRow(ToggleOptions, CurrentSettings.bShowInteractionIndicators ? 1 : 0, FText::FromString(TEXT("Interaction Indicators")));
        InteractionIndicatorsRow->OnSelectionChanged.AddDynamic(this, &UGameplayWidget::OnInteractionIndicatorsChanged);
    }

    if (AutoPickupRow)
    {
        AutoPickupRow->InitializeRow(ToggleOptions, CurrentSettings.bAutoPickupItems ? 1 : 0, FText::FromString(TEXT("Auto Pickup")));
        AutoPickupRow->OnSelectionChanged.AddDynamic(this, &UGameplayWidget::OnAutoPickupChanged);
    }

    if (CameraShakeRow)
    {
        CameraShakeRow->InitializeRow(PercentageOptions, PercentageToIndex(CurrentSettings.CameraShakeMagnitude / 2.0f), FText::FromString(TEXT("Camera Shake")));
        CameraShakeRow->OnSelectionChanged.AddDynamic(this, &UGameplayWidget::OnCameraShakeChanged);
    }

    if (ScreenBlurRow)
    {
        ScreenBlurRow->InitializeRow(PercentageOptions, PercentageToIndex(CurrentSettings.ScreenBlurAmount), FText::FromString(TEXT("Screen Blur")));
        ScreenBlurRow->OnSelectionChanged.AddDynamic(this, &UGameplayWidget::OnScreenBlurChanged);
    }
}

TArray<FText> UGameplayWidget::MakeToggleOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Off")));
    Out.Add(FText::FromString(TEXT("On")));
    return Out;
}

TArray<FText> UGameplayWidget::MakeDifficultyOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("Easy")));
    Out.Add(FText::FromString(TEXT("Normal")));
    Out.Add(FText::FromString(TEXT("Hard")));
    Out.Add(FText::FromString(TEXT("Nightmare")));
    return Out;
}

TArray<FText> UGameplayWidget::MakeMultiplierOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("0.5x")));
    Out.Add(FText::FromString(TEXT("0.75x")));
    Out.Add(FText::FromString(TEXT("1.0x")));
    Out.Add(FText::FromString(TEXT("1.5x")));
    Out.Add(FText::FromString(TEXT("2.0x")));
    return Out;
}

TArray<FText> UGameplayWidget::MakeHintTimeOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("10s")));
    Out.Add(FText::FromString(TEXT("30s")));
    Out.Add(FText::FromString(TEXT("60s")));
    Out.Add(FText::FromString(TEXT("120s")));
    Out.Add(FText::FromString(TEXT("180s")));
    Out.Add(FText::FromString(TEXT("300s")));
    return Out;
}

TArray<FText> UGameplayWidget::MakePercentageOptions() const
{
    TArray<FText> Out;
    Out.Add(FText::FromString(TEXT("0%")));
    Out.Add(FText::FromString(TEXT("25%")));
    Out.Add(FText::FromString(TEXT("50%")));
    Out.Add(FText::FromString(TEXT("75%")));
    Out.Add(FText::FromString(TEXT("100%")));
    return Out;
}

void UGameplayWidget::LoadSettings(const FS_GameplaySettings& Settings)
{
    bIsLoadingSettings = true;
    CurrentSettings = Settings;

    // Update selection rows (do not trigger delegates)
    if (DifficultyRow)
        DifficultyRow->SetCurrentSelection(static_cast<int32>(CurrentSettings.DifficultyLevel), false);

    if (SanityDrainRow)
        SanityDrainRow->SetCurrentSelection(MultiplierToIndex(CurrentSettings.SanityDrainMultiplier), false);

    if (EntityDetectionRow)
        EntityDetectionRow->SetCurrentSelection(MultiplierToIndex(CurrentSettings.EntityDetectionRangeMultiplier), false);

    if (PuzzleHintRow)
        PuzzleHintRow->SetCurrentSelection(CurrentSettings.bPuzzleHintSystemEnabled ? 1 : 0, false);

    if (HintTimeRow)
        HintTimeRow->SetCurrentSelection(HintTimeToIndex(CurrentSettings.AutoRevealHintTime), false);

    if (SkipPuzzlesRow)
        SkipPuzzlesRow->SetCurrentSelection(CurrentSettings.bAllowSkipPuzzles ? 1 : 0, false);

    if (ObjectiveMarkersRow)
        ObjectiveMarkersRow->SetCurrentSelection(CurrentSettings.bShowObjectiveMarkers ? 1 : 0, false);

    if (InteractionIndicatorsRow)
        InteractionIndicatorsRow->SetCurrentSelection(CurrentSettings.bShowInteractionIndicators ? 1 : 0, false);

    if (AutoPickupRow)
        AutoPickupRow->SetCurrentSelection(CurrentSettings.bAutoPickupItems ? 1 : 0, false);

    if (CameraShakeRow)
        CameraShakeRow->SetCurrentSelection(PercentageToIndex(CurrentSettings.CameraShakeMagnitude / 2.0f), false);

    if (ScreenBlurRow)
        ScreenBlurRow->SetCurrentSelection(PercentageToIndex(CurrentSettings.ScreenBlurAmount), false);

    bIsLoadingSettings = false;
}

FS_GameplaySettings UGameplayWidget::GetCurrentSettings() const
{
    return CurrentSettings;
}

TArray<FString> UGameplayWidget::ValidateSettings() const
{
    TArray<FString> Errors;

    if (CurrentSettings.SanityDrainMultiplier < 0.0f || CurrentSettings.SanityDrainMultiplier > 3.0f)
    {
        Errors.Add(TEXT("Sanity drain multiplier out of range (0.0 - 3.0)"));
    }
    if (CurrentSettings.EntityDetectionRangeMultiplier < 0.0f || CurrentSettings.EntityDetectionRangeMultiplier > 3.0f)
    {
        Errors.Add(TEXT("Entity detection range multiplier out of range (0.0 - 3.0)"));
    }
    if (CurrentSettings.AutoRevealHintTime < 0.0f || CurrentSettings.AutoRevealHintTime > 600.0f)
    {
        Errors.Add(TEXT("Auto reveal hint time out of range (0 - 600 seconds)"));
    }
    if (CurrentSettings.CameraShakeMagnitude < 0.0f || CurrentSettings.CameraShakeMagnitude > 2.0f)
    {
        Errors.Add(TEXT("Camera shake magnitude out of range (0.0 - 2.0)"));
    }
    if (CurrentSettings.ScreenBlurAmount < 0.0f || CurrentSettings.ScreenBlurAmount > 1.0f)
    {
        Errors.Add(TEXT("Screen blur amount out of range (0.0 - 1.0)"));
    }

    if (Errors.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameplayWidget: Validation found %d errors"), Errors.Num());
    }

    return Errors;
}

// Conversion helpers

float UGameplayWidget::IndexToMultiplier(int32 Index) const
{
    TArray<float> Multipliers = { 0.5f, 0.75f, 1.0f, 1.5f, 2.0f };
    if (Index >= 0 && Index < Multipliers.Num())
    {
        return Multipliers[Index];
    }
    return 1.0f; // Default
}

int32 UGameplayWidget::MultiplierToIndex(float Value) const
{
    TArray<float> Multipliers = { 0.5f, 0.75f, 1.0f, 1.5f, 2.0f };

    int32 ClosestIndex = 2; // Default to 1.0x
    float ClosestDiff = FLT_MAX;

    for (int32 i = 0; i < Multipliers.Num(); i++)
    {
        float Diff = FMath::Abs(Value - Multipliers[i]);
        if (Diff < ClosestDiff)
        {
            ClosestDiff = Diff;
            ClosestIndex = i;
        }
    }

    return ClosestIndex;
}

float UGameplayWidget::IndexToPercentage(int32 Index) const
{
    TArray<float> Percentages = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
    if (Index >= 0 && Index < Percentages.Num())
    {
        return Percentages[Index];
    }
    return 0.5f; // Default
}

int32 UGameplayWidget::PercentageToIndex(float Value) const
{
    TArray<float> Percentages = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };

    int32 ClosestIndex = 2; // Default to 50%
    float ClosestDiff = FLT_MAX;

    for (int32 i = 0; i < Percentages.Num(); i++)
    {
        float Diff = FMath::Abs(Value - Percentages[i]);
        if (Diff < ClosestDiff)
        {
            ClosestDiff = Diff;
            ClosestIndex = i;
        }
    }

    return ClosestIndex;
}

float UGameplayWidget::IndexToHintTime(int32 Index) const
{
    TArray<float> Times = { 10.0f, 30.0f, 60.0f, 120.0f, 180.0f, 300.0f };
    if (Index >= 0 && Index < Times.Num())
    {
        return Times[Index];
    }
    return 60.0f; // Default
}

int32 UGameplayWidget::HintTimeToIndex(float Value) const
{
    TArray<float> Times = { 10.0f, 30.0f, 60.0f, 120.0f, 180.0f, 300.0f };

    int32 ClosestIndex = 2; // Default to 60s
    float ClosestDiff = FLT_MAX;

    for (int32 i = 0; i < Times.Num(); i++)
    {
        float Diff = FMath::Abs(Value - Times[i]);
        if (Diff < ClosestDiff)
        {
            ClosestDiff = Diff;
            ClosestIndex = i;
        }
    }

    return ClosestIndex;
}

// Selection callbacks

void UGameplayWidget::OnDifficultyChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;

    EE_DifficultyLevel NewDifficulty = static_cast<EE_DifficultyLevel>(NewIndex);
    CurrentSettings.DifficultyLevel = NewDifficulty;

    UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Difficulty changed to index %d (local)"), NewIndex);

    // Auto-adjust multipliers based on difficulty if subsystem is available
    if (SettingsSubsystem)
    {
        FS_DifficultyMultiplier Multipliers = SettingsSubsystem->GetDifficultyMultiplier(NewDifficulty);
        CurrentSettings.SanityDrainMultiplier = Multipliers.SanityDrainMultiplier;
        CurrentSettings.EntityDetectionRangeMultiplier = Multipliers.AIDetectionMultiplier;

        // Update UI to reflect auto-adjusted values (without triggering callbacks)
        bIsLoadingSettings = true;
        if (SanityDrainRow)
            SanityDrainRow->SetCurrentSelection(MultiplierToIndex(CurrentSettings.SanityDrainMultiplier), false);
        if (EntityDetectionRow)
            EntityDetectionRow->SetCurrentSelection(MultiplierToIndex(CurrentSettings.EntityDetectionRangeMultiplier), false);
        bIsLoadingSettings = false;

        SettingsSubsystem->ApplyGameplaySettings(CurrentSettings);
    }
}

void UGameplayWidget::OnSanityDrainChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.SanityDrainMultiplier = IndexToMultiplier(NewIndex);
    UE_LOG(LogTemp, Log, TEXT("GameplayWidget: SanityDrain changed to %.2f (local)"), CurrentSettings.SanityDrainMultiplier);
    if (SettingsSubsystem) SettingsSubsystem->ApplyGameplaySettings(CurrentSettings);
}

void UGameplayWidget::OnEntityDetectionChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.EntityDetectionRangeMultiplier = IndexToMultiplier(NewIndex);
    UE_LOG(LogTemp, Log, TEXT("GameplayWidget: EntityDetection changed to %.2f (local)"), CurrentSettings.EntityDetectionRangeMultiplier);
    if (SettingsSubsystem) SettingsSubsystem->ApplyGameplaySettings(CurrentSettings);
}

void UGameplayWidget::OnPuzzleHintChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bPuzzleHintSystemEnabled = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("GameplayWidget: PuzzleHints changed to %s (local)"), CurrentSettings.bPuzzleHintSystemEnabled ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyGameplaySettings(CurrentSettings);
}

void UGameplayWidget::OnHintTimeChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.AutoRevealHintTime = IndexToHintTime(NewIndex);
    UE_LOG(LogTemp, Log, TEXT("GameplayWidget: HintTime changed to %.0fs (local)"), CurrentSettings.AutoRevealHintTime);
    if (SettingsSubsystem) SettingsSubsystem->ApplyGameplaySettings(CurrentSettings);
}

void UGameplayWidget::OnSkipPuzzlesChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bAllowSkipPuzzles = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("GameplayWidget: SkipPuzzles changed to %s (local)"), CurrentSettings.bAllowSkipPuzzles ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyGameplaySettings(CurrentSettings);
}

void UGameplayWidget::OnObjectiveMarkersChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bShowObjectiveMarkers = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("GameplayWidget: ObjectiveMarkers changed to %s (local)"), CurrentSettings.bShowObjectiveMarkers ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyGameplaySettings(CurrentSettings);
}

void UGameplayWidget::OnInteractionIndicatorsChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bShowInteractionIndicators = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("GameplayWidget: InteractionIndicators changed to %s (local)"), CurrentSettings.bShowInteractionIndicators ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyGameplaySettings(CurrentSettings);
}

void UGameplayWidget::OnAutoPickupChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.bAutoPickupItems = (NewIndex == 1);
    UE_LOG(LogTemp, Log, TEXT("GameplayWidget: AutoPickup changed to %s (local)"), CurrentSettings.bAutoPickupItems ? TEXT("true") : TEXT("false"));
    if (SettingsSubsystem) SettingsSubsystem->ApplyGameplaySettings(CurrentSettings);
}

void UGameplayWidget::OnCameraShakeChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    float Percentage = IndexToPercentage(NewIndex);
    CurrentSettings.CameraShakeMagnitude = Percentage * 2.0f; // Convert 0-1 to 0-2 range
    UE_LOG(LogTemp, Log, TEXT("GameplayWidget: CameraShake changed to %.2f (local)"), CurrentSettings.CameraShakeMagnitude);
    if (SettingsSubsystem) SettingsSubsystem->ApplyGameplaySettings(CurrentSettings);
}

void UGameplayWidget::OnScreenBlurChanged(int32 NewIndex)
{
    if (bIsLoadingSettings) return;
    CurrentSettings.ScreenBlurAmount = IndexToPercentage(NewIndex);
    UE_LOG(LogTemp, Log, TEXT("GameplayWidget: ScreenBlur changed to %.2f (local)"), CurrentSettings.ScreenBlurAmount);
    if (SettingsSubsystem) SettingsSubsystem->ApplyGameplaySettings(CurrentSettings);
}