// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ItemData.h"
#include "DocumentComponent.generated.h"

// Forward declarations
class UTexture2D;
class USoundBase;
class UDocumentWidget;
class UAudioComponent;

// Delegate để thông báo khi document được đọc
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDocumentRead, FName, DocumentID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDocumentContentChanged, const FText&, NewContent, UTexture2D*, NewImage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDocumentClosed);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ESCAPEIT_API UDocumentComponent : public UActorComponent
{
    GENERATED_BODY()

public: 
    UDocumentComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public: 
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // DOCUMENT MANAGEMENT
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Document")
    bool DisplayDocument(FName ItemID, UDataTable* DataTable);

    UFUNCTION(BlueprintCallable, Category = "Document")
    bool DisplayDocumentWithWidget(FName ItemID, UDataTable* DataTable, TSubclassOf<UDocumentWidget> WidgetClass);

    UFUNCTION(BlueprintCallable, Category = "Document")
    void CloseDocument();

    UFUNCTION(BlueprintPure, Category = "Document")
    UDocumentWidget* GetCurrentDocumentWidget() const;

    UFUNCTION(BlueprintPure, Category = "Document")
    bool IsDocumentOpen() const;

    UFUNCTION(BlueprintPure, Category = "Document")
    FItemData GetCurrentDocumentData() const { return CurrentDocumentData; }

    UFUNCTION(BlueprintPure, Category = "Document")
    FName GetCurrentDocumentID() const;

    UFUNCTION(BlueprintCallable, Category = "Document")
    void MarkDocumentAsRead(FName DocumentID);

    UFUNCTION(BlueprintPure, Category = "Document")
    bool HasReadDocument(FName DocumentID) const;

    UFUNCTION(BlueprintPure, Category = "Document")
    TArray<FName> GetReadDocuments() const { return ReadDocuments; }

    UFUNCTION(BlueprintPure, Category = "Document")
    int32 GetReadDocumentCount() const { return ReadDocuments.Num(); }

    // ========================================================================
    // AUDIO/VISUAL EFFECTS
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Document|Audio")
    void PlayOpenSound();

    UFUNCTION(BlueprintCallable, Category = "Document|Audio")
    void PlayCloseSound();

    UFUNCTION(BlueprintCallable, Category = "Document|Effects")
    void PlayDiscoveryEffect();

    // ========================================================================
    // HORROR GAME SPECIFIC FEATURES
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Document|Horror")
    void ApplySanityLossOnRead(float SanityLoss);

    UFUNCTION(BlueprintCallable, Category = "Document|Horror")
    void TriggerHorrorEvent(FName EventID);

    UFUNCTION(BlueprintPure, Category = "Document|Horror")
    bool ShouldTriggerHorrorEvent() const { return bTriggerHorrorOnRead; }

    // ========================================================================
    // READING STATS
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Document|Stats")
    float GetCurrentReadingTime() const;

    // ========================================================================
    // DELEGATES
    // ========================================================================

    UPROPERTY(BlueprintAssignable, Category = "Document|Events")
    FOnDocumentRead OnDocumentRead;

    UPROPERTY(BlueprintAssignable, Category = "Document|Events")
    FOnDocumentContentChanged OnDocumentContentChanged;

    UPROPERTY(BlueprintAssignable, Category = "Document|Events")
    FOnDocumentClosed OnDocumentClosed;

protected:
    // ========================================================================
    // UI WIDGET
    // ========================================================================

    UPROPERTY(EditDefaultsOnly, Category = "Document|UI")
    TSubclassOf<UDocumentWidget> DefaultDocumentWidgetClass;

    UPROPERTY(BlueprintReadOnly, Category = "Document|UI")
    TObjectPtr<UDocumentWidget> CurrentDocumentWidget;

    // ========================================================================
    // DOCUMENT STATE
    // ========================================================================

    UPROPERTY(BlueprintReadOnly, Category = "Document")
    bool bIsDocumentOpen;

    UPROPERTY(BlueprintReadOnly, Category = "Document")
    FItemData CurrentDocumentData;

    UPROPERTY(BlueprintReadOnly, Category = "Document")
    FName CurrentDocumentID;

    UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Document")
    TArray<FName> ReadDocuments;

    // ========================================================================
    // AUDIO ASSETS
    // ========================================================================

    UPROPERTY(EditDefaultsOnly, Category = "Document|Audio")
    TObjectPtr<USoundBase> DocumentOpenSound;

    UPROPERTY(EditDefaultsOnly, Category = "Document|Audio")
    TObjectPtr<USoundBase> DocumentCloseSound;

    UPROPERTY(EditDefaultsOnly, Category = "Document|Audio")
    TObjectPtr<USoundBase> DocumentDiscoverySound;

    UPROPERTY(EditDefaultsOnly, Category = "Document|Audio")
    TObjectPtr<USoundBase> ReadingAmbientSound;

    UPROPERTY()
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    // ========================================================================
    // HORROR FEATURES
    // ========================================================================

    UPROPERTY(EditAnywhere, Category = "Document|Horror")
    bool bTriggerHorrorOnRead;

    UPROPERTY(EditAnywhere, Category = "Document|Horror", meta = (EditCondition = "bTriggerHorrorOnRead"))
    FName HorrorEventID;

    UPROPERTY(EditAnywhere, Category = "Document|Horror", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float SanityLossOnRead;

    UPROPERTY(EditAnywhere, Category = "Document|Horror", meta = (EditCondition = "bTriggerHorrorOnRead"))
    float HorrorEventDelay;

    // ========================================================================
    // READING TRACKING
    // ========================================================================

    UPROPERTY(BlueprintReadOnly, Category = "Document|Stats")
    float CurrentReadingTime;

    UPROPERTY()
    bool bIsTrackingReadTime;

    // ========================================================================
    // HELPER FUNCTIONS (Protected)
    // ========================================================================
    
    bool LoadDocumentData(FName ItemID, UDataTable* DataTable);
    void ResetDocumentState();
    void StartReadingTimeTracking();
    void StopReadingTimeTracking();
    void CleanupResources();
    
    // Audio helpers
    void PlayAmbientSound();
    void StopAmbientSound();
    
    // Horror event helpers
    void ScheduleHorrorEvent();
    void ClearHorrorEventTimer();
    
    UFUNCTION()
    void ExecuteDelayedHorrorEvent();
    
    // Input mode management
    void SetInputModeUI();
    void SetInputModeGame();

    // ========================================================================
    // TIMERS
    // ========================================================================

    FTimerHandle HorrorEventTimerHandle;

private:
    // ========================================================================
    // CACHED REFERENCES
    // ========================================================================

    UPROPERTY()
    TObjectPtr<APlayerController> CachedPlayerController;
};