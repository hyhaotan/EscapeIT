
#pragma once

#include "CoreMinimal.h"
#include "EscapeIT/Data/SettingsTypes.h"

/**
 * Class xử lý việc apply graphics settings vào engine
 */
class ESCAPEIT_API FGraphicsSettingsHandler
{
public:
    // Apply settings to engine
    static void ApplyToEngine(const FS_GraphicsSettings& Settings, UWorld* World);

    // Auto-detect optimal settings
    static FS_GraphicsSettings AutoDetectSettings();

    // Get/Set individual settings
    static void SetResolution(int32 X, int32 Y);
    static void SetVSync(bool bEnabled);
    static void SetFrameRateLimit(int32 FrameRate);
    static void SetOverallQuality(EE_GraphicsQuality Quality);
    static void SetRayTracing(bool bEnabled, EE_RayTracingQuality Quality, UWorld* World);
    static void SetMotionBlur(float Amount, UWorld* World);
    static void SetFieldOfView(float FOV, UWorld* World);

    // Custom quality settings
    static void SetViewDistance(int32 Quality);
    static void SetShadowQuality(int32 Quality);
    static void SetTextureQuality(int32 Quality);
    static void SetAntiAliasing(int32 Quality);
    static void SetPostProcessing(int32 Quality);
    static void SetEffectsQuality(int32 Quality);
    static void SetFoliageQuality(int32 Quality);
    static void SetShadingQuality(int32 Quality);

private:
    static class UGameUserSettings* GetGameUserSettings();
    static void ExecuteConsoleCommand(UWorld* World, const FString& Command);
};
