#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "EscapeIT/Data/SettingsTypes.h"
#include "EscapeIT/Data/SettingsStructs.h"
#include "SettingsSaveManager.generated.h"

UCLASS()
class ESCAPEIT_API USettingsSaveManager : public USaveGame
{
    GENERATED_BODY()
public:
    // Lưu toàn bộ settings (FS_AllSettings phải là USTRUCT với UPROPERTY trên các member)
    UPROPERTY(VisibleAnywhere, SaveGame)
    FS_AllSettings AllSettings;

    UPROPERTY(VisibleAnywhere, SaveGame)
    FS_HardwareInfo HardwareSnapshot;

    // Lưu thời điểm dưới dạng Unix timestamp (seconds) để an toàn khi serialize
    UPROPERTY(VisibleAnywhere, SaveGame)
    FDateTime LastSavedTime;

    UPROPERTY(VisibleAnywhere, SaveGame)
    FString ProfileName;

    UPROPERTY(VisibleAnywhere, SaveGame)
    int32 SettingsVersion = 1;
};
