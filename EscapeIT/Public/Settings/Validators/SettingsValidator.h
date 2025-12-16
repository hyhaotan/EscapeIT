#pragma once

#include "CoreMinimal.h"
#include "Data/SettingsTypes.h"

class ESCAPEIT_API FSettingsValidator
{
public:
    // Validate tất cả settings
    static bool ValidateAllSettings(const FS_AllSettings& Settings, FString& OutErrorMessage);

    // Validate từng category
    static bool ValidateGraphicsSettings(const FS_GraphicsSettings& Settings, FString& OutError);
    static bool ValidateAudioSettings(const FS_AudioSettings& Settings, FString& OutError);
    static bool ValidateGameplaySettings(const FS_GameplaySettings& Settings, FString& OutError);
    static bool ValidateControlSettings(const FS_ControlSettings& Settings, FString& OutError);
    static bool ValidateAccessibilitySettings(const FS_AccessibilitySettings& Settings, FString& OutError);

private:
    // Helper functions
    static bool ValidateRange(float Value, float Min, float Max, const FString& FieldName, FString& OutError);
    static bool ValidateRange(int32 Value, int32 Min, int32 Max, const FString& FieldName, FString& OutError);
};