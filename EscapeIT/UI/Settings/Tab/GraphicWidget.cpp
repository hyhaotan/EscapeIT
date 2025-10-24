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
	, bIsLoadingSettings(false)
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

	// Load current settings from subsystem
	LoadCurrentSettings();

	// Start FPS counter if widget exists (with lower frequency to reduce overhead)
	if (FPSText)
	{
		GetWorld()->GetTimerManager().SetTimer(
			FPSTimerHandle,
			this,
			&UGraphicWidget::UpdateFPSCounter,
			1.0f,  // Update every 1 second instead of 0.5s
			true
		);
	}
}

void UGraphicWidget::NativeDestruct()
{
	// Clear FPS timer
	if (FPSTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(FPSTimerHandle);
		FPSTimerHandle.Invalidate();
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
		QualityPresetSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Ultra")) });
		QualityPresetSelection->AddOption(FSelectionOption{ FText::FromString(TEXT("Custom")) });
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

		// Disable if ray tracing is off
		RayTracingQualitySelection->SetIsEnabled(CurrentSettings.bRayTracingEnabled);
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

	// Get settings from subsystem and save to CurrentSettings
	CurrentSettings = SettingsSubsystem->GetAllSettings().GraphicsSettings;

	// Load settings to UI
	LoadSettings(CurrentSettings);

	UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Loaded current settings from subsystem"));
}

// ===== PUBLIC API =====

void UGraphicWidget::LoadSettings(const FS_GraphicsSettings& Settings)
{
	// Save to CurrentSettings
	CurrentSettings = Settings;

	// Load to UI (don't trigger callbacks)
	bIsLoadingSettings = true;

	// Quality Preset
	if (QualityPresetSelection)
	{
		int32 Index = static_cast<int32>(Settings.QualityPreset);
		QualityPresetSelection->SetCurrentSelection(Index);
	}

	// Resolution
	if (ResolutionSelection)
	{
		FIntPoint Resolution(Settings.ResolutionX, Settings.ResolutionY);
		int32 Index = ResolutionToIndex(Resolution);
		ResolutionSelection->SetCurrentSelection(Index);
	}

	// VSync
	if (VSyncSelection)
	{
		VSyncSelection->SetCurrentSelection(Settings.bVSyncEnabled ? 1 : 0);
	}

	// Frame Rate Cap
	if (FrameRateCapSelection)
	{
		int32 Index = FrameRateCapToIndex(Settings.FrameRateCap);
		FrameRateCapSelection->SetCurrentSelection(Index);
	}

	// Ray Tracing
	if (RayTracingSelection)
	{
		RayTracingSelection->SetCurrentSelection(Settings.bRayTracingEnabled ? 1 : 0);
	}

	// Ray Tracing Quality
	if (RayTracingQualitySelection)
	{
		int32 Index = static_cast<int32>(Settings.RayTracingQuality);
		RayTracingQualitySelection->SetCurrentSelection(Index);
		RayTracingQualitySelection->SetIsEnabled(Settings.bRayTracingEnabled);
	}

	// Shadow Quality
	if (ShadowQualitySelection)
	{
		int32 Index = static_cast<int32>(Settings.ShadowQuality);
		ShadowQualitySelection->SetCurrentSelection(Index);
	}

	// Texture Quality
	if (TextureQualitySelection)
	{
		int32 Index = static_cast<int32>(Settings.TextureQuality);
		TextureQualitySelection->SetCurrentSelection(Index);
	}

	// Anti-Aliasing
	if (AntiAliasingSelection)
	{
		int32 Index = static_cast<int32>(Settings.AntiAliasingMethod);
		AntiAliasingSelection->SetCurrentSelection(Index);
	}

	// Motion Blur (0-100%)
	if (MotionBlurSelection)
	{
		int32 Index = PercentageToIndex(Settings.MotionBlurAmount);
		MotionBlurSelection->SetCurrentSelection(Index);
	}

	// Field of View
	if (FieldOfViewSelection)
	{
		int32 Index = FOVToIndex(Settings.FieldOfView);
		FieldOfViewSelection->SetCurrentSelection(Index);
	}

	bIsLoadingSettings = false;

	UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Settings loaded into UI"));
}

FS_GraphicsSettings UGraphicWidget::GetCurrentSettings() const
{
	return CurrentSettings;
}

TArray<FString> UGraphicWidget::ValidateSettings() const
{
	TArray<FString> Errors;

	// Validate Resolution
	if (CurrentSettings.ResolutionX < 640 || CurrentSettings.ResolutionX > 7680)
	{
		Errors.Add(FString::Printf(
			TEXT("Invalid resolution width: %d (must be 640-7680)"),
			CurrentSettings.ResolutionX
		));
	}

	if (CurrentSettings.ResolutionY < 480 || CurrentSettings.ResolutionY > 4320)
	{
		Errors.Add(FString::Printf(
			TEXT("Invalid resolution height: %d (must be 480-4320)"),
			CurrentSettings.ResolutionY
		));
	}

	// Validate Frame Rate Cap
	if (CurrentSettings.FrameRateCap != 0 &&
		(CurrentSettings.FrameRateCap < 30 || CurrentSettings.FrameRateCap > 300))
	{
		Errors.Add(FString::Printf(
			TEXT("Invalid frame rate cap: %d (must be 0 or 30-300)"),
			CurrentSettings.FrameRateCap
		));
	}

	// Validate FOV
	if (CurrentSettings.FieldOfView < 60.0f || CurrentSettings.FieldOfView > 120.0f)
	{
		Errors.Add(FString::Printf(
			TEXT("Invalid field of view: %.1f (must be 60-120)"),
			CurrentSettings.FieldOfView
		));
	}

	// Validate Motion Blur
	if (CurrentSettings.MotionBlurAmount < 0.0f || CurrentSettings.MotionBlurAmount > 1.0f)
	{
		Errors.Add(FString::Printf(
			TEXT("Invalid motion blur amount: %.2f (must be 0-1)"),
			CurrentSettings.MotionBlurAmount
		));
	}

	// Check Ray Tracing compatibility
	if (CurrentSettings.bRayTracingEnabled)
	{
		// Check if hardware supports ray tracing
		// This would need platform-specific checks in production
		UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
		if (UserSettings)
		{
			// Add hardware capability check here
			// For now, just log a warning
			UE_LOG(LogTemp, Warning, TEXT("GraphicWidget: Ray tracing enabled - ensure hardware supports it"));
		}
	}

	// Warn about performance-heavy settings
	if (CurrentSettings.QualityPreset == EE_GraphicsQuality::Ultra &&
		CurrentSettings.ResolutionX >= 3840 &&
		CurrentSettings.bRayTracingEnabled)
	{
		Errors.Add(TEXT("Warning: Ultra quality with 4K resolution and ray tracing may cause performance issues"));
	}

	if (Errors.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GraphicWidget: Validation found %d issues"), Errors.Num());
	}

	return Errors;
}

// ===== CALLBACKS =====
// Only update CurrentSettings, DO NOT call subsystem

void UGraphicWidget::OnQualityPresetChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	EE_GraphicsQuality Quality = static_cast<EE_GraphicsQuality>(NewIndex);
	CurrentSettings.QualityPreset = Quality;

	// When changing quality preset, automatically adjust other settings
	// But only if not Custom
	if (Quality != EE_GraphicsQuality::Custom && SettingsSubsystem)
	{
		FS_GraphicsSettings PresetSettings = SettingsSubsystem->GetGraphicsPreset(Quality);

		// Copy preset values to current settings (except resolution and FPS cap)
		CurrentSettings.bRayTracingEnabled = PresetSettings.bRayTracingEnabled;
		CurrentSettings.RayTracingQuality = PresetSettings.RayTracingQuality;
		CurrentSettings.ShadowQuality = PresetSettings.ShadowQuality;
		CurrentSettings.TextureQuality = PresetSettings.TextureQuality;
		CurrentSettings.AntiAliasingMethod = PresetSettings.AntiAliasingMethod;
		CurrentSettings.MotionBlurAmount = PresetSettings.MotionBlurAmount;

		// Update UI to reflect automatic changes
		bIsLoadingSettings = true;

		if (RayTracingSelection)
		{
			RayTracingSelection->SetCurrentSelection(CurrentSettings.bRayTracingEnabled ? 1 : 0);
		}

		if (RayTracingQualitySelection)
		{
			RayTracingQualitySelection->SetCurrentSelection(static_cast<int32>(CurrentSettings.RayTracingQuality));
			RayTracingQualitySelection->SetIsEnabled(CurrentSettings.bRayTracingEnabled);
		}

		if (ShadowQualitySelection)
		{
			ShadowQualitySelection->SetCurrentSelection(static_cast<int32>(CurrentSettings.ShadowQuality));
		}

		if (TextureQualitySelection)
		{
			TextureQualitySelection->SetCurrentSelection(static_cast<int32>(CurrentSettings.TextureQuality));
		}

		if (AntiAliasingSelection)
		{
			AntiAliasingSelection->SetCurrentSelection(static_cast<int32>(CurrentSettings.AntiAliasingMethod));
		}

		if (MotionBlurSelection)
		{
			MotionBlurSelection->SetCurrentSelection(PercentageToIndex(CurrentSettings.MotionBlurAmount));
		}

		bIsLoadingSettings = false;
	}
	else if (Quality == EE_GraphicsQuality::Custom)
	{
		// When switching to custom, keep current values
		UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Switched to Custom preset - keeping current values"));
	}

	UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Quality preset changed to index %d (not saved yet)"), NewIndex);
}

void UGraphicWidget::OnResolutionChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	FIntPoint Resolution = IndexToResolution(NewIndex);
	CurrentSettings.ResolutionX = Resolution.X;
	CurrentSettings.ResolutionY = Resolution.Y;

	// Mark as custom if user manually changes
	if (CurrentSettings.QualityPreset != EE_GraphicsQuality::Custom)
	{
		CurrentSettings.QualityPreset = EE_GraphicsQuality::Custom;
		if (QualityPresetSelection)
		{
			bIsLoadingSettings = true;
			QualityPresetSelection->SetCurrentSelection(static_cast<int32>(EE_GraphicsQuality::Custom));
			bIsLoadingSettings = false;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Resolution changed to %dx%d (not saved yet)"),
		Resolution.X, Resolution.Y);
}

void UGraphicWidget::OnVSyncChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bVSyncEnabled = (NewIndex == 1);

	UE_LOG(LogTemp, Log, TEXT("GraphicWidget: VSync %s (not saved yet)"),
		CurrentSettings.bVSyncEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UGraphicWidget::OnFrameRateCapChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	int32 FrameRate = IndexToFrameRateCap(NewIndex);
	CurrentSettings.FrameRateCap = FrameRate;

	UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Frame rate cap changed to %d (not saved yet)"), FrameRate);
}

void UGraphicWidget::OnRayTracingChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	CurrentSettings.bRayTracingEnabled = (NewIndex == 1);

	// Enable/disable ray tracing quality selection
	if (RayTracingQualitySelection)
	{
		RayTracingQualitySelection->SetIsEnabled(CurrentSettings.bRayTracingEnabled);
	}

	// Mark as custom
	MarkAsCustomPreset();

	UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Ray tracing %s (not saved yet)"),
		CurrentSettings.bRayTracingEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UGraphicWidget::OnRayTracingQualityChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	EE_RayTracingQuality Quality = static_cast<EE_RayTracingQuality>(NewIndex);
	CurrentSettings.RayTracingQuality = Quality;

	MarkAsCustomPreset();

	UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Ray tracing quality changed to index %d (not saved yet)"), NewIndex);
}

void UGraphicWidget::OnShadowQualityChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	EE_ShadowQuality Quality = static_cast<EE_ShadowQuality>(NewIndex);
	CurrentSettings.ShadowQuality = Quality;

	MarkAsCustomPreset();

	UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Shadow quality changed to index %d (not saved yet)"), NewIndex);
}

void UGraphicWidget::OnTextureQualityChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	EE_TextureQuality Quality = static_cast<EE_TextureQuality>(NewIndex);
	CurrentSettings.TextureQuality = Quality;

	MarkAsCustomPreset();

	UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Texture quality changed to index %d (not saved yet)"), NewIndex);
}

void UGraphicWidget::OnAntiAliasingChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	EE_AntiAliasingMethod Method = static_cast<EE_AntiAliasingMethod>(NewIndex);
	CurrentSettings.AntiAliasingMethod = Method;

	MarkAsCustomPreset();

	UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Anti-aliasing changed to index %d (not saved yet)"), NewIndex);
}

void UGraphicWidget::OnMotionBlurChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	float Amount = IndexToPercentage(NewIndex);
	CurrentSettings.MotionBlurAmount = Amount;

	MarkAsCustomPreset();

	UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Motion blur changed to %.2f (not saved yet)"), Amount);
}

void UGraphicWidget::OnFieldOfViewChanged(int32 NewIndex)
{
	if (bIsLoadingSettings)
		return;

	float FOV = IndexToFOV(NewIndex);
	CurrentSettings.FieldOfView = FOV;

	UE_LOG(LogTemp, Log, TEXT("GraphicWidget: Field of view changed to %.0f° (not saved yet)"), FOV);
}

// ===== HELPER FUNCTIONS =====

void UGraphicWidget::MarkAsCustomPreset()
{
	if (CurrentSettings.QualityPreset != EE_GraphicsQuality::Custom)
	{
		CurrentSettings.QualityPreset = EE_GraphicsQuality::Custom;
		if (QualityPresetSelection)
		{
			bIsLoadingSettings = true;
			QualityPresetSelection->SetCurrentSelection(static_cast<int32>(EE_GraphicsQuality::Custom));
			bIsLoadingSettings = false;
		}
	}
}

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
	if (!FPSText || !GEngine)
		return;

	// Get actual frame time from engine
	float AverageFPS = 0.0f;

	//if (GEngine->GetAverageFPS() > 0.0f)
	//{
	//	AverageFPS = GEngine->GetAverageFPS();
	//}
	//else
	//{
	//	// Fallback calculation
	//	float DeltaTime = GetWorld()->GetDeltaSeconds();
	//	if (DeltaTime > 0.0f)
	//	{
	//		AverageFPS = 1.0f / DeltaTime;
	//	}
	//}

	// Format with color based on FPS
	FString FPSString;
	if (AverageFPS >= 60.0f)
	{
		FPSString = FString::Printf(TEXT("FPS: %.0f"), AverageFPS);
	}
	else if (AverageFPS >= 30.0f)
	{
		FPSString = FString::Printf(TEXT("FPS: %.0f (OK)"), AverageFPS);
	}
	else
	{
		FPSString = FString::Printf(TEXT("FPS: %.0f (Low)"), AverageFPS);
	}

	FPSText->SetText(FText::FromString(FPSString));
}

TArray<FIntPoint> UGraphicWidget::GetAvailableResolutions()
{
	TArray<FIntPoint> Resolutions;

	// Get resolutions from engine
	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	if (UserSettings)
	{
		// Try to get supported fullscreen resolutions
		TArray<FIntPoint> SupportedResolutions;
		//UserSettings->GetSupportedFullscreenResolutions(SupportedResolutions);

		if (SupportedResolutions.Num() > 0)
		{
			// Filter and sort unique resolutions
			for (const FIntPoint& Res : SupportedResolutions)
			{
				// Only add 16:9 and 16:10 aspect ratios
				float AspectRatio = static_cast<float>(Res.X) / static_cast<float>(Res.Y);
				if ((FMath::IsNearlyEqual(AspectRatio, 16.0f / 9.0f, 0.1f) ||
					FMath::IsNearlyEqual(AspectRatio, 16.0f / 10.0f, 0.1f)) &&
					!Resolutions.Contains(Res))
				{
					Resolutions.Add(Res);
				}
			}
		}
	}

	// If no resolutions found, add common ones
	if (Resolutions.Num() == 0)
	{
		Resolutions.Add(FIntPoint(1280, 720));   // HD
		Resolutions.Add(FIntPoint(1920, 1080));  // Full HD
		Resolutions.Add(FIntPoint(2560, 1440));  // QHD
		Resolutions.Add(FIntPoint(3840, 2160));  // 4K
	}

	// Sort by total pixels
	Resolutions.Sort([](const FIntPoint& A, const FIntPoint& B)
		{
			return (A.X * A.Y) < (B.X * B.Y);
		});

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

	// Find closest match if exact not found
	int32 ClosestIndex = 1; // Default to Full HD
	int32 ClosestDiff = INT_MAX;

	for (int32 i = 0; i < Resolutions.Num(); i++)
	{
		int32 Diff = FMath::Abs((Resolutions[i].X * Resolutions[i].Y) - (Resolution.X * Resolution.Y));
		if (Diff < ClosestDiff)
		{
			ClosestDiff = Diff;
			ClosestIndex = i;
		}
	}

	return ClosestIndex;
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

float UGraphicWidget::IndexToPercentage(int32 Index)
{
	TArray<float> Percentages = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
	if (Index >= 0 && Index < Percentages.Num())
	{
		return Percentages[Index];
	}
	return 0.5f; // Default
}

int32 UGraphicWidget::PercentageToIndex(float Value)
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