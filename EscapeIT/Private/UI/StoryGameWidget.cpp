// Fill out your copyright notice in the Description page of Project Settings.
#include "UI/StoryGameWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Kismet/GameplayStatics.h"
#include "EscapeITCameraManager.h"
#include "UI/HUD/WidgetManager.h"

void UStoryGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UStoryGameWidget::OnCloseButtonClick);
	}
}

void UStoryGameWidget::OnCloseButtonClick()
{
	// Disable button để tránh click nhiều lần
	if (CloseButton)
	{
		CloseButton->SetIsEnabled(false);
	}

	// Fade to black
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
		{
			CameraManager->StartCameraFade(0.0f, 1.0f, 1.5f, FLinearColor::Black, false, true);
		}
	}

	// Delay để fade xong rồi remove widget và hiện lại game widgets
	FTimerHandle FadeTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, [this, PC]()
		{
			// Remove StoryGameWidget
			RemoveFromParent();

			// Hiện lại các widgets game
			if (UWorld* World = GetWorld())
			{
				AWidgetManager* WidgetManager = Cast<AWidgetManager>(
					UGameplayStatics::GetActorOfClass(World, AWidgetManager::StaticClass()));

				if (WidgetManager)
				{
					WidgetManager->ShowAllWidgets();
				}
			}

			// Fade in từ black
			if (PC)
			{
				if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
				{
					CameraManager->StartCameraFade(1.0f, 0.0f, 2.0f, FLinearColor::Black, false, true);
				}
			}

		}, 1.5f, false);
}

void UStoryGameWidget::SetStoryText(FString& Text)
{
	if (StoryText)
	{
		StoryText->SetText(FText::FromString(Text));
	}
}