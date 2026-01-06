#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuButton.generated.h"

class UButton;
class UImage;
class UWidgetAnimation;
class USoundBase;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuButtonClicked);

UCLASS()
class ESCAPEIT_API UMenuButton : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Menu Button")
	FOnMenuButtonClicked OnButtonClicked;

	void SetHoverSound(USoundBase* Sound) { HoverSound = Sound; }
    
	void SetEnabled(bool bEnabled);
	
	void SetButtonText(FText NameText);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MainButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> HoverImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ButtonText;
	
	// Animations
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HoverAnim;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* UnhoverAnim;

	// Settings
	UPROPERTY(EditAnywhere, Category = "Menu Button")
	float HoverScale = 1.05f;

	UPROPERTY(EditAnywhere, Category = "Menu Button")
	float HoverSoundVolume = 0.4f;

private:
	UFUNCTION()
	void OnButtonHovered();

	UFUNCTION()
	void OnButtonUnhovered();

	UFUNCTION()
	void OnButtonClickedInternal();

	UPROPERTY()
	TObjectPtr<USoundBase> HoverSound;
};