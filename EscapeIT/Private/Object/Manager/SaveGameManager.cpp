#include "Object/Manager/SaveGameManager.h"
#include "SaveGames/SanitySaveGame.h"
#include "Actor/Components/SanityComponent.h"
#include "Kismet/GameplayStatics.h"

const FString USaveGameManager::SLOT_QUICKSAVE = TEXT("QuickSave");
const FString USaveGameManager::SLOT_AUTOSAVE = TEXT("AutoSave");
const FString USaveGameManager::SLOT_CHECKPOINT = TEXT("Checkpoint");

bool USaveGameManager::SaveGame(USanitySaveGame* SaveGameObject, const FString& SlotName, int32 UserIndex)
{
    if (!SaveGameObject)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveGameObject is null"));
        return false;
    }

    SaveGameObject->SaveSlotName = SlotName;
    SaveGameObject->UserIndex = UserIndex;

    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, UserIndex);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Game saved successfully to slot: %s"), *SlotName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save game to slot: %s"), *SlotName);
    }

    return bSuccess;
}

bool USaveGameManager::QuickSave(AActor* PlayerCharacter)
{
    return SaveToSlot(PlayerCharacter, SLOT_QUICKSAVE);
}

bool USaveGameManager::AutoSave(AActor* PlayerCharacter)
{
    return SaveToSlot(PlayerCharacter, SLOT_AUTOSAVE);
}

bool USaveGameManager::SaveToSlot(AActor* PlayerCharacter, const FString& SlotName)
{
    if (!PlayerCharacter)
    {
        return false;
    }

    USanitySaveGame* SaveGameObject = CreateSaveGameObject(PlayerCharacter);
    if (!SaveGameObject)
    {
        return false;
    }

    return UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, 0);
}

bool USaveGameManager::CreateCheckpoint(AActor* PlayerCharacter)
{
    return SaveToSlot(PlayerCharacter, SLOT_CHECKPOINT);
}

USanitySaveGame* USaveGameManager::LoadGame(const FString& SlotName, int32 UserIndex)
{
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Save game does not exist: %s"), *SlotName);
        return nullptr;
    }

    USanitySaveGame* SaveGame = Cast<USanitySaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex)
    );

    if (SaveGame)
    {
        UE_LOG(LogTemp, Log, TEXT("Game loaded successfully from slot: %s"), *SlotName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load game from slot: %s"), *SlotName);
    }

    return SaveGame;
}

bool USaveGameManager::QuickLoad(AActor* PlayerCharacter)
{
    return LoadFromSlot(PlayerCharacter, SLOT_QUICKSAVE);
}

bool USaveGameManager::LoadFromSlot(AActor* PlayerCharacter, const FString& SlotName)
{
    if (!PlayerCharacter)
    {
        return false;
    }

    USanitySaveGame* SaveGame = LoadGame(SlotName, 0);
    if (!SaveGame)
    {
        return false;
    }

    ApplySaveDataToPlayer(PlayerCharacter, SaveGame);
    return true;
}

bool USaveGameManager::LoadCheckpoint(AActor* PlayerCharacter)
{
    return LoadFromSlot(PlayerCharacter, SLOT_CHECKPOINT);
}

bool USaveGameManager::DoesSaveGameExist(const FString& SlotName, int32 UserIndex)
{
    return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

bool USaveGameManager::DeleteSaveGame(const FString& SlotName, int32 UserIndex)
{
    return UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
}

TArray<FString> USaveGameManager::GetAllSaveSlots()
{
    TArray<FString> SaveSlots;
    SaveSlots.Add(SLOT_QUICKSAVE);
    SaveSlots.Add(SLOT_AUTOSAVE);
    SaveSlots.Add(SLOT_CHECKPOINT);

    // Có thể thêm manual save slots
    for (int32 i = 1; i <= 5; i++)
    {
        SaveSlots.Add(FString::Printf(TEXT("ManualSave_%d"), i));
    }

    return SaveSlots;
}

USanitySaveGame* USaveGameManager::CreateSaveGameObject(AActor* PlayerCharacter)
{
    if (!PlayerCharacter)
    {
        return nullptr;
    }

    USanitySaveGame* SaveGame = Cast<USanitySaveGame>(
        UGameplayStatics::CreateSaveGameObject(USanitySaveGame::StaticClass())
    );

    if (!SaveGame)
    {
        return nullptr;
    }

    // Capture Sanity data
    USanityComponent* SanityComp = GetSanityComponent(PlayerCharacter);
    if (SanityComp)
    {
        SaveGame->SanityData = SanityComp->CaptureSaveData();
    }

    // Capture player location
    SaveGame->PlayerLocation = PlayerCharacter->GetActorLocation();
    SaveGame->PlayerRotation = PlayerCharacter->GetActorRotation();

    // Capture game progress (cần implement)
    // SaveGame->CurrentRoomIndex = GetCurrentRoomIndex();

    return SaveGame;
}

USanityComponent* USaveGameManager::GetSanityComponent(AActor* PlayerCharacter)
{
    if (!PlayerCharacter)
    {
        return nullptr;
    }

    return PlayerCharacter->FindComponentByClass<USanityComponent>();
}

void USaveGameManager::ApplySaveDataToPlayer(AActor* PlayerCharacter, USanitySaveGame* SaveGame)
{
    if (!PlayerCharacter || !SaveGame)
    {
        return;
    }

    // Restore Sanity
    USanityComponent* SanityComp = GetSanityComponent(PlayerCharacter);
    if (SanityComp)
    {
        SanityComp->LoadFromSaveData(SaveGame->SanityData);
    }

    // Restore player location
    PlayerCharacter->SetActorLocation(SaveGame->PlayerLocation);
    PlayerCharacter->SetActorRotation(SaveGame->PlayerRotation);

    // Restore game progress
    // LoadRoom(SaveGame->CurrentRoomIndex);
}