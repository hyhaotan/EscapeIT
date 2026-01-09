#pragma once

#include "CoreMinimal.h"
#include "InteractionTypes.generated.h"

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	Hold UMETA(DisplayName = "Hold"),
	Press UMETA(DisplayName = "Press"),
	Both UMETA(DisplayName = "Both (Hold or Press)")
};