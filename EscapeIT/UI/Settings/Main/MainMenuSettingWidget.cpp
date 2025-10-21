// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuSettingWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "CommonTextBlock.h"
#include "EscapeIT/UI/Settings/Tab/GameplayWidget.h"
#include "EscapeIT/UI/Settings/Tab/GraphicWidget.h"
#include "EscapeIT/UI/Settings/Tab/AudioWidget.h"
#include "EscapeIT/UI/Settings/Tab/ControlWidget.h"
#include "EscapeIT/UI/Settings/Tab/AccessibilityWidget.h"
#include "EscapeIT/Subsystem/SettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"

UMainMenuSettingWidget::UMainMenuSettingWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentCategoryIndex(0)
{
}

void UMainMenuSettingWidget::NativeConstruct()
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
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: Failed to get SettingsSubsystem"));
	}

	// Bind all button events
	BindButtonEvents();

	// Start with Gameplay category
	SwitchToCategory(ESettingsCategory::Gameplay);
}

void UMainMenuSettingWidget::NativeDestruct()
{
	// Unbind all button events
	UnbindButtonEvents();

	Super::NativeDestruct();
}

void UMainMenuSettingWidget::BindButtonEvents()
{
	if (GameplayButton)
	{
		GameplayButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnGameplayButtonClicked);
	}

	if (GraphicsButton)
	{
		GraphicsButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnGraphicsButtonClicked);
	}

	if (AudioButton)
	{
		AudioButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnAudioButtonClicked);
	}

	if (ControlsButton)
	{
		ControlsButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnControlsButtonClicked);
	}

	if (AccessibilityButton)
	{
		AccessibilityButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnAccessibilityButtonClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnBackButtonClicked);
	}

	if (ApplyAllButton)
	{
		ApplyAllButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnApplyAllButtonClicked);
	}

	if (ResetAllButton)
	{
		ResetAllButton->OnClicked.AddDynamic(this, &UMainMenuSettingWidget::OnResetAllButtonClicked);
	}
}

void UMainMenuSettingWidget::UnbindButtonEvents()
{
	if (GameplayButton)
	{
		GameplayButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnGameplayButtonClicked);
	}

	if (GraphicsButton)
	{
		GraphicsButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnGraphicsButtonClicked);
	}

	if (AudioButton)
	{
		AudioButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnAudioButtonClicked);
	}

	if (ControlsButton)
	{
		ControlsButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnControlsButtonClicked);
	}

	if (AccessibilityButton)
	{
		AccessibilityButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnAccessibilityButtonClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnBackButtonClicked);
	}

	if (ApplyAllButton)
	{
		ApplyAllButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnApplyAllButtonClicked);
	}

	if (ResetAllButton)
	{
		ResetAllButton->OnClicked.RemoveDynamic(this, &UMainMenuSettingWidget::OnResetAllButtonClicked);
	}
}

// ===== CATEGORY BUTTON CALLBACKS =====

void UMainMenuSettingWidget::OnGameplayButtonClicked()
{
	SwitchToCategory(ESettingsCategory::Gameplay);
}

void UMainMenuSettingWidget::OnGraphicsButtonClicked()
{
	SwitchToCategory(ESettingsCategory::Graphics);
}

void UMainMenuSettingWidget::OnAudioButtonClicked()
{
	SwitchToCategory(ESettingsCategory::Audio);
}

void UMainMenuSettingWidget::OnControlsButtonClicked()
{
	SwitchToCategory(ESettingsCategory::Controls);
}

void UMainMenuSettingWidget::OnAccessibilityButtonClicked()
{
	SwitchToCategory(ESettingsCategory::Accessibility);
}

// ===== MAIN BUTTON CALLBACKS =====

void UMainMenuSettingWidget::OnBackButtonClicked()
{
	// Save settings before closing
	if (SettingsSubsystem)
	{
		SettingsSubsystem->SaveSettingsToSlot();
	}

	// Broadcast event for parent widgets to handle
	OnBackClicked.Broadcast();

	// Remove widget from viewport
	RemoveFromParent();
}

void UMainMenuSettingWidget::OnApplyAllButtonClicked()
{
	if (SettingsSubsystem)
	{
		// Save all current settings
		SettingsSubsystem->SaveSettingsToSlot();

		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: All settings applied and saved"));
	}
}

void UMainMenuSettingWidget::OnResetAllButtonClicked()
{
	if (SettingsSubsystem)
	{
		// Reset all settings to default
		SettingsSubsystem->ResetSettingsToDefault();

		// Reload all widget UIs
		if (GameplayWidget)
		{
			// Force widgets to reload their settings
			// You might need to call a refresh function on each widget
		}

		UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: All settings reset to default"));
	}
}

// ===== CATEGORY SWITCHING =====

void UMainMenuSettingWidget::SwitchToCategory(ESettingsCategory Category)
{
	if (!SettingsSwitcher)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuSettingWidget: SettingsSwitcher is null"));
		return;
	}

	// Switch to the appropriate widget
	int32 CategoryIndex = static_cast<int32>(Category);
	SettingsSwitcher->SetActiveWidgetIndex(CategoryIndex);
	CurrentCategoryIndex = CategoryIndex;

	// Update button states
	UpdateCategoryButtons(Category);

	// Update title
	UpdateCategoryTitle(Category);

	UE_LOG(LogTemp, Log, TEXT("MainMenuSettingWidget: Switched to category %s"),
		*GetCategoryName(Category).ToString());
}

void UMainMenuSettingWidget::UpdateCategoryButtons(ESettingsCategory ActiveCategory)
{
	// Set all buttons to inactive first
	SetButtonActive(GameplayButton, false);
	SetButtonActive(GraphicsButton, false);
	SetButtonActive(AudioButton, false);
	SetButtonActive(ControlsButton, false);
	SetButtonActive(AccessibilityButton, false);

	// Set active button
	switch (ActiveCategory)
	{
	case ESettingsCategory::Gameplay:
		SetButtonActive(GameplayButton, true);
		break;
	case ESettingsCategory::Graphics:
		SetButtonActive(GraphicsButton, true);
		break;
	case ESettingsCategory::Audio:
		SetButtonActive(AudioButton, true);
		break;
	case ESettingsCategory::Controls:
		SetButtonActive(ControlsButton, true);
		break;
	case ESettingsCategory::Accessibility:
		SetButtonActive(AccessibilityButton, true);
		break;
	}
}

void UMainMenuSettingWidget::UpdateCategoryTitle(ESettingsCategory Category)
{
	if (CategoryTitleText)
	{
		CategoryTitleText->SetText(GetCategoryName(Category));
	}
}

FText UMainMenuSettingWidget::GetCategoryName(ESettingsCategory Category) const
{
	switch (Category)
	{
	case ESettingsCategory::Gameplay:
		return FText::FromString(TEXT("GAMEPLAY"));
	case ESettingsCategory::Graphics:
		return FText::FromString(TEXT("GRAPHICS"));
	case ESettingsCategory::Audio:
		return FText::FromString(TEXT("AUDIO"));
	case ESettingsCategory::Controls:
		return FText::FromString(TEXT("CONTROLS"));
	case ESettingsCategory::Accessibility:
		return FText::FromString(TEXT("ACCESSIBILITY"));
	default:
		return FText::FromString(TEXT("SETTINGS"));
	}
}

void UMainMenuSettingWidget::SetButtonActive(UButton* Button, bool bActive)
{
	if (!Button)
		return;

	// You can customize button appearance here
	// For example, change color tint or style

	if (bActive)
	{
		// Active button style
		// Button->SetColorAndOpacity(FLinearColor::Yellow);
		// Or use a different style in Blueprint
	}
	else
	{
		// Inactive button style
		// Button->SetColorAndOpacity(FLinearColor::White);
	}

	// Note: Button visual changes are better done in Blueprint using Style
	// This is just a placeholder for C++ logic
}