// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "EscapeITCameraManager.generated.h"

UCLASS()
class AEscapeITCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:
	/** Constructor */
	AEscapeITCameraManager();
	
	UFUNCTION()
	void ClearPostProcessEffects()
	{
		this->ClearCachedPPBlends();
	}
};
