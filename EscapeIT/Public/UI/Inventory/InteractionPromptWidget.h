// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/InteractionTypes.h"
#include "InteractionPromptWidget.generated.h"

class UTextBlock;
class UImage;
class UBorder;

UCLASS()
class ESCAPEIT_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Display Control
	void ShowPrompt();
	void HidePrompt();
	void SetInteractionType(EInteractionType Type);

	// Hold Interaction
	void StartHold();
	void UpdateHoldProgress(float Progress);
	void CancelHold();
	void CompleteHold();

	// Press Interaction
	void OnPress();

protected:
	// Widget Components
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InteractKeyText;

	UPROPERTY(meta = (BindWidget))
	UImage* ProgressOverlay;

	UPROPERTY(meta = (BindWidget))
	UBorder* InteractBorder;

	// Animations
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* DisplayInteractAnim;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* HiddenInteractAnim;

	// Material Parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Material")
	float InnerRadius = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Material")
	float OuterRadius = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Material")
	float Feather = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Material")
	float GlowIntensity = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Material")
	float StartAngle = -90.0f;

	// Colors
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Colors")
	FLinearColor IdleColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Colors")
	FLinearColor ProgressColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Colors")
	FLinearColor CompleteColor = FLinearColor(0.0f, 1.0f, 0.3f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Colors")
	FLinearColor PressColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Colors")
	FLinearColor GlowColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);

	// Animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Animation")
	float PulseSpeed = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance|Animation")
	float PulseIntensity = 0.1f;

private:
	void UpdateVisuals();

	UPROPERTY()
	UMaterialInstanceDynamic* ProgressMaterial;

	EInteractionType CurrentInteractionType = EInteractionType::Hold;
	bool bIsHolding = false;
	bool bIsPress = false;
	float CurrentProgress = 0.0f;
};