// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EscapeITGameMode.generated.h"

class UStoryGameWidget;
class USoundBase;
class UPowerSystemManager;
class AWidgetManager;

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
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Class")
	UPowerSystemManager* PowerSystemManager;
	
private:
	void TriggerPowerEvent();
	
	UPROPERTY()
	TObjectPtr<AWidgetManager> WidgetManager;
};