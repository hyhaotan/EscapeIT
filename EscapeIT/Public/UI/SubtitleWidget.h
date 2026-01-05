// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/DialogueData.h"
#include "SubtitleWidget.generated.h"

// Events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateSubtitleText, FText, Text);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateNameText, FText, Text);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSubtitleTextChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNameTextChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSubtitleSequenceComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubtitleLineComplete, int32, LineIndex);

class UTextBlock;
class UBorder;
class UWidgetAnimation;

UCLASS()
class ESCAPEIT_API USubtitleWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    // ============================================
    // DISPLAY FUNCTIONS
    // ============================================
    
    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void SetNameText(const FText& Text);
    
    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void SetSubtitleText(const FText& Text);
    
    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void ShowSubtitle();
    
    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void HideSubtitle();
    
    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void HideSubtitleTextOnly();
    
    // Display single subtitle
    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void DisplaySubtitle(const FText& Name, const FText& Subtitle, USoundBase* Voice, float Duration);
    
    // Display sequence of subtitles
    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void DisplaySubtitleSequence(const TArray<FSubtitleLine>& Subtitles);

    // ============================================
    // CONTROL FUNCTIONS
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void StopSubtitleSequence();
    
    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void ClearSubtitles();

    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void SkipCurrentSubtitle();

    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void PauseSubtitleSequence();

    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void ResumeSubtitleSequence();

    // ============================================
    // QUERY FUNCTIONS
    // ============================================

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Subtitle")
    bool IsPlayingSubtitle() const { return bIsDisplayingSubtitle; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Subtitle")
    int32 GetRemainingSubtitleCount();
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Subtitle")
    bool HasMoreSubtitlesInQueue() const;
    
    // ============================================
    // ANIMATION FUNCTIONS
    // ============================================
    
    UFUNCTION(BlueprintCallable, Category = "Subtitle|Animation")
    void ShowSubtitleAnimWidget() 
    { 
        if (ShowSubtitleAnim) 
            PlayAnimation(ShowSubtitleAnim); 
    }
    
    UFUNCTION(BlueprintCallable, Category = "Subtitle|Animation")
    void HideSubtitleAnimWidget() 
    { 
        if (HideSubtitleAnim) 
            PlayAnimation(HideSubtitleAnim); 
    }
    
    UFUNCTION(BlueprintCallable, Category = "Subtitle|Animation")
    void DisplaySubtitleTextAnimWidget() 
    { 
        if (DisplaySubtitleTextAnim) 
            PlayAnimation(DisplaySubtitleTextAnim); 
    }
    
    UFUNCTION(BlueprintCallable, Category = "Subtitle|Animation")
    void HideSubtitleTextAnimWidget() 
    { 
        if (HideSubtitleTextAnim) 
            PlayAnimation(HideSubtitleTextAnim); 
    }
    
    // ============================================
    // EVENTS
    // ============================================
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnUpdateSubtitleText OnUpdateSubtitleText;
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnUpdateNameText OnUpdateNameText;
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSubtitleTextChanged OnSubtitleTextChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnNameTextChanged OnNameTextChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSubtitleSequenceComplete OnSubtitleSequenceComplete;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSubtitleLineComplete OnSubtitleLineComplete;
    
protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    
private:
    // Timer handles
    FTimerHandle AutoHideTimerHandle;
    FTimerHandle DelayTimerHandle;
    
    // Queue and state
    TQueue<FSubtitleLine> SubtitleQueue;
    bool bIsDisplayingSubtitle = false;
    bool bIsPaused = false;
    FText CurrentName;
    int32 CurrentLineIndex = 0;
    FSubtitleLine CurrentSubtitle;
    
    // Widget bindings
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SubtitleText;
    
    UPROPERTY(meta = (BindWidget))
    UBorder* SubtitleBorder;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* NameText;
    
    // Animations
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* ShowSubtitleAnim;
    
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* HideSubtitleAnim;
    
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* DisplaySubtitleTextAnim;
    
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* HideSubtitleTextAnim;
    
    // Internal functions
    void OnAutoHideTimer();
    void ProcessNextSubtitle();
    void DisplaySubtitleInternal(const FSubtitleLine& Line);
};