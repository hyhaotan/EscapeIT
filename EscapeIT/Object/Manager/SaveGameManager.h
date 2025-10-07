#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGameManager.generated.h"

class USanityComponent;
class USanitySaveGame;

UCLASS()
class ESCAPEIT_API USaveGameManager : public UObject
{
    GENERATED_BODY()

public:
    // Slot names
    static const FString SLOT_QUICKSAVE;
    static const FString SLOT_AUTOSAVE;
    static const FString SLOT_CHECKPOINT;

    // === SAVE FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    static bool SaveGame(USanitySaveGame* SaveGameObject, const FString& SlotName, int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    static bool QuickSave(AActor* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    static bool AutoSave(AActor* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    static bool SaveToSlot(AActor* PlayerCharacter, const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    static bool CreateCheckpoint(AActor* PlayerCharacter);

    // === LOAD FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    static USanitySaveGame* LoadGame(const FString& SlotName, int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    static bool QuickLoad(AActor* PlayerCharacter);

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    static bool LoadFromSlot(AActor* PlayerCharacter, const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    static bool LoadCheckpoint(AActor* PlayerCharacter);

    // === UTILITY ===

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    static bool DoesSaveGameExist(const FString& SlotName, int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    static bool DeleteSaveGame(const FString& SlotName, int32 UserIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    static TArray<FString> GetAllSaveSlots();

    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    static USanitySaveGame* CreateSaveGameObject(AActor* PlayerCharacter);

private:
    static USanityComponent* GetSanityComponent(AActor* PlayerCharacter);
    static void ApplySaveDataToPlayer(AActor* PlayerCharacter, USanitySaveGame* SaveGame);
};
