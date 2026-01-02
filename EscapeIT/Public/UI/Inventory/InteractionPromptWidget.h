// InteractionPromptWidget.h - Complete Header với Circular Progress Support

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPromptWidget.generated.h"

class UTextBlock;
class UBorder;
class UImage;
class UWidgetAnimation;
class UMaterialInstanceDynamic;
class USoundBase;

UCLASS()
class ESCAPEIT_API UInteractionPromptWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ============================================
    // CORE FUNCTIONS
    // ============================================
    
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ============================================
    // WIDGET BINDINGS
    // ============================================
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> InteractKeyText;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> InteractBorder;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ProgressOverlay;
    
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    TObjectPtr<UWidgetAnimation> DisplayInteractAnim;
    
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    TObjectPtr<UWidgetAnimation> HiddenInteractAnim;

    // ============================================
    // INTERACTION PROPERTIES
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float HoldDuration = 1.5f;

    // ============================================
    // COLOR PROPERTIES
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Colors")
    FLinearColor IdleColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.8f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Colors")
    FLinearColor ProgressColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); 
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Colors")
    FLinearColor CompleteColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); 

    // ============================================
    // VISUAL PROPERTIES
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Visual", meta = (ClampMin = "0.05", ClampMax = "0.4"))
    float RingThickness = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Visual")
    float PulseSpeed = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Visual")
    float PulseIntensity = 0.05f;

    // ============================================
    // AUDIO (OPTIONAL)
    // ============================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Audio")
    TObjectPtr<USoundBase> HoldStartSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Audio")
    TObjectPtr<USoundBase> HoldCompleteSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction | Audio")
    TObjectPtr<USoundBase> HoldCancelSound;

    // ============================================
    // PUBLIC FUNCTIONS
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ShowPrompt();
    
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void HidePrompt();
    
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StartHold();
    
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void UpdateHoldProgress(float Progress);
    
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void CancelHold();
    
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void CompleteHold();
    
    UFUNCTION(BlueprintPure, Category = "Interaction")
    bool IsHolding() const { return bIsHolding; }
    
    UFUNCTION(BlueprintPure, Category = "Interaction")
    float GetCurrentProgress() const { return CurrentProgress; }

private:
    // ============================================
    // INTERNAL STATE
    // ============================================
    
    bool bIsHolding = false;
    
    float CurrentProgress = 0.0f;
    
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> ProgressMaterial;
    
    // ============================================
    // INTERNAL FUNCTIONS
    // ============================================
    
    void UpdateVisuals();
};