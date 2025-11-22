// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NotificationWidget.generated.h"

class UWidgetAnimation;

UCLASS()
class ESCAPEIT_API UNotificationWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Notification")
    void ShowSuccess(const FText& SuccessText);

    UFUNCTION(BlueprintCallable, Category = "Notification")
    void ShowError(const FText& ErrorText);
	
	UFUNCTION(BlueprintCallable, Category = "Notification")
	void ShowNotification(const FText& Text);
	
	UFUNCTION()
	void ShowAnimWidget(){PlayAnimation(DisplayAnim);}
	
	UFUNCTION()
	void HideAnimWidget(){PlayAnimation(HiddenAnim);}

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NotificationText;
	
	float SetDelayNotification(float Delay);
	
private:
	UPROPERTY(meta=(BindWidgetAnim),Transient)
	TObjectPtr<UWidgetAnimation> DisplayAnim;
	
	UPROPERTY(meta=(BindWidgetAnim),Transient)
	TObjectPtr<UWidgetAnimation> HiddenAnim;
	
	FTimerHandle TimerHandle;
};
