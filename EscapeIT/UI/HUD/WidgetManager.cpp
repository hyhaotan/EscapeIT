// Fill out your copyright notice in the Description page of Project Settings.

#include "WidgetManager.h"
#include "EscapeIT/UI/SanityWidget.h"
#include "EscapeIT/UI/Inventory/InventoryWidget.h"
#include "EscapeIT/UI/Inventory/QuickbarWidget.h"
#include "EscapeIT/UI/Settings/PauseMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "EscapeIT/UI/FPSWidget.h"
#include "Kismet/GameplayStatics.h"
#include "EscapeIT/Actor/Components/InventoryComponent.h"
#include "EscapeIT/Actor/Components/FlashlightComponent.h"
#include "EscapeIT/UI/NotificationWidget.h"
#include "EscapeIT/UI/Inventory/InteractionPromptWidget.h"

AWidgetManager::AWidgetManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWidgetManager::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("WidgetManager: PlayerController not found!"));
		return;
	}

	InitializeWidgets();
}

void AWidgetManager::InitializeWidgets()
{
	TObjectPtr<APlayerController> PC = UGameplayStatics::GetPlayerController(this, 0);
	PC->bShowMouseCursor = false;

	FInputModeGameOnly InputMode;
	PC->SetInputMode(InputMode);

	if (SanityWidgetClass && !SanityWidget)
	{
		SanityWidget = CreateWidget<USanityWidget>(GetWorld(), SanityWidgetClass);
		if (SanityWidget)
		{
			SanityWidget->AddToViewport(0);
		}
	}
	
	if (PauseMenuClass && !PauseMenu)
	{
		PauseMenu = CreateWidget<UPauseMenuWidget>(GetWorld(), PauseMenuClass);
		if (PauseMenu)
		{
			UE_LOG(LogTemp, Log, TEXT("Pause menu created"));
		}
	}
	
	if (MainMenuClass && !MainMenu)
	{
		MainMenu = CreateWidget<UUserWidget>(GetWorld(), MainMenuClass);
		if (MainMenu)
		{
			UE_LOG(LogTemp, Log, TEXT("Main menu created"));
		}
	}

	if (CrossHairClass && !CrossHair)
	{
		CrossHair = CreateWidget<UUserWidget>(GetWorld(), CrossHairClass);
		if (CrossHair)
		{
			CrossHair->AddToViewport(0);
		}
	}
	
	if (QuickbarWidgetClass && !QuickbarWidget)
	{
		QuickbarWidget = CreateWidget<UQuickbarWidget>(GetWorld(), QuickbarWidgetClass);
		if (QuickbarWidget)
		{
			QuickbarWidget->AddToViewport(0);
			
			GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
			{
				if (APlayerController* PC = GetOwningPlayerController())
				{
					if (APawn* PlayerPawn = PC->GetPawn())
					{
						UInventoryComponent* InvComp = PlayerPawn->FindComponentByClass<UInventoryComponent>();
						UFlashlightComponent* FlashComp = PlayerPawn->FindComponentByClass<UFlashlightComponent>();
						
						if (InvComp)
						{
							QuickbarWidget->InitQuickBar(InvComp, FlashComp);
							UE_LOG(LogTemp, Log, TEXT("✅ QuickbarWidget initialized successfully!"));
						}
					}
				}
			});
		}
	}
	
	if (InventoryWidgetClass && !InventoryWidget)
	{
		InventoryWidget = CreateWidget<UInventoryWidget>(GetWorld(), InventoryWidgetClass);
		
		if (InventoryWidget)
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
			{
				if (APlayerController* PC = GetOwningPlayerController())
				{
					if (APawn* PlayerPawn = PC->GetPawn())
					{
						UInventoryComponent* InvComp = PlayerPawn->FindComponentByClass<UInventoryComponent>();
						
						if (InvComp)
						{
							InventoryWidget->InitInventory(InvComp);
							UE_LOG(LogTemp, Log, TEXT("✅ InventoryWidget initialized successfully!"));
						}
					}
				}
			});
		}
	}
	
	if (StaminaWidgetClass && !StaminaWidget)
	{
		StaminaWidget = CreateWidget<UUserWidget>(GetWorld(), StaminaWidgetClass);
		if (StaminaWidget)
		{
			StaminaWidget->AddToViewport(0);
		}
	}
	
	if (DeathWidgetClass && !DeathWidget)
	{
		DeathWidget = CreateWidget<UUserWidget>(GetWorld(), DeathWidgetClass);
	}

	if (FPSWidgetClass && !FPSWidget)
	{
		FPSWidget = CreateWidget<UFPSWidget>(GetWorld(),FPSWidgetClass);
		if (FPSWidget)
		{
			FPSWidget->AddToViewport(0);
		}
	}

	if (NotificationWidgetClass && !NotificationWidget)
	{
		NotificationWidget = CreateWidget<UNotificationWidget>(GetWorld(), NotificationWidgetClass);
	}
	
	if (InteractionPromptWidgetClass && !InteractionPromptWidget)
	{
		InteractionPromptWidget = CreateWidget<UInteractionPromptWidget>(GetWorld(), InteractionPromptWidgetClass);
	}
}

void AWidgetManager::InitializeSanityWidget(USanityComponent* SanityComponent)
{
	if (SanityWidget && SanityComponent)
	{
		SanityWidget->InitializeSanityWidget(SanityComponent);
	}
}

void AWidgetManager::ShowSanityWidget()
{
	ShowWidget(SanityWidget);
}

void AWidgetManager::HideSanityWidget()
{
	HideWidget(SanityWidget);
}

void AWidgetManager::ShowPauseMenu()
{
	if (PauseMenu)
	{
		if (!PauseMenu->IsInViewport())
		{
			PauseMenu->AddToViewport(10);
			PauseMenu->WidgetManager = this;
		}
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		if (PlayerController && IsValid(PlayerController))
		{
			PlayerController->bShowMouseCursor = true;
			PlayerController->SetInputMode(FInputModeUIOnly());
		}
	}
}

void AWidgetManager::HidePauseMenu()
{
	if (PauseMenu)
	{
		HideWidget(PauseMenu);
	}

	// Resume game và ẩn con trỏ chuột
	if (PlayerController && IsValid(PlayerController))
	{
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}

void AWidgetManager::TogglePauseMenu()
{
	if (IsPauseMenuVisible())
	{
		HidePauseMenu();
	}
	else
	{
		ShowPauseMenu();
	}
}

void AWidgetManager::ShowMainMenu()
{
	ShowWidget(MainMenu);

	// Hiện con trỏ chuột cho main menu
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = true;
		PlayerController->SetInputMode(FInputModeUIOnly());
	}
}

bool AWidgetManager::IsPauseMenuVisible() const
{
	// Add null check before calling IsInViewport()
	if (!PauseMenu || !IsValid(PauseMenu))
	{
		return false;
	}

	return PauseMenu->IsInViewport() && PauseMenu->GetVisibility() == ESlateVisibility::Visible;
}

void AWidgetManager::HideMainMenu()
{
	HideWidget(MainMenu);
}

void AWidgetManager::HideAllWidgets()
{
	HideWidget(PauseMenu);
	HideWidget(MainMenu);
	HideWidget(DeathWidget);
	HideInventoryScreen();
}

void AWidgetManager::RemoveAllWidgets()
{
	if (SanityWidget && SanityWidget->IsInViewport())
	{
		SanityWidget->RemoveFromParent();
	}

	if (PauseMenu && PauseMenu->IsInViewport())
	{
		PauseMenu->RemoveFromParent();
	}

	if (MainMenu && MainMenu->IsInViewport())
	{
		MainMenu->RemoveFromParent();
	}

	if (QuickbarWidget && QuickbarWidget->IsInViewport())
	{
		QuickbarWidget->RemoveFromParent();
	}

	if (InventoryWidget && InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromParent();
	}

	if (StaminaWidget && StaminaWidget->IsInViewport())
	{
		StaminaWidget->RemoveFromParent();
	}
}

void AWidgetManager::ShowWidget(UUserWidget* Widget)
{
	if (Widget)
	{
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport();
		}
		Widget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AWidgetManager::HideWidget(UUserWidget* Widget)
{
	if (Widget && Widget->IsInViewport())
	{
		Widget->SetVisibility(ESlateVisibility::Hidden);
		// Hoặc dùng: Widget->RemoveFromParent();
	}
}

// ============================================
// INVENTORY SCREEN
// ============================================
void AWidgetManager::Inventory()
{
	if (bIsInventoryOpen)
	{
		HideInventoryScreen();
	}
	else
	{
		ShowInventoryScreen();
	}
}

void AWidgetManager::ShowInventoryScreen()
{
	if (!InventoryWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowInventory: InventoryWidget is null"));
		return;
	}

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("ShowInventory: PlayerController is null"));
		return;
	}

	if (!bIsInventoryOpen)
	{
		InventoryWidget->AddToViewport(10); // Z-order cao hơn quickbar
		bIsInventoryOpen = true;

		// Pause game và hiện chuột
		UGameplayStatics::SetGamePaused(GetWorld(), true);
		PlayerController->bShowMouseCursor = true;
		PlayerController->SetInputMode(FInputModeGameAndUI());

		UE_LOG(LogTemp, Log, TEXT("Inventory opened"));
	}
}

void AWidgetManager::HideInventoryScreen()
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("HideInventory: PlayerController is null"));
		return;
	}

	if (InventoryWidget && bIsInventoryOpen)
	{
		InventoryWidget->RemoveFromParent();
		bIsInventoryOpen = false;

		// Unpause và ẩn chuột
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());

		UE_LOG(LogTemp, Log, TEXT("Inventory closed"));
	}
}

void AWidgetManager::ShowAllWidgets()
{
	// Show lại các widgets cơ bản
	if (SanityWidget && !SanityWidget->IsInViewport())
	{
		SanityWidget->AddToViewport(0);
	}

	if (QuickbarWidget && !QuickbarWidget->IsInViewport())
	{
		QuickbarWidget->AddToViewport(1);
	}

	if (CrossHair && !CrossHair->IsInViewport())
	{
		CrossHair->AddToViewport();
	}

	// Set input mode về game mode
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}