// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WidgetManager.generated.h"

class UUserWidget;
class USanityWidget;
class USanityComponent;

UCLASS()
class ESCAPEIT_API AWidgetManager : public AHUD
{
	GENERATED_BODY()

public:
	AWidgetManager();

protected:
	virtual void BeginPlay() override;

public:
	// Widget Classes
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<USanityWidget> SanityWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> PauseMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> MainMenuClass;

	// Widget Instances
	UPROPERTY()
	USanityWidget* SanityWidget;

	UPROPERTY()
	UUserWidget* PauseMenu;

	UPROPERTY()
	UUserWidget* MainMenu;

	// Initialization
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void InitializeWidgets();

	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void InitializeSanityWidget(USanityComponent* SanityComponent);

	// Sanity Widget
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void ShowSanityWidget();

	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void HideSanityWidget();

	// Pause Menu
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void HidePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void TogglePauseMenu();

	// Main Menu
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void ShowMainMenu();

	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void HideMainMenu();

	// Utility
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void HideAllWidgets();

	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void RemoveAllWidgets();

	// Getters
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	USanityWidget* GetSanityWidget() const { return SanityWidget; }

	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	bool IsPauseMenuVisible() const;

private:
	// Helper function để tạo widget
	template<typename T>
	T* CreateWidgetInstance(TSubclassOf<UUserWidget> WidgetClass);

	// Helper function để show/hide widget
	void ShowWidget(UUserWidget* Widget);
	void HideWidget(UUserWidget* Widget);
};