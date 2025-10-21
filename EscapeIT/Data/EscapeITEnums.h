#pragma once

#include "CoreMinimal.h"
#include "EscapeITEnums.generated.h"

/**
 * Difficulty levels for the game
 */
UENUM(BlueprintType)
enum class EE_DifficultyLevel : uint8
{
	Easy		UMETA(DisplayName = "Easy"),
	Normal		UMETA(DisplayName = "Normal"),
	Hard		UMETA(DisplayName = "Hard"),
	Nightmare	UMETA(DisplayName = "Nightmare")
};

/**
 * Graphics quality presets
 */
UENUM(BlueprintType)
enum class EE_GraphicsQuality : uint8
{
	Low		UMETA(DisplayName = "Low"),
	Medium	UMETA(DisplayName = "Medium"),
	High	UMETA(DisplayName = "High"),
	Epic	UMETA(DisplayName = "Epic")
};

/**
 * Supported audio languages
 */
UENUM(BlueprintType)
enum class EE_AudioLanguage : uint8
{
	English		UMETA(DisplayName = "English"),
	Vietnamese	UMETA(DisplayName = "Vietnamese"),
	French		UMETA(DisplayName = "French"),
	German		UMETA(DisplayName = "German"),
	Spanish		UMETA(DisplayName = "Spanish"),
	Japanese	UMETA(DisplayName = "Japanese")
};

/**
 * Color blind modes for UI
 */
UENUM(BlueprintType)
enum class EE_ColorBlindMode : uint8
{
	Normal			UMETA(DisplayName = "Normal"),
	Deuteranopia	UMETA(DisplayName = "Red-Green (Deuteranopia)"),
	Protanopia		UMETA(DisplayName = "Red-Green (Protanopia)"),
	Tritanopia		UMETA(DisplayName = "Blue-Yellow (Tritanopia)")
};

/**
 * UI size presets
 */
UENUM(BlueprintType)
enum class EE_TextSize : uint8
{
	Small		UMETA(DisplayName = "Small"),
	Normal		UMETA(DisplayName = "Normal"),
	Large		UMETA(DisplayName = "Large"),
	ExtraLarge	UMETA(DisplayName = "Extra Large"),
	Huge		UMETA(DisplayName = "Huge")
};

/**
 * Text contrast levels
 */
UENUM(BlueprintType)
enum class EE_TextContrast : uint8
{
	Normal		UMETA(DisplayName = "Normal"),
	High		UMETA(DisplayName = "High"),
	Maximum		UMETA(DisplayName = "Maximum")
};

/**
 * Photosensitivity mode
 */
UENUM(BlueprintType)
enum class EE_PhotosensitivityMode : uint8
{
	Off			UMETA(DisplayName = "Off"),
	Reduced		UMETA(DisplayName = "Reduced"),
	Maximum		UMETA(DisplayName = "Maximum")
};

/**
 * Single-handed mode setting
 */
UENUM(BlueprintType)
enum class EE_SingleHandedMode : uint8
{
	Off		UMETA(DisplayName = "Off"),
	Left	UMETA(DisplayName = "Left Hand"),
	Right	UMETA(DisplayName = "Right Hand")
};

/**
 * Ray tracing quality
 */
UENUM(BlueprintType)
enum class EE_RayTracingQuality : uint8
{
	Off		UMETA(DisplayName = "Off"),
	Low		UMETA(DisplayName = "Low"),
	Medium	UMETA(DisplayName = "Medium"),
	High	UMETA(DisplayName = "High")
};

/**
 * Anti-aliasing methods
 */
UENUM(BlueprintType)
enum class EE_AntiAliasingMethod : uint8
{
	FXAA	UMETA(DisplayName = "FXAA"),
	TSR		UMETA(DisplayName = "Temporal Super Resolution"),
	DLSS	UMETA(DisplayName = "NVIDIA DLSS"),
	Native	UMETA(DisplayName = "Native")
};

/**
 * Shadow quality
 */
UENUM(BlueprintType)
enum class EE_ShadowQuality : uint8
{
	Low		UMETA(DisplayName = "Low"),
	Medium	UMETA(DisplayName = "Medium"),
	High	UMETA(DisplayName = "High"),
	Epic	UMETA(DisplayName = "Epic")
};

/**
 * Texture quality
 */
UENUM(BlueprintType)
enum class EE_TextureQuality : uint8
{
	Low		UMETA(DisplayName = "Low"),
	Medium	UMETA(DisplayName = "Medium"),
	High	UMETA(DisplayName = "High"),
	Epic	UMETA(DisplayName = "Epic")
};

/**
 * Audio output types
 */
UENUM(BlueprintType)
enum class EE_AudioOutput : uint8
{
	Stereo          UMETA(DisplayName = "Stereo"),
	Surround_5_1    UMETA(DisplayName = "Surround 5.1"),
	Surround_7_1    UMETA(DisplayName = "Surround 7.1"),
	Headphones      UMETA(DisplayName = "Headphones")
};