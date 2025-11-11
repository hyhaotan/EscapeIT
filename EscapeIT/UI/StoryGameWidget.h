// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoryGameWidget.generated.h"

class UTextBlock;
class UBorder;
class UScrollBox;
class UButton;

UCLASS()
class ESCAPEIT_API UStoryGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void SetStoryText(FString& Text);
	
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StoryText;

	UPROPERTY(meta = (BindWidget))
	UBorder* BackgroundBorder;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* StoryBox;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	UFUNCTION()
	void OnCloseButtonClick();
};
