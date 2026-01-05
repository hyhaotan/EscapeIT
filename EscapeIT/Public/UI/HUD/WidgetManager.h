// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WidgetManager.generated.h"

class UUserWidget;
class USanityWidget;
class USanityComponent;
class UQuickbarWidget;
class UPauseMenuWidget;
class UInventoryWidget;
class UFPSWidget;
class UInteractionPromptWidget;
class UNotificationWidget;
class UMainMenuSettingWidget;
class USubtitleWidget;

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
	TSubclassOf<UPauseMenuWidget> PauseMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> MainMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> CrossHairClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UQuickbarWidget> QuickbarWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UFPSWidget> FPSWidgetClass;

	UPROPERTY(EditAnywhere,Category = "Widgets")
	TSubclassOf<UUserWidget> StaminaWidgetClass;
	
	UPROPERTY(EditAnywhere,Category = "Widgets")
	TSubclassOf<UUserWidget> DeathWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UInteractionPromptWidget> InteractionPromptWidgetClass;
    
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UNotificationWidget> NotificationWidgetClass;
	
	UPROPERTY(EditDefaultsOnly,Category="Widgets")
	TSubclassOf<USubtitleWidget> SubtitleWidgetClass;

	// =============================== FUNCTION ==================================
	// Initialization
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void InitializeWidgets();

	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void InitializeSanityWidget(USanityComponent* SanityComponent);

	// Inventory Widget
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void Inventory();

	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void ShowInventoryScreen();

	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void HideInventoryScreen();

	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	bool IsInventoryOpen() const { return bIsInventoryOpen; }

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

	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void ShowAllWidgets();
	
	// ===================== SETTERS ======================
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	void SetInventoryWidget(UInventoryWidget* Widget) { InventoryWidget = Widget; }
	
	// ===================== GETTERS ======================
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	USanityWidget* GetSanityWidget() const { return SanityWidget; }
	
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	USubtitleWidget* GetSubtitleWidget() const { return SubtitleWidget; }
	
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	UNotificationWidget* GetNotificationWidget() const { return NotificationWidget; }
	
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	UFPSWidget* GetFPSWidget() const { return FPSWidget; }
	
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	UUserWidget* GetDeathWidget() const { return DeathWidget; }
	
	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	UInventoryWidget* GetInventoryWidget() const { return InventoryWidget; }

	UFUNCTION(BlueprintCallable, Category = "Widget Manager")
	bool IsPauseMenuVisible() const;
	
	void ShowWidget(UUserWidget* Widget);
	void HideWidget(UUserWidget* Widget);

private:
	// Widget Instances
	UPROPERTY()
	TObjectPtr<USanityWidget> SanityWidget;

	UPROPERTY()
	TObjectPtr<UPauseMenuWidget> PauseMenu;

	UPROPERTY()
	TObjectPtr<UUserWidget> MainMenu;

	UPROPERTY()
	TObjectPtr<UUserWidget> CrossHair;
	
	UPROPERTY()
	USubtitleWidget* SubtitleWidget;

	UPROPERTY()
	TObjectPtr<UQuickbarWidget> QuickbarWidget;

	UPROPERTY()
	TObjectPtr<UInventoryWidget> InventoryWidget;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> StaminaWidget;	
	
	UPROPERTY()
	TObjectPtr<UFPSWidget> FPSWidget;	
	
	UPROPERTY()
	TObjectPtr<UUserWidget> DeathWidget;
	
	UPROPERTY()
	TObjectPtr<UNotificationWidget> NotificationWidget;
	
	UPROPERTY()
	TObjectPtr<UInteractionPromptWidget> InteractionPromptWidget;
	
	// Helper function để tạo widget
	template<typename T>
	T* CreateWidgetInstance(TSubclassOf<UUserWidget> WidgetClass);

	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	// Helper function để show/hide widget

	bool bIsInventoryOpen = false;
};

