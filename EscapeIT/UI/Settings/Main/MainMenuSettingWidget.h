// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuSettingWidget.generated.h"

class UWidgetSwitcher;
class UButton;
class UCommonTextBlock;
class UGameplayWidget;
class UGraphicWidget;
class UAudioWidget;
class UControlWidget;
class UAccessibilityWidget;
class USettingsSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackButtonEvent);

UCLASS()
class ESCAPEIT_API UMainMenuSettingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuSettingWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ===== WIDGET BINDINGS =====

	/** Widget Switcher to switch between different settings categories */
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* SettingsSwitcher;

	/** Category Buttons */
	UPROPERTY(meta = (BindWidget))
	UButton* GameplayButton;

	UPROPERTY(meta = (BindWidget))
	UButton* GraphicsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* AudioButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ControlsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* AccessibilityButton;

	/** Back/Close Button */
	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	/** Apply All Button (applies settings and saves) */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* ApplyAllButton;

	/** Reset All Button (resets all settings to default) */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* ResetAllButton;

	/** Category Title Text */
	UPROPERTY(meta = (BindWidgetOptional))
	UCommonTextBlock* CategoryTitleText;

	// ===== SETTINGS WIDGET REFERENCES =====

	/** Gameplay Settings Widget */
	UPROPERTY(meta = (BindWidget))
	UGameplayWidget* GameplayWidget;

	/** Graphics Settings Widget */
	UPROPERTY(meta = (BindWidget))
	UGraphicWidget* GraphicsWidget;

	/** Audio Settings Widget */
	UPROPERTY(meta = (BindWidget))
	UAudioWidget* AudioWidget;

	/** Controls Settings Widget */
	UPROPERTY(meta = (BindWidget))
	UControlWidget* ControlsWidget;

	/** Accessibility Settings Widget */
	UPROPERTY(meta = (BindWidget))
	UAccessibilityWidget* AccessibilityWidget;

	// ===== CALLBACKS =====

	UFUNCTION()
	void OnGameplayButtonClicked();

	UFUNCTION()
	void OnGraphicsButtonClicked();

	UFUNCTION()
	void OnAudioButtonClicked();

	UFUNCTION()
	void OnControlsButtonClicked();

	UFUNCTION()
	void OnAccessibilityButtonClicked();

	UFUNCTION()
	void OnBackButtonClicked();

	UFUNCTION()
	void OnApplyAllButtonClicked();

	UFUNCTION()
	void OnResetAllButtonClicked();

	// ===== EVENTS =====

	/** Event called when Back button is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnBackButtonEvent OnBackClicked;

private:
	/** Reference to Settings Subsystem */
	UPROPERTY()
	USettingsSubsystem* SettingsSubsystem;

	/** Current active category index */
	int32 CurrentCategoryIndex;

	/** Enum for settings categories */
	enum class ESettingsCategory : uint8
	{
		Gameplay = 0,
		Graphics = 1,
		Audio = 2,
		Controls = 3,
		Accessibility = 4
	};

	/** Switch to a specific settings category */
	void SwitchToCategory(ESettingsCategory Category);

	/** Update button states based on current category */
	void UpdateCategoryButtons(ESettingsCategory ActiveCategory);

	/** Update category title text */
	void UpdateCategoryTitle(ESettingsCategory Category);

	/** Get category name as text */
	FText GetCategoryName(ESettingsCategory Category) const;

	/** Bind all button events */
	void BindButtonEvents();

	/** Unbind all button events */
	void UnbindButtonEvents();

	/** Style button as active */
	void SetButtonActive(UButton* Button, bool bActive);
};