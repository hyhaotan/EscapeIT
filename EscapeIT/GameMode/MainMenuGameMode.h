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

	// ============= Ambient Sound =============
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Audio|Ambient")
	TObjectPtr<USoundBase> MenuAmbientSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Audio|Ambient", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AmbientVolume = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Audio|Ambient")
	bool bEnableAmbient = true;

private:
	UPROPERTY()
	TObjectPtr<UAudioComponent> AmbientAudioComponent;

	void StartAmbientSound();
	void StopAmbientSound();

public:
	// ============= Public Control Functions =============
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetAmbientVolume(float Volume);
};