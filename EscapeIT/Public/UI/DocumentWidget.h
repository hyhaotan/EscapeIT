// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemData.h"
#include "DocumentWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UScrollBox;
class UDocumentComponent;
class ADocumentActor;

UCLASS()
class ESCAPEIT_API UDocumentWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // ========================================================================
    // WIDGET COMPONENTS (Bind these in UMG Designer)
    // ========================================================================
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> CloseButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TitleText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DocumentTypeText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ContentText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> DocumentImage;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UScrollBox> ContentScrollBox;

    // ========================================================================
    // ANIMATIONS (Create these in UMG Designer)
    // ========================================================================
    
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> FadeInAnim;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    TObjectPtr<UWidgetAnimation> FadeOutAnim;

    // ========================================================================
    // SETUP & CONTENT
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Document")
    void SetupDocument(const FItemData& DocumentData);

    UFUNCTION(BlueprintCallable, Category = "Document")
    void UpdateContent(const FText& NewContent);
    
    UFUNCTION(BlueprintCallable, Category = "Document")
    void UpdateImage(UTexture2D* NewImage);

    // ========================================================================
    // ANIMATIONS
    // ========================================================================
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ShowAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void HideAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetFadeOutDuration() const;

    // ========================================================================
    // REFERENCE MANAGEMENT
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Document")
    void SetDocumentComponentReference(UDocumentComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Document")
    void SetDocumentActorReference(ADocumentActor* Actor);

protected:
    // ========================================================================
    // WIDGET LIFECYCLE
    // ========================================================================
    
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    // ========================================================================
    // UI EVENTS
    // ========================================================================
    
    UFUNCTION()
    void OnCloseButtonClicked();
    void HandleCloseRequest();
    void CloseFallback();

    // ========================================================================
    // FORMATTING HELPERS
    // ========================================================================
    
    FText FormatDocumentContent(const FItemData& DocumentData) const;
    FText GetDocumentTypeDisplayName(EDocumentType DocType) const;
    void ApplyDocumentStyling(EDocumentType DocType);

    // ========================================================================
    // STATE
    // ========================================================================
    
    UPROPERTY()
    FItemData CurrentDocumentData;

    UPROPERTY()
    TObjectPtr<UDocumentComponent> DocumentComponentRef;

    UPROPERTY()
    TObjectPtr<ADocumentActor> DocumentActorRef;

    // Prevent double-close
    bool bIsClosing = false;

    // ========================================================================
    // OPTIONAL: Custom fonts for different document types
    // ========================================================================
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Styling")
    FSlateFontInfo HandwrittenFont;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Styling")
    FSlateFontInfo TypewriterFont;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Styling")
    FSlateFontInfo DefaultFont;
};