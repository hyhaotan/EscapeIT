// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EscapeIT/Data/EscapeITSettingsStructs.h"
#include "SettingsSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPEIT_API USettingsSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere) FS_AllSettings AllSettings;
	UPROPERTY(VisibleAnywhere) FDateTime LastSavedTime;
};
