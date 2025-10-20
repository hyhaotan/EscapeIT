
#include "InteractionPromptWidget.h"
#include "EscapeIT/Actor/ItemPickupActor.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Kismet/KismetMathLibrary.h"

void UInteractionPromptWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Start hidden
    HidePrompt();
}

void UInteractionPromptWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsVisible)
    {
        UpdateAnimation(InDeltaTime);
    }
}

void UInteractionPromptWidget::ShowPrompt(const FText& Action, const FText& Target, UTexture2D* KeyTexture)
{
    if (!PromptBackground)
    {
        return;
    }

    bIsVisible = true;
    PromptBackground->SetVisibility(ESlateVisibility::Visible);

    // Set action text (e.g., "Press")
    if (ActionText)
    {
        ActionText->SetText(Action);
    }

    // Set target text (e.g., "Pick Up Medkit")
    if (TargetText)
    {
        TargetText->SetText(Target);
    }

    // Set key icon
    if (KeyIcon)
    {
        TObjectPtr<UTexture2D> IconToUse;
        if (IconToUse)
        {
            IconToUse = KeyTexture;
            KeyIcon->SetBrushFromTexture(IconToUse);
        }
        else
        {
            IconToUse = KeyTexture_E;
        }
    }

    // Reset animation
    AnimationTime = 0.0f;
}

void UInteractionPromptWidget::HidePrompt()
{
    bIsVisible = false;

    if (PromptBackground)
    {
        PromptBackground->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UInteractionPromptWidget::UpdatePromptForActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        HidePrompt();
        return;
    }

    // Check if it's an item pickup
    AItemPickupActor* PickupActor = Cast<AItemPickupActor>(TargetActor);
    if (PickupActor)
    {
        FItemData ItemData;
        if (PickupActor->GetItemData(ItemData))
        {
            ActionTexts = FText::FromString(TEXT("Press"));
            TargetTexts = FText::Format(
                FText::FromString(TEXT("to Pick Up {0}")),
                ItemData.ItemName
            );

            ShowPrompt(ActionTexts, TargetTexts, KeyTexture_E);
            return;
        }
    }

    // Check for other interactable types
    if (TargetActor->ActorHasTag("Door"))
    {
        ActionTexts = FText::FromString(TEXT("Press"));
        TargetTexts = FText::FromString(TEXT("to Open Door"));
        ShowPrompt(ActionTexts, TargetTexts, KeyTexture_E);
        return;
    }

    if (TargetActor->ActorHasTag("Puzzle"))
    {
        ActionTexts = FText::FromString(TEXT("Press"));
        TargetTexts = FText::FromString(TEXT("to Examine"));
        ShowPrompt(ActionTexts, TargetTexts, KeyTexture_E);
        return;
    }

    if (TargetActor->ActorHasTag("Document"))
    {
        ActionTexts = FText::FromString(TEXT("Press"));
        TargetTexts = FText::FromString(TEXT("to Read"));
        ShowPrompt(ActionTexts, TargetTexts, KeyTexture_E);
        return;
    }
}

bool UInteractionPromptWidget::IsPromptVisible() const
{
    return bIsVisible;
}

void UInteractionPromptWidget::UpdateAnimation(float DeltaTime)
{
    AnimationTime += DeltaTime * PulseSpeed;

    // Pulsing effect
    float PulseValue = FMath::Sin(AnimationTime) * PulseIntensity;
    float Scale = 1.0f + PulseValue;

    if (PromptContainer)
    {
        PromptContainer->SetRenderScale(FVector2D(Scale, Scale));
    }

    // Color pulse
    if (PromptBackground)
    {
        FLinearColor CurrentColor = FMath::Lerp(NormalColor, HighlightColor, (PulseValue + 1.0f) * 0.5f);
        PromptBackground->SetBrushColor(CurrentColor);
    }

    // Fade in animation for text
    if (TargetText)
    {
        float Alpha = FMath::Clamp(AnimationTime * 3.0f, 0.0f, 1.0f);
        TargetText->SetRenderOpacity(Alpha);
    }
}