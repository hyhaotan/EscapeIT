// Fill out your copyright notice in the Description page of Project Settings.

#include "GraphicWidget.h"
#include "EscapeIT/UI/Settings/Tab/Selection/SelectionWidget.h"
#include "EscapeIT/Subsystem/SettingsSubsystem.h"
#include "Components/Button.h"
#include "CommonTextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"

UGraphicWidget::UGraphicWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bSettingsChanged(false)
{
}

void UGraphicWidget::NativeConstruct()
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
		UE_LOG(LogTemp, Error, TEXT("GraphicWidget: Failed to get SettingsSubsystem"));
		return;
	}

	// Initialize all selections
	InitializeSelections();

	// Load current settings
	LoadCurrentSettings();

	// Bind buttons
	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddDynamic(this, &UGraphicWidget::OnApplyButtonClicked);
		ApplyButton->SetIsEnabled(false); // Initially disabled
	}

	if (ResetButton)
	{
		ResetButton->OnClicked.AddDynamic(this, &UGraphicWidget::OnResetButtonClicked);
	}

	// Start FPS counter if widget exists
	if (FPSText)
	{
		GetWorld()->GetTimerManager().SetTimer(FPSTimerHandle, this, &UGraphicWidget::UpdateFPSCounter, 0.5f, true);
	}

	bSettingsChanged = false;
}

void UGraphicWidget::NativeDestruct()
{
	// Clear FPS timer
	if (FPSTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(FPSTimerHandle);
	}

	// Unbind all delegates
	if (ApplyButton)
	{
		ApplyButton->OnClicked.RemoveDynamic(this, &UGraphicWidget::OnApplyButtonClicked);
	}

	if (ResetButton)
	{
		ResetButton->OnClicked.RemoveDynamic(this, &UGraphicWidget::OnResetButtonClicked);
	}

	Super::NativeDestruct();
}

void UGraphicWidget::InitializeSelections()
{
	// Quality Preset
	if (QualityPresetSelection)
	{
		QualityPresetSelection->Clear();
		QualityPresetSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Low")) });
		QualityPresetSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Medium")) });
		QualityPresetSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("High")) });
		QualityPresetSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Epic")) });
		QualityPresetSelection->OnSelectionChanged.BindDynamic(this, &UGraphicWidget::OnQualityPresetChanged);
	}

	// Resolution
	if (ResolutionSelection)
	{
		ResolutionSelection->Clear();
		TArray<FIntPoint> Resolutions = GetAvailableResolutions();
		for (const FIntPoint& Res : Resolutions)
		{
			FString ResText = FString::Printf(TEXT("%dx%d"), Res.X, Res.Y);
			ResolutionSelection->AddOption(FSelectionOption{ FText::FromString(ResText) });
		}
		ResolutionSelection->OnSelectionChanged.BindDynamic(this, &UGraphicWidget::OnResolutionChanged);
	}

	// VSync
	if (VSyncSelection)
	{
		VSyncSelection->Clear();
		AddToggleOptions(VSyncSelection);
		VSyncSelection->OnSelectionChanged.BindDynamic(this, &UGraphicWidget::OnVSyncChanged);
	}

	// Frame Rate Cap
	if (FrameRateCapSelection)
	{
		FrameRateCapSelection->Clear();
		FrameRateCapSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("30 FPS")) });
		FrameRateCapSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("60 FPS")) });
		FrameRateCapSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("90 FPS")) });
		FrameRateCapSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("120 FPS")) });
		FrameRateCapSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("144 FPS")) });
		FrameRateCapSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Unlimited")) });
		FrameRateCapSelection->OnSelectionChanged.BindDynamic(this, &UGraphicWidget::OnFrameRateCapChanged);
	}

	// Ray Tracing
	if (RayTracingSelection)
	{
		RayTracingSelection->Clear();
		AddToggleOptions(RayTracingSelection);
		RayTracingSelection->OnSelectionChanged.BindDynamic(this, &UGraphicWidget::OnRayTracingChanged);
	}

	// Ray Tracing Quality
	if (RayTracingQualitySelection)
	{
		RayTracingQualitySelection->Clear();
		RayTracingQualitySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Low")) });
		RayTracingQualitySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Medium")) });
		RayTracingQualitySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("High")) });
		RayTracingQualitySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Epic")) });
		RayTracingQualitySelection->OnSelectionChanged.BindDynamic(this, &UGraphicWidget::OnRayTracingQualityChanged);
	}

	// Shadow Quality
	if (ShadowQualitySelection)
	{
		ShadowQualitySelection->Clear();
		ShadowQualitySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Low")) });
		ShadowQualitySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Medium")) });
		ShadowQualitySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("High")) });
		ShadowQualitySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Epic")) });
		ShadowQualitySelection->OnSelectionChanged.BindDynamic(this, &UGraphicWidget::OnShadowQualityChanged);
	}

	// Texture Quality
	if (TextureQualitySelection)
	{
		TextureQualitySelection->Clear();
		TextureQualitySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Low")) });
		TextureQualitySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Medium")) });
		TextureQualitySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("High")) });
		TextureQualitySelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Epic")) });
		TextureQualitySelection->OnSelectionChanged.BindDynamic(this, &UGraphicWidget::OnTextureQualityChanged);
	}

	// Anti-Aliasing
	if (AntiAliasingSelection)
	{
		AntiAliasingSelection->Clear();
		AntiAliasingSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Off")) });
		AntiAliasingSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("FXAA")) });
		AntiAliasingSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("TAA")) });
		AntiAliasingSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("MSAA x2")) });
		AntiAliasingSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("MSAA x4")) });
		AntiAliasingSelection->OnSelectionChanged.BindDynamic(this, &UGraphicWidget::OnAntiAliasingChanged);
	}

	// Motion Blur
	if (MotionBlurSelection)
	{
		MotionBlurSelection->Clear();
		AddPercentageOptions(MotionBlurSelection);
		MotionBlurSelection->OnSelectionChanged.BindDynamic(this, &UGraphicWidget::OnMotionBlurChanged);
	}

	// Field of View (70-120)
	if (FieldOfViewSelection)
	{
		FieldOfViewSelection->Clear();
		FieldOfViewSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("70°")) });
		FieldOfViewSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("80°")) });
		FieldOfViewSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("90°")) });
		FieldOfViewSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("100°")) });
		FieldOfViewSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("110°")) });
		FieldOfViewSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("120°")) });
		FieldOfViewSelection->OnSelectionChanged.BindDynamic(this, &UGraphicWidget::OnFieldOfViewChanged);
	}
}

void UGraphicWidget::LoadCurrentSettings()
{
	if (!SettingsSubsystem)
		return;

	FS_GraphicsSettings CurrentSettings = SettingsSubsystem->GetAllSettings().GraphicsSettings;

	// Quality Preset
	if (QualityPresetSelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.QualityPreset);
		QualityPresetSelection->SetCurrentSelection(Index);
	}

	// Resolution
	if (ResolutionSelection)
	{
		FIntPoint Resolution(CurrentSettings.ResolutionX, CurrentSettings.ResolutionY);
		int32 Index = ResolutionToIndex(Resolution);
		ResolutionSelection->SetCurrentSelection(Index);
	}

	// VSync
	if (VSyncSelection)
	{
		VSyncSelection->SetCurrentSelection(CurrentSettings.bVSyncEnabled ? 1 : 0);
	}

	// Frame Rate Cap
	if (FrameRateCapSelection)
	{
		int32 Index = FrameRateCapToIndex(CurrentSettings.FrameRateCap);
		FrameRateCapSelection->SetCurrentSelection(Index);
	}

	// Ray Tracing
	if (RayTracingSelection)
	{
		RayTracingSelection->SetCurrentSelection(CurrentSettings.bRayTracingEnabled ? 1 : 0);
	}

	// Ray Tracing Quality
	if (RayTracingQualitySelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.RayTracingQuality);
		RayTracingQualitySelection->SetCurrentSelection(Index);
	}

	// Shadow Quality
	if (ShadowQualitySelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.ShadowQuality);
		ShadowQualitySelection->SetCurrentSelection(Index);
	}

	// Texture Quality
	if (TextureQualitySelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.TextureQuality);
		TextureQualitySelection->SetCurrentSelection(Index);
	}

	// Anti-Aliasing
	if (AntiAliasingSelection)
	{
		int32 Index = static_cast<int32>(CurrentSettings.AntiAliasingMethod);
		AntiAliasingSelection->SetCurrentSelection(Index);
	}

	// Motion Blur (0-100%)
	if (MotionBlurSelection)
	{
		int32 Index = FMath::RoundToInt(CurrentSettings.MotionBlurAmount * 4.0f); // 0-4 for 5 options
		MotionBlurSelection->SetCurrentSelection(Index);
	}

	// Field of View
	if (FieldOfViewSelection)
	{
		int32 Index = FOVToIndex(CurrentSettings.FieldOfView);
		FieldOfViewSelection->SetCurrentSelection(Index);
	}

	bSettingsChanged = false;
	UpdateApplyButtonState();
}

// ===== CALLBACKS =====

void UGraphicWidget::OnQualityPresetChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		EE_GraphicsQuality Quality = static_cast<EE_GraphicsQuality>(NewIndex);
		SettingsSubsystem->SetGraphicsQuality(Quality);

		// Quality preset changes other settings, reload UI
		LoadCurrentSettings();
	}
}

void UGraphicWidget::OnResolutionChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		FIntPoint Resolution = IndexToResolution(NewIndex);
		SettingsSubsystem->SetResolution(Resolution.X, Resolution.Y);
		MarkSettingsChanged();
	}
}

void UGraphicWidget::OnVSyncChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetVSyncEnabled(NewIndex == 1);
		MarkSettingsChanged();
	}
}

void UGraphicWidget::OnFrameRateCapChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		int32 FrameRate = IndexToFrameRateCap(NewIndex);
		SettingsSubsystem->SetFrameRateCap(FrameRate);
		MarkSettingsChanged();
	}
}

void UGraphicWidget::OnRayTracingChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SetRayTracingEnabled(NewIndex == 1);
		MarkSettingsChanged();
	}
}

void UGraphicWidget::OnRayTracingQualityChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		EE_RayTracingQuality Quality = static_cast<EE_RayTracingQuality>(NewIndex);
		SettingsSubsystem->SetRayTracingQuality(Quality);
		MarkSettingsChanged();
	}
}

void UGraphicWidget::OnShadowQualityChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		EE_ShadowQuality Quality = static_cast<EE_ShadowQuality>(NewIndex);
		SettingsSubsystem->SetShadowQuality(Quality);
		MarkSettingsChanged();
	}
}

void UGraphicWidget::OnTextureQualityChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		EE_TextureQuality Quality = static_cast<EE_TextureQuality>(NewIndex);
		SettingsSubsystem->SetTextureQuality(Quality);
		MarkSettingsChanged();
	}
}

void UGraphicWidget::OnAntiAliasingChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		EE_AntiAliasingMethod Method = static_cast<EE_AntiAliasingMethod>(NewIndex);
		SettingsSubsystem->SetAntiAliasingMethod(Method);
		MarkSettingsChanged();
	}
}

void UGraphicWidget::OnMotionBlurChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		float Amount = NewIndex / 4.0f; // 0-4 to 0.0-1.0
		SettingsSubsystem->SetMotionBlurAmount(Amount);
		MarkSettingsChanged();
	}
}

void UGraphicWidget::OnFieldOfViewChanged(int32 NewIndex)
{
	if (SettingsSubsystem)
	{
		float FOV = IndexToFOV(NewIndex);
		SettingsSubsystem->SetFieldOfView(FOV);
		MarkSettingsChanged();
	}
}

void UGraphicWidget::OnApplyButtonClicked()
{
	if (SettingsSubsystem)
	{
		// Settings are already applied, just need to save and mark as not changed
		SettingsSubsystem->SaveSettingsToSlot();
		bSettingsChanged = false;
		UpdateApplyButtonState();
	}
}

void UGraphicWidget::OnResetButtonClicked()
{
	if (SettingsSubsystem)
	{
		// Reset only graphics settings to default
		FS_GraphicsSettings DefaultSettings;
		SettingsSubsystem->ApplyGraphicsSettings(DefaultSettings);

		// Reload UI
		LoadCurrentSettings();
	}
}

// ===== HELPER FUNCTIONS =====

void UGraphicWidget::AddToggleOptions(USelectionWidget* Selection)
{
	if (Selection)
	{
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("Off")) });
		Selection->AddOption(FSelectionOption{ FText::FromString(TEXT("On")) });
	}
}

void UGraphicWidget::AddPercentageOptions(USelectionWidget* Selection)
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

void UGraphicWidget::UpdateFPSCounter()
{
	if (FPSText && GEngine)
	{
		float FPS = 1.0f / GEngine->GetMaxTickRate(0.0f);
		if (FPS > 0.0f)
		{
			FPS = 1.0f / FPS;
		}

		FString FPSString = FString::Printf(TEXT("FPS: %.0f"), FPS);
		FPSText->SetText(FText::FromString(FPSString));
	}
}

TArray<FIntPoint> UGraphicWidget::GetAvailableResolutions()
{
	TArray<FIntPoint> Resolutions;

	// Common resolutions
	Resolutions.Add(FIntPoint(1280, 720));   // HD
	Resolutions.Add(FIntPoint(1920, 1080));  // Full HD
	Resolutions.Add(FIntPoint(2560, 1440));  // QHD
	Resolutions.Add(FIntPoint(3840, 2160));  // 4K

	// Get available resolutions from engine if possible
	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	if (UserSettings)
	{
		FIntPoint ScreenResolution = UserSettings->GetScreenResolution();
		if (!Resolutions.Contains(ScreenResolution))
		{
			Resolutions.Add(ScreenResolution);
		}
	}

	return Resolutions;
}

FIntPoint UGraphicWidget::IndexToResolution(int32 Index)
{
	TArray<FIntPoint> Resolutions = GetAvailableResolutions();
	if (Index >= 0 && Index < Resolutions.Num())
	{
		return Resolutions[Index];
	}
	return FIntPoint(1920, 1080); // Default Full HD
}

int32 UGraphicWidget::ResolutionToIndex(FIntPoint Resolution)
{
	TArray<FIntPoint> Resolutions = GetAvailableResolutions();

	for (int32 i = 0; i < Resolutions.Num(); i++)
	{
		if (Resolutions[i] == Resolution)
		{
			return i;
		}
	}

	return 1; // Default to Full HD (index 1)
}

int32 UGraphicWidget::IndexToFrameRateCap(int32 Index)
{
	TArray<int32> FrameRates = { 30, 60, 90, 120, 144, 0 }; // 0 = Unlimited
	if (Index >= 0 && Index < FrameRates.Num())
	{
		return FrameRates[Index];
	}
	return 60; // Default
}

int32 UGraphicWidget::FrameRateCapToIndex(int32 Value)
{
	TArray<int32> FrameRates = { 30, 60, 90, 120, 144, 0 };

	for (int32 i = 0; i < FrameRates.Num(); i++)
	{
		if (FrameRates[i] == Value)
		{
			return i;
		}
	}

	return 1; // Default to 60 FPS
}

float UGraphicWidget::IndexToFOV(int32 Index)
{
	TArray<float> FOVs = { 70.0f, 80.0f, 90.0f, 100.0f, 110.0f, 120.0f };
	if (Index >= 0 && Index < FOVs.Num())
	{
		return FOVs[Index];
	}
	return 90.0f; // Default
}

int32 UGraphicWidget::FOVToIndex(float Value)
{
	TArray<float> FOVs = { 70.0f, 80.0f, 90.0f, 100.0f, 110.0f, 120.0f };

	// Find closest match
	int32 ClosestIndex = 2; // Default to 90°
	float ClosestDiff = FLT_MAX;

	for (int32 i = 0; i < FOVs.Num(); i++)
	{
		float Diff = FMath::Abs(Value - FOVs[i]);
		if (Diff < ClosestDiff)
		{
			ClosestDiff = Diff;
			ClosestIndex = i;
		}
	}

	return ClosestIndex;
}

void UGraphicWidget::MarkSettingsChanged()
{
	bSettingsChanged = true;
	UpdateApplyButtonState();
}

void UGraphicWidget::UpdateApplyButtonState()
{
	if (ApplyButton)
	{
		ApplyButton->SetIsEnabled(bSettingsChanged);
	}
}