#include "GameplayWidget.h"
#include "EscapeIT/UI/Settings/Tab/Selection/SelectionWidget.h"
#include "EscapeIT/Subsystem/SettingsSubsystem.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "AccessibilityWidget.h"

UGameplayWidget::UGameplayWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get Settings Subsystem
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		SettingsSubsystem = GameInstance->GetSubsystem<USettingsSubsystem>();
	}

	if (!SettingsSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("GameplayWidget: Failed to get SettingsSubsystem"));
		return;
	}

	// Initialize all selections
	InitializeSelections();

	// Load current settings from subsystem
	LoadCurrentSettings();

	// Bind Reset Button
	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &UGameplayWidget::OnResetButtonClicked);
	}
}

void UGameplayWidget::NativeDestruct()
{
	// Unbind all delegates
	if (ResetButton)
	{
		ResetButton->OnClicked.RemoveDynamic(this, &UGameplayWidget::OnResetButtonClicked);
	}

	Super::NativeDestruct();
}

void UGameplayWidget::InitializeSelections()
{
	// Difficulty Level
	if (DifficultySelection)
	{
		DifficultySelection->Clear();
		DifficultySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Easy")) });
		DifficultySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Normal")) });
		DifficultySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Hard")) });
		DifficultySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Nightmare")) });
		DifficultySelection->OnSelectionChanged.BindDynamic(this, &UGameplayWidget::OnDifficultyChanged);
	}

	// Sanity Drain Multiplier
	if (SanityDrainSelection)
	{
		SanityDrainSelection->Clear();
		AddMultiplierOptions(SanityDrainSelection);
		SanityDrainSelection->OnSelectionChanged.BindDynamic(this, &UGameplayWidget::OnSanityDrainChanged);
	}

	// Entity Detection Range
	if (EntityDetectionSelection)
	{
		EntityDetectionSelection->Clear();
		AddMultiplierOptions(EntityDetectionSelection);
		EntityDetectionSelection->OnSelectionChanged.BindDynamic(this, &UGameplayWidget::OnEntityDetectionChanged);
	}

	// Puzzle Hint System
	if (PuzzleHintSelection)
	{
		PuzzleHintSelection->Clear();
		AddToggleOptions(PuzzleHintSelection);
		PuzzleHintSelection->OnSelectionChanged.BindDynamic(this, &UGameplayWidget::OnPuzzleHintChanged);
	}

	// Auto Reveal Hint Time (10s to 300s in steps)
	if (HintTimeSelection)
	{
		HintTimeSelection->Clear();
		HintTimeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("10s")) });
		HintTimeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("30s")) });
		HintTimeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("60s")) });
		HintTimeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("120s")) });
		HintTimeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("180s")) });
		HintTimeSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("300s")) });
		HintTimeSelection->OnSelectionChanged.BindDynamic(this, &UGameplayWidget::OnHintTimeChanged);
	}

	// Skip Puzzles
	if (SkipPuzzlesSelection)
	{
		SkipPuzzlesSelection->Clear();
		AddToggleOptions(SkipPuzzlesSelection);
		SkipPuzzlesSelection->OnSelectionChanged.BindDynamic(this, &UGameplayWidget::OnSkipPuzzlesChanged);
	}

	// Objective Markers
	if (ObjectiveMarkersSelection)
	{
		ObjectiveMarkersSelection->Clear();
		AddToggleOptions(ObjectiveMarkersSelection);
		ObjectiveMarkersSelection->OnSelectionChanged.BindDynamic(this, &UGameplayWidget::OnObjectiveMarkersChanged);
	}

	// Interaction Indicators
	if (InteractionIndicatorsSelection)
	{
		InteractionIndicatorsSelection->Clear();
		AddToggleOptions(InteractionIndicatorsSelection);
		InteractionIndicatorsSelection->OnSelectionChanged.BindDynamic(this, &UGameplayWidget::OnInteractionIndicatorsChanged);
	}

	// Auto Pickup
	if (AutoPickupSelection)
	{
		AutoPickupSelection->Clear();
		AddToggleOptions(AutoPickupSelection);
		AutoPickupSelection->OnSelectionChanged.BindDynamic(this, &UGameplayWidget::OnAutoPickupChanged);
	}

	// Camera Shake
	if (CameraShakeSelection)
	{
		CameraShakeSelection->Clear();
		AddPercentageOptions(CameraShakeSelection);
		CameraShakeSelection->OnSelectionChanged.BindDynamic(this, &UGameplayWidget::OnCameraShakeChanged);
	}

	// Screen Blur
	if (ScreenBlurSelection)
	{
		ScreenBlurSelection->Clear();
		AddPercentageOptions(ScreenBlurSelection);
		ScreenBlurSelection->OnSelectionChanged.BindDynamic(this, &UGameplayWidget::OnScreenBlurChanged);
	}
}

void UGameplayWidget::LoadCurrentSettings()
{
	if (!SettingsSubsystem)
		return;

	// Lấy settings từ subsystem và lưu vào CurrentSettings
	CurrentSettings = SettingsSubsystem->GetAllSettings().GameplaySettings;

	// Load settings vào UI
	LoadSettings(CurrentSettings);

	UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Loaded current settings from subsystem"));
}

// ===== PUBLIC API =====

void UGameplayWidget::LoadSettings(const FS_GameplaySettings& Settings)
{
	// Lưu vào CurrentSettings
	CurrentSettings = Settings;

	// Load vào UI (không trigger callbacks)
	bIsLoadingSettings = true;

	// Difficulty Level
	if (DifficultySelection)
	{
		int32 DifficultyIndex = static_cast<int32>(Settings.DifficultyLevel);
		DifficultySelection->SetCurrentSelection(DifficultyIndex);
	}

	// Sanity Drain Multiplier
	if (SanityDrainSelection)
	{
		int32 Index = MultiplierToIndex(Settings.SanityDrainMultiplier);
		SanityDrainSelection->SetCurrentSelection(Index);
	}

	// Entity Detection Range
	if (EntityDetectionSelection)
	{
		int32 Index = MultiplierToIndex(Settings.EntityDetectionRangeMultiplier);
		EntityDetectionSelection->SetCurrentSelection(Index);
	}

	// Puzzle Hint System
	if (PuzzleHintSelection)
	{
		PuzzleHintSelection->SetCurrentSelection(Settings.bPuzzleHintSystemEnabled ? 1 : 0);
	}

	// Auto Reveal Hint Time
	if (HintTimeSelection)
	{
		int32 Index = 0;
		TArray<float> Times = { 10.0f, 30.0f, 60.0f, 120.0f, 180.0f, 300.0f };
		for (int32 i = 0; i < Times.Num(); i++)
		{
			if (FMath::IsNearlyEqual(Settings.AutoRevealHintTime, Times[i], 0.1f))
			{
				Index = i;
				break;
			}
		}
		HintTimeSelection->SetCurrentSelection(Index);
	}

	// Skip Puzzles
	if (SkipPuzzlesSelection)
	{
		SkipPuzzlesSelection->SetCurrentSelection(Settings.bAllowSkipPuzzles ? 1 : 0);
	}

	// Objective Markers
	if (ObjectiveMarkersSelection)
	{
		ObjectiveMarkersSelection->SetCurrentSelection(Settings.bShowObjectiveMarkers ? 1 : 0);
	}

	// Interaction Indicators
	if (InteractionIndicatorsSelection)
	{
		InteractionIndicatorsSelection->SetCurrentSelection(Settings.bShowInteractionIndicators ? 1 : 0);
	}

	// Auto Pickup
	if (AutoPickupSelection)
	{
		AutoPickupSelection->SetCurrentSelection(Settings.bAutoPickupItems ? 1 : 0);
	}

	// Camera Shake
	if (CameraShakeSelection)
	{
		int32 Index = PercentageToIndex(Settings.CameraShakeMagnitude / 2.0f); // 0-2 range to 0-1
		CameraShakeSelection->SetCurrentSelection(Index);
	}

	// Screen Blur
	if (ScreenBlurSelection)
	{
		int32 Index = PercentageToIndex(Settings.ScreenBlurAmount);
		ScreenBlurSelection->SetCurrentSelection(Index);
	}

	bIsLoadingSettings = false;

	UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Settings loaded into UI"));
}

FS_GameplaySettings UGameplayWidget::GetCurrentSettings() const
{
	return CurrentSettings;
}

TArray<FString> UGameplayWidget::ValidateSettings() const
{
	TArray<FString> Errors;

	// Validate Sanity Drain Multiplier
	if (CurrentSettings.SanityDrainMultiplier < 0.0f || CurrentSettings.SanityDrainMultiplier > 3.0f)
	{
		Errors.Add(TEXT("Sanity drain multiplier must be between 0.0 and 3.0"));
	}

	// Validate Entity Detection Range Multiplier
	if (CurrentSettings.EntityDetectionRangeMultiplier < 0.0f || CurrentSettings.EntityDetectionRangeMultiplier > 3.0f)
	{
		Errors.Add(TEXT("Entity detection range multiplier must be between 0.0 and 3.0"));
	}

	// Validate Auto Reveal Hint Time
	if (CurrentSettings.AutoRevealHintTime < 0.0f || CurrentSettings.AutoRevealHintTime > 600.0f)
	{
		Errors.Add(TEXT("Auto reveal hint time must be between 0 and 600 seconds"));
	}

	// Validate Camera Shake
	if (CurrentSettings.CameraShakeMagnitude < 0.0f || CurrentSettings.CameraShakeMagnitude > 2.0f)
	{
		Errors.Add(TEXT("Camera shake magnitude must be between 0.0 and 2.0"));
	}

	// Validate Screen Blur
	if (CurrentSettings.ScreenBlurAmount < 0.0f || CurrentSettings.ScreenBlurAmount > 1.0f)
	{
		Errors.Add(TEXT("Screen blur amount must be between 0.0 and 1.0"));
	}

	if (Errors.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameplayWidget: Validation found %d errors"), Errors.Num());
	}

	return Errors;
}

// ===== CALLBACKS =====
// CHỈ cập nhật CurrentSettings, KHÔNG gọi subsystem

void UGameplayWidget::OnDifficultyChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	EE_DifficultyLevel OldDifficulty = CurrentSettings.DifficultyLevel;
	EE_DifficultyLevel NewDifficulty = static_cast<EE_DifficultyLevel>(NewIndex);

	CurrentSettings.DifficultyLevel = NewDifficulty;

	// Khi thay đổi difficulty, tự động điều chỉnh các multipliers
	if (SettingsSubsystem)
	{
		FS_DifficultyMultiplier Multipliers = SettingsSubsystem->GetDifficultyMultiplier(NewDifficulty);
		CurrentSettings.SanityDrainMultiplier = Multipliers.SanityDrainMultiplier;
		CurrentSettings.EntityDetectionRangeMultiplier = Multipliers.AIDetectionMultiplier;

		// Cập nhật lại UI để reflect các thay đổi tự động
		if (SanityDrainSelection)
		{
			int32 Index = MultiplierToIndex(CurrentSettings.SanityDrainMultiplier);
			SanityDrainSelection->SetCurrentSelection(Index);
		}

		if (EntityDetectionSelection)
		{
			int32 Index = MultiplierToIndex(CurrentSettings.EntityDetectionRangeMultiplier);
			EntityDetectionSelection->SetCurrentSelection(Index);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Difficulty changed to index %d (not saved yet)"), NewIndex);
}

void UGameplayWidget::OnSanityDrainChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	float Multiplier = IndexToMultiplier(NewIndex);
	CurrentSettings.SanityDrainMultiplier = Multiplier;

	UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Sanity drain changed to %.2f (not saved yet)"), Multiplier);
}

void UGameplayWidget::OnEntityDetectionChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	float Multiplier = IndexToMultiplier(NewIndex);
	CurrentSettings.EntityDetectionRangeMultiplier = Multiplier;

	UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Entity detection changed to %.2f (not saved yet)"), Multiplier);
}

void UGameplayWidget::OnPuzzleHintChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bPuzzleHintSystemEnabled = (NewIndex == 1);

	UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Puzzle hints %s (not saved yet)"),
		CurrentSettings.bPuzzleHintSystemEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UGameplayWidget::OnHintTimeChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	TArray<float> Times = { 10.0f, 30.0f, 60.0f, 120.0f, 180.0f, 300.0f };
	if (NewIndex >= 0 && NewIndex < Times.Num())
	{
		CurrentSettings.AutoRevealHintTime = Times[NewIndex];
		UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Hint time changed to %.0fs (not saved yet)"),
			Times[NewIndex]);
	}
}

void UGameplayWidget::OnSkipPuzzlesChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bAllowSkipPuzzles = (NewIndex == 1);

	UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Skip puzzles %s (not saved yet)"),
		CurrentSettings.bAllowSkipPuzzles ? TEXT("allowed") : TEXT("not allowed"));
}

void UGameplayWidget::OnObjectiveMarkersChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bShowObjectiveMarkers = (NewIndex == 1);

	UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Objective markers %s (not saved yet)"),
		CurrentSettings.bShowObjectiveMarkers ? TEXT("shown") : TEXT("hidden"));
}

void UGameplayWidget::OnInteractionIndicatorsChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bShowInteractionIndicators = (NewIndex == 1);

	UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Interaction indicators %s (not saved yet)"),
		CurrentSettings.bShowInteractionIndicators ? TEXT("shown") : TEXT("hidden"));
}

void UGameplayWidget::OnAutoPickupChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bAutoPickupItems = (NewIndex == 1);

	UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Auto pickup %s (not saved yet)"),
		CurrentSettings.bAutoPickupItems ? TEXT("enabled") : TEXT("disabled"));
}

void UGameplayWidget::OnCameraShakeChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	float Percentage = IndexToPercentage(NewIndex);
	float Magnitude = Percentage * 2.0f; // Convert 0-1 to 0-2 range
	CurrentSettings.CameraShakeMagnitude = Magnitude;

	UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Camera shake changed to %.2f (not saved yet)"), Magnitude);
}

void UGameplayWidget::OnScreenBlurChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	float Amount = IndexToPercentage(NewIndex);
	CurrentSettings.ScreenBlurAmount = Amount;

	UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Screen blur changed to %.2f (not saved yet)"), Amount);
}

void UGameplayWidget::OnResetButtonClicked()
{
	// Reset về default settings
	FS_GameplaySettings DefaultSettings;
	LoadSettings(DefaultSettings);

	UE_LOG(LogTemp, Log, TEXT("GameplayWidget: Reset to default settings (not saved yet)"));
}

// ===== HELPER FUNCTIONS =====

void UGameplayWidget::AddToggleOptions(USelectionWidget* Selection)
{
	if (Selection)
	{
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("Off")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("On")) });
	}
}

void UGameplayWidget::AddPercentageOptions(USelectionWidget* Selection)
{
	if (Selection)
	{
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("0%")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("25%")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("50%")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("75%")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("100%")) });
	}
}

void UGameplayWidget::AddMultiplierOptions(USelectionWidget* Selection)
{
	if (Selection)
	{
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("0.5x")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("0.75x")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("1.0x")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("1.5x")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("2.0x")) });
	}
}

float UGameplayWidget::IndexToMultiplier(int32 Index)
{
	TArray<float> Multipliers = { 0.5f, 0.75f, 1.0f, 1.5f, 2.0f };
	if (Index >= 0 && Index < Multipliers.Num())
	{
		return Multipliers[Index];
	}
	return 1.0f; // Default
}

int32 UGameplayWidget::MultiplierToIndex(float Value)
{
	TArray<float> Multipliers = { 0.5f, 0.75f, 1.0f, 1.5f, 2.0f };

	// Find closest match
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

float UGameplayWidget::IndexToPercentage(int32 Index)
{
	TArray<float> Percentages = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
	if (Index >= 0 && Index < Percentages.Num())
	{
		return Percentages[Index];
	}
	return 0.5f; // Default
}

int32 UGameplayWidget::PercentageToIndex(float Value)
{
	TArray<float> Percentages = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };

	// Find closest match
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