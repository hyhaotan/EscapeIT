// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EscapeITGameMode.generated.h"

class UStoryGameWidget;

UCLASS(minimalapi)
class AEscapeITGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEscapeITGameMode();

protected:
	virtual void BeginPlay() override;

	void HideAllGameWidgets();
	void FadeInAndShowStory();
	void ShowStoryGameWidget();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UStoryGameWidget> StoryGameWidgetClass;

	UPROPERTY()
	UStoryGameWidget* StoryGameWidget;
};