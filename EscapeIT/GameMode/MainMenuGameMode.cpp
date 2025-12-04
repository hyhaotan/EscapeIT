#include "MainMenuGameMode.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = APlayerController::StaticClass();
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Show mouse cursor for menu
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeUIOnly());
	}

	// Start ambient sound
	if (bEnableAmbient)
	{
		StartAmbientSound();
	}

	UE_LOG(LogTemp, Log, TEXT("MainMenuGameMode: Started"));
}

void AMainMenuGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAmbientSound();
	Super::EndPlay(EndPlayReason);
}

void AMainMenuGameMode::StartAmbientSound()
{
	if (!MenuAmbientSound)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuGameMode: No ambient sound assigned"));
		return;
	}

	AmbientAudioComponent = UGameplayStatics::SpawnSound2D(
		this,
		MenuAmbientSound,
		AmbientVolume,
		1.0f,
		0.0f,
		nullptr,
		true, 
		false 
	);

	if (AmbientAudioComponent)
	{
		AmbientAudioComponent->bIsUISound = true;
		AmbientAudioComponent->Play();

		UE_LOG(LogTemp, Log, TEXT("MainMenuGameMode: Ambient sound started"));
	}
}

void AMainMenuGameMode::StopAmbientSound()
{
	if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
	{
		AmbientAudioComponent->Stop();
		UE_LOG(LogTemp, Log, TEXT("MainMenuGameMode: Ambient sound stopped"));
	}
}

void AMainMenuGameMode::SetAmbientVolume(float Volume)
{
	AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

	if (AmbientAudioComponent)
	{
		AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume);
	}
}