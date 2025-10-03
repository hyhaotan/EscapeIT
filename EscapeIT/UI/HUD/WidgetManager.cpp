// Fill out your copyright notice in the Description page of Project Settings.

#include "WidgetManager.h"
#include "EscapeIT/UI/SanityWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AWidgetManager::AWidgetManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWidgetManager::BeginPlay()
{
	Super::BeginPlay();

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
		PauseMenu = CreateWidget<UUserWidget>(GetWorld(), PauseMenuClass);
	}

	// Tạo Main Menu (không add vào viewport ngay)
	if (MainMenuClass && !MainMenu)
	{
		MainMenu = CreateWidget<UUserWidget>(GetWorld(), MainMenuClass);
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
			PauseMenu->AddToViewport(10); // Z-Order 10 (cao hơn)
		}
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		// Pause game và hiện con trỏ chuột
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			UGameplayStatics::SetGamePaused(GetWorld(), true);
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeUIOnly());
		}
	}
}

void AWidgetManager::HidePauseMenu()
{
	HideWidget(PauseMenu);

	// Resume game và ẩn con trỏ chuột
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
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
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeUIOnly());
	}
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
}

bool AWidgetManager::IsPauseMenuVisible() const
{
	return PauseMenu && PauseMenu->IsInViewport() && PauseMenu->GetVisibility() == ESlateVisibility::Visible;
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