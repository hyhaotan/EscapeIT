// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NotificationWidget.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPEIT_API UNotificationWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Notification")
    void ShowSuccess(const FText& SuccessText);

    UFUNCTION(BlueprintCallable, Category = "Notification")
    void ShowError(const FText& ErrorText);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NotificationText;
};
