// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

class USoundBase;
class UAudioComponent;

UCLASS()
class ESCAPEIT_API AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainMenuGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ============= Audio =============
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Music")
	TObjectPtr<USoundBase> MenuMusicSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Music")
	float MusicFadeInDuration = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Music")
	float MusicFadeOutDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Ambient")
	TObjectPtr<USoundBase> MenuAmbientSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Ambient")
	float AmbientVolume = 0.5f;

	// Horror ambient sounds
	UPROPERTY(EditDefaultsOnly, Category = "Audio|Horror")
	TArray<USoundBase*> RandomHorrorSounds;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Horror")
	float HorrorSoundChance = 0.01f; // Per second

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Horror")
	float MinTimeBetweenHorrorSounds = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Audio|Horror")
	float MaxTimeBetweenHorrorSounds = 120.0f;

private:
	UPROPERTY()
	TObjectPtr<UAudioComponent> MusicAudioComponent;

	UPROPERTY()
	TObjectPtr<UAudioComponent> AmbientAudioComponent;

	FTimerHandle HorrorSoundTimer;
	float LastHorrorSoundTime;

	void InitializeAudio();
	void StartBackgroundMusic();
	void StartAmbientSound();
	void ScheduleNextHorrorSound();
	void PlayRandomHorrorSound();
	void StopAllAudio();

public:
	// Public functions to control audio from widgets
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetAmbientVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void FadeOutMusic(float Duration = 1.5f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void FadeInMusic(float Duration = 2.0f);
};