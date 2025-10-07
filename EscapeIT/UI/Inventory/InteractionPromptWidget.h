// InteractionPromptWidget.h - Widget hiển thị "Press E to..." prompt

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "InteractionPromptWidget.generated.h"

UCLASS()
class ESCAPEIT_API UInteractionPromptWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // ============================================
    // WIDGET BINDINGS
    // ============================================

    UPROPERTY(meta = (BindWidget))
    UBorder* PromptBackground;

    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* PromptContainer;

    UPROPERTY(meta = (BindWidget))
    UImage* KeyIcon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ActionText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TargetText;

    // ============================================
    // PROPERTIES
    // ============================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
    UTexture2D* KeyTexture_E;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
    UTexture2D* KeyTexture_F;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
    UTexture2D* KeyTexture_Mouse;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction|Style")
    FLinearColor NormalColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.8f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction|Style")
    FLinearColor HighlightColor = FLinearColor(0.8f, 0.6f, 0.2f, 0.9f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction|Animation")
    float PulseSpeed = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction|Animation")
    float PulseIntensity = 0.2f;

    // ============================================
    // FUNCTIONS
    // ============================================

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ShowPrompt(const FText& Action, const FText& Target, UTexture2D* KeyTexture = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void HidePrompt();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void UpdatePromptForActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction")
    bool IsPromptVisible() const;

protected:
    void UpdateAnimation(float DeltaTime);

private:
    bool bIsVisible = false;
    float AnimationTime = 0.0f;
    FText ActionTexts;
    FText TargetTexts;
};