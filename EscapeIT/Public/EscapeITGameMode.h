// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EscapeITGameMode.generated.h"

class UStoryGameWidget;
class USoundBase;

UCLASS(minimalapi)
class AEscapeITGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEscapeITGameMode();

protected:
	virtual void BeginPlay() override;

	// ========================== FUNCTION ==========================
	// =============== Story =================
	void HideAllGameWidgets();
	void FadeInAndShowStory();
	void ShowStoryGameWidget();

	// ========================== WIDGET ============================
	UPROPERTY()
	UStoryGameWidget* StoryGameWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UStoryGameWidget> StoryGameWidgetClass;
	
	// ========================== PROPERTIES =========================
	// ================ AVAIABLE ==============
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Power")
	float PowerOffDuration;
	
	// ========================== SOUND ==============================
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Sound")
	TObjectPtr<USoundBase> PowerOffSound;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Sound")
	TObjectPtr<USoundBase> PowerOnSound;
	
private:
	void TriggerPowerEvent();
	
	FTimerHandle DelayPowerEvent;
};