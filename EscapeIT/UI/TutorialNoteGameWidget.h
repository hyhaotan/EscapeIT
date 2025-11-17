// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialNoteGameWidget.generated.h"

class UButton;
class ATutorialNoteGameActor;

UCLASS()
class ESCAPEIT_API UTutorialNoteGameWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	UFUNCTION()
	void OnCloseButtonClicked();

private:
	UPROPERTY()
	ATutorialNoteGameActor* NoteActorRef;

public:
	void SetNoteActorReference(ATutorialNoteGameActor* Actor);
};