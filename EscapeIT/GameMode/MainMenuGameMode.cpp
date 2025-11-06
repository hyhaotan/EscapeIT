// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuGameMode.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	// Set default pawn and controller classes
	DefaultPawnClass = nullptr; // No pawn needed in main menu
	PlayerControllerClass = APlayerController::StaticClass(); // Or your custom MainMenuPlayerController

	LastHorrorSoundTime = 0.0f;
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	TObjectPtr<APlayerController> PC = UGameplayStatics::GetPlayerController(this, 0);
	PC->bShowMouseCursor = true;

	InitializeAudio();

	// Schedule first random horror sound
	if (RandomHorrorSounds.Num() > 0)
	{
		ScheduleNextHorrorSound();
	}
}

void AMainMenuGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAllAudio();

	// Clear timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HorrorSoundTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void AMainMenuGameMode::InitializeAudio()
{
	StartBackgroundMusic();
	StartAmbientSound();
}

void AMainMenuGameMode::StartBackgroundMusic()
{
	if (!MenuMusicSound) return;

	// Create audio component for music
	MusicAudioComponent = UGameplayStatics::SpawnSound2D(
		this,
		MenuMusicSound,
		0.0f, // Start at 0 volume for fade in
		1.0f, // Pitch
		0.0f, // Start time
		nullptr,
		true, // Persist across level transitions
		false // Don't auto destroy
	);

	if (MusicAudioComponent)
	{
		MusicAudioComponent->bIsUISound = true;
		MusicAudioComponent->SetVolumeMultiplier(0.0f);
		MusicAudioComponent->Play();

		// Fade in
		MusicAudioComponent->FadeIn(MusicFadeInDuration, 1.0f);
	}
}

void AMainMenuGameMode::StartAmbientSound()
{
	if (!MenuAmbientSound) return;

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
	}
}

void AMainMenuGameMode::ScheduleNextHorrorSound()
{
	if (!GetWorld()) return;

	float RandomDelay = FMath::RandRange(MinTimeBetweenHorrorSounds, MaxTimeBetweenHorrorSounds);

	GetWorld()->GetTimerManager().SetTimer(
		HorrorSoundTimer,
		this,
		&AMainMenuGameMode::PlayRandomHorrorSound,
		RandomDelay,
		false
	);
}

void AMainMenuGameMode::PlayRandomHorrorSound()
{
	if (RandomHorrorSounds.Num() == 0) return;

	// Pick random sound
	int32 RandomIndex = FMath::RandRange(0, RandomHorrorSounds.Num() - 1);
	USoundBase* SelectedSound = RandomHorrorSounds[RandomIndex];

	if (SelectedSound)
	{
		UGameplayStatics::PlaySound2D(
			this,
			SelectedSound,
			0.3f, // Lower volume for subtle effect
			FMath::RandRange(0.9f, 1.1f) // Slight pitch variation
		);

		UE_LOG(LogTemp, Log, TEXT("Playing random horror sound: %s"), *SelectedSound->GetName());
	}

	// Schedule next sound
	ScheduleNextHorrorSound();
}

void AMainMenuGameMode::StopAllAudio()
{
	if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
	{
		MusicAudioComponent->FadeOut(MusicFadeOutDuration, 0.0f);
	}

	if (AmbientAudioComponent && AmbientAudioComponent->IsPlaying())
	{
		AmbientAudioComponent->Stop();
	}
}

void AMainMenuGameMode::SetMusicVolume(float Volume)
{
	if (MusicAudioComponent)
	{
		MusicAudioComponent->SetVolumeMultiplier(FMath::Clamp(Volume, 0.0f, 1.0f));
	}
}

void AMainMenuGameMode::SetAmbientVolume(float Volume)
{
	if (AmbientAudioComponent)
	{
		AmbientAudioComponent->SetVolumeMultiplier(FMath::Clamp(Volume, 0.0f, 1.0f));
	}
}

void AMainMenuGameMode::FadeOutMusic(float Duration)
{
	if (MusicAudioComponent && MusicAudioComponent->IsPlaying())
	{
		MusicAudioComponent->FadeOut(Duration, 0.0f);
	}
}

void AMainMenuGameMode::FadeInMusic(float Duration)
{
	if (MusicAudioComponent)
	{
		if (!MusicAudioComponent->IsPlaying())
		{
			MusicAudioComponent->Play();
		}
		MusicAudioComponent->FadeIn(Duration, 1.0f);
	}
}