// Fill out your copyright notice in the Description page of Project Settings.

#include "WidgetManager.h"
#include "EscapeIT/UI/SanityWidget.h"
#include "EscapeIT/UI/Inventory/InventoryWidget.h"
#include "EscapeIT/UI/Inventory/QuickbarWidget.h"
#include "EscapeIT/UI/Settings/PauseMenuWidget.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AWidgetManager::AWidgetManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWidgetManager::BeginPlay()
{
	Super::BeginPlay();

	// Lấy PlayerController reference
	PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("WidgetManager: PlayerController not found!"));
		return;
	}

	// Tự động khởi tạo widgets khi game bắt đầu
	InitializeWidgets();
}

void AWidgetManager::InitializeWidgets()
{
	// Tạo Sanity Widget
	if (SanityWidgetClass && !SanityWidget)
	{
		SanityWidget = CreateWidget<USanityWidget>(GetWorld(), SanityWidgetClass);
		if (SanityWidget)
		{
			SanityWidget->AddToViewport(0); // Z-Order 0 (nền)
		}
	}

	// Tạo Pause Menu (không add vào viewport ngay)
	if (PauseMenuClass && !PauseMenu)
	{
		PauseMenu = CreateWidget<UPauseMenuWidget>(GetWorld(), PauseMenuClass);
		if (PauseMenu)
		{
			UE_LOG(LogTemp, Log, TEXT("Pause menu created"));
		}
	}

	// Tạo Main Menu (không add vào viewport ngay)
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
			CrossHair->AddToViewport();
		}
	}

	// Create quickbar widget
	if (QuickbarWidgetClass && !QuickbarWidget)
	{
		QuickbarWidget = CreateWidget<UQuickbarWidget>(GetWorld(), QuickbarWidgetClass);
		if (QuickbarWidget)
		{
			QuickbarWidget->AddToViewport(1); // Z-order 1
		}
	}

	// Create inventory widget (nhưng không hiện ngay)
	if (InventoryWidgetClass && !InventoryWidget)
	{
		InventoryWidget = CreateWidget<UInventoryWidget>(GetWorld(), InventoryWidgetClass);
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
	HideWidget(SanityWidget);
	HideWidget(PauseMenu);
	HideWidget(MainMenu);
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