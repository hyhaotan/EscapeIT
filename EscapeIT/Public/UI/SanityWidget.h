// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SanityWidget.generated.h"

class UProgressBar;
class UTextBlock;
class USanityComponent;

UCLASS()
class ESCAPEIT_API USanityWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	void InitializeSanityWidget(USanityComponent* InSanityComponent);

	UFUNCTION(BlueprintCallable, Category = "Sanity")
	void UpdateSanityDisplay(float CurrentSanity);

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SanityText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* SanityProgress;

	UPROPERTY()
	TObjectPtr<USanityComponent> SanityComponent;

	UFUNCTION()
	void OnSanityChanged(float NewSanity);
};