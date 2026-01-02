
#include "UI/Inventory/InteractionPromptWidget.h"
#include "Actor/ItemPickupActor.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Kismet/KismetMathLibrary.h"

void UInteractionPromptWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (InteractKeyText)
    {
        InteractKeyText->SetText(FText::FromString(TEXT("E")));
    }

    if (ProgressOverlay)
    {
        UObject* ResourceObject = ProgressOverlay->GetBrush().GetResourceObject();
        
        if (UMaterialInterface* BaseMaterial = Cast<UMaterialInterface>(ResourceObject))
        {
            ProgressMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            
            if (ProgressMaterial)
            {
                ProgressOverlay->SetBrushFromMaterial(ProgressMaterial);
                
                // Initialize material parameters
                ProgressMaterial->SetScalarParameterValue(TEXT("Progress"), 0.0f);
                ProgressMaterial->SetScalarParameterValue(TEXT("Thickness"), RingThickness);
                ProgressMaterial->SetScalarParameterValue(TEXT("StartAngle"), 0.75f); 
                ProgressMaterial->SetVectorParameterValue(TEXT("ProgressColor"), ProgressColor);
                
                UE_LOG(LogTemp, Log, TEXT("Circular Progress Material initialized successfully"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create Dynamic Material Instance"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No material assigned to ProgressOverlay in UMG Widget"));
        }
        
        ProgressOverlay->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (InteractBorder)
    {
        InteractBorder->SetBrushColor(IdleColor);
    }
    
    SetVisibility(ESlateVisibility::Collapsed);
}

void UInteractionPromptWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsHolding)
    {
        float PulseValue = FMath::Sin(GetWorld()->GetTimeSeconds() * PulseSpeed) * PulseIntensity + 1.0f;
        SetRenderScale(FVector2D(PulseValue, PulseValue));
    }
}

void UInteractionPromptWidget::ShowPrompt()
{
    SetVisibility(ESlateVisibility::Visible);
    
    if (InteractBorder)
    {
        InteractBorder->SetBrushColor(IdleColor);
    }
    
    if (ProgressOverlay)
    {
        ProgressOverlay->SetVisibility(ESlateVisibility::Collapsed);
    }

    SetRenderScale(FVector2D(1.0f, 1.0f));
    
    PlayAnimation(DisplayInteractAnim);
}

void UInteractionPromptWidget::HidePrompt()
{
    PlayAnimation(HiddenInteractAnim);
    CancelHold();
    
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle,[this]()
    {
        SetVisibility(ESlateVisibility::Collapsed);
    },0.5f,false);
}

void UInteractionPromptWidget::StartHold()
{
    if (bIsHolding) return;
    
    bIsHolding = true;
    CurrentProgress = 0.0f;

    if (ProgressOverlay)
    {
        ProgressOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);

        if (ProgressMaterial)
        {
            ProgressMaterial->SetScalarParameterValue(TEXT("Progress"), 0.0f);
            ProgressMaterial->SetVectorParameterValue(TEXT("ProgressColor"), ProgressColor);
        }
    }
    
    UpdateVisuals(); 
}

void UInteractionPromptWidget::UpdateHoldProgress(float Progress)
{
    if (!bIsHolding) return;
    
    CurrentProgress = FMath::Clamp(Progress,0.0f,1.0f);

    if (ProgressMaterial)
    {
        ProgressMaterial->SetScalarParameterValue(TEXT("Progress"), CurrentProgress);

        if (CurrentProgress >= 0.9f)
        {
            FLinearColor TransitionColor = FMath::Lerp(
                ProgressColor,
                CompleteColor,
                (CurrentProgress - 0.9f) / 0.1f);
            
            ProgressMaterial->SetVectorParameterValue(TEXT("ProgressColor"), TransitionColor);
        }
    }
    
    UpdateVisuals();
}

void UInteractionPromptWidget::CancelHold()
{
    if (!bIsHolding) return;
    
    bIsHolding = false;
    CurrentProgress = 0.0f;

    if (ProgressOverlay)
    {
        ProgressOverlay->SetVisibility(ESlateVisibility::Collapsed);
        
        if (ProgressMaterial)
        {
            ProgressMaterial->SetScalarParameterValue(TEXT("Progress"), 0.0f);
        }
    }

    if (InteractBorder)
    {
        InteractBorder->SetBrushColor(IdleColor);
    }
    
    SetRenderScale(FVector2D(1.0f, 1.0f));
}

void UInteractionPromptWidget::CompleteHold()
{
    bIsHolding = false;

    if (ProgressMaterial)
    {
        ProgressMaterial->SetScalarParameterValue(TEXT("Progress"), 1.0f);
        ProgressMaterial->SetVectorParameterValue(TEXT("ProgressColor"), CompleteColor);
    }

    if (InteractBorder)
    {
        InteractBorder->SetBrushColor(CompleteColor);
    }
    
    SetRenderScale(FVector2D(1.1f,1.1f));
    
    FTimerHandle HideTimer;
    GetWorld()->GetTimerManager().SetTimer(
        HideTimer,
        [this]()
        {
            HidePrompt();
        },0.15f,false);
}

void UInteractionPromptWidget::UpdateVisuals()
{
    if (!InteractBorder && !bIsHolding) return;
    
    FLinearColor CurrentColor;
    
    if (CurrentProgress < 0.9f)
    {
        float NormalizedProgress = CurrentProgress / 0.9f;
        CurrentColor = FMath::Lerp(IdleColor, ProgressColor, NormalizedProgress);
    }
    else
    {
        float FinalProgress = (CurrentProgress - 0.9f) / 0.1f;
        CurrentColor = FMath::Lerp(ProgressColor, CompleteColor, FinalProgress);
    }
    InteractBorder->SetBrushColor(CurrentColor);
}
