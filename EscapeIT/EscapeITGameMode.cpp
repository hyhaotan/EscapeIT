// Copyright Epic Games, Inc. All Rights Reserved.

#include "EscapeITGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "EscapeIT/Actor/Components/SanityComponent.h"
#include "EscapeIT/UI/HUD/WidgetManager.h"

AEscapeITGameMode::AEscapeITGameMode()
{
}

void AEscapeITGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerCon = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerCon)
	{
		APlayerCameraManager* PlayerCam = PlayerCon->PlayerCameraManager;
		if (PlayerCam)
		{
			PlayerCam->SetManualCameraFade(1.0f, FLinearColor::Black, false);

			FTimerHandle FadeTimer;
			GetWorldTimerManager().SetTimer(FadeTimer, [PlayerCam]()
				{
					if (PlayerCam)
					{
						PlayerCam->StartCameraFade(1.0f, 0.0f, 3.0f, FLinearColor::Black, false, true);
					}
				}, 0.1f, false); 
		}
	}
}
