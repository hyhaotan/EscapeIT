#include "GameplayWidget.h"
#include "EscapeIT/UI/Settings/Tab/Selection/SelectionWidget.h"
#include "EscapeIT/Subsystem/SettingsSubsystem.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

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

	// Load current settings
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

	FS_GameplaySettings CurrentSettings = SettingsSubsystem->GetAllSettings().GameplaySettings;

	// Difficulty Level
	if (DifficultySelection)
	{
		int32 DifficultyIndex = static_cast<int32>(CurrentSettings.DifficultyLevel);
		DifficultySelection->SetCurrentSelection(DifficultyIndex);
	}

	// Sanity Drain Multiplier
	if (SanityDrainSelection)
	{
		int32 Index = MultiplierToIndex(CurrentSettings.SanityDrainMultiplier);
		SanityDrainSelection->SetCurrentSelection(Index);
	}

	// Entity Detection Range
	if (EntityDetectionSelection)
	{
		int32 Index = MultiplierToIndex(CurrentSettings.EntityDetectionRangeMultiplier);
		EntityDetectionSelection->SetCurrentSelection(Index);
	}

	// Puzzle Hint System
	if (PuzzleHintSelection)
	{
		PuzzleHintSelection->SetCurrentSelection(CurrentSettings.bPuzzleHintSystemEnabled ? 1 : 0);
	}

	// Auto Reveal Hint Time
	if (HintTimeSelection)
	{
		int32 Index = 0;
		TArray<float> Times = { 10.0f, 30.0f, 60.0f, 120.0f, 180.0f, 300.0f };
		for (int32 i = 0; i < Times.Num(); i++)
		{
			if (FMath::IsNearlyEqual(CurrentSettings.AutoRevealHintTime, Times[i], 0.1f))
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
		SkipPuzzlesSelection->SetCurrentSelection(CurrentSettings.bAllowSkipPuzzles ? 1 : 0);
	}

	// Objective Markers
	if (ObjectiveMarkersSelection)
	{
		ObjectiveMarkersSelection->SetCurrentSelection(CurrentSettings.bShowObjectiveMarkers ? 1 : 0);
	}

	// Interaction Indicators
	if (InteractionIndicatorsSelection)
	{
		InteractionIndicatorsSelection->SetCurrentSelection(CurrentSettings.bShowInteractionIndicators ? 1 : 0);
	}

	// Auto Pickup
	if (AutoPickupSelection)
	{
		AutoPickupSelection->SetCurrentSelection(CurrentSettings.bAutoPickupItems ? 1 : 0);
	}

	// Camera Shake
	if (CameraShakeSelection)
	{
		int32 Index = PercentageToIndex(CurrentSettings.CameraShakeMagnitude / 2.0f); // 0-2 range to 0-1
		CameraShakeSelection->SetCurrentSelection(Index);
	}

	// Screen Blur
	if (ScreenBlurSelection)
	{
		int32 Index = PercentageToIndex(CurrentSettings.ScreenBlurAmount);
		ScreenBlurSelection->SetCurrentSelection(Index);
	}
}

// ===== CALLBACKS =====

void UGameplayWidget::OnDifficultyChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		EE_DifficultyLevel Difficulty = static_cast<EE_DifficultyLevel>(NewIndex);
		SettingsSubsystem->SetDifficultyLevel(Difficulty);

		// Reload settings as difficulty changes other values
		LoadCurrentSettings();
	}
}

void UGameplayWidget::OnSanityDrainChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		float Multiplier = IndexToMultiplier(NewIndex);
		SettingsSubsystem->SetSanityDrainMultiplier(Multiplier);
	}
}

void UGameplayWidget::OnEntityDetectionChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		float Multiplier = IndexToMultiplier(NewIndex);
		SettingsSubsystem->SetEntityDetectionRangeMultiplier(Multiplier);
	}
}

void UGameplayWidget::OnPuzzleHintChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetPuzzleHintSystemEnabled(NewIndex == 1);
	}
}

void UGameplayWidget::OnHintTimeChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		TArray<float> Times = { 10.0f, 30.0f, 60.0f, 120.0f, 180.0f, 300.0f };
		if (NewIndex >= 0 && NewIndex < Times.Num())
		{
			SettingsSubsystem->SetAutoRevealHintTime(Times[NewIndex]);
		}
	}
}

void UGameplayWidget::OnSkipPuzzlesChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetAllowSkipPuzzles(NewIndex == 1);
	}
}

void UGameplayWidget::OnObjectiveMarkersChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetShowObjectiveMarkers(NewIndex == 1);
	}
}

void UGameplayWidget::OnInteractionIndicatorsChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetShowInteractionIndicators(NewIndex == 1);
	}
}

void UGameplayWidget::OnAutoPickupChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetAutoPickupItems(NewIndex == 1);
	}
}

void UGameplayWidget::OnCameraShakeChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		float Percentage = IndexToPercentage(NewIndex);
		float Magnitude = Percentage * 2.0f; // Convert 0-1 to 0-2 range
		SettingsSubsystem->SetCameraShakeMagnitude(Magnitude);
	}
}

void UGameplayWidget::OnScreenBlurChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		float Amount = IndexToPercentage(NewIndex);
		SettingsSubsystem->SetScreenBlurAmount(Amount);
	}
}

void UGameplayWidget::OnResetButtonClicked()
{
	if (SettingsSubsystem)
	{
		// Reset only gameplay settings to default
		FS_GameplaySettings DefaultSettings;
		SettingsSubsystem->ApplyGameplaySettings(DefaultSettings);

		// Reload UI
		LoadCurrentSettings();
	}
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