// Fill out your copyright notice in the Description page of Project Settings.

#include "TutorialNoteGameWidget.h"
#include "Components/Button.h"
#include "EscapeIT/Actor/TutorialNoteGameActor.h"

void UTutorialNoteGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
    
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UTutorialNoteGameWidget::OnCloseButtonClicked);
	}
}

void UTutorialNoteGameWidget::OnCloseButtonClicked()
{ 
	// Tìm actor trong world thay vì dùng GetDefaultObject
	if (NoteActorRef)
	{
		NoteActorRef->HideDocument(NoteActorRef);
	}
	this->HideAnimation();
}

void UTutorialNoteGameWidget::SetNoteActorReference(ATutorialNoteGameActor* Actor)
{
	NoteActorRef = Actor;
}