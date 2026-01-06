#include "UI/MainMenu/MenuButton.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UMenuButton::NativeConstruct()
{
	Super::NativeConstruct();

	// Hide hover image by default
	if (HoverImage)
	{
		HoverImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Bind button events
	if (MainButton)
	{
		MainButton->OnClicked.AddDynamic(this, &UMenuButton::OnButtonClickedInternal);
		MainButton->OnHovered.AddDynamic(this, &UMenuButton::OnButtonHovered);
		MainButton->OnUnhovered.AddDynamic(this, &UMenuButton::OnButtonUnhovered);
	}
}

void UMenuButton::OnButtonHovered()
{
	if (!MainButton) return;

	// Play hover sound
	if (HoverSound)
	{
		UGameplayStatics::PlaySound2D(this, HoverSound, HoverSoundVolume);
	}

	// Scale effect
	MainButton->SetRenderScale(FVector2D(HoverScale, HoverScale));

	// Show image and play animation
	if (HoverImage)
	{
		HoverImage->SetVisibility(ESlateVisibility::Visible);
	}

	if (HoverAnim)
	{
		PlayAnimation(HoverAnim);
	}
}

void UMenuButton::OnButtonUnhovered()
{
	if (!MainButton) return;

	// Reset scale
	MainButton->SetRenderScale(FVector2D(1.0f, 1.0f));

	// Keep image visible but play unhover animation
	if (HoverImage)
	{
		HoverImage->SetVisibility(ESlateVisibility::Visible);
	}

	if (UnhoverAnim)
	{
		PlayAnimation(UnhoverAnim);
	}
}

void UMenuButton::OnButtonClickedInternal()
{
	// Broadcast to parent widget
	OnButtonClicked.Broadcast();
}

void UMenuButton::SetEnabled(bool bEnabled)
{
	if (MainButton)
	{
		MainButton->SetIsEnabled(bEnabled);
	}
}

void UMenuButton::SetButtonText(FText NameText)
{
	if (ButtonText)
	{
		ButtonText->SetText(NameText);
	}
}
