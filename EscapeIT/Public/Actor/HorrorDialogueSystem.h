// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/DialogueData.h"
#include "UI/HUD/WidgetManager.h"
#include "HorrorDialogueSystem.generated.h"

class AWidgetManager;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, FName, SequenceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueCompleted, FName, SequenceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIntensityChanged, EHorrorIntensity, OldIntensity, EHorrorIntensity, NewIntensity);

class USubtitleWidget;

UCLASS()
class ESCAPEIT_API AHorrorDialogueSystem : public AActor
{
    GENERATED_BODY()
    
public:    
    AHorrorDialogueSystem();

protected:
    virtual void BeginPlay() override;

public:    
    // ============================================
    // PUBLIC FUNCTIONS - Trigger Dialogues
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Horror Dialogue")
    void PlayDialogueSequence(FName SequenceID);

    UFUNCTION(BlueprintCallable, Category = "Horror Dialogue")
    void PlayQuickDialogue(
        const FText& Text,
        EHorrorDialogueType Type = EHorrorDialogueType::PlayerReaction,
        EHorrorIntensity Intensity = EHorrorIntensity::Calm
    );
    
    USubtitleWidget* GetReferenceSubtitleWidget() const {return WidgetManager->GetSubtitleWidget();}

    UFUNCTION(BlueprintCallable, Category = "Horror Dialogue")
    void StopCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Horror Dialogue")
    void PlayRandomReaction(EHorrorIntensity Intensity);

    UFUNCTION(BlueprintCallable, Category = "Horror Dialogue")
    void PlayInvestigationDialogue(const FText& ObjectName, const FText& Description);

    UFUNCTION(BlueprintCallable, Category = "Horror Dialogue")
    void PlayReadNoteDialogue(const FText& NoteTitle, const TArray<FText>& NoteContent);

    UFUNCTION(BlueprintCallable, Category = "Horror Dialogue")
    void PlayRadioMessage(const FText& Message, bool bIsDistorted = true);

    UFUNCTION(BlueprintCallable, Category = "Horror Dialogue")
    void PlayWhisperDialogue(const FText& WhisperText);

    // ============================================
    // QUERY FUNCTIONS
    // ============================================

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Horror Dialogue")
    bool HasSequenceBeenPlayed(FName SequenceID) const;

    UFUNCTION(BlueprintCallable, Category = "Horror Dialogue")
    void ResetSequence(FName SequenceID);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Horror Dialogue")
    bool IsDialoguePlaying() const;

    // ============================================
    // DATATABLE SUPPORT
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Horror Dialogue|Database")
    void LoadDialoguesFromDataTable(UDataTable* DataTable);

    // ============================================
    // EVENTS
    // ============================================

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueCompleted OnDialogueCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnIntensityChanged OnIntensityChanged;

    // ============================================
    // DATA - Dialogue Database
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Database")
    TMap<FName, FHorrorDialogueSequence> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Database")
    UDataTable* DialogueDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Reactions")
    TArray<FText> CalmReactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Reactions")
    TArray<FText> NervousReactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Reactions")
    TArray<FText> ScaredReactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Reactions")
    TArray<FText> TerrifiedReactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random Reactions")
    TArray<FText> PanicReactions;

    // ============================================
    // AUDIO - Horror Sound Effects
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Horror Effects")
    USoundBase* RadioStaticSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Horror Effects")
    USoundBase* HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Horror Effects")
    USoundBase* BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Horror Effects")
    USoundBase* WhisperAmbientSound;

    // ============================================
    // REFERENCES
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

private:
    UPROPERTY()
    AWidgetManager* WidgetManager;

    UPROPERTY()
    APlayerController* PlayerController;

    EHorrorIntensity CurrentIntensity;
    FName CurrentSequenceID;

    void PlayHorrorEffects(const FHorrorDialogueLine& Line);
    void OnDialogueSequenceFinished();
    void InitializeDefaultReactions();
    
    UFUNCTION()
    void HandleSubtitleComplete();
};