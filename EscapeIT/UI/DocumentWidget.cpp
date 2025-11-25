// Fill out your copyright notice in the Description page of Project Settings.

#include "DocumentWidget.h"
#include "EscapeIT/Actor/Components/DocumentComponent.h"
#include "EscapeIT/Actor/DocumentActor.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Animation/WidgetAnimation.h"

void UDocumentWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (CloseButton)
    {
        CloseButton->OnClicked.AddDynamic(this, &UDocumentWidget::OnCloseButtonClicked);
    }

    // Set focus AFTER a small delay to ensure widget is fully constructed
    FTimerHandle FocusTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(FocusTimerHandle, [this]()
    {
        if (IsValid(this))
        {
            SetKeyboardFocus();
            SetFocus();
        }
    }, 0.1f, false);
}

void UDocumentWidget::NativeDestruct()
{
    // Clear all timer handles before destruction
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }
    
    Super::NativeDestruct();
    
    DocumentComponentRef = nullptr;
    DocumentActorRef = nullptr;
}

// ============================================================================
// SETUP & UPDATE
// ============================================================================

void UDocumentWidget::SetupDocument(const FItemData& DocumentData)
{
    CurrentDocumentData = DocumentData;

    if (TitleText)
    {
        TitleText->SetText(DocumentData.ItemName);
    }

    if (DocumentTypeText)
    {
        DocumentTypeText->SetText(GetDocumentTypeDisplayName(DocumentData.DocumentType));
    }

    if (ContentText)
    {
        FText FormattedContent = FormatDocumentContent(DocumentData);
        ContentText->SetText(FormattedContent);
    }

    // Handle image visibility
    if (DocumentImage)
    {
        if (DocumentData.DocumentImage)
        {
            DocumentImage->SetBrushFromTexture(DocumentData.DocumentImage);
            DocumentImage->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            DocumentImage->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    ApplyDocumentStyling(DocumentData.DocumentType);
}

void UDocumentWidget::UpdateContent(const FText& NewContent)
{
    if (ContentText)
    {
        ContentText->SetText(NewContent);
    }
}

void UDocumentWidget::UpdateImage(UTexture2D* NewImage)
{
    if (!DocumentImage) return;
    
    if (NewImage)
    {
        DocumentImage->SetBrushFromTexture(NewImage);
        DocumentImage->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        DocumentImage->SetVisibility(ESlateVisibility::Collapsed);
    }
}

// ============================================================================
// EVENTS
// ============================================================================

void UDocumentWidget::OnCloseButtonClicked()
{
    // Prevent double-close
    if (bIsClosing) return;
    
    bIsClosing = true;
    HandleCloseRequest();
}

FReply UDocumentWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    // Prevent input if already closing
    if (bIsClosing)
    {
        return FReply::Handled();
    }
    
    // Close on ESC key
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        OnCloseButtonClicked();
        return FReply::Handled();
    }

    // Close on E key (interact key)
    if (InKeyEvent.GetKey() == EKeys::E)
    {
        OnCloseButtonClicked();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UDocumentWidget::HandleCloseRequest()
{
    // Priority 1: Use DocumentComponent (recommended way)
    if (DocumentComponentRef && IsValid(DocumentComponentRef))
    {
        DocumentComponentRef->CloseDocument();
        return;
    }
    
    // Priority 2: Use DocumentActor (legacy support)
    if (DocumentActorRef && IsValid(DocumentActorRef))
    {
        DocumentActorRef->HideDocument(DocumentActorRef);
        return;
    }
    
    // Fallback: Close widget directly
    UE_LOG(LogTemp, Warning, TEXT("DocumentWidget: No component or actor reference, using fallback close"));
    CloseFallback();
}

void UDocumentWidget::CloseFallback()
{
    HideAnimation();
    
    // Get animation duration or use default
    float AnimDuration = 0.5f;
    if (FadeOutAnim)
    {
        AnimDuration = FadeOutAnim->GetEndTime();
    }
    
    FTimerHandle RemoveTimer;
    GetWorld()->GetTimerManager().SetTimer(RemoveTimer, [this]()
    {
        if (IsValid(this) && IsInViewport())
        {
            RemoveFromParent();
        }
    }, AnimDuration, false);
}

// ============================================================================
// ANIMATIONS
// ============================================================================

void UDocumentWidget::ShowAnimation()
{
    if (FadeInAnim)
    {
        PlayAnimation(FadeInAnim);
    }
}

void UDocumentWidget::HideAnimation()
{
    if (FadeOutAnim)
    {
        PlayAnimation(FadeOutAnim);
    }
}

float UDocumentWidget::GetFadeOutDuration() const
{
    if (FadeOutAnim)
    {
        return FadeOutAnim->GetEndTime();
    }
    return 0.5f; // Default fallback
}

// ============================================================================
// REFERENCE MANAGEMENT
// ============================================================================

void UDocumentWidget::SetDocumentComponentReference(UDocumentComponent* Component)
{
    if (IsValid(Component))
    {
        DocumentComponentRef = Component;
    }
}

void UDocumentWidget::SetDocumentActorReference(ADocumentActor* Actor)
{
    if (IsValid(Actor))
    {
        DocumentActorRef = Actor;
    }
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

FText UDocumentWidget::FormatDocumentContent(const FItemData& DocumentData) const
{
    FString Content = DocumentData.DocumentContent.ToString();

    switch (DocumentData.DocumentType)
    {
        case EDocumentType::DiaryPage:
            Content = FString::Printf(TEXT("--- Diary Entry ---\n\n%s"), *Content);
            break;

        case EDocumentType::LabReport:
            Content = FString::Printf(TEXT("LABORATORY REPORT\n━━━━━━━━━━━━━━━━━━\n\n%s"), *Content);
            break;

        case EDocumentType::Note:
            // Keep simple for notes
            break;

        case EDocumentType::FinalNote:
            Content = FString::Printf(TEXT("⚠ IMPORTANT ⚠\n\n%s"), *Content);
            break;

        case EDocumentType::Photo:
            // Photos might have minimal text
            break;

        default:
            break;
    }

    return FText::FromString(Content);
}

FText UDocumentWidget::GetDocumentTypeDisplayName(EDocumentType DocType) const
{
    switch (DocType)
    {
        case EDocumentType::Note:
            return FText::FromString("Note");
        case EDocumentType::DiaryPage:
            return FText::FromString("Diary Entry");
        case EDocumentType::LabReport:
            return FText::FromString("Lab Report");
        case EDocumentType::Photo:
            return FText::FromString("Photograph");
        case EDocumentType::FinalNote:
            return FText::FromString("Final Message");
        default:
            return FText::FromString("Document");
    }
}

void UDocumentWidget::ApplyDocumentStyling(EDocumentType DocType)
{
    if (!ContentText) return;

    // Customize based on document type
    // You can set fonts, colors, sizes here
    switch (DocType)
    {
        case EDocumentType::DiaryPage:
            // Handwritten style
            // ContentText->SetFont(HandwrittenFont);
            break;

        case EDocumentType::LabReport:
            // Formal typewriter style
            // ContentText->SetFont(TypewriterFont);
            break;

        case EDocumentType::FinalNote:
            // Dramatic style
            // ContentText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
            break;

        default:
            break;
    }
}