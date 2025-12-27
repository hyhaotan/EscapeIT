// Copyright Epic Games, Inc. All Rights Reserved.
#include "EscapeITGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/WidgetManager.h"
#include "UI/StoryGameWidget.h"

AEscapeITGameMode::AEscapeITGameMode()
{
}

void AEscapeITGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Ẩn tất cả widgets từ WidgetManager trước
	HideAllGameWidgets();

	// Camera bắt đầu từ black, sau đó fade in và show story
	FadeInAndShowStory();
}

void AEscapeITGameMode::HideAllGameWidgets()
{
	if (TObjectPtr<UWorld> World = GetWorld())
	{
		TObjectPtr<AWidgetManager> WidgetManager = Cast<AWidgetManager>(
			UGameplayStatics::GetActorOfClass(World, AWidgetManager::StaticClass()));

		if (WidgetManager)
		{
			// Khởi tạo widgets nhưng ẩn đi
			WidgetManager->InitializeWidgets();
			WidgetManager->HideAllWidgets();
		}
	}
}

void AEscapeITGameMode::FadeInAndShowStory()
{
	if (TObjectPtr<APlayerController> PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (TObjectPtr<APlayerCameraManager> CameraManager = PC->PlayerCameraManager)
		{
			CameraManager->SetManualCameraFade(1.0f, FLinearColor::Black, false);
			
			FTimerHandle FadeTimer;
			GetWorldTimerManager().SetTimer(FadeTimer, [this, CameraManager]()
			{
				if (CameraManager)
				{
					CameraManager->StartCameraFade(1.0f, 0.0f, 1.5f, FLinearColor::Black, false, true);
				}

				// Hiện StoryGameWidget ngay khi bắt đầu fade in
				ShowStoryGameWidget();

			}, 0.1f, false);
		}
	}
}

void AEscapeITGameMode::ShowStoryGameWidget()
{
	if (!StoryGameWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowStoryGameWidget: StoryGameWidgetClass is null!"));
		return;
	}

	StoryGameWidget = CreateWidget<UStoryGameWidget>(GetWorld(), StoryGameWidgetClass);
	if (StoryGameWidget)
	{
		FString StoryStr = TEXT(
			"Today has no date. The clocks are an afterthought down here—frozen hands beneath dust and a faint smear where someone tried to rub a calendar clean. "
			"I woke on a tile floor with a headache like a drum and a key in my hand that did not belong to me. The corridor smelled of old paper and disinfectant gone rancid. "
			"Fluorescent lights hummed a tired tune; the sound threaded through the halls like a nervous laugh.\n\n"
			"There are names written in the margins of the building: Project Cycle, Consortium 11, and someone's hurried scrawl—remember Mina. I can feel gaps in my memory, "
			"like pages torn from a book I should have finished. Sometimes I see a child in the corner of my eye; sometimes I hear her calling my name in a voice I cannot place. "
			"The building answers with movement—the walls rearrange themselves, doors that were shut become ajar, shadows lengthen and fold into each other.\n\n"
			"This place was designed to hold memories, to bind them. Instead it trapped a promise. A promise is a small thing until it must be kept. I do not yet know whether "
			"I came here to keep it, to break it, or to be consumed by it.\n\n"
			"If you find these notes—if someone is reading what I cannot remember—learn this: truth is patient, but memory is jealous. It will show you what it wants, when it wants. "
			"The only way forward is to listen, collect what is scattered, and face what remembers you."
		);
		StoryGameWidget->SetStoryText(StoryStr);
		StoryGameWidget->AddToViewport(1000);
		
		if (TObjectPtr<APlayerController> PC = GetWorld()->GetFirstPlayerController())
		{
			PC->bShowMouseCursor = true;
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(StoryGameWidget->TakeWidget());
			PC->SetInputMode(InputMode);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowStoryGameWidget: Failed to create StoryGameWidget instance."));
	}
}
